/**
 * @file DangerArrowMenu.h
 * @brief 危険矢印UI（1本分）のMenuクラス
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
    namespace ui
    {
        /**
         * @brief 危険矢印Menu
         * @details シロクマに襲われている子ペンギン1体分の方向矢印を管理する。
         *          DangerArrowSystem が毎フレーム SetArrowScreenPos / SetArrowAngleRad /
         *          SetVisible を呼び出し、Update() で UIIcon のトランスフォームに反映する。
         *
         *          矢印DDSは上方向（+Y）を向いたデザインを想定。
         *          - フラスタム外（edge）: Z回転で目標方向を指す
         *          - フラスタム内（overhead）: 180°回転で下向きにし、ペンギン真上に配置
         */
        class DangerArrowMenu : public MenuBase
        {
            using Base = MenuBase;


        public:
            DangerArrowMenu() = default;
            ~DangerArrowMenu() override = default;


        public:
            /** @brief UIのロジック初期化 */
            void InitializeLogic() override final;
            /** @brief 毎フレーム更新 */
            void Update() override final;


        public:
            /**
             * @brief スクリーン座標（スプライト空間）を設定する
             * @param pos スクリーン座標（中心=(0,0)、範囲±フレームバッファハーフサイズ）
             */
            void SetArrowScreenPos(const Vector2& pos) { m_arrowScreenPos = pos; }

            /**
             * @brief 矢印の回転角度（ラジアン）を設定する
             * @param rad Z軸回転。DDSが上向きを基準とし、atan2(dy, dx) - π/2 を渡す
             */
            void SetArrowAngleRad(const float rad) { m_arrowAngleRad = rad; }

            /**
             * @brief 矢印の表示・非表示を設定する
             */
            void SetVisible(const bool visible) { m_isVisible = visible; }

            /**
             * @brief サブビューに映っているペンギンを指す矢印かどうかを設定する
             * @details true にするとサイン波スケールで小刻みに拡縮する
             */
            void SetPulsing(const bool pulsing) { m_isPulsing = pulsing; }


        private:
            /** @brief UIIconのトランスフォームと isDraw を更新する */
            void UpdateArrowTransform();


        private:
            /** 矢印アイコン */
            UIIcon* m_arrowIcon = nullptr;

            /** 現在のスクリーン座標 */
            Vector2 m_arrowScreenPos = Vector2::Zero;
            /** 現在の回転角度（ラジアン） */
            float m_arrowAngleRad = 0.0f;
            /** 表示フラグ */
            bool m_isVisible = false;
            /** パルスアニメーション中かどうか */
            bool m_isPulsing = false;
            /** パルスアニメーション用タイマー（ラジアン） */
            float m_pulseTimer = 0.0f;
        };
    }
}
