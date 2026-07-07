/**
 * @file ParticleEmitter.h
 * @brief モジュールを組み合わせてエフェクトを制御する
 * @author 忽那
 */
#pragma once
#include "Particle.h"
#include "ParticleModule.h"
#include "Source/Util/RandomDevice.h"
#include <memory>
#include <vector>


namespace app
{
	/**
	 * @brief ナイアガラ風のモジュール式2Dエフェクトシステム
	 */
	class ParticleEmitter
	{
	private:
		/** パーティクルプール */
		std::vector<Particle> m_particles;

		/** モジュール */
		SpawnModule m_spawnModule;
		std::vector<ParticleModule*> m_initModules;
		std::vector<ParticleModule*> m_updateModules;

		/** メモリ管理 */
		std::vector<std::unique_ptr<ParticleModule>> m_ownedModules;

		/** エミッター状態 */
		bool m_isPlaying;
		float m_emitterAge;
		float m_emitterDuration;  // 0以下で無限
		int m_aliveCount;

		/** エミッター座標 */
		Vector3 m_emitterPosition;


	public:
		ParticleEmitter()
			: m_isPlaying(false)
			, m_emitterAge(0.0f)
			, m_emitterDuration(0.0f)
			, m_aliveCount(0)
			, m_emitterPosition(Vector3::Zero)
		{
			m_particles.resize(m_spawnModule.GetMaxParticles());
		}

		~ParticleEmitter()
		{
			m_initModules.clear();
			m_updateModules.clear();
			m_ownedModules.clear();
		}

		/**
		 * @brief 全モジュールをクリアする（ホットリロード用）SpawnModuleはデフォルトにリセットされる
		 */
		void ClearModules()
		{
			m_initModules.clear();
			m_updateModules.clear();
			m_ownedModules.clear();
			m_spawnModule = SpawnModule();
		}

		// ----- モジュール追加 -----

		/**
		 * @brief SpawnModuleを取得
		 * @return SpawnModuleの参照
		 */
		SpawnModule& GetSpawnModule() { return m_spawnModule; }

		/**
		 * @brief モジュールを追加
		 * @tparam T モジュールの型
		 * @return 追加されたモジュールのポインタ
		 */
		template <typename T>
		T* AddModule()
		{
			std::unique_ptr<T> mod(new T());
			T* ptr = mod.get();

			EnParticleModuleType type = ptr->GetType();
			switch (type)
			{
			case EnParticleModuleType::InitLifeTime:
			case EnParticleModuleType::InitPosition:
			case EnParticleModuleType::InitVelocity:
			case EnParticleModuleType::InitScale:
			case EnParticleModuleType::InitRotation:
			case EnParticleModuleType::InitColor:
				m_initModules.push_back(ptr);
				break;

			case EnParticleModuleType::ScaleOverLife:
			case EnParticleModuleType::RotationOverLife:
			case EnParticleModuleType::AlphaOverLife:
			case EnParticleModuleType::SpeedOverLife:
			case EnParticleModuleType::ConvergenceOverLife:
				m_initModules.push_back(ptr);
				m_updateModules.push_back(ptr);
				break;

			case EnParticleModuleType::Acceleration:
			case EnParticleModuleType::VelocityDamping:
				m_updateModules.push_back(ptr);
				break;

			default:
				break;
			}

			m_ownedModules.push_back(std::move(mod));
			return ptr;
		}

		// ----- 制御 -----

		/**
		 * @brief エフェクトを再生
		 */
		void Play()
		{
			m_isPlaying = true;
			m_emitterAge = 0.0f;
			m_spawnModule.Reset();
		}

		/**
		 * @brief エフェクトを停止
		 */
		void Stop() { m_isPlaying = false; }

		/**
		 * @brief エフェクトをリセット
		 */
		void Reset()
		{
			m_isPlaying = false;
			m_emitterAge = 0.0f;
			m_aliveCount = 0;
			m_spawnModule.Reset();
			for (size_t i = 0; i < m_particles.size(); ++i) {
				m_particles[i].isAlive = false;
			}
		}

