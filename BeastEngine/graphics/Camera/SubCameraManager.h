/**
 * @file SubCameraManager.h
 * @brief サブカメラの管理を行うクラス
 * @author 竹林
 */
#pragma once
#include <functional>
#include "Geometry/Frustum.h"


namespace nsBeastEngine
{
	/**
	 * @brief サブカメラマネージャー
	 * @details サブカメラの生成・破棄・ON/OFF・ターゲット座標の管理を行う。
	 *          開始・終了時にコールバック（ラムダ式）を呼び出すことができる。
	 *          Game側からは本クラスを通じてサブカメラを操作する。
	 */
	class SubCameraManager
	{
	public:
		/**
		 * @brief サブカメラを起動する
		 * @details CameraSystem にサブカメラを生成させ、RenderTargetとSpriteを初期化する。
		 *          onBegin は生成完了後に呼ばれる（例: 効果音の再生）
		 * @param onBegin 起動時に呼ばれるコールバック
		 */
		void Begin(std::function<void()> onBegin = nullptr);

		/**
		 * @brief サブカメラを停止する
		 * @details CameraSystem のサブカメラを破棄し、RenderTargetとSpriteを解放する。
		 *          onEnd は破棄完了後に呼ばれる（例: 効果音の再生）
		 * @param onEnd 停止時に呼ばれるコールバック
		 */
		void End(std::function<void()> onEnd = nullptr);

		/**
		 * @brief 毎フレームの更新処理
		 * @details サブカメラのターゲット追従を行う
		 */
		void Update();

		/**
		 * @brief オフスクリーンの描画処理
		 * @details サブカメラ視点でRenderTargetに描画する
		 * @param rc レンダリングコンテキスト
		 */
		void RenderOffscreen(nsK2EngineLow::RenderContext& rc);

		/**
		 * @brief 小窓の描画処理
		 * @details RenderTarget の内容をSpriteとして画面上に描画する
		 * @param rc レンダリングコンテキスト
		 */
		void RenderToScreen(nsK2EngineLow::RenderContext& rc);

		/**
		 * @brief カメラ座標を設定する
		 * @details サブカメラの座標を設定する
		 * @param position カメラのワールド座標
		 */
		void SetCameraPosition(const Vector3& position)
		{
			m_cameraPosition = position;
		}

		/**
		 * @brief ターゲット座標を設定する
		 * @details サブカメラが注視する座標を設定する
		 * @param position ターゲットのワールド座標
		 */
		void SetTargetPosition(const Vector3& position)
		{
			m_targetPosition = position;
		}

		/**
		 * @brief サブカメラが動作中かどうかを返す
		 * @return 動作中であればtrue
		 */
		bool IsActive() const
		{
			return m_isActive;
		}

		/**
		 * @brief 小窓スプライトのスクリーン座標を設定する
		 * @details DangerArrowSystem から毎フレーム呼び出す。
		 *          中心=(0,0)、範囲±フレームバッファハーフサイズの座標系。
		 * @param pos スクリーン座標
		 */
		void SetSpriteScreenPosition(const Vector2& pos)
		{
			m_spriteScreenPos = pos;
		}

		/**
		 * @brief 小窓スプライトの表示・非表示を設定する
		 * @details DangerArrowSystem がフラスタム内かつ近距離のときに非表示にする。
		 * @param visible 表示するか
		 */
		void SetSpriteVisible(const bool visible)
		{
			m_isSpriteVisible = visible;
		}


	private:
		/**
		 * @brief RenderTargetとSpriteを初期化する
		 */
		void InitRenderTargetAndSprite();

		/**
		 * @brief サブカメラの姿勢を更新する
		 * @details ターゲット座標をもとにカメラのposition/targetを計算する
		 */
		void UpdateSubCameraTransform();


	private:
		/** 動作中フラグ */
		bool m_isActive = false;
		/** 小窓スプライト表示フラグ（Begin()でtrueにリセット） */
		bool m_isSpriteVisible = true;
		/** カメラのワールド座標 */
		Vector3 m_cameraPosition = Vector3::Zero;
		/** ターゲットのワールド座標 */
		Vector3 m_targetPosition = Vector3::Zero;
		/** 小窓スプライトのスクリーン座標（DangerArrowSystemから毎フレームセット） */
		Vector2 m_spriteScreenPos = Vector2::Zero;
		/** オフスクリーン描画用RenderTarget */
		RenderTarget m_renderTarget;
		/** 小窓表示用Sprite */
		Sprite m_sprite;
		/** サブカメラ用フラスタム */
		Frustum m_frustum;


		//============================================//
		// シングルトン関連
		//============================================//

	private:
		SubCameraManager() = default;
		~SubCameraManager() = default;

		static SubCameraManager* m_instance;


	public:
		/** @brief シングルトンインスタンスを生成する */
		static void CreateInstance()
		{
			if (m_instance == nullptr) {
				m_instance = new SubCameraManager();
			}
		}
		/** @brief シングルトンインスタンスを取得する */
		static SubCameraManager& Get()
		{
			return *m_instance;
		}
		/** @brief シングルトンインスタンスを破棄する */
		static void DestroyInstance()
		{
			delete m_instance;
			m_instance = nullptr;
		}
	};
}