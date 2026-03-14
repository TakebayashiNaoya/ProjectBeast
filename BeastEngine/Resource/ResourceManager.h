#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <memory>
#include <string>
#include <functional>
#include <atomic>
#include <typeindex>
#include <cassert>
#include <cstdio>


namespace nsBeastEngine
{
	// ============================================================
	//  リソース基底
	// ============================================================
	class IResource {
	public:
		/** 状態 */
		enum class State
		{
			None,
			Loading,
			Completed,
			Error
		};

		virtual ~IResource() = default;

		/** 状態を取得 */
		State GetState() const
		{
			std::lock_guard<std::mutex> lk(mutex_);
			return state_;
		}

		/** 完了状態か */
		bool IsCompleted() const { return GetState() == State::Completed; }
		/** エラー状態か */
		bool IsError()     const { return GetState() == State::Error; }


	protected:
		IResource() = default;


	private:
		friend class ResourceManager;

		void SetState(State s)
		{
			std::lock_guard<std::mutex> lk(mutex_);
			state_ = s;
		}

		State state_ = State::None;	/** 状態 */
		mutable std::mutex mutex_;	/** 状態保護用ミューテックス */
	};

	// ============================================================
	//  ローダー基底
	//    テンプレート派生 ResourceLoader<T> を使う想定
	// ============================================================
	class IResourceLoader {
	public:
		virtual ~IResourceLoader() = default;

		// ワーカースレッド上で呼ばれる
		virtual bool DoLoad(IResource& resource, const std::string& key) = 0;

		// リソースの新規インスタンスを生成
		virtual std::shared_ptr<IResource> CreateResource() = 0;
	};

	// ============================================================
	//  型付きローダーのヘルパー基底
	//    利用側はこれを継承して LoadImpl を実装する
	// ============================================================
	template <typename T>
	class ResourceLoader : public IResourceLoader {
	public:
		// 利用者が実装する : resource にデータを詰めて成功なら true
		virtual bool LoadImpl(T& resource, const std::string& key) = 0;

		bool DoLoad(IResource& resource, const std::string& key) override {
			return LoadImpl(static_cast<T&>(resource), key);
		}

		std::shared_ptr<IResource> CreateResource() override {
			return std::make_shared<T>();
		}
	};

	// ============================================================
	//  内部リクエスト
	// ============================================================
	/** メインスレッドからワーカースレッドへ渡す作業依頼書 */
	struct LoadRequest_ {
		std::shared_ptr<IResourceLoader> loader;	/** 誰が */
		std::string key;							/** 何を */
		std::shared_ptr<IResource> resource;		/** どこに */
	};

	// ============================================================
	//  ResourceManager
	// ============================================================
	class ResourceManager {
	public:
		ResourceManager() = default;
		~ResourceManager() { Shutdown(); }

		/** コピー禁止 */
		ResourceManager(const ResourceManager&) = delete;
		ResourceManager& operator=(const ResourceManager&) = delete;

		// ----- 型 + ローダー登録 -----
		//   mgr.Register<TextureResource>(std::make_shared<TextureLoader>());
		template <typename T>
		void Register(std::shared_ptr<ResourceLoader<T>> loader)
		{
			/** loaderMutex_をロック */
			std::lock_guard<std::mutex> lk(loaderMutex_);
			/** 型名をIDに変換し、そのIDをキーとしてマップ（loaders_）に登録する */
			loaders_[std::type_index(typeid(T))] = std::move(loader);
		}/** loaderMutex_をアンロック */

		// ----- 読み込み要求 -----
		//   auto tex = mgr.Load<TextureResource>("assets/player.png");
		//   // tex->IsCompleted() が true になったら使える
		template <typename T>
		std::shared_ptr<T> Load(const std::string& key);

		// ----- スレッド制御 -----
		void Start();
		void Shutdown();

		// 全リクエスト処理済みか
		bool IsIdle() const { return pendingCount_.load() == 0; }

	private:
		void WorkerLoop();

		// キャッシュ  key = type_index の hash ^ string hash で一意化
		struct CacheKey
		{
			std::type_index type;
			std::string     name;

