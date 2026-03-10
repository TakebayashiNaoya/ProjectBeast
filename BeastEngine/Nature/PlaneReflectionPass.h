#pragma once


namespace nsBeastEngine
{
	namespace
	{
		const Vector3 INIT_PLANE_NORMAL = g_vec3Up;	// 平面の法線
		const Vector3 INIT_PLANE_POSITION = g_vec3Zero;// 平面のポジション（平面が通る点）
	}
	/// <summary>
	/// 平面の方程式を表す構造体。
	/// 平面を扱いやすいように方程式の形にしている。
	/// 平面の方程式：ax + by + cz + d = 0
	/// P｛a, b, c｝はワールド座標の任意の点
	/// 
	/// </summary>
	struct Plane
	{
	public:
		/// <summary>
		/// 法線ベクトルと平面上の任意の点を指定して平面を生成する。
		/// </summary>
		/// <param name="n">平面の法線</param>
		/// <param name="p">平面上の点</param>
		Plane(const Vector3& n, const Vector3& p)
			: normal(n)
		{
			//nが0ベクトルか確かめるための変数
			//（Length()で長さを求めると、Sqrt()を使うので、Dot()の方が効率がいい）
			const float nn = Dot(n, n);
			if (nn < 1e-12f) {
				//nが0ベクトルに近いときは、Y軸方向を法線とする
				normal = Vector3{ 0,1,0 };
			}
			normal.Normalize();
			equation = Vector4(normal.x, normal.y, normal.z, -Dot(normal, p));
			signedDistance = equation.w; // Dの符号を反転させたものが原点からの符号付き距離
		}

		/// <summary>
		/// 平面の方程式のdを指定して平面を生成する。
		/// </summary>
		/// <param name="n">平面の法線</param>
		/// <param name="d">平面の方程式のD。原点から平面までの符号付き距離（n が単位ベクトルのとき）</param>
		Plane(const Vector3& n, float d)
			: normal(n), signedDistance(d)
		{
			normal.Normalize();
			equation = Vector4(normal.x, normal.y, normal.z, d);
		}


		/// <summary>
		/// 平面方程式の係数(a,b,c,d)からPlaneをつくる。
		/// 法線(a,b,c) は正規化され、D も同じスケールで割られるため、
		/// 結果として単位法線を持つ正規化済みの平面が得られる。
		/// </summary>
		/// <param name="eq"></param>
		/// <returns></returns>
		static Plane FromEquationNormalized(const Vector4& eq) {
			Vector3 n{ eq.x, eq.y, eq.z };
			float len = n.Length();
			if (len < 1e-12f) {
				return Plane(Vector3{ 0,1,0 }, 0.0f);
			}
			n /= len;
			float d = eq.w / len;
			return Plane(n, d);
		}

		/// <summary>
		/// 任意の点とこの平面の符号付き距離を返す。
		/// </summary>
		/// <param name="p">任意の点</param>
		/// <returns></returns>
		float SignedDistanceTo(const Vector3& p) const {
			//法線が正規化されている前提。
			//コンストラクタで必ず正規化するようにしているので問題ない。
			return Dot(normal, p) + signedDistance;
		}

		const Vector3& GetNormal() const { return normal; }
		const Vector4& GetEquation() const { return equation; }
		float GetSignedDistance() const { return signedDistance; }

	private:
		Vector3 normal;       // 法線ベクトル
		Vector4 equation;     // 平面の方程式 Ax + By + Cz + D = 0 の係数 (A, B, C, D)
		float signedDistance; // 原点から平面の符号付き最短距離
	};

	/// <summary>
	/// 反射に映るモデルの定数バッファー
	/// </summary>
	struct ReflectionModelCB
	{
		Vector4 reflectionPlane;		// 反射面の方程式
		Light light;					// ライト情報
	};

	class PlaneReflectionPass
	{
	public:


		PlaneReflectionPass();
		~PlaneReflectionPass();
		void Init();
		void Update();
		void Execute(RenderContext& rc, std::vector<ModelRender*>& obj);

		/// <summary>
		/// 反射カメラを更新する。
		/// </summary>
		/// <param name="plane"></param>
		void UpdateReflectCamera(const Plane& plane);

		Camera& GetReflectCamera() {
			return m_reflectCamera;
		}

		RenderTarget& GetPlanarReflectionTarget() {
			return m_planarReflectionTarget;
		}

		Matrix GetReflectViewProjectionMatrix() {
			return m_reflectCamera.GetViewProjectionMatrix();
		}

		ReflectionModelCB& GetConstantBuffer() {
			return m_constantBuffer;
		}



		void SetConstatntBuffer(const Light& inLight, const Vector4& plane) {
			m_constantBuffer.light = inLight;
			m_constantBuffer.reflectionPlane = plane;
		};

		void UpdatePlaneCB(const Vector4& plane) {
			m_constantBuffer.reflectionPlane = plane;
		}

	private:
		/// <summary>
		/// 点を指定した平面に対して鏡映させた位置を返す。
		/// </summary>
		/// <param name="point">反射させたい点</param>
		/// <param name="plane">平面</param>
		/// <returns></returns>
		Vector3 ReflectPointAcrossPlane(const Vector3& point, const Plane& plane) {
			// 平面までの符号付き距離を計算（法線は単位長の前提）
			float dist = plane.SignedDistanceTo(point);

			return point - (plane.GetNormal() * dist * 2.0f);
		}

		/// <summary>
		/// ベクトルを平面に対して反射させる。
		/// </summary>
		/// <param name="v">反射させたい方向ベクトル</param>
		/// <param name="plane">平面（法線とオフセットを持つが、ベクトル反射では法線だけ使う）</param>
		/// <returns>反射後のベクトル</returns>
		Vector3 ReflectVectorAcrossPlane(const Vector3& vector, const Plane& plane)
		{
			const Vector3& n = plane.GetNormal();   // 法線ベクトル

			// 射影係数（n方向成分の大きさ / |n|^2）
			const float proj = Dot(n, vector);

			return vector - (n * proj * 2);
		}

		/// <summary>
		/// ワールド空間の平面をビュー空間に変換する。
		/// </summary>
		/// <param name="planeWS"></param>
		/// <param name="V"></param>
		/// <returns></returns>
		Plane TransformPlaneToView(const Plane& planeWS, const Matrix& V);

		/// <summary>
		/// 与えられた射影行列をもとに、斜めクリップ面を持つ射影行列を作成する。
		/// </summary>
		/// <param name="proj"></param>
		/// <param name="planeVS"></param>
		/// <returns></returns>
		Matrix MakeObliqueProjectionLike(const Matrix& proj, const Plane& planeVS);

	private:
		RenderTarget m_planarReflectionTarget;
		float clearColor[4] = { 0.0f,0.0f,0.0f,1.0f };	//カラーバッファーは真っ黒
		Camera m_reflectCamera;                         // 反射用カメラ
		ReflectionModelCB m_constantBuffer;             // 反射に映るモデルの定数バッファー
		Plane m_reflectionPlane = Plane(INIT_PLANE_NORMAL, INIT_PLANE_POSITION);	// 反射面
	};
}

