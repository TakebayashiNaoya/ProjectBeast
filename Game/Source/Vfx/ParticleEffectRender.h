/**
 * @file ParticleEffectRender.h
 * @brief パーティクルエフェクトの描画処理(ホットリロード対応)
 */
#pragma once
#include "ParticleEmitter.h"
#include "ParticleEffectLoader.h"
#include <vector>
#include <string>
#include <cmath>
#include <sys/stat.h>


namespace app
{
	/**
	 * @brief パーティクルエフェクトレンダラー(エミッター + スプライト描画を統合)
	 */
	class ParticleEffectRender
	{
	private:
		/** エミッター */
		ParticleEmitter m_emitter;

        /** スプライトプール */
        std::vector<std::unique_ptr<SpriteRender>> m_sprites;

        /** テクスチャ情報 */
        std::string m_texturePath;
        float m_spriteWidth;
        float m_spriteHeight;

        /** 初期化済みか */
        bool m_isInitialized;

        /** ホットリロード用 */
        std::string m_jsonFilePath;
        bool m_hotReloadEnabled;
        float m_hotReloadInterval;     // チェック間隔（秒）
        float m_hotReloadTimer;        // タイマー
        long long m_lastModifiedTime;  // ファイル最終更新時刻

    public:
        ParticleEffectRender()
            : m_spriteWidth(64.0f)
            , m_spriteHeight(64.0f)
            , m_isInitialized(false)
            , m_hotReloadEnabled(false)
            , m_hotReloadInterval(1.0f)
            , m_hotReloadTimer(0.0f)
            , m_lastModifiedTime(0)
        {}

        /**
         * JSONファイルから初期化
         * @param jsonFilePath  エフェクト定義JSON
         * @param texFilePath   パーティクル用テクスチャ(.dds)
         * @param width         スプライトの基本幅
         * @param height        スプライトの基本高さ
         */
        void Init(
            const char* jsonFilePath,
            const char* texFilePath,
            float width = 64.0f,
            float height = 64.0f)
        {
            m_jsonFilePath = jsonFilePath;
            m_texturePath = texFilePath;
            m_spriteWidth = width;
            m_spriteHeight = height;

            // JSONからエミッターを構築
            ParticleEffectLoader::LoadFromFile(jsonFilePath, m_emitter);

            // スプライトプール初期化
            InitSpritePool();

            // ファイル更新時刻を記録
            m_lastModifiedTime = GetFileModifiedTime(m_jsonFilePath);

            m_isInitialized = true;
        }

        /** エミッターへの直接アクセス */
        ParticleEmitter& GetEmitter() { return m_emitter; }

        // ----- ホットリロード設定 -----

        /**
         * @brief ホットリロードを有効化
         * @param intervalSec  ファイル変更チェック間隔（秒）
         */
        void EnableHotReload(float intervalSec = 1.0f)
        {
            m_hotReloadEnabled = true;
            m_hotReloadInterval = intervalSec;
            m_hotReloadTimer = 0.0f;
        }

        /**
         * @brief ホットリロードを無効化
         */
        void DisableHotReload()
        {
            m_hotReloadEnabled = false;
        }

        /**
         * @brief 手動でJSONをリロード
         * @return 成功したらtrue
         * 座標とプレイ状態は維持される
         */
        bool Reload()
        {
            if (m_jsonFilePath.empty()) return false;

            // 現在の状態を保存
            Vector3 pos = m_emitter.GetPosition();
            bool wasPlaying = m_emitter.IsPlaying();

            // エミッターをクリアして再構築
            m_emitter.ClearModules();
            bool success = ParticleEffectLoader::LoadFromFile(m_jsonFilePath.c_str(), m_emitter);

            if (success) {
                // スプライトプールのサイズが変わった場合は再初期化
                int maxParticles = m_emitter.GetSpawnModule().GetMaxParticles();
                if (static_cast<int>(m_sprites.size()) != maxParticles) {
                    InitSpritePool();
                }

                // 状態復帰
                m_emitter.SetPosition(pos);
                m_emitter.Reset();
                if (wasPlaying) {
                    m_emitter.Play();
                }

                // 更新時刻を記録
                m_lastModifiedTime = GetFileModifiedTime(m_jsonFilePath);
            }

            return success;
        }

