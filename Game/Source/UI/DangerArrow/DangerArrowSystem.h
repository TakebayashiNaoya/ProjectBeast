/**
 * @file DangerArrowSystem.h
 * @brief 危険矢印UIのシステムクラス
 * @author 竹林
 */
#pragma once
#include <array>
#include <vector>
#include "DangerArrowMenu.h"
#include "DangerArrowCalc.h"
#include "Source/UI/System/SystemPacket.h"


namespace app
{
	namespace actor
	{
		class ChildPenguin;
	}


	namespace ui
	{
		/**
		 * @brief 危険矢印システム
		 * @details 攻撃中のシロクマのターゲット子ペンギンをUIで示す。
		 *
		 *   - フラスタム外 → 画面中央を中心とした円縁に矢印（edge arrow）を配置。
		 *                     距離に関わらずサブビューも表示する。
		 *   - フラスタム内 → 子ペンギンの真上にワールド空間矢印（overhead arrow）を配置。
		 *     - 距離 < SUB_VIEW_HIDE_DIST → サブビューを非表示
		 *     - 距離 >= SUB_VIEW_HIDE_DIST → サブビューを表示
		 *
		 *   サブビューは「最もカメラに近い攻撃対象」の矢印位置に追従する。
		 *
		 *   矢印DDSは上向き（+Y）を向いたデザインを想定。
		 */
		class DangerArrowSystem
		{
		public:
			DangerArrowSystem() = default;
			~DangerArrowSystem() = default;


		public:
			/** @brief 初期化 */
			void Initialize();
			/** @brief 更新（application->Update() 内から呼ぶ） */
			void Update();
			/** @brief 描画（application->Render() 内から呼ぶ） */
			void Render(RenderContext& rc);


		public:
			/** 1エネミー分のターゲットキャッシュ */
			struct EnemySlot
			{
				const actor::ChildPenguin* cachedTarget = nullptr;
			};

			/** 1本の矢印の描画情報 */
			struct ArrowInfo
			{
				Vector2 screenPos;
				float   angleRad;
				float   distSq;
				bool    isOverhead;
				bool    visible;
			};


		private:
			/** 矢印の最大本数（EnemyManagerのエネミー数上限に合わせる） */
			static constexpr int MAX_ARROWS = 3;
			/** これ以下の距離（ワールド単位）かつフラスタム内ならサブビューを非表示 */
			static constexpr float SUB_VIEW_HIDE_DIST = 1000.0f;
			/** サブビューの表示スケール（1.0 = 480x270px 原寸） */
			static constexpr float SUB_VIEW_SCALE = 1.0f;
			/** 矢印の配置・回転定数は DangerArrowCalc.h に一元化 */


		private:
			/** @brief エネミーごとのターゲットキャッシュを更新する */
			void RefreshTargetCache();

			/** @brief 各ターゲットの矢印情報を計算してm_arrowInfosに格納する */
			void CalcArrowInfos();

			/**
			 * @brief サブビューの表示位置と可視性を更新する
			 * @details 最近傍の攻撃対象に基づいてSubCameraManagerを操作する
			 */
			void UpdateSubView();

			/**
			 * @brief edge矢印（フラスタム外）の情報を計算して返す
			 * @param screenPos ワールド→スクリーン変換後の座標
			 * @param distSq   カメラからの距離2乗
			 * @return ArrowInfo（visible=true, isOverhead=false）
			 */
			ArrowInfo CalcEdgeArrow(const Vector2& screenPos, float distSq) const;

			/**
			 * @brief overhead矢印（フラスタム内）の情報を計算して返す
			 * @param screenPos ワールド→スクリーン変換後の座標
			 * @param distSq   カメラからの距離2乗
			 * @return ArrowInfo（visible=true, isOverhead=true）
			 */
			ArrowInfo CalcOverheadArrow(const Vector2& screenPos, float distSq) const;


		private:
			/** エネミースロット（ターゲットキャッシュ） */
			std::vector<EnemySlot> m_enemySlots;

			/** 今フレームの矢印情報リスト（最大MAX_ARROWS本） */
			std::vector<ArrowInfo> m_arrowInfos;

			/** 矢印メニューのパケット */
			std::array<SystemPacket<DangerArrowMenu>, MAX_ARROWS> m_packets;
		};
	}
}
