/**
 * @file ShadowMap.cpp
 * @brief カスケードシャドウマップクラスの実装
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

		// ライトが真上・真下を向いていると上方向ベクトルが縮退するため、
		// その場合だけ別の軸を上方向に使う
		Vector3 up = Vector3::Up;
		if (fabsf(lightDirection.y) > 0.99f)
		{
			up = Vector3::Front;
		}

		const float areaSize = sphereRadius * 2.0f;
		m_texelWorldSizes[cascadeIndex] = areaSize / static_cast<float>(SHADOW_MAP_SIZE);

		// カメラが動くたびに影の輪郭がちらつくのを防ぐため、
		// 覆う範囲の中心をライト空間でテクセル単位に丸める。
		//
		// 丸める基準の座標系は「ワールド原点から見たライト空間」にする。
		// カメラ追従の点を原点にすると物差し自体が毎フレーム動いてしまい、
		// 丸めても升目が固定されずちらつきが消えない。
		Matrix lightSpaceMatrix;
		lightSpaceMatrix.MakeLookAt(Vector3::Zero, lightDirection, up);

		Vector3 centerInLight = sphereCenter;
		lightSpaceMatrix.Apply(centerInLight);

		const float texelSize = m_texelWorldSizes[cascadeIndex];
		centerInLight.x = floorf(centerInLight.x / texelSize) * texelSize;
		centerInLight.y = floorf(centerInLight.y / texelSize) * texelSize;

		Matrix invLightSpaceMatrix;
		invLightSpaceMatrix.Inverse(lightSpaceMatrix);
		invLightSpaceMatrix.Apply(centerInLight);

		// カリング用に、丸めた後の範囲を控えておく
		m_cascadeCenters[cascadeIndex] = centerInLight;
		m_cascadeRadii[cascadeIndex] = sphereRadius;

		// 丸めた中心で本番のライト行列を作る
		const Vector3 lightPosition = centerInLight - lightDirection * (sphereRadius + LIGHT_MARGIN);
		m_lightViewMatrices[cascadeIndex].MakeLookAt(lightPosition, centerInLight, up);

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
		const Vector3& center = m_cascadeCenters[cascadeIndex];
		const float radius = m_cascadeRadii[cascadeIndex];

		// AABBと球の最近接距離で判定する。
		// ライト側にあるキャスターは球の外でも中へ影を落とすため、
		// ライトの方向にだけ範囲を伸ばした位置でも判定する。
		Vector3 extendedCenter = center - m_lightDirection * radius;

		for (int i = 0; i < 2; i++)
		{
			const Vector3& testCenter = (i == 0) ? center : extendedCenter;

			const float closestX = max(aabbMin.x, min(testCenter.x, aabbMax.x));
			const float closestY = max(aabbMin.y, min(testCenter.y, aabbMax.y));
			const float closestZ = max(aabbMin.z, min(testCenter.z, aabbMax.z));

			const float dx = closestX - testCenter.x;
			const float dy = closestY - testCenter.y;
			const float dz = closestZ - testCenter.z;

			if (dx * dx + dy * dy + dz * dz <= radius * radius)
			{
				return true;
			}
		}

		return false;
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

		m_lightDirection = lightDirection;
		UpdateCascadeDistances(camera);

		for (int i = 0; i < NUM_SHADOW_CASCADES; i++)
		{
			UpdateLightMatrix(i, lightDirection, camera);
			RenderCascade(rc, i, deferredModels, forwardModels);
		}

		EndGPUEvent();
	}

} // namespace nsBeastEngine