        // ----- 制御 -----
        
        /**
         * @brief エフェクトを再生
         */
        void Play()
        {
            m_emitter.Play();
        }

        /**
         * @brief エフェクトを停止
         */
        void Stop()
        {
            m_emitter.Stop();
        }

        /**
         * @brief エフェクトをリセット
         */
        void Reset()
        {
            m_emitter.Reset();
            for (size_t i = 0; i < m_sprites.size(); ++i) {
                m_sprites[i]->SetScale(Vector3::Zero);
            }
        }
        
        /**
         * @brief 位置を設定
         * @param pos 位置
         */
        void SetPosition(const Vector3& pos)
        {
            m_emitter.SetPosition(pos);
        }

		/**
		 * @brief 位置を取得
		 * @return 位置
		 */
        bool IsPlaying() const
        {
            return m_emitter.IsPlaying();
        }


        // ----- 更新・描画 -----

        /**
         * @brief 更新
         * @param deltaTime 時間差分
         */
        void Update(float deltaTime)
        {
            if (!m_isInitialized) return;

            // ホットリロードチェック
            if (m_hotReloadEnabled) {
                m_hotReloadTimer += deltaTime;
                if (m_hotReloadTimer >= m_hotReloadInterval) {
                    m_hotReloadTimer = 0.0f;
                    CheckAndReload();
                }
            }

            // エミッター更新
            m_emitter.Update(deltaTime);

            // パーティクル → スプライト反映
            const std::vector<Particle>& particles = m_emitter.GetParticles();
            for (size_t i = 0; i < particles.size() && i < m_sprites.size(); ++i) {
                const Particle& p = particles[i];

                if (!p.isAlive) {
                    m_sprites[i]->SetScale(Vector3::Zero);
                    continue;
                }

                m_sprites[i]->SetPosition(p.position);
                m_sprites[i]->SetScale(p.scaleValue);

                Quaternion rot;
                float rad = p.rotationAngle * 3.14159265f / 180.0f;
                rot.SetRotation(Vector3(0.0f, 0.0f, 1.0f), rad);
                m_sprites[i]->SetRotation(rot);

                m_sprites[i]->SetMulColor(p.color);
            }

            for (size_t i = 0; i < m_sprites.size(); ++i) {
                m_sprites[i]->Update();
            }
        }

        /**
         * @brief 描画
         * @param rc 描画コンテキスト
         */
        void Draw(RenderContext& rc)
        {
            if (!m_isInitialized) return;

            const std::vector<Particle>& particles = m_emitter.GetParticles();
            for (size_t i = 0; i < particles.size() && i < m_sprites.size(); ++i) {
                if (particles[i].isAlive) {
                    m_sprites[i]->Draw(rc);
                }
            }
        }

    private:
        /** 
         * @brief スプライトプールを（再）初期化
         */
        void InitSpritePool()
        {
            int maxParticles = m_emitter.GetSpawnModule().GetMaxParticles();
            m_sprites.clear();
            m_sprites.resize(maxParticles);
            for (int i = 0; i < maxParticles; ++i) {
                m_sprites[i] = std::make_unique<SpriteRender>();
                m_sprites[i]->Init(m_texturePath.c_str(), m_spriteWidth, m_spriteHeight, AlphaBlendMode_Trans);
                m_sprites[i]->SetScale(Vector3::Zero);
            }
        }

        /** 
         * @brief ファイルの最終更新時刻を取得
         * @param filePath ファイルパス
         * @return 最終更新時刻
         */
        static long long GetFileModifiedTime(const std::string& filePath)
        {
            struct stat fileStat;
            if (stat(filePath.c_str(), &fileStat) == 0) {
                return static_cast<long long>(fileStat.st_mtime);
            }
            return 0;
        }

        /** 
         * @brief ファイルが変更されていたらリロード 
         */
        void CheckAndReload()
        {
            long long currentTime = GetFileModifiedTime(m_jsonFilePath);
            if (currentTime != 0 && currentTime != m_lastModifiedTime) {
                Reload();
            }
        }
    };
}
