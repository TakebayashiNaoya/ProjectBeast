#include "ResourceManager.h"

// ============================================================
//  スレッド制御
// ============================================================
void ResourceManager::Start()
{
	/** すでに走っているなら何もしない */
	if (running_.load()) return;
	/** ワーカースレッド開始 */
	running_ = true;
	shutdownRequested_ = false;
	worker_ = std::thread(&ResourceManager::WorkerLoop, this);
}

void ResourceManager::Shutdown()
{
	/** すでに停止しているなら何もしない */
	if (!running_.load()) return;
	/** シャットダウン要求を出してワーカースレッドの終了を待つ */
	shutdownRequested_ = true;
	/** 待機しているスレッドを1つ起床させる */
	queueCV_.notify_one();
	/** ワーカーが有効なスレッドを管理している場合は、今のスレッドを止めて待つ */
	if (worker_.joinable()) {
		worker_.join();
	}
	/** ワーカーが停止したので、状態を更新する */
	running_ = false;
}

// ============================================================
//  ワーカースレッド
// ============================================================
void ResourceManager::WorkerLoop()
{
	while (true)
	{
		LoadRequest_ req;

		// --- キューから取り出し ---
		{
			/** queueMutex_をロック */
			std::unique_lock<std::mutex> lk(queueMutex_);

			/**
			 * タスクがある（キューが空ではない） or シャットダウン要求が来たら、waitから抜け出す。
			 * キューが空 or シャットダウン要求が来ていなければ、スレッドを停止する。
			 */
			queueCV_.wait(lk, [this] {
				return !queue_.empty() || shutdownRequested_.load();
				});

			/** シャットダウン要求が来ていて、かつキューも空ならループを抜ける */
			if (shutdownRequested_.load() && queue_.empty()) break;

			/** キューから先頭のリクエストを取り出す */
			req = std::move(queue_.front());
			queue_.pop();
		}/** queueMutex_をアンロック */

		// --- 読み込み実行 ---
		req.resource->SetState(IResource::State::Loading);

		/** ローダーの DoLoad を適切に呼び出せたらsuccessをtrueに、それ以外（エラー等）はfalseにする。 */
		bool success = false;
		try {
			success = req.loader->DoLoad(*req.resource, req.key);
		}
		catch (...) {
			success = false;
		}

		/** ロードの成否に応じてリソースの状態を Completed か Error にする */
		req.resource->SetState(success ? IResource::State::Completed : IResource::State::Error);
		/** 処理待ちの数を1つ減らす */
		pendingCount_.fetch_sub(1);
	}
}
