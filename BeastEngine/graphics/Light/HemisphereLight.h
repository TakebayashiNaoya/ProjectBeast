/**
 * @file HemisphereLight.h
 * @brief 半球ライト
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	class HemisphereLight
	{
	public:
		HemisphereLight();
		~HemisphereLight() = default;

		/**
		 * @brief 半球ライトの初期化
		 * @param groundColor	地面の色
		 * @param skyColor		空の色
		 * @param groundNormal	地面の法線
		 */
		void Init(const Vector3& groundColor, const Vector3& skyColor, const Vector3& groundNormal)
		{
			SetGroundColor(groundColor);
			SetSkyColor(skyColor);
			SetGroundNormal(groundNormal);
		}

		/**
		 * @brief 地面の色の設定
		 * @param color 地面の色
		 */
		void SetGroundColor(const Vector3& color)
		{
			m_hemisphereLight->SetGroundColor(color);
		}
		/**
		 * @brief 地面の色の設定
		 * @param x 赤成分
		 * @param y 緑成分
		 * @param z 青成分
		 */
		void SetGroundColor(float x, float y, float z)
		{
			SetGroundColor({ x,y,z });
		}

		/**
		 * @brief 空の色の設定
		 * @param color 空の色
		 */
		void SetSkyColor(const Vector3& color)
		{
			m_hemisphereLight->SetSkyColor(color);
		}
		/**
		 * @brief 空の色の設定
		 * @param x 赤成分
		 * @param y 緑成分
		 * @param z 青成分
		 */
		void SetSkyColor(float x, float y, float z)
		{
			SetSkyColor({ x,y,z });
		}

		/**
		 * @brief 地面の法線の設定
		 * @param normal 地面の法線
		 */
		void SetGroundNormal(const Vector3& normal)
		{
			m_hemisphereLight->SetGroundNormal(normal);
		}
		/**
		 * @brief 地面の法線の設定
		 * @param x x成分
		 * @param y y成分
		 * @param z z成分
		 */
		void SetGroundNormal(float x, float y, float z)
		{
			SetGroundNormal({ x,y,z });
		}


	private:
		/** 半球ライト */
		SHemisphereLight* m_hemisphereLight;
	};
}
