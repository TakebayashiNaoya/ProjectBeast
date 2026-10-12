/**
 * @file HomingParticleRender.h
 * @brief 3D空間から2DのUI座標へ向かって次元を跨いで移動するパーティクルレンダラー
 */
#pragma once
#include "ParticleEffectRender.h"
#include "Source/Util/Curve.h"
#include "graphics/Camera/CameraSystem.h"


namespace app
{
    /** 前方宣言 */
    class actor::ChildPenguin;


    class HomingParticleRender
    {
    private:
        /**
		 * @brief ホーミングパーティクルの情報
         */
        struct HomingParticleInfo
        {
            /** 演出用のカーブ */
            util::Vector3Curve curve;
            /** 開始位置 */
            Vector3 startPosition;
            /** ターゲットアクター */
            actor::ChildPenguin* targetActor;
            /** パーティクルエフェクトレンダー */
            std::unique_ptr<ParticleEffectRender> effectRender;


            HomingParticleInfo();
        };

        /** ホーミングパーティクルのリスト */
        std::vector<std::unique_ptr<HomingParticleInfo>> m_homingParticles;
        /**
         * 使っていないパーティクルエフェクトレンダー（使い回し用）
         * @details 生成（JSON読み込み・スプライト初期化）は重いので、初期化時に最大数ぶん作っておき、
         *          飛ばすときに取り出して、飛び終わったら戻す。
         */
        std::vector<std::unique_ptr<ParticleEffectRender>> m_freeEffectRenders;
        /**  2DのUIの座標 */
        Vector3 m_goalPosition;
        /** パーティクルの数 */
        int m_particleCount;
        /** 同時に飛ばすパーティクルの最大数 */
        int m_maxParticles;


    public:
        HomingParticleRender();

        /** 
         * @brief 初期化 
         */
        void Initialize();

        /**
		 * @brief ターゲット座標を設定
		 * @param targetPosition 2DのUI座標
         */
		void SetGoalPosition(const Vector3 goalPosition) { m_goalPosition = goalPosition; }

        /**
		 * @brief ターゲットアクターを設定
		 * @param target ターゲットの子ペンギン
         */
        void AddTarget(actor::ChildPenguin* target);

        /**
         * @brief 描画
         * @param  rc レンダリングコンテキスト
         */
        void Render(RenderContext& rc);

        /**
         * @brief 更新
         */
        void Update();
    };
}