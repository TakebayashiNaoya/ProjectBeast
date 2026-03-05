/**
 * @file SkyCube.h
 * @brief スカイキューブ
 * @author 竹林尚哉
 */
#pragma once
#include "Graphics/ModelRender.h"


namespace nsBeastEngine
{
	/**
	 * @brief スカイキューブのタイプ
	 */
	enum EnSkyCubeType
	{
		enSkyCubeType_Day,			/** 昼間 */
		enSkyCubeType_Night,		/** 夜間 */
		enSkyCubeType_Snow,			/** 雪山 */
		enSkyCubeType_Snow_2,		/** 雪山_2 */
		enSkyCubeType_Wild,			/** 荒野 */
		enSkyCubeType_Wild_2,		/** 荒野_2 */
		enSkyCubeType_Wild_Night,	/** 荒野(夜間) */
		enSkyCubeType_Grass,		/** 芝生 */
		enSkyCubeType_Euro,			/** 欧州 */
		enSkyCubeType_DayToon,		/** 昼間(トゥーン調) */
		enSkyCubeType_DayToon_2,	/** 昼間(トゥーン調)_2 */
		enSkyCubeType_DayToon_3,	/** 昼間(トゥーン調)_3 */
		enSkyCubeType_DayToon_4,	/** 昼間(トゥーン調)_4 */
		enSkyCubeType_NightToon,	/** 夜間(トゥーン調) */
		enSkyCubeType_NightToon_2,	/** 夜間(トゥーン調)_2 */
		enSkyCubeType_SunriseToon,	/** 明け方(トゥーン調) */
		enSkyCubeType_SpaceToon_2,	/** 大気圏(トゥーン調) */
		enSkyCubeType_Num,
	};


	/**
	 * @brief スカイキューブ
	 */
	class SkyCube : public IGameObject
	{
	public:
		/**
		 * @brief 位置を設定
		 * @param pos 位置
		 */
		void SetPosition(const Vector3& pos)
		{
			m_position = pos;
			m_isChanged = true;
		}

		/**
		 * @brief 大きさを設定
		 * @param scale 大きさ
		 */
		void SetScale(const Vector3& scale)
		{
			m_scale = scale;
			m_isChanged = true;
		}
		/**
		 * @brief 大きさを設定
		 * @param scale 大きさ
		 */
		void SetScale(const float scale)
		{
			m_scale = g_vec3One;
			m_scale.Scale(scale);
			m_isChanged = true;
		}

		/**
		 * @brief スカイキューブのタイプを設定
		 * @param type タイプ
		 */
		void SetType(EnSkyCubeType type)
		{
			m_type = type;
		}

		/**
		 * @brief 明るさを設定
		 * @param lum 輝度
		 */
		void SetLuminance(float lum)
		{
			m_luminance = lum;
		}
		/**
		 * @brief スカイキューブのテクスチャファイルパスを取得
		 * @return スカイキューブのテクスチャファイルパス
		 */
		const wchar_t* GetTextureFilePath()
		{
			return m_textureFilePaths[m_type];
		}


	public:
		SkyCube();
		~SkyCube() = default;
		bool Start()override final;
		void Update()override final;
		void Render(RenderContext& rc)override final;


	private:
		/** スカイキューブの描画に使用するモデルレンダー */
		nsBeastEngine::ModelRender m_modelRender;
		/** スカイキューブのテクスチャ */
		Texture m_texture[enSkyCubeType_Num];
		/** スカイキューブのテクスチャファイルパス */
		const wchar_t* m_textureFilePaths[enSkyCubeType_Num];
		/** スカイキューブの位置 */
		Vector3 m_position;
		/** スカイキューブの大きさ */
		Vector3 m_scale;
		/** スカイキューブの輝度 */
		float m_luminance;
		/** スカイキューブの状態が変更されたか */
		bool m_isChanged;
		/** スカイキューブのタイプ */
		EnSkyCubeType m_type;
	};
}

