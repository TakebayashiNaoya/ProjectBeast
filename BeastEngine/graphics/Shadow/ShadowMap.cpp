/**
 * @file ShadowMap.cpp
 * @brief カスケードシャドウマップクラスの実装
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

		/** シャドウマップ1枚あたりの解像度 */
		constexpr UINT SHADOW_MAP_SIZE = 2048;

		/**
		 * @brief 影を出す距離の初期値
		 * @details 最も遠いカスケードの終端。カスケード分割により、
		 *          ここまで伸ばしても近景の精細さは保たれる。
		 */
		constexpr float INITIAL_SHADOW_DISTANCE = 12000.0f;

		/**
		 * @brief カスケードの分割の偏り
		 * @details 0.0で等分割、1.0で完全な対数分割。
		 *          等分割だと近景の区間が広くなりすぎてキャラクターの影が粗くなるため、
		 *          対数寄りにして手前を細かく配る。
		 */
		constexpr float CASCADE_SPLIT_LAMBDA = 0.85f;

		/**
		 * @brief 覆う範囲の球より手前にライトを引く余裕
		 * @details 球の外側にあっても影を落としうるキャスター（高い氷壁など）を
		 *          nearクリップで切らないための余白。
		 */
		constexpr float LIGHT_MARGIN = 3000.0f;

		/** ライトの近クリップ面 */
		constexpr float SHADOW_NEAR = 1.0f;

		/** シャドウマップシェーダーのファイルパス */
		constexpr const char* SHADOW_MAP_FX_PATH = "Assets/shader/shadowMap.fx";

		/**
		 * @brief 影の中で直接光を何割残すかの初期値
		 * @details 0.0で直接光を完全に遮る。
		 */
		constexpr float INITIAL_DIRECT_LIGHT_RATE = 0.0f;

		/**
		 * @brief 影の中で環境光を何割残すかの初期値
		 * @details 影の見え方に最も効くパラメータ。
		 *          環境光は 0.6 と強めに設定されており、1.0のままだと影の中も
		 *          環境光で埋まってしまい、トーンマップ後にはほぼ差が見えなくなる。
		 */
		constexpr float INITIAL_AMBIENT_RATE = 0.5f;
	}


	void ShadowMap::Init()
	{
		m_directLightRate = INITIAL_DIRECT_LIGHT_RATE;
		m_ambientRate = INITIAL_AMBIENT_RATE;
		m_shadowDistance = INITIAL_SHADOW_DISTANCE;

		// 何も描かれていない場所は「最も遠い」とみなしたいのでクリア値を1.0にする
		// 0.0でクリアすると範囲外が影だらけになる
		float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		for (auto& renderTarget : m_shadowMapRenderTargets)
		{
			renderTarget.Create(
				SHADOW_MAP_SIZE,
				SHADOW_MAP_SIZE,
				1,
				1,
				DXGI_FORMAT_R32_FLOAT,
				DXGI_FORMAT_D32_FLOAT,
				clearColor
			);
		}
	}


	void ShadowMap::UpdateCascadeDistances(nsK2EngineLow::Camera& camera)
	{
		const float nearZ = camera.GetNear();
		const float farZ = min(camera.GetFar(), m_shadowDistance);
		const float range = farZ - nearZ;
		const float ratio = farZ / nearZ;

		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			const float p = static_cast<float>(i + 1) / static_cast<float>(NUM_SHADOW_CASCADES);

			// 対数分割は手前を細かく配れるが、近景が極端に狭くなりすぎる。
			// 等分割と混ぜて偏りを調整する（Practical Split Scheme）
			const float logSplit = nearZ * powf(ratio, p);
			const float uniformSplit = nearZ + range * p;

			m_cascadeFarDistances[i] =
				CASCADE_SPLIT_LAMBDA * logSplit + (1.0f - CASCADE_SPLIT_LAMBDA) * uniformSplit;
		}

		// 最遠は必ず指定距離で終わらせる
		m_cascadeFarDistances[NUM_SHADOW_CASCADES - 1] = farZ;
	}


	void ShadowMap::CalcFrustumSphere(
		nsK2EngineLow::Camera& camera,
		float nearZ,
		float farZ,
		Vector3& outCenter,
		float& outRadius) const
	{
		const Vector3 cameraPosition = camera.GetPosition();
		const Vector3 forward = camera.GetForward();
		const Vector3 right = camera.GetRight();

		// カメラの上方向を forward と right から作り直す。
		// Camera::GetUp() は設定値であり forward と直交しているとは限らないため。
		// ここでは ±up の両方を使うので、外積の向き（符号）はどちらでもよい。
		Vector3 up;
		up.Cross(forward, right);
		up.Normalize();

		const float tanHalfFov = tanf(camera.GetViewAngle() * 0.5f);
		const float aspect = camera.GetAspect();

		// 区間の手前と奥、それぞれの面の4隅で計8点を求める
		Vector3 corners[8];
		int index = 0;
		for (int i = 0; i < 2; i++)
		{
			const float z = (i == 0) ? nearZ : farZ;
			const float halfHeight = tanHalfFov * z;
			const float halfWidth = halfHeight * aspect;
			const Vector3 planeCenter = cameraPosition + forward * z;

			corners[index++] = planeCenter + up * halfHeight + right * halfWidth;
			corners[index++] = planeCenter + up * halfHeight - right * halfWidth;
			corners[index++] = planeCenter - up * halfHeight + right * halfWidth;
			corners[index++] = planeCenter - up * halfHeight - right * halfWidth;
		}

		// 8点の重心を中心とし、最も遠い点までを半径にする
		outCenter = Vector3::Zero;
		for (const auto& corner : corners)
		{
			outCenter += corner;
		}
		outCenter /= 8.0f;

		outRadius = 0.0f;
		for (const auto& corner : corners)
		{
			Vector3 toCorner = corner - outCenter;
			outRadius = max(outRadius, toCorner.Length());
		}
	}


	void ShadowMap::UpdateLightMatrix(
		const int cascadeIndex,
		const Vector3& lightDirection,
		nsK2EngineLow::Camera& camera)
	{
		// このカスケードが担当する区間
		const float nearZ = (cascadeIndex == 0)
			? camera.GetNear()
			: m_cascadeFarDistances[cascadeIndex - 1];
		const float farZ = m_cascadeFarDistances[cascadeIndex];

		Vector3 sphereCenter;
		float sphereRadius = 0.0f;
		CalcFrustumSphere(camera, nearZ, farZ, sphereCenter, sphereRadius);

		const float areaSize = sphereRadius * 2.0f;
		m_texelWorldSizes[cascadeIndex] = areaSize / static_cast<float>(SHADOW_MAP_SIZE);

		// カメラが動くたびに影の輪郭がちらつくのを防ぐため、
		// 覆う範囲の中心をライト空間でテクセル単位に丸める
		Vector3 centerInLight = sphereCenter;
		m_lightSpaceMatrix.Apply(centerInLight);

		const float texelSize = m_texelWorldSizes[cascadeIndex];
		centerInLight.x = floorf(centerInLight.x / texelSize) * texelSize;
		centerInLight.y = floorf(centerInLight.y / texelSize) * texelSize;

		// カリング用に、丸めた後の範囲をライト空間のまま控えておく
		m_cascadeCentersInLight[cascadeIndex] = centerInLight;
		m_cascadeRadii[cascadeIndex] = sphereRadius;

		// 丸めた中心をワールドへ戻す
		Vector3 centerInWorld = centerInLight;
		Matrix invLightSpaceMatrix;
		invLightSpaceMatrix.Inverse(m_lightSpaceMatrix);
		invLightSpaceMatrix.Apply(centerInWorld);

		// 丸めた中心で本番のライト行列を作る
		const Vector3 lightPosition = centerInWorld - lightDirection * (sphereRadius + LIGHT_MARGIN);
		m_lightViewMatrices[cascadeIndex].MakeLookAt(lightPosition, centerInWorld, m_lightUp);

		// 遠クリップはライト位置から球の裏側まで届く距離にする
		const float shadowFar = sphereRadius * 2.0f + LIGHT_MARGIN;
		m_lightProjMatrices[cascadeIndex].MakeOrthoProjectionMatrix(
			areaSize, areaSize, SHADOW_NEAR, shadowFar);

		m_lvpMatrices[cascadeIndex].Multiply(
			m_lightViewMatrices[cascadeIndex], m_lightProjMatrices[cascadeIndex]);
	}


	bool ShadowMap::IsCasterVisibleInCascade(
		const int cascadeIndex,
		const Vector3& aabbMin,
		const Vector3& aabbMax) const
	{
		const Vector3& center = m_cascadeCentersInLight[cascadeIndex];
		const float radius = m_cascadeRadii[cascadeIndex];

		// AABBの8頂点をライト空間へ移し、包む範囲を求める
		Vector3 casterMin(FLT_MAX, FLT_MAX, FLT_MAX);
		Vector3 casterMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (int i = 0; i < 8; i++)
		{
			Vector3 corner(
				(i & 1) ? aabbMax.x : aabbMin.x,
				(i & 2) ? aabbMax.y : aabbMin.y,
				(i & 4) ? aabbMax.z : aabbMin.z
			);
			m_lightSpaceMatrix.Apply(corner);

			casterMin.x = min(casterMin.x, corner.x);
			casterMin.y = min(casterMin.y, corner.y);
			casterMin.z = min(casterMin.z, corner.z);

			casterMax.x = max(casterMax.x, corner.x);
			casterMax.y = max(casterMax.y, corner.y);
			casterMax.z = max(casterMax.z, corner.z);
		}

		// 直交投影が覆うのは一辺 radius*2 の正方形。球で判定すると隅が抜ける
		if (casterMax.x < center.x - radius || casterMin.x > center.x + radius) { return false; }
		if (casterMax.y < center.y - radius || casterMin.y > center.y + radius) { return false; }

		// 奥行きはライトの手前側に LIGHT_MARGIN のぶん余裕がある。
		// 範囲より手前（ライト側）にあるものも中へ影を落とすため、そこまでを対象にする
		if (casterMax.z < center.z - radius - LIGHT_MARGIN) { return false; }
		if (casterMin.z > center.z + radius) { return false; }

		return true;
	}


	void ShadowMap::RenderCascade(
		RenderContext& rc,
		const int cascadeIndex,
		const std::vector<ModelRender*>& deferredModels,
		const std::vector<ModelRender*>& forwardModels)
	{
		RenderTarget& renderTarget = m_shadowMapRenderTargets[cascadeIndex];

		rc.WaitUntilToPossibleSetRenderTarget(renderTarget);
		rc.SetRenderTargetAndViewport(renderTarget);
		rc.ClearRenderTargetView(renderTarget);

		// ビュー行列に単位行列、プロジェクション行列に合成済みLVPを渡す。
		// シェーダー側は mul(mProj, mul(mView, worldPos)) を計算するので、
		// これで受け手側の mul(LVP, worldPos) と完全に同一の変換になり、
		// 行列の適用経路による差が出ない。
		const Matrix& lvp = m_lvpMatrices[cascadeIndex];

		// カメラの視錐台ではなく、このカスケードが覆う範囲でカリングする。
		// カメラから見えないキャスターでも画面内へ影を落とすことがあるため。
		// カメラの視錐台ではなく、このカスケードが覆う範囲でカリングする。
		// カメラから見えないキャスターでも画面内へ影を落とすことがあるため。
		for (auto* model : deferredModels)
		{
			if (!IsCasterVisibleInCascade(cascadeIndex, model->GetWorldAABBMin(), model->GetWorldAABBMax()))
			{
				continue;
			}
			model->OnRenderShadowMap(rc, cascadeIndex, Matrix::Identity, lvp);
		}
		for (auto* model : forwardModels)
		{
			if (!IsCasterVisibleInCascade(cascadeIndex, model->GetWorldAABBMin(), model->GetWorldAABBMax()))
			{
				continue;
			}
			model->OnRenderShadowMap(rc, cascadeIndex, Matrix::Identity, lvp);
		}

		rc.WaitUntilFinishDrawingToRenderTarget(renderTarget);
	}


	void ShadowMap::Render(
		RenderContext& rc,
		const Vector3& lightDirection,
		nsK2EngineLow::Camera& camera,
		const std::vector<ModelRender*>& deferredModels,
		const std::vector<ModelRender*>& forwardModels)
	{
		if (!m_isEnable)
		{
			return;
		}

		BeginGPUEvent("ShadowMap");

		// ライト空間はライトの向きだけで決まるため、カスケードごとに作り直さず一度だけ求める。
		// ライトが真上・真下を向いていると上方向ベクトルが縮退するため、その場合だけ別の軸を使う
		m_lightUp = (fabsf(lightDirection.y) > 0.99f) ? Vector3::Front : Vector3::Up;
		// 基準をワールド原点にすることで升目がワールドに固定される。
		// カメラ追従の点を原点にすると物差し自体が毎フレーム動き、丸めてもちらつきが消えない
		m_lightSpaceMatrix.MakeLookAt(Vector3::Zero, lightDirection, m_lightUp);

		UpdateCascadeDistances(camera);

		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			UpdateLightMatrix(i, lightDirection, camera);
			RenderCascade(rc, i, deferredModels, forwardModels);
		}

		EndGPUEvent();
	}

} // namespace nsBeastEngine
