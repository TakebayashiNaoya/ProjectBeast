/**
 * @file SceneLight.h
 * @brief シーンライト
 * @author 竹林尚哉
 */
#pragma once
#include "Graphics/Camera/CameraSystem.h"


namespace nsBeastEngine
{
	/** ポイントライトの最大数 */
	static const int MAX_POINT_LIGHT = 32;
	/** スポットライトの最大数 */
	static const int MAX_SPOT_LIGHT = 32;


	/**
	 * @brief ディレクションライトの構造体
	 */
	struct SDirectionLight
	{
		Vector3 m_direction;	/** ディレクションライトの方向 */
		float	padding0;
		Vector3 m_color;		/** ディレクションライトの色 */
		float	padding1;
		Matrix	m_LVP;			/** ライトビュープロジェクション行列 */


		/**
		 * @brief コンストラクタ
		 */
		SDirectionLight()
			: m_direction(Vector3::Zero)
			, padding0(0.0f)
			, m_color(Vector3::Zero)
			, padding1(0.0f)
			, m_LVP(Matrix::Identity)
		{}

		/**
		 * @brief ディレクションライトの方向の設定
		 * @param direction ディレクションライトの方向
		 */
		void SetDirection(const Vector3& direction)
		{
			m_direction = direction;
			m_direction.Normalize();
		}
		/**
		 * @brief ディレクションライトの方向の設定
		 * @param x x成分
		 * @param y y成分
		 * @param z z成分
		 */
		void SetDirection(float x, float y, float z)
		{
			SetDirection({ x,y,z });
		}

		/**
		 * @brief 色の設定
		 * @param color 色
		 */
		void SetColor(const Vector3& color)
		{
			m_color = color;
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

		///**
		// * @brief ライトビュープロジェクション行列の設定
		// * @param LVP ライトビュープロジェクション行列
		// * @note シャドウマップの生成に使用される行列を設定する
		// */
		//void UpdateLVP(const Matrix LVP)
		//{
		//	m_LVP = LVP;
		//}
	};




	/*******************************************************/


	/**
	 * @brief ポイントライトの構造体
	 */
	struct SPointLight
	{
		Vector3 m_position;			/** 位置 */
		int		m_isUsed;			/** 使用中かどうか */
		Vector3	m_color;			/** 色 */
		float	m_range;			/** 影響範囲 */
		Vector3 m_positionInView;	/** カメラ空間での座標 */
		float   padding;


		/**
		 * @brief コンストラクタ
		 */
		SPointLight()
			: m_position(Vector3::Zero)
			, m_isUsed(false)
			, m_color(Vector3::Zero)
			, m_range(0.0f)
			, m_positionInView(Vector3::Zero)
			, padding(0.0f)
		{}

		/**
		 * @brief 位置の設定
		 * @param position 位置
		 */
		void SetPosition(const Vector3& position)
		{
			m_position = position;
		}
		/**
		 * @brief 位置の設定
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
			m_color = color;
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
			m_range = range;
		}

		/**
		 * @brief 使用中にする
		 */
		void SetIsUsed()
		{
			m_isUsed = true;
		}

		/**
		 * @brief 更新
		 */
		void Update();
	};




	/*******************************************************/


	/**
	 * @brief スポットライトの構造体
	 */
	struct SSpotLight
	{
		Vector3 m_position;			/** 位置 */
		int		m_isUsed;			/** ライトの使用状況 */
		Vector3 m_color;			/** 色 */
		float	m_range;			/** 影響範囲 */
		Vector3 m_direction;		/** 向き */
		float	m_angle;			/** 射出角度 */
		Vector3 m_positionInView;	/** カメラ空間での座標 */
		float	padding;


		/**
		 * @brief コンストラクタ
		 */
		SSpotLight()
			: m_position(Vector3::Zero)
			, m_isUsed(false)
			, m_color(Vector3::Zero)
			, m_range(0.0f)
			, m_direction(Vector3::Zero)
			, m_angle(0.0f)
			, m_positionInView(Vector3::Zero)
			, padding(0.0f)
		{}

		/**
		 * @brief 位置の設定
		 * @param position 位置
		 */
		void SetPosition(const Vector3& position)
		{
			m_position = position;
		}
		/**
		 * @brief 位置の設定
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
			m_color = color;
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
			m_range = range;
		}

		/**
		 * @brief 向きの設定
		 * @param direction 向き
		 */
		void SetDirection(const Vector3& direction)
		{
			m_direction = direction;
			m_direction.Normalize();
		}
		/**
		 * @brief 向きの設定
		 * @param x x成分
		 * @param y y成分
		 * @param z z成分
		 */
		void SetDirection(float x, float y, float z)
		{
			SetDirection({ x,y,z });
		}

		/**
		 * @brief 射出角度の設定
		 * @param angle 射出角度(単位：Degree)
		 */
		void SetAngle(const float angle)
		{
			m_angle = Math::DegToRad(angle);
		}

		/**
		 * @brief 使用中にする
		 */
		void SetIsUsed()
		{
			m_isUsed = true;
		}

		/**
		 * @brief 更新
		 */
		void Update();
	};




	/*******************************************************/


	/**
	 * @brief 半球ライトの構造体
	 */
	struct SHemisphereLight
	{
		Vector3 m_groundColor;	/** 地面の色 */
		float padding0;
		Vector3 m_skyColor;		/** 空の色 */
		float padding1;
		Vector3 m_groundNormal;	/** 地面の法線 */
		float padding2;


