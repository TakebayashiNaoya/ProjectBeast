/**
 * @file WpWarningSystem.h
 * @brief WpWarningのシステムクラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/System/SystemPacket.h"

#include "WpWarningMenu.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class DaddyPenguin;
	}


	namespace ui
	{
		/** パケットの数 */
		constexpr int PACKET_NUM = 3;


		/** 前方宣言 */
		class WpWarningStatus;


		/**
		 * @brief WpWarningのシステムクラス
		 */
		class WpWarningSystem
		{
		public:
			WpWarningSystem();
			~WpWarningSystem();


		public:
			/**
			 * @brief 親ペンギンのセット
			 * @param daddyPenguin 親ペンギンのポインタ
			 */
			void SetDaddyPenguin(actor::DaddyPenguin* daddyPenguin)
			{
				m_daddyPenguin = daddyPenguin;
			}


		public:
			/** @brief 初期化 */
			void Initialize();

			/** @brief 更新 */
			void Update();

			/** @brief 描画 */
			void Render(RenderContext& rc);


		private:
			/** @brief 描画フラグの更新 */
			void UpdateDrawFlags();


		private:
			/** パケットの配列 */
			std::array<SystemPacket<WpWarningMenu>, PACKET_NUM> m_packets;
			/** 親ステータス */
			std::unique_ptr<WpWarningStatus> m_parentStatus;
			/** 親ペンギン */
			actor::DaddyPenguin* m_daddyPenguin;


		private:
			/** 渦潮の情報を格納する構造体 */
			struct WpInfo
			{
				float lengthSq;
				nature::Whirlpool* wp;
			};

			/** 渦潮の情報の配列 */
			std::vector<WpInfo> m_wpInfos;
		};
	}
}