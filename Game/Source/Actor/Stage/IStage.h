/**
 * @file IStage.h
 * @brief ステージの基底クラス
 * @author 藤谷
 */
#pragma once
#include "../../../BeastEngine/Physics/PhysicalBody.h"
#include "Resource/ModelResource.h"
#include "Source/Actor/Actor.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief ステージの基底クラス
		 */
		class IStageObject : public Actor
		{
		public:
			IStageObject() = default;
			virtual ~IStageObject() = default;


		protected:
			/** 初期化処理 */
			virtual void Start()override;
			/** 更新処理 */
			virtual void Update()override;
			/** 描画処理 */
			virtual void Render(RenderContext& rc)override;


		public:
			/**
			 * @brief モデルレンダーを初期化
			 * @param fileName  アセットファイルパス
			 * @param pbrName   PBRParameter.jsonの"name"キー（空文字の場合はデフォルト値を使用）
			 */
			void Init(const char* fileName, const std::string& pbrName = "");

			/**
			 * @brief モデルと物理コリジョンのロードが完了しているか
			 * @return 完了していればtrue
			 */
			bool IsLoaded() const { return m_isModelLoaded; }

			/**
			 * @brief 物理判定が必要かどうかを設定
			 * @param needCollision 物理判定が必要かどうか
			 */
			void SetIsNeedCollision(const bool needCollision) { m_IsNeedCollision = needCollision; }

			/**
			 * @brief 表示状態を設定する
			 * @param isVisible 表示する場合はtrue
			 */
			void SetIsVisible(const bool isVisible) { m_isVisible = isVisible; }

			/**
			 * @brief 表示状態を取得する
			 * @return 表示状態
			 */
			bool IsVisible() const { return m_isVisible; }

		private:
			/** 物理判定が必要かどうか */
			bool m_IsNeedCollision;
			/** 物理静的オブジェクト */
			nsBeastEngine::nsCollision::PhysicalBody m_physicalObj;


		private:
			/** モデルロード完了フラグ */
			bool m_isModelLoaded = false;
			/** モデルファイルパス（非同期ロード完了後にModelRender::Initで使用） */
			std::string m_pendingModelPath;
			/** TKM非同期ローダー */
			nsBeastEngine::TkmModelLoader m_tkmLoader;
			/** PBRParameter.jsonの"name"キー（空文字の場合はデフォルト値を使用） */
			std::string m_pbrName;


		private:
			bool m_isVisible = true;
		};
	}
}