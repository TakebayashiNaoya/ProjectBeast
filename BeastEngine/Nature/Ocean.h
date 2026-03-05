#pragma once

namespace nsBeastEngine
{
	namespace
	{
		const Vector3 INIT_OCEAN_PLANE_NORMAL = g_vec3Up;	// 平面の法線
		const Vector3 INIT_OCEAN_PLANE_POSITION = g_vec3Zero;// 平面のポジション（平面が通る点）
	}

	class Ocean :public IGameObject
	{
		struct OceanConstantBuffer
		{
			//Matrix ReflectionCameraVP; // 反射用カメラビュー投影行列
			Light light;

			//反射の割合の下限値、必ずこの値以上は反射する。
			//（真上から見た反射率）
			float baseReflectance = 0.0f; // 基本反射率

			float waveScroll = 0.0f; // 波のスクロール値
		};


	public:
		Ocean() = default;
		~Ocean() = default;

		/**
		 * @brief 波のスクロール速度を設定
		 */
		inline void SetWaveSpeed(float speed) { m_waveSpeed = speed; }


	private:
		bool Start()override final;
		void Update()override final;
		void Render(RenderContext& rc)override final;

		/**
		 * @brief 波のスクロール値を更新
		 */
		void UpdateWaveOffset()
		{
			float deltaTime = g_gameTime->GetFrameDeltaTime();
			m_constantBuffer.waveScroll += deltaTime * m_waveSpeed;
		}

		/**
		 * @brief 位置を設定
		 * @param pos 位置
		 */
		void SetPosition(const Vector3& pos)
		{
			m_position = pos;
			m_isDirty = true;
		}

		/**
		 * @brief 大きさを設定
		 * @param scale 拡大率
		 */
		void SetScale(const Vector3& scale)
		{
			m_scale = scale;
			m_isDirty = true;
		}

		/**
		 * @brief 大きさを設定
		 * @param scale 拡大率
		 */
		void SetScale(const float scale)
		{
			m_scale = g_vec3One;
			m_scale.Scale(scale);
			m_isDirty = true;
		}

		/**
		 * @brief 回転を設定
		 * @param rotation 回転
		 */
		void SetRotation(Quaternion rotation)
		{
			m_rotation = rotation;
			m_isDirty = true;
		}

		/**
		 * @brief 定数バッファを設定
		 * @param vpMatrix ビュープロジェクション行列
		 * @param inLight ライト
		 * @param cameraPos カメラ位置
		 * @param baseReflectance 基本反射率
		 */
		void SetConstatntBuffer(
			const Matrix& vpMatrix,
			const Light& inLight,
			Vector3 cameraPos,
			float baseReflectance
		) {
			//m_constantBuffer.ReflectionCameraVP = vpMatrix;
			m_constantBuffer.light = inLight;
			m_constantBuffer.light.m_cameraPosition = cameraPos;
			m_constantBuffer.baseReflectance = baseReflectance;
		};


	private:
		ModelRender m_modelRender;
		Vector3 m_position = g_vec3Zero;
		Vector3 m_scale = g_vec3One * 5.0f;
		Quaternion m_rotation = Quaternion::Identity;
		bool m_isDirty = false;
		RenderTarget* m_reflectionRenderTarget;
		Light m_light;
		Plane m_plane = Plane(INIT_OCEAN_PLANE_NORMAL, INIT_OCEAN_PLANE_POSITION);
		OceanConstantBuffer m_constantBuffer;
		float m_waveSpeed = 0.01f;
	};
}

