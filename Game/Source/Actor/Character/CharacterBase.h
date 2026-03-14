/**
 * @file CharacterBase.h
 * @brief キャラクターの基底クラス
 * @author 藤谷
 */
#pragma once
#include "Physics/CharacterController.h"	// あとで確認
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
			inline nsBeastEngine::nsCollision::CharacterController* GetCharacterController() { return &m_characterController; }


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
			virtual void Start() override;
			virtual void Update() override;
			virtual void Render(RenderContext& rc) override;


		protected:
			/** アニメーションクリップ */
			AnimationClip* m_animationClips;
			/** キャラクターコントローラー */
			nsBeastEngine::nsCollision::CharacterController m_characterController;
		};
	}
}

