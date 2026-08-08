/**
 * @file ShadowMap.cpp
 * @brief シャドウマップクラスの実装
 * @author 竹林尚哉
 */
#include "BeastEnginePreCompile.h"
#include "Graphics/Shadow/ShadowMap.h"
#include "Graphics/ModelRender.h"


namespace nsBeastEngine
{
	namespace
	{
		//============================================//
		// シャドウマップパラメーター
		// 調整する場合はここの値を変更する
		//============================================//

		/** シャドウマップの解像度 */
		constexpr UINT SHADOW_MAP_SIZE = 2048;

		/**
		 * @brief シャドウマップが覆うワールド空間の一辺の長さ
		 * @details 地形は12000×12000あるため全域は覆えない。注視点まわりだけを覆う。
		 *          小さくするほど影は精細になるが、範囲外に影が出なくなる。
		 *          現在は 3000 / 2048 で 1テクセル ≒ 1.46ユニット。
		 */
		constexpr float SHADOW_AREA_SIZE = 3000.0f;

		/**
		 * @brief 注視点からライトを引く距離
		 * @details この距離より高い位置にあるキャスターは範囲外になって影を落とさない。
		 *          地形の高さスケールが500なので、それを十分に超える値にしている。
		 */
		constexpr float LIGHT_DISTANCE = 3000.0f;

		/** ライトの近クリップ面 */
		constexpr float SHADOW_NEAR = 1.0f;

		/** ライトの遠クリップ面 */
		constexpr float SHADOW_FAR = 8000.0f;

		/** シャドウマップシェーダーのファイルパス */
		constexpr const char* SHADOW_MAP_FX_PATH = "Assets/shader/shadowMap.fx";
	}


	void ShadowMap::Init()
	{
		// 何も描かれていない場所は「最も遠い」とみなしたいのでクリア値を1.0にする
		// 0.0でクリアすると範囲外が影だらけになる
		float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		m_shadowMapRenderTarget.Create(
			SHADOW_MAP_SIZE,
			SHADOW_MAP_SIZE,
			1,
			1,
			DXGI_FORMAT_R32_FLOAT,
			DXGI_FORMAT_D32_FLOAT,
			clearColor
		);
	}


	void ShadowMap::UpdateLightMatrix(const Vector3& lightDirection, const Vector3& focusPosition)
	{
		// ライトの向きと逆方向へ引いた位置から注視点を見る
		Vector3 lightPosition = focusPosition - lightDirection * LIGHT_DISTANCE;

		// ライトが真上・真下を向いていると上方向ベクトルが縮退するため、
		// その場合だけ別の軸を上方向に使う
		Vector3 up = Vector3::Up;
		if (fabsf(lightDirection.y) > 0.99f)
		{
			up = Vector3::Front;
		}

		m_lightViewMatrix.MakeLookAt(lightPosition, focusPosition, up);
		m_lightProjMatrix.MakeOrthoProjectionMatrix(
			SHADOW_AREA_SIZE,
			SHADOW_AREA_SIZE,
			SHADOW_NEAR,
			SHADOW_FAR
		);

		m_lvpMatrix.Multiply(m_lightViewMatrix, m_lightProjMatrix);
	}


	void ShadowMap::Render(
		RenderContext& rc,
		const Vector3& lightDirection,
		const Vector3& focusPosition,
		const std::vector<ModelRender*>& deferredModels,
		const std::vector<ModelRender*>& forwardModels)
	{
		if (!m_isEnable)
		{
			return;
		}

		BeginGPUEvent("ShadowMap");

		// ライト行列を更新する
		// この行列はディファードライティングでも使うため、SceneLight側へも反映される
		UpdateLightMatrix(lightDirection, focusPosition);

		rc.WaitUntilToPossibleSetRenderTarget(m_shadowMapRenderTarget);
		rc.SetRenderTargetAndViewport(m_shadowMapRenderTarget);
		rc.ClearRenderTargetView(m_shadowMapRenderTarget);

		// キャスターを深度のみで描画する
		// フラスタムカリングは行わない。カメラから見えないキャスターでも
		// 画面内へ影を落とすことがあるため。
		// ビュー行列に単位行列、プロジェクション行列に合成済みLVPを渡す。
		// シェーダー側は mul(mProj, mul(mView, worldPos)) を計算するので、
		// これでディファードライティング側の mul(LVP, worldPos) と
		// 完全に同一の変換になり、行列の適用経路による差が出ない。
		for (auto* model : deferredModels)
		{
			model->OnRenderShadowMap(rc, Matrix::Identity, m_lvpMatrix);
		}
		for (auto* model : forwardModels)
		{
			model->OnRenderShadowMap(rc, Matrix::Identity, m_lvpMatrix);
		}

		rc.WaitUntilFinishDrawingToRenderTarget(m_shadowMapRenderTarget);

		EndGPUEvent();
	}

} // namespace nsBeastEngine
