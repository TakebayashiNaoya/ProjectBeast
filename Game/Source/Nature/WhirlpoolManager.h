/**
 * @file WhirlpoolManager.h
 * @brief 渦潮を管理するクラス
 * @author 藤谷、竹林
 */
#pragma once
#include "Nature/INatureObject.h"


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
				return static_cast<uint8_t>(m_whirlpoolMap.size());
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
			/** 渦潮のマップ */
			std::unordered_map<uint8_t, std::unique_ptr<Whirlpool>> m_whirlpoolMap;
			/** 渦潮の座標マップ（JSONから読み込んだインデックスと座標の対応表） */
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