/**
 * @file CharacterBase.h
 * @brief キャラクターの基底クラス
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Actor.h"




namespace app
{
	namespace actor
	{
		/** アニメーションデータ */
		struct AnimationData
		{
			/** ファイルネーム */
			const char* fileName;
			/** ループフラグ */
			const bool isLoop;
		};




		/************************************/


		/** モデルデータ */
		struct ModelData
		{
			/** ファイルネーム */
			const char* fileName;
			/** アニメーションデータ */
			const AnimationData* animationData;
			/** モデルの上方向 */
			const EnModelUpAxis upAxis;
			/** アニメーションクリップの数 */
			const uint8_t clipNum;
		};




		/************************************/


		/**
		 * @brief キャラクターの基底クラス
		 */
		class CharacterBase : public Actor
		{
		public:
			/**
			 * @brief キャラクターコントローラーを取得
			 * @return キャラクターコントローラーのポインタ
			 */
			inline CharacterController* GetCharacterController() { return &m_characterController; }


		public:
			CharacterBase();
			virtual ~CharacterBase() override = default;


		protected:
			/**
			 * @brief 初期化処理
			 * @param data モデルデータ
			 */
			void Init(const ModelData& data);


		protected:
			virtual bool Start() override;
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


		protected:
			/** アニメーションクリップ */
			std::unique_ptr<AnimationClip[]> m_animationClips;
			/** キャラクターコントローラー */
			CharacterController m_characterController;
		};
	}
}

