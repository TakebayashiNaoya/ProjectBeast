/**
 * @file CharacterBase.h
 * @brief キャラクターの基底クラス
 * @author 藤谷
 */
#pragma once
#include "Physics/CharacterController.h"
#include "Resource/ModelResource.h"
#include "Source/Actor/Actor.h"
#include "Source/Effect/Decal.h"


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
			virtual void UpdateModelOnly();


		public:
			CharacterBase();
			virtual ~CharacterBase() override;


		protected:
			/**
			 * @brief 初期化処理
			 * @param data モデルデータ
			 */
			void Init(const ModelData& data);
			/**
			 * @brief モデルの非同期ロードの更新処理
			 */
			void ModelLoadUpdate();

			/**
			 * @brief 足跡の描画を更新する（毎フレーム呼び出す）
			 * @details 移動する全キャラクター共通の処理。抑制条件やサイズなどの
			 *          キャラクター固有の違いは下記の仮想関数で調整する。
			 */
			void UpdateFootprints();

			/**
			 * @brief 足跡を出さない状態かどうか（ジャンプ中・遊泳中など）
			 * @details キャラクター固有のステート判定は派生クラスでオーバーライドする
			 */
			virtual bool ShouldSuppressFootprint() const { return false; }

			/** @brief 足跡のサイズ */
			virtual float GetFootprintSize() const { return 12.0f; }

			/** @brief 足跡の優先度（大きいほど優先度が高い） */
			virtual int GetFootprintPriority() const { return 1; }

			/** @brief 左右の足跡の横方向オフセット量（足の間隔） */
			virtual float GetFootprintStanceWidth() const { return 3.0f; }

			/** @brief 足跡を刻む移動距離のしきい値 */
			virtual float GetFootprintStepDistance() const { return 15.0f; }

			/**
			 * @brief 足跡の見た目（テクスチャ種別）
			 * @details デフォルトはSnowFootprintだが、GetFootprintAutoDetectSurface()が
			 *          trueの場合は地形判定でSnow/Grass/Rockに上書きされる。
			 *          種族固有の見た目（肉球など）を使いたい場合はここをオーバーライドし、
			 *          GetFootprintAutoDetectSurface()もfalseにすること。
			 */
			virtual app::effect::DecalKind GetFootprintKind() const { return app::effect::DecalKind::SnowFootprint; }

			/** @brief 地形に応じてGetFootprintKind()の結果を自動で上書きするか */
			virtual bool GetFootprintAutoDetectSurface() const { return true; }

			/** @brief 足跡の色（GetFootprintAutoDetectSurface()がfalseのときのみ使われる） */
			virtual Vector4 GetFootprintColor() const { return { 0.2f, 0.5f, 1.0f, 1.0f }; }

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

			/** 直前に足跡を出した座標 */
			Vector3 m_lastFootprintPos = Vector3::Zero;
			/** 次に出すのが右足かどうか */
			bool m_isRightFoot = true;
		};
	}
}