			/** 種類と名前が同じならtrueを返す */
			bool operator==(const CacheKey& o) const
			{
				return type == o.type && name == o.name;
			}
		};

		struct CacheKeyHash {
			std::size_t operator()(const CacheKey& k) const {
				auto h1 = k.type.hash_code();
				auto h2 = std::hash<std::string>{}(k.name);
				return h1 ^ (h2 * 2654435761u);
			}
		};

		mutable std::mutex cacheMutex_;
		std::unordered_map<CacheKey, std::shared_ptr<IResource>, CacheKeyHash> cache_;

		// ローダー (type_index → loader)
		mutable std::mutex loaderMutex_;
		std::unordered_map<std::type_index, std::shared_ptr<IResourceLoader>> loaders_;

		// リクエストキュー
		std::mutex              queueMutex_;
		std::condition_variable queueCV_;
		std::queue<LoadRequest_> queue_;

		// スレッド
		std::thread       worker_;
		std::atomic<bool> running_{ false };			/** 処理が走っているか */
		std::atomic<bool> shutdownRequested_{ false };	/** シャットダウン要求が来たか */
		std::atomic<int>  pendingCount_{ 0 };			/** 処理待ちリクエスト数 (IsIdle() 用) */


	public:
		static ResourceManager& GetInstance() {
			static ResourceManager instance;
			return instance;
		}
	};

	// ============================================================
	//  Load<T> テンプレート実装
	// ============================================================
	template <typename T>
	std::shared_ptr<T> ResourceManager::Load(const std::string& key)
	{
		/** ID（型名）とkeyでキャッシュキーを作る */
		CacheKey ck{ std::type_index(typeid(T)), key };

		// --- キャッシュ確認 ---
		{
			/** cacheMutex_をロック */
			std::lock_guard<std::mutex> lk(cacheMutex_);
			/** キャッシュにあればそれを返す */
			auto it = cache_.find(ck);
			/** もし見つかったら、既に Load 済み or Loading 中 → 同じ shared_ptr を返す */
			if (it != cache_.end()) {
				return std::static_pointer_cast<T>(it->second);
			}
		}/** cacheMutex_をアンロック */

		// --- ローダー取得 ---
		/** ローダーのローカル変数を用意 */
		std::shared_ptr<IResourceLoader> loader;
		{
			/** loaderMutex_をロック */
			std::lock_guard<std::mutex> lk(loaderMutex_);
			/** loadersに登録されていれば loader にセットする */
			auto it = loaders_.find(std::type_index(typeid(T)));
			if (it != loaders_.end()) {
				loader = it->second;
			}
		}/** loaderMutex_をアンロック */

		/** ローダーが見つからなければエラー出して nullptr を返す */
		if (!loader) {
			std::fprintf(stderr, "[ResourceManager] no loader registered for type\n");
			return nullptr;
		}

		// --- リソース生成してキャッシュに入れる ---
		/** ローダーに CreateResource() させて新しいリソースを作る */
		auto resource = loader->CreateResource();
		/** それをキャッシュに入れる（競合で先に入っていたらそちらを使う） */
		auto typed = std::static_pointer_cast<T>(resource);

		{
			/** cacheMutex_をロック */
			std::lock_guard<std::mutex> lk(cacheMutex_);
			/** 競合で先に入っていたらそちらを返す */
			auto result = cache_.emplace(ck, resource);
			/** もし先に入っていたら、そちらを返す */
			if (!result.second) {
				return std::static_pointer_cast<T>(result.first->second);
			}
		}/** cacheMutex_をアンロック */

		// --- キューに投入 ---
		{
			/** queueMutex_をロック */
			std::lock_guard<std::mutex> lk(queueMutex_);
			/** ローダー、キー、リソースをセットにしてキューに入れる */
            LoadRequest_ request;
			request.loader = loader;
			request.key = key;
			request.resource = resource;
			queue_.push(std::move(request));
		}/** queueMutex_をアンロック */

		/** 処理待ちの数を1つ増やす */
		pendingCount_.fetch_add(1);
		/** 待機しているスレッドを1つ起床させる */
		queueCV_.notify_one();
		/** 新しいリソースの shared_ptr を返す */
		return typed;
	}
}