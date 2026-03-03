/**
 * @file SpotLight.h
 * @brief スポットライト
 * @author 竹林尚哉
 */
#pragma once


namespace nsBeastEngine
{
	class SpotLight
	{
	public:
		SpotLight();
		~SpotLight() = default;

		/**
		 * @brief スポットライトの初期化
		 * @param position	位置
		 * @param color		色
		 * @param range		影響距離
		 * @param direction	ライトの向き
		 * @param angle		ライトの影響範囲
		 */
		void Init(const Vector3& position, const Vector3& color, const float& range, const Vector3& direction, const float angle);

		/**
		 * @brief スポットライトの位置の設定
		 * @param position	位置
		 */
		void SetPosition(const Vector3& position)
		{
			m_spotLight->SetPosition(position);
		}
		/**
		 * @brief スポットライトの位置の設定
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
		 * @param color	色
		 */
		void SetColor(const Vector3& color)
		{
			m_spotLight->SetColor(color);
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
		 * @brief 影響距離の設定
		 * @param range	影響距離
		 */
		void SetRange(const float& range)
		{
			m_spotLight->SetRange(range);
		}

		/**
		 * @brief ライトの向きの設定
		 * @param direction ライトの向き
		 */
		void SetDirection(const Vector3& direction)
		{
			m_spotLight->SetDirection(direction);
		}
		/**
		 * @brief ライトの向きの設定
		 * @param x x成分
		 * @param y y成分
		 * @param z z成分
		 */
		void SetDirection(float x, float y, float z)
		{
			SetDirection({ x,y,z });
		}

		/**
		 * @brief ライトの影響範囲の設定
		 * @param angle ライトの影響範囲
		 */
		void SetAngle(const float& angle)
		{
			m_spotLight->SetAngle(angle);
		}

		/**
		 * @brief 使用中にする
		 */
		void SetIsUsed()
		{
			m_spotLight->m_isUsed = true;
		}


	private:
		/** スポットライト */
		SSpotLight* m_spotLight;
	};
}