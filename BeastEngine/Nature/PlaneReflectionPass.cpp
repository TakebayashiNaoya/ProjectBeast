#include "BeastEnginePreCompile.h"
#include "PlaneReflectionPass.h"


namespace nsBeastEngine
{
	PlaneReflectionPass::PlaneReflectionPass()
	{
	}

	PlaneReflectionPass::~PlaneReflectionPass()
	{
	}

	void PlaneReflectionPass::Init()
	{
		//SetConstatntBuffer(g_renderingEngine->GetSceneLight().GetLight(), m_reflectionPlane.GetEquation());

		m_planarReflectionTarget.Create(
			g_graphicsEngine->GetFrameBufferWidth(),
			g_graphicsEngine->GetFrameBufferHeight(),
			1,
			1,
			DXGI_FORMAT_R11G11B10_FLOAT,
			DXGI_FORMAT_D24_UNORM_S8_UINT
		);
	}

	void PlaneReflectionPass::Update()
	{
		UpdatePlaneCB(m_reflectionPlane.GetEquation());
	}

	void PlaneReflectionPass::Execute(RenderContext& rc, std::vector<ModelRender*>& obj)
	{
		rc.WaitUntilToPossibleSetRenderTarget(m_planarReflectionTarget);
		rc.SetRenderTargetAndViewport(m_planarReflectionTarget);
		rc.ClearRenderTargetView(m_planarReflectionTarget);
		rc.ClearDepthStencilView(m_planarReflectionTarget.GetDSVCpuDescriptorHandle(), 1.0f);

		// まとめて平面に映すモデルレンダーを描画
		//for (auto MobjData : obj)
		//{
		//	//平面の描画
		//	MobjData->OnRenderReflectionMap(rc, m_reflectCamera);
		//}

		rc.WaitUntilFinishDrawingToRenderTarget(m_planarReflectionTarget);
	}

	void PlaneReflectionPass::UpdateReflectCamera(const Plane& plane)
	{
		//メインカメラの場所を鏡映させた位置に反射カメラを置く
		m_reflectCamera.SetPosition(ReflectPointAcrossPlane(g_camera3D->GetPosition(), plane));

		m_reflectCamera.SetTarget(ReflectPointAcrossPlane(g_camera3D->GetTarget(), plane));

		m_reflectCamera.SetUp(ReflectVectorAcrossPlane(g_camera3D->GetUp(), plane));

		m_reflectCamera.SetFar(g_camera3D->GetFar());

		m_reflectCamera.SetViewAngle(g_camera3D->GetViewAngle());


		// 斜めクリップ面をビュー空間へ
		m_reflectionPlane = TransformPlaneToView(plane, m_reflectCamera.GetViewMatrix());

		//// 面をクリップ面として使うような射影行列を作成
		//m_reflectCamera.SetProjectionMatrix(MakeObliqueProjectionLike(g_camera3D->GetProjectionMatrix(), m_reflectionPlane));

		//m_reflectCamera.SetProjectionMatrix(g_camera3D->GetProjectionMatrix());

	}

	Plane PlaneReflectionPass::TransformPlaneToView(const Plane& planeWS, const Matrix& view)
	{
		//ワールド空間→カメラ空間に平面を変換する
		//その際、法線を正しく変換するためにview行列の逆転置を使用する
		Matrix viewInvTranspose;
		viewInvTranspose.Inverse(view);
		viewInvTranspose.Transpose();

		// planeWSの方程式をview空間に変換
		Vector4 planeEquationVS = planeWS.GetEquation();
		viewInvTranspose.Apply(planeEquationVS);

		// 正規化されたビュー空間の平面を生成
		Plane  planeVS = Plane::FromEquationNormalized(planeEquationVS);

		// 斜めニア面として使う前提で、原点が内側（通常は負側）になるよう向きを統一
		if (planeVS.GetSignedDistance() > 0.0f) {
			planeVS = Plane(planeVS.GetNormal() * -1, planeVS.GetSignedDistance() * -1);
		}


		return planeVS;
	}

	Matrix PlaneReflectionPass::MakeObliqueProjectionLike(const Matrix& proj, const Plane& planeVS)
	{

		// projの逆行列を用い「clip-space のどの点が消えるかを求める
		Vector4 q;
		q.x = (std::copysign(1.0f, planeVS.GetEquation().x) + proj._13) / proj._11;
		q.y = (std::copysign(1.0f, planeVS.GetEquation().y) + proj._23) / proj._22;
		q.z = 1.0f;
		q.w = (1.0f - proj._33) / proj._34;

		// 新しい行を計算
		Vector4 c = planeVS.GetEquation() * (2.0f / Dot(planeVS.GetEquation(), q));

		Matrix obliqueProj = proj;
		//obliqueProj.SetRow3(c - proj.GetRow4());  // 第3行を差し替える
		return obliqueProj;
	}

}