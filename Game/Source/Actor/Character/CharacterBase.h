#pragma once
#include "Source/Actor/Actor.h"




namespace app
{
	namespace actor
	{

		struct AnimationData
		{
			/** ファイルネーム */
			const char* fileName;
			/** ループフラグ */
			const bool isLoop;
		};


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


		class CharacterBase : public Actor
		{
		protected:
			/** アニメーションクリップ */
			AnimationClip* m_animationClips;
			/** キャラクターコントローラー */
			CharacterController m_characterController;


		public:
			/** キャラクターコントローラーを取得 */
			inline CharacterController* GetCharacterController() { return &m_characterController; }


		protected:
			/** 初期化 */
			//template<typename TStatus>
			void Init(const ModelData* data);
		};
	}
}

