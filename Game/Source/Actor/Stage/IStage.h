/**
 * @file IStage.h
 * @brief ステージの基底クラス
 * @author 藤谷
 */
#pragma once
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
			 * @param fileName アセットファイルパス
			 * @param position 座標
			 * @param rotation 回転
			 @param scale 拡大率
			 */
			void Init(const char* fileName);


		private:
			/** 物理静的オブジェクト */
			PhysicsStaticObject m_physicsStaticObject;
		};
	}
}

