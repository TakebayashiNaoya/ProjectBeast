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
		protected:
			/** アニメーションクリップ */
			AnimationClip* m_animationClips;
			/** キャラクターコントローラー */
			CharacterController m_characterController;


		protected:
			/**
			 * @brief 初期化処理
			 * @param data モデルデータ
			 */
			void Init(const ModelData* data);


		public:
			/**
			 * @brief キャラクターコントローラーを取得
			 * @return キャラクターコントローラーのポインタ
			 */
			inline CharacterController* GetCharacterController() { return &m_characterController; }
		};
	}
}

