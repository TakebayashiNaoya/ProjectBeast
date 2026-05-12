/**
 * @file CharacterBase.h
 * @brief キャラクターの基底クラス
 * @author 藤谷
 */
#pragma once
#include "Physics/CharacterController.h"	// あとで確認
#include "Resource/ModelResource.h"
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


		/** 前方宣言 */
		class CharacterStateMachine;


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
			/**
			 * @brief モデルレンダーを取得する
			 * @details InGameSceneからOcclusionDitherManager::SetPlayerTarget()に渡すために使用する。
			 * @return モデルレンダーへの参照
			 */
			nsBeastEngine::ModelRender& GetModelRender() { return m_modelRender; }
			/**
			 * @brief モデルの行列更新のみ行う（AI・ステートマシンは動かさない）
			 * @detail カウントダウン中など、描画は必要だが動かしたくない場合に使用
			 */
			void UpdateModelOnly();


		public:
			CharacterBase();
			virtual ~CharacterBase() override;


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
			/** キャラクターステートマシン */
			CharacterStateMachine* m_characterStateMachine;
			/** アニメーションクリップ */
			std::unique_ptr<AnimationClip[]> m_animationClips;
			/** キャラクターコントローラー */
			nsBeastEngine::nsCollision::CharacterController m_characterController;

			/** スケルトン */
			Skeleton m_skeleton;
			/** リソースまとめローダー */
			ModelAssetsLoader m_assetsLoader;
			/** モデル読み込み完了フラグ */
			bool m_modelReady = false;
			/** クリップ数 */
			int m_clipNum = 0;
			/** モデルの上方向 */
			EnModelUpAxis m_upAxis = enModelUpAxisZ;
		};
	}
}

