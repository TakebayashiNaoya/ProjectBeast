/**
 * @file WpWarningSystem.h
 * @brief WpWarningのシステムクラス
 * @author 藤谷
 */
#pragma once
#include "Source/UI/Modules/FrontChecker/FrontChecker.h"
#include "Source/UI/Modules/System/SystemPacket.h"



namespace app
{
	namespace ui
	{
		/** パケットの数 */
		constexpr int PACKET_NUM = 3;


		/** 前方宣言 */
		class WpWarningStatus;
		class WpWarningMenu;


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
			 * @brief 親ペンギンの位置を設定
			 * @param position 親ペンギンの位置
			 */
			void SetDaddyTRS(const core::Transform& trs)
			{
				m_daddyTransform.m_position = trs.m_position;
				m_daddyTransform.m_rotation = trs.m_rotation;
				m_daddyTransform.m_scale = trs.m_scale;
			}


			/**
			 * @brief 渦潮の位置を設定
			 * @param positions 渦潮の位置の配列
			 */
			void SetWhirlpoolPositions(const std::vector<Vector3>& positions)
			{
				m_whirlpoolPositions = positions;
			}


			/**
			 * @brief 描画フラグの更新
			 */
			void UpdateDrawFlags();


		public:
			/** @brief 初期化 */
			void Initialize();

			/** @brief 更新 */
			void Update();

			/** @brief 描画 */
			void Render(RenderContext& rc);


		private:
			/** パケットの配列 */
			std::array<UIPacket<WpWarningMenu>, PACKET_NUM> m_packets;
			/** 親ステータス */
			std::unique_ptr<WpWarningStatus> m_parentStatus;

			/** 親ペンギンの変換 */
			core::Transform m_daddyTransform;
			/** 渦潮の位置の配列 */
			std::vector<Vector3> m_whirlpoolPositions;
		};
	}
}