		/**
		 * @brief 位置を設定
		 * @param pos 位置
		 */
		void SetPosition(const Vector3& pos) { m_emitterPosition = pos; }
		/**
		 * @brief 位置を取得
		 * @return 位置
		 */
		const Vector3& GetPosition() const { return m_emitterPosition; }
		/**
		 * @brief 持続時間を設定
		 * @param duration 持続時間
		 */
		void SetDuration(float duration) { m_emitterDuration = duration; }
		/**
		 * @brief 再生中かを取得
		 * @return 再生中ならtrue
		 */
		bool IsPlaying() const { return m_isPlaying; }
		/**
		 * @brief 生存しているパーティクルの数を取得
		 * @return 生存しているパーティクルの数
		 */
		int GetAliveCount() const { return m_aliveCount; }

		// ----- 更新 -----

		/**
		 * @brief エフェクトを更新
		 * @param deltaTime 時間差
		 */
		void Update(float deltaTime)
		{
			if (!m_isPlaying) return;

			m_emitterAge += deltaTime;
			bool canSpawn = true;
			if (m_emitterDuration > 0.0f && m_emitterAge >= m_emitterDuration) {
				canSpawn = false;
				if (m_aliveCount == 0) {
					m_isPlaying = false;
					return;
				}
			}

			// 生成
			if (canSpawn) {
				int count = m_spawnModule.CalcSpawnCount(deltaTime, m_aliveCount);
				for (int i = 0; i < count; ++i) {
					SpawnParticle();
				}
			}

			// 更新
			m_aliveCount = 0;
			for (size_t i = 0; i < m_particles.size(); ++i) {
				Particle& p = m_particles[i];
				if (!p.isAlive) continue;

				p.age += deltaTime;
				if (p.age >= p.lifeTime) {
					p.isAlive = false;
					continue;
				}

				for (size_t m = 0; m < m_updateModules.size(); ++m) {
					if (m_updateModules[m]->IsEnabled()) {
						m_updateModules[m]->OnParticleUpdate(p, deltaTime, m_emitterPosition);
					}
				}

				if (!p.hasRotationCurve) {
					p.rotationAngle += p.angularVelocity * deltaTime;
				}

				p.position.x += p.velocity.x * deltaTime;
				p.position.y += p.velocity.y * deltaTime;
				p.position.z += p.velocity.z * deltaTime;

				m_aliveCount++;
			}
		}

		/**
		 * @brief パーティクルのリストを取得
		 * @return パーティクルのリスト
		 */
		const std::vector<Particle>& GetParticles() const { return m_particles; }

		/**
		 * @brief パーティクルプールのサイズを変更
		 */
		void ResizePool()
		{
			m_particles.resize(m_spawnModule.GetMaxParticles());
		}

		/**
		 * @brief エミッターと発生済みのパーティクル全体を一緒に平行移動させる
		 * @param offset 平行移動させるオフセット
		 */
		void AddPositionOffset(const Vector3& offset)
		{
			m_emitterPosition.x += offset.x;
			m_emitterPosition.y += offset.y;
			m_emitterPosition.z += offset.z;

			for (size_t i = 0; i < m_particles.size(); ++i) {
				if (m_particles[i].isAlive) {
					m_particles[i].position.x += offset.x;
					m_particles[i].position.y += offset.y;
					m_particles[i].position.z += offset.z;
				}
			}
		}


	private:
		/**
		 * @brief パーティクルを生成
		 * @return 生成されたパーティクルの参照
		 */
		void SpawnParticle()
		{
			for (size_t i = 0; i < m_particles.size(); ++i) {
				if (!m_particles[i].isAlive) {
					Particle& p = m_particles[i];
					p = Particle();
					p.isAlive = true;
					p.position = Vector3::Zero;

					for (size_t m = 0; m < m_initModules.size(); ++m) {
						if (m_initModules[m]->IsEnabled()) {
							m_initModules[m]->OnParticleSpawn(p);
						}
					}

					// 初期化が終わった時点の速度を記憶しておく。
					p.initialVelocity = p.velocity;

					// InitPositionはローカル座標 → エミッター座標を加算
					p.position.x += m_emitterPosition.x;
					p.position.y += m_emitterPosition.y;
					p.position.z += m_emitterPosition.z;

					return;
				}
			}
		}
	};
}
