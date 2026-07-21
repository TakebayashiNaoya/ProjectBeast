/**
 * @file WhirlpoolManager.h
 * @brief 渦潮を管理するクラス
 * @author 藤谷、竹林
 */
#pragma once
#include "Nature/INatureObject.h"
#include <vector>


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
	}


	namespace nature
	{
		/** 前方宣言 */
		class Whirlpool;

		/**
		 * @brief 渦潮を管理するクラス
		 */
		class WhirlpoolManager : public nsBeastEngine::INatureObject
		{
		public:
			/**
			 * @brief 初期化処理
			 * @param positionsJsonPath 渦潮の配置JSONパス
			 * @param parameterJsonPath 渦潮のパラメーターJSONパス
			 */
			void Start(const char* positionsJsonPath, const char* parameterPath);
			/**
			 * @brief 更新処理
			 */
			void Update();
			/**
			 * @brief 描画処理
			 * @param rc レンダリングコンテキスト
			 */
			void Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view) override;


		private:
			WhirlpoolManager();
			~WhirlpoolManager();


		public:
			/**
			 * @brief 渦潮に対してコールバック関数を実行する関数
			 * @param cb コールバック関数
			 */
			void ForEach(std::function<void(Whirlpool* info)> cb);


			/**
			 * @brief 現在の渦潮の数を取得する
			 * @return 渦潮の数
			 */
			uint8_t GetWhirlpoolCount() const
			{
				uint8_t count = 0;
				for (const auto& whirlpool : m_whirlpoolSlots)
				{
					if (whirlpool) ++count;
				}
				return count;
			}


			/**
			 * @brief 渦潮の最大数を取得する
			 * @return 渦潮の最大数
			 */
			uint8_t GetWhirlpoolCountMax() const
			{
				return static_cast<uint8_t>(m_positionMap.size());
			}

			/**
			 * @brief 削除される子ペンギンを全渦潮の追跡対象から無効化する
			 * @details ChildPenguinManager::RemoveAndDestroy() から呼ばれる
			 * @param penguin 削除される子ペンギンのポインタ
			 */
			void NotifyPenguinDestroyed(actor::ChildPenguin* penguin);


		private:
			/**
			 * @brief 渦潮の座標JSONを読み込んでm_positionMapを更新する
			 * @param json 読み込んだJSONオブジェクト
			 */
			void LoadPositionMap(const nlohmann::json& json);

			/**
			 * @brief 渦潮を生成する関数
			 */
			void CreateWhirlpool();


		private:
			/**
			 * @brief 渦潮のスロット配列（配置インデックスそのものを添字として使う）
			 * @details CreateWhirlpool()は空いている配置インデックスをランダムに選んで生成するため、
			 *          std::map等の「キー順にソートされたコンテナ」だと、新しい渦潮が既存の渦潮より
			 *          小さいインデックスで生成された瞬間、既存の渦潮のイテレーション順（＝ForEach()での
			 *          出現位置）がずれてしまう。ミニマップはこの出現位置だけでアイコンと渦潮を対応付けて
			 *          いるため、順序がずれると既存アイコンが別の渦潮の座標を指してしまい、
			 *          瞬間移動したように見えてしまっていた。
			 *          配置インデックスを配列の添字にそのまま使えば、他の渦潮の生成・削除が
			 *          既存の渦潮の位置（添字）に一切影響しなくなり、この問題が起きない。
			 *          （配置インデックスは連番とは限らないため、未使用の添字はnullptrのまま空けておく）
			 */
			std::vector<std::unique_ptr<Whirlpool>> m_whirlpoolSlots;
			/** 渦潮の座標マップ（JSONから読み込んだインデックスと座標の対応表。読み込み後は不変なので走査順の変動は起きない） */
			std::unordered_map<uint8_t, Vector3> m_positionMap;
			/** 渦潮の生成タイマー */
			float m_timer;
			/** 渦潮の配置JSONパス（ホットリロード用に保持） */
			std::string m_positionsJsonPath;
			/** 座標JSONの最終更新時刻（ホットリロード用） */
			time_t m_posLastWriteTime;


		public:
			/**
			 * @brief インスタンスの生成
			 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new WhirlpoolManager();
				}
			}

			/**
			 * @brief インスタンスの取得
			 * @return インスタンスのポインタ
			 */
			static WhirlpoolManager* GetInstance()
			{
				return m_instance;
			}

			/**
			 * @brief インスタンスの破棄
			 */
			static void DestroyInstance()
			{
				delete m_instance;
				m_instance = nullptr;
			}


		private:
			/** インスタンス */
			static WhirlpoolManager* m_instance;
		};
	}
}