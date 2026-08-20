/**
 * @file AchievementManager.h
 * @brief アチーブメントの管理クラス
 * @author 藤谷
 */
#pragma once
#include "AchievementBase.h"
#include "Json/json.hpp"


namespace app
{
	namespace achievement
	{



		/**
		* @brief アチーブメントの管理クラス
		*/
		class AchievementManager : public Noncopyable
		{
		public:
			/**
			 * @brief 初期化処理
			 * @param jsonPath ステージ別アチーブメント定義JSONのパス
			 */
			void Start(const char* jsonPath);
			/**
			 * @brief 更新処理
			 */
			void Update();
			/**
			 * @brief 描画処理
			 */
			void Render(RenderContext& rc);

			/**
			 * @brief ゲーム終了時に FinalConditionAchievement を一括評価する
			 */
			void FinalizeAchievements();

			/**
			 * @brief シロクマに倒された回数を1増やす
			 */
			void AddBearKill()       { m_bearKillCount++; }

			/**
			 * @brief 渦潮に飲まれた回数を1増やす
			 */
			void AddWhirlpoolCapture() { m_whirlpoolCaptureCount++; }

			/**
			 * @brief シロクマに倒された回数を取得する
			 * @return 倒された回数
			 * @note プレイログの記録に使う
			 */
			int GetBearKillCount() const { return m_bearKillCount; }

			/**
			 * @brief 渦潮に飲まれた回数を取得する
			 * @return 飲まれた回数
			 * @note プレイログの記録に使う
			 */
			int GetWhirlpoolCaptureCount() const { return m_whirlpoolCaptureCount; }

			/**
			 * @brief 達成済みのアチーブメントの配列を取得する
			 */
			std::vector<AchievementBase*> GetAllAchievements() const;

			/**
			 * @brief IDからアチーブメントを取得する
			 */
			AchievementBase* GetAchievement(uint32_t id);

			/** ホットリロードのたびに増加するカウンター。各UIメニューが自分の見たバージョンと比較して再初期化する */
			int GetReloadVersion() const { return m_reloadVersion; }

			/** JSONファイルの変更を検知してリロードする（ポーズ中など Update() が止まる状況でも呼べる） */
			void CheckHotReload();


		public:
			AchievementManager();
			~AchievementManager();


		private:
			/**
			 * @brief アチーブメントを作成する
			 * @todo 未実装なので、この先実装する
			 */
			void CreateAchievement(const nlohmann::json& json);


			using AchieveKey = uint32_t;
			using Achieve = std::unique_ptr<AchievementBase>;


		private:
			std::vector<Achieve> m_achievementList;
			std::unordered_map<AchieveKey, AchievementBase*> m_achievementMap;

			int m_bearKillCount       = 0;
			int m_whirlpoolCaptureCount = 0;

			std::string m_jsonPath;
			time_t      m_lastUpdateTime = 0;
			int         m_reloadVersion  = 0;


			/** シングルトン関係 */
		public:
			/**
			 * @brief インスタンスを作成
			 */
			static void CreateInstance()
			{
				if (m_instance == nullptr)
				{
					m_instance = new AchievementManager();
				}
			}


			/**
			 * @brief インスタンスを取得
			 * @return インスタンス
			 */
			static AchievementManager* GetInstance()
			{
				return m_instance;
			}


			/**
			 * @brief インスタンスを破棄
			 */
			static void DestroyInstance()
			{
				delete m_instance;
				m_instance = nullptr;
			}



		private:
			/** シングルトンインスタンス */
			static AchievementManager* m_instance;
		};
	}
}