		/**
		 * @brief コンストラクタ
		 */
		SHemisphereLight()
			: m_groundColor(Vector3::Zero)
			, padding0(0.0f)
			, m_skyColor(Vector3::Zero)
			, padding1(0.0f)
			, m_groundNormal(Vector3::Up)
		{}

		/**
		 * @brief 地面の色の設定
		 * @param color 地面の色
		 */
		void SetGroundColor(const Vector3& color)
		{
			m_groundColor = color;
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
			m_skyColor = color;
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
			m_groundNormal = normal;
			m_groundNormal.Normalize();
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
	};




	/*******************************************************/


	/**
	 * @brief ライトの構造体
	 */
	struct Light
	{
		SDirectionLight		m_directionLight;				// ディレクションライト
		// 現在未使用（実装時に有効化）						
		//SPointLight		m_pointLight[MAX_POINT_LIGHT];	// ポイントライト
		//SSpotLight		m_spotLight[MAX_SPOT_LIGHT];	// スポットライト
		//SHemisphereLight	m_hemisphereLight;				// 半球ライト
		//int				m_usedPointLightCount;			// 使用中のポイントライトの数
		//int				m_usedSpotLightCount;			// 使用中のスポットライトの数
		Vector3				m_cameraPosition;				// カメラの位置
		float				padding0;						// パディング（16バイトアラインメントのため）
		Vector3				m_ambientLightColor;			// 環境光の色
		float				padding1;						// パディング（16バイトアラインメントのため）
		Vector3				m_rimLightColor;				// リムライトの色
		float				padding2;						// パディング（16バイトアラインメントのため）
		Matrix				m_mViewProjInv;					// カメラのビュープロジェクション行列の逆行列


		/**
		 * @brief コンストラクタ
		 */
		Light()
			: m_cameraPosition(Vector3::Zero)
			, padding0(0.0f)
			, m_ambientLightColor(Vector3::Zero)
			, padding1(0.0f)
			, m_rimLightColor(Vector3::Zero)
			, padding2(0.0f)
			, m_mViewProjInv(Matrix::Identity)
		{}

		/**
		 * @brief カメラの位置の設定
		 */
		void SetCameraPos()
		{
			m_cameraPosition = CameraSystem::Get().GetMainCamera().GetPosition();
		}

		/**
		 * @brief 環境光の色の設定
		 * @param color 環境光の色
		 */
		void SetAmbientLight(const Vector3& color)
		{
			m_ambientLightColor = color;
		}
		/**
		 * @brief 環境光の色の設定
		 * @param x 赤成分
		 * @param y 緑成分
		 * @param z 青成分
		 */
		void SetAmbientLight(float x, float y, float z)
		{
			SetAmbientLight({ x,y,z });
		}

		/**
		 * @brief リムライトの色の設定
		 * @param color リムライトの色
		 */
		void SetRimLight(const Vector3& color)
		{
			m_rimLightColor = color;
		}
		/**
		 * @brief リムライトの色の設定
		 * @param x 赤成分
		 * @param y 緑成分
		 * @param z 青成分
		 */
		void SetRimLight(float x, float y, float z)
		{
			SetRimLight({ x,y,z });
		}
	};




	/*******************************************************/


	class SceneLight
	{
	public:
		SceneLight() = default;
		~SceneLight() = default;

		/**
		 * @brief 初期化
		 */
		void Init();

		/**
		 * @brief 更新
		 */
		void Update();

		/**
		 * @brief ディレクションライトの位置の設定
		 * @param pos ディレクションライトの位置
		 */
		void SetLightPos(const Vector3& pos)
		{
			m_lightPosition = pos;
		}

		/**
		 * @brief ディレクションライトの位置をリセットする
		 */
		void RemoveLightPos()
		{
			m_lightPosition = Vector3::Zero;
		}

		/**
		 * @brief ライトの取得
		 * @return ライト
		 */
		Light* GetLight()
		{
			return &m_light;
		}

		/**
		 * @brief ディレクションライトのライトビュープロジェクション行列の取得
		 * @details RenderingEngine::RenderShadowMap() が毎フレーム更新する。
		 *          海や渦潮など、独自シェーダーで影を受けるオブジェクトが参照する。
		 * @return ライトビュープロジェクション行列の参照
		 */
		Matrix& GetLVP()
		{
			return m_light.m_directionLight.m_LVP;
		}

		// 現在未使用（実装時に有効化）
		///**
		// * @brief ポイントライトの生成
		// */
		//SPointLight* NewPointLight();
		///**
		// * @brief スポットライトの生成
		// */
		//SSpotLight* NewSpotLight();
		///**
		// * @brief 半球ライトの取得
		// * @return 半球ライト
		// */
		//SHemisphereLight* GetHemisphereLight() { return &m_light.m_hemisphereLight; }


	public:
		/** ディレクションライトのライトビュープロジェクション行列 */
		Matrix m_mLVP;
		/** シーンライト */
		Light m_light;


	private:
		/** ディレクションライトの位置 */
		Vector3 m_lightPosition = Vector3::Zero;

		//// 現在未使用（実装時に有効化）
		///** 使用されていないポイントライトのキュー */
		//std::deque<SPointLight*> m_unusePointLightQueue;
		///** 使用されていないスポットライトのキュー */
		//std::deque<SSpotLight*> m_unuseSpotLightQueue;
	};
};