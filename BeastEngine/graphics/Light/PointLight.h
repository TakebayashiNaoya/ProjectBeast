/**
 * @file PointLight.h
 * @brief ポイントライト
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	class PointLight
	{
	public:
		PointLight();
		~PointLight() = default;

		/**
		 * @brief ポイントライトの初期化
		 * @param pos	位置
		 * @param color	色
		 * @param range	影響距離
		 */
		void Init(const Vector3& pos, const Vector3& color, const float& range);

		/**
		 * @brief ポイントライトの位置の設定
		 * @param pos 位置
		 */
		void SetPosition(const Vector3& pos)
		{
			m_pointLight->SetPosition(pos);
		}
		/**
		 * @brief ポイントライトの位置の設定
		 * @param x x座標
		 * @param y y座標
		 * @param z z座標
		 */
		void SetPosition(float x, float y, float z)
		{
			SetPosition({ x,y,z });
		}

		/**
		 * @brief 色の設定
		 * @param color 色
		 */
		void SetColor(const Vector3& color)
		{
			m_pointLight->SetColor(color);
		}
		/**
		 * @brief 色の設定
		 * @param x 赤成分
		 * @param y 緑成分
		 * @param z 青成分
		 */
		void SetColor(float x, float y, float z)
		{
			SetColor({ x,y,z });
		}

		/**
		 * @brief 影響範囲の設定
		 * @param range 影響範囲
		 */
		void SetRange(const float& range)
		{
			m_pointLight->SetRange(range);
		}


	private:
		/** ポイントライト */
		SPointLight* m_pointLight;
	};
}
