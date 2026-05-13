/**
 * @file CPReactionStatus.h
 * @brief CPReactionのステータス
 * @author 藤谷
 */
#pragma once
#include "UIStatus.h"


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
		};
	}
}


