/**
 * @file CPReactionStatus.h
 * @brief CPReactionのステータス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Model/UIStatus.h"


namespace app
{
	namespace ui
	{
		/**
		 * @biref CPReactionのステータス
		 */
		class CPReactionStatus : public UIStatus
		{
		public:
			CPReactionStatus();
			virtual ~CPReactionStatus() override;

			/**
			 * @brief セットアップUI
			 * @detail ステータスの持ち主が呼び出す。
			 */
			void SetUpUI() override final;
			/**
			 * @brief 更新処理
			 */
			void Update() override final;


		public:
			/** @brief 揺れ時間を取得する */
			inline float GetSwayTime() const { return m_swayTime; }
			/** @brief アイコンのYオフセットを取得する */
			inline float GetIconOffsetY() const { return m_iconOffsetY; }
			/** @brief 吹き出しのオフセットを取得する */
			inline Vector3 GetSpeechBubbleOffset() const { return m_speechBubbleOffset; }
			/** @brief トラブルリアクションのオフセットを取得する */
			inline Vector3 GetTroubleReactionOffset() const { return m_troubleReactionOffset; }
			/** @brief ハッピーリアクションのオフセットを取得する */
			inline Vector3 GetHappyReactionOffset() const { return m_happyReactionOffset; }
			/** @brief 真面目リアクションのカラーを取得する */
			inline Vector4 GetSeriousReactionColor() const { return m_seriousReactionColor; }
			/** @brief 甘えん坊リアクションのカラーを取得する */
			inline Vector4 GetClingyReactionColor() const { return m_clingyReactionColor; }
			/** @brief やんちゃリアクションのカラーを取得する */
			inline Vector4 GetNaughtyReactionColor() const { return m_naughtyReactionColor; }
			/** @brief おっちょこちょいリアクションのカラーを取得する */
			inline Vector4 GetClumsyReactionColor() const { return m_clumsyReactionColor; }
			/** @brief 世話焼きリアクションのカラーを取得する */
			inline Vector4 GetCaringReactionColor() const { return m_caringReactionColor; }


		private:
			/** 揺れ時間 */
			float m_swayTime;
			/** アイコンのYオフセット */
			float m_iconOffsetY;
			/** 吹き出しのオフセット */
			Vector3 m_speechBubbleOffset;
			/** 困りリアクションのオフセット */
			Vector3 m_troubleReactionOffset;
			/** 喜びリアクションのオフセット */
			Vector3 m_happyReactionOffset;
			/** 真面目リアクションのカラー */
			Vector4 m_seriousReactionColor;
			/** 甘えん坊リアクションのカラー */
			Vector4 m_clingyReactionColor;
			/** やんちゃリアクションのカラー */
			Vector4 m_naughtyReactionColor;
			/** おっちょこちょいリアクションのカラー */
			Vector4 m_clumsyReactionColor;
			/** 世話焼きリアクションのカラー */
			Vector4 m_caringReactionColor;
		};
	}
}


