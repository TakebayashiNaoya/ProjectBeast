/**
 * @file OrcaManager.h
 * @brief シャチの管理
 */
#pragma once
#include "OrcaSeaGrid.h"
#include "json/json.hpp"
#include <string>
#include <vector>


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class Orca;
		class OrcaController;
		class ChildPenguin;


		/**
		 * @brief シャチの管理（シングルトン）
		 * @details 配置JSON（OrcaLayout_*.json）からシャチを生成し、泳げる場所の地図（OrcaSeaGrid）を持つ。
		 *          シロクマの EnemyManager とは別に持つ（EnemyLayout の件数をクマの数として表示している画面があるため）
		 */
		class OrcaManager
		{
		public:
			void Update();
			void UpdateModelOnly();
			/** @brief 全員のモデルが描画できる状態になったか */
			bool IsAllModelReady() const;
			void Render(RenderContext& rc);

			/**
			 * @brief 配置JSONから地図を作り、シャチを生成する
			 * @details 地形（StageSystem の TerrainObject）が読み込み済みであること
			 */
			void LoadOrcas(const nlohmann::json& json);

			/** @brief シャチを全部消す */
			void ClearOrcas();

			/** @brief シャチの一覧 */
			std::vector<Orca*> GetOrcas() const;
			/** @brief AIの一覧（並びは GetOrcas と同じ） */
			std::vector<OrcaController*> GetControllers() const;

			/** @brief 泳げる場所の地図 */
			const OrcaSeaGrid& GetSeaGrid() const { return m_seaGrid; }

			/**
			 * @brief シャチが噛んだ子の幽霊が消えたことを伝える（幽霊デバフ）
			 * @param orcaId シャチのログ用ID（Orca::GetLogId）
			 */
			void NotifyVictimGhostHidden(int orcaId);

			/**
			 * @brief 指定した子を追っているシャチのうち、最も近いものの口先の位置を返す（EnemyManager::FindNearestChaserOf と同じ）
			 * @param penguin 子
			 * @param outPos 口先の位置（出力）
			 * @return 追っているシャチがいれば true
			 */
			bool FindNearestChaserOf(const ChildPenguin* penguin, Vector3& outPos) const;


		public:
			static void CreateInstance()
			{
				if (m_instance == nullptr) m_instance = new OrcaManager();
			}
			static OrcaManager* GetInstance() { return m_instance; }
			static void DestroyInstance()
			{
				if (m_instance != nullptr)
				{
					delete m_instance;
					m_instance = nullptr;
				}
			}


		private:
			OrcaManager();
			~OrcaManager();

			/** @brief デバッグ欄「シャチ」を描く */
			void DrawDebug();
			/** @brief デバッグ欄の地図の色を作り直す */
			void BuildDebugMapRuns();


		private:
			struct OrcaData
			{
				Orca* orca = nullptr;
				OrcaController* controller = nullptr;
			};
			std::vector<OrcaData> m_orcaList;

			/** 泳げる場所の地図 */
			OrcaSeaGrid m_seaGrid;

			// --- デバッグ表示 ---
			/** 地図を作るのにかかった時間（ミリ秒） */
			double m_buildMilliseconds = 0.0;
			/** セルフテストの結果 */
			std::string m_selfTestMessage;
			/** 航行可能でなかったので寄せた巡回点・出現地点の数 */
			int m_snappedPointCount = 0;
			/** デバッグ欄に登録したか */
			bool m_isDebugRegistered = false;
			/** 地図を描くときの横一列の同じ色の並び（x0, x1, y, 色） */
			struct MapRun
			{
				int x0;
				int x1;
				int y;
				uint32_t color;
			};
			std::vector<MapRun> m_debugMapRuns;
			/** 地図の1辺の画素数 */
			int m_debugMapPixels = 0;

			static OrcaManager* m_instance;
		};
	}
}
