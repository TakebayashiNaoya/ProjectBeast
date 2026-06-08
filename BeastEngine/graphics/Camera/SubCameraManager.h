/**
 * @file SubCameraManager.h
 * @brief サブカメラの管理を行うクラス
 * @author 竹林
 */
#pragma once
#include <functional>


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
		 * @brief 小窓スプライトの表示・非表示を設定する
		 * @details - true: スライドイン開始。ただしスライドアウト中なら一度完全に隠れてからスライドイン。
		 *          - false: スライドアウト開始。保留中のスライドインもキャンセルする。
		 * @param visible 表示するか
		 */
		void SetSpriteVisible(bool visible);

		/**
		 * @brief 小窓スプライトの表示スケールを設定する
		 * @details 1.0 が原寸（480x270px）。DangerArrowSystem::Initialize() から呼び出す。
		 * @param scale 均等スケール
		 */
		void SetSpriteScale(const float scale)
		{
			m_spriteScale = scale;
		}

		/**
		 * @brief 描画をブロックする（ポーズ画面の背後に隠す用途）
		 * @details true のとき RenderToScreen は描画をスキップする。
		 *          スライドアニメーションの状態は保持されるため、
		 *          false に戻すと再び表示される。
		 * @param blocked ブロックするか
		 */
		void SetRenderingBlocked(bool blocked) { m_renderingBlocked = blocked; }

		/**
		 * @brief サブカメラをアニメーションなしで即座に停止する
		 * @details タイトル遷移時など、すぐに消す必要がある場合に使用する。
		 *          End() と異なりスライドアウト演出は行わない。
		 */
		void ForceEnd();


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
		/** 描画ブロックフラグ（ポーズ中など） */
		bool m_renderingBlocked = false;
		/** 表示したいか（DangerArrowSystemからセット） */
		bool m_targetVisible = false;
		/** スライドアウト完了後にスライドインする保留フラグ */
		bool m_pendingShow = false;
		/** スライドアウト完了後に終了処理を実行する保留フラグ */
		bool m_pendingEnd = false;
		/** End() の保留中コールバック */
		std::function<void()> m_pendingEndCallback;
		/** スライドアニメーション進行度（0=画面外、1=完全表示） */
		float m_slideProgress = 0.0f;
		/** カメラのワールド座標 */
		Vector3 m_cameraPosition = Vector3::Zero;
		/** ターゲットのワールド座標 */
		Vector3 m_targetPosition = Vector3::Zero;
		/** 小窓スプライトの表示スケール（1.0 = 原寸） */
		float m_spriteScale = 1.0f;
		/** 小窓の背景（枠）スプライト */
		Sprite m_bgSprite;
		/** 背景スプライトが初期化済みか（再Begin時に再初期化しない） */
		bool m_bgSpriteInitialized = false;
		/** 小窓表示用Sprite */
		Sprite m_sprite;


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