/**
 * @file ParticleModule.h
 * @brief パーティクルモジュール群(ナイアガラ風のモジュール式パーティクルシステム)
 * @author 忽那
 */
#pragma once
#include "Particle.h"
#include "ParticleValueProvider.h"
#include <vector>
#include <random>
#include <string>


namespace
{
	/** 収束の強さ */
	constexpr float PULL_STRENGTH = 10.0f;
}


namespace app
{
	/**
	 * @brief パーティクルモジュールの種類
	 */
	enum class EnParticleModuleType
	{
		Spawn,					// 生成
		InitLifeTime,			// 寿命初期化
		InitPosition,			// 初期座標
		InitVelocity,			// 初期速度
		InitScale,				// 初期スケール
		InitRotation,			// 初期回転
		InitColor,				// 初期カラー
		ScaleOverLife,			// 寿命に応じたスケール変化
		RotationOverLife,		// 寿命に応じた回転変化
		AlphaOverLife,			// 寿命に応じた透明度変化
		SpeedOverLife,			// 寿命に応じた速度変化
		ConvergenceOverLife,	// 寿命に応じた収束
		Acceleration,			// 加速度(重力等)
		VelocityDamping,		// 速度減衰
	};


	/**
	 * @brief パーティクルモジュールの基底クラス
	 */
	class ParticleModule
	{
	protected:
		/** パーティクルモジュールタイプを所持 */
		EnParticleModuleType m_moduleType;
		/** enum class の有効/無効フラグ */
		bool m_enabled;


	public:
		ParticleModule(EnParticleModuleType type)
			: m_moduleType(type)
			, m_enabled(true)
		{}

		virtual ~ParticleModule() {}


	public:
		/**
		 * @brief モジュールの種類を取得
		 * @return モージュールタイプ
		 */
		EnParticleModuleType GetType() const { return m_moduleType; }

		/**
		 * @brief モジュールの有効/無効を取得
		 * @return 有効ならture、無効ならfalse
		 */
		bool IsEnabled() const { return m_enabled; }

		/**
		 * @brief モジュールの有効/無効を設定
		 * @param enabled 有効ならtrue、無効ならfalse
		 */
		void SetEnabled(bool enabled) { m_enabled = enabled; }

		/**
		 * @brief パーティクル生成時に呼ばれる
		 * @param p 生成されたパーティクル
		 * @param rng 乱数生成器(シード値)
		 */
		virtual void OnParticleSpawn(Particle& p, std::mt19937& rng) {}

		/**
		 * @brief マイフレーム呼ばれる(パーティクルが生存している限り毎フレーム呼ばれる)
		 * @param p 更新するパーティクル
		 * @param deltaTime 前フレームからの経過時間(秒)
		 * @param emitterPos エミッターの位置
		 */
		virtual void OnParticleUpdate(Particle& p, float deltaTime,const Vector3& emitterPos) {}
	};




	/***********************************************/


   /**
	* @brief パーティクルの生成制御
	*/
	class SpawnModule : public ParticleModule
	{
	private:
		float m_spawnRate;		// 1秒あたりの生成数
		int m_burstCount;		// 一括生成数
		float m_burstInterval;  // バースト間隔(秒)
		int m_maxParticles;		// 最大パーティクル数
		bool m_isLooping;		// ループするか

		// ランタイム。
		float m_spawnAccumulator;
		float m_burstTimer;
		int m_burstFired;


	public:
		SpawnModule()
			: ParticleModule(EnParticleModuleType::Spawn)
			, m_spawnRate(10.0f)
			, m_burstCount(0)
			, m_burstInterval(0.0f)
			, m_maxParticles(100)
			, m_isLooping(true)
			, m_spawnAccumulator(0.0f)
			, m_burstTimer(0.0f)
			, m_burstFired(0)
		{}


	public:
		/** セッター群 */
		void SetSpawnRate(float rate) { m_spawnRate = rate; }
		void SetBurst(int count, float interval = 0.0f) { m_burstCount = count; m_burstInterval = interval; }
		void SetMaxParticles(int max) { m_maxParticles = max; }
		void SetLooping(bool loop) { m_isLooping = loop; }

		/** ゲッター群 */
		float GetSpawnRate() const { return m_spawnRate; }
		int GetBurstCount() const { return m_burstCount; }
		float GetBurstInterval() const { return m_burstInterval; }
		int GetMaxParticles() const { return m_maxParticles; }
		bool IsLooping() const { return m_isLooping; }


	public:
		/**
		 * @brief 生成するパーティクル数を計算
		 * @param deltaTime 時間差
		 * @param currentAliveCount 現在生存しているパーティクルの数
		 * @return 生成するパーティクル数
		 */
		int CalcSpawnCount(float deltaTime, int currentAliveCount)
		{
			int spawnCount = 0;

			// レート生成。
			if (m_spawnRate > 0.0f)
			{
				m_spawnAccumulator += m_spawnRate * deltaTime;
				int rateCount = static_cast<int>(m_spawnAccumulator);
				m_spawnAccumulator -= rateCount;
				spawnCount += rateCount;
			}

			// バースト生成。
			if (m_burstCount > 0)
			{
				m_burstTimer += deltaTime;
				if (m_burstFired == 0 || (m_burstInterval > 0.0f && m_burstTimer >= m_burstInterval)) {
					spawnCount += m_burstCount;
					m_burstTimer = 0.0f;
					if (!m_isLooping)
					{
						m_burstFired++;
					}
				}
			}


			// 最大数制限。
			int remaing = m_maxParticles - currentAliveCount;
			if (remaing < 0) remaing = 0;
			if (spawnCount > remaing) spawnCount = remaing;

			return spawnCount;
		}

		/**
		 * @brief スポーンモジュールの状態をリセット
		 */
		void Reset()
		{
			m_spawnAccumulator = 0.0f;
			m_burstTimer = 0.0f;
			m_burstFired = 0;
		}
	};




	/***********************************************/
	

	/**
	 * @brief 寿命初期化
	 */
	class InitLifeTimeModule : public ParticleModule
	{
	private:
		FloatValueProvider m_lifeTime;


	public:
		InitLifeTimeModule()
			: ParticleModule(EnParticleModuleType::InitLifeTime)
		{
			m_lifeTime.SetFixed(1.0f);
		}

		/**
		 * @brief 寿命時間を取得
		 * @return 寿命時間の値プロバイダ
		 */
		FloatValueProvider& LifeTime() { return m_lifeTime; }

		/**
		 * @brief パーティクル生成時に寿命を初期化
		 * @param p 生成されたパーティクル
		 * @param rng 乱数生成器(シード値)
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			p.lifeTime = m_lifeTime.ResolveInitial(rng);
			p.age = 0.0f;
		}
	};


	/***********************************************/

	/**
	 * @brief 初期座標
	 */
	class InitPositionModule : public ParticleModule
	{
	private:
		Vector3ValueProvider m_position;


	public:
		InitPositionModule()
			: ParticleModule(EnParticleModuleType::InitPosition)
		{}

		/**
		 * @brief 初期座標の値プロバイダを取得
		 * @return 初期座標の値プロバイダ
		 */
		Vector3ValueProvider& Position() { return m_position; }

		/**
		 * @brief パーティクル生成時に座標を初期化
		 * @param p 生成されたパーティクル
		 * @param rng 乱数生成器(シード値)
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			p.position = m_position.ResolveInitial(rng);
		}
	};




	/***********************************************/


	/**
	 * @brief 初期速度
	 */
	class InitVelocityModule : public ParticleModule
	{
	private:
		Vector3ValueProvider m_velocity;


	public:
		InitVelocityModule()
			: ParticleModule(EnParticleModuleType::InitVelocity)
		{}

		/**
		 * @brief 初期速度の値プロバイダを取得
		 * @return 初期速度の値プロバイダ
		 */
		Vector3ValueProvider& Velocity() { return m_velocity; }

		/**
		 * @brief パーティクル生成時に速度を初期化
		 * @param p 生成されたパーティクル
		 * @param rng 乱数生成器(シード値)
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			p.velocity = m_velocity.ResolveInitial(rng);
		}
	};




	/***********************************************/


	/**
	 * @brief 初期スケール
	 */
	class InitScaleModule : public ParticleModule
	{
	private:
		Vector3ValueProvider m_scale;


	public:
		InitScaleModule()
			: ParticleModule(EnParticleModuleType::InitScale)
		{}

		/**
		 * @brief 初期スケールの値プロバイダを取得
		 * @return 初期スケールの値プロバイダ
		 */
		Vector3ValueProvider& Scale() { return m_scale; }

		/**
		 * @brief パーティクル生成時にスケールを初期化
		 * @param p 生成されたパーティクル
		 * @param rng 乱数生成器(シード値)
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			p.scaleValue = m_scale.ResolveInitial(rng);
		}
	};




	/***********************************************/


	/**
	 * @brief 初期回転(z軸)
	 */
	class InitRotationModule : public ParticleModule
	{
	private:
		FloatValueProvider m_angle;				// 初期角度
		FloatValueProvider m_angularVelocity;	// 回転速度


	public:
		InitRotationModule()
			: ParticleModule(EnParticleModuleType::InitRotation)
		{
			m_angle.SetFixed(0.0f);
			m_angularVelocity.SetFixed(0.0f);
		}

		/**
		 * @brief 初期角度の値プロバイダを取得
		 * @return 初期角度の値プロバイダ
		 */
		FloatValueProvider& Angle() { return m_angle; }
		/**
		 * @brief 回転速度の値プロバイダを取得
		 * @return 回転速度の値プロバイダ
		 */
		FloatValueProvider& AngularVelocity() { return m_angularVelocity; }

		/**
		 * @brief パーティクル生成時に回転を初期化
		 * @param p 生成されたパーティクル
		 * @param rng 乱数生成器(シード値)
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			p.rotationAngle = m_angle.ResolveInitial(rng);
			p.angularVelocity = m_angularVelocity.ResolveInitial(rng);
		}
	};




	/***********************************************/


	/**
	 * @brief 初期カラー
	 */
	class InitColorModule : public ParticleModule
	{
	private:
		FloatValueProvider m_r;
		FloatValueProvider m_g;
		FloatValueProvider m_b;
		FloatValueProvider m_a;


	public:
		InitColorModule()
			: ParticleModule(EnParticleModuleType::InitColor)
		{
			m_r.SetFixed(1.0f);
			m_g.SetFixed(1.0f);
			m_b.SetFixed(1.0f);
			m_a.SetFixed(1.0f);
		}

		/**
		 * @brief RGBAの値プロバイダを取得
		 * @return RGBAの値プロバイダ
		 */
		FloatValueProvider& R() { return m_r; }
		FloatValueProvider& G() { return m_g; }
		FloatValueProvider& B() { return m_b; }
		FloatValueProvider& A() { return m_a; }

		/**
		 * @brief パーティクル生成時にカラーを初期化
		 * @param p 生成されたパーティクル
		 * @param rng 乱数生成器(シード値)
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			p.color.x = m_r.ResolveInitial(rng);
			p.color.y = m_g.ResolveInitial(rng);
			p.color.z = m_b.ResolveInitial(rng);
			p.color.w = m_a.ResolveInitial(rng);
		}
	};




	/***********************************************/


	/**
	 * @brief 寿命に応じたスケール変化
	 */
	class ScaleOverLifeModule : public ParticleModule
	{
	private:
		FloatValueProvider m_startScaleX;
		FloatValueProvider m_endScaleX;
		FloatValueProvider m_startScaleY;
		FloatValueProvider m_endScaleY;
		util::EasingType m_easingType;
		bool m_uniform;


	public:
		ScaleOverLifeModule()
			: ParticleModule(EnParticleModuleType::ScaleOverLife)
			, m_easingType(util::EasingType::Linear)
			, m_uniform(false)
		{
			m_startScaleX.SetFixed(1.0f);
			m_endScaleX.SetFixed(0.0f);
			m_startScaleY.SetFixed(1.0f);
			m_endScaleY.SetFixed(0.0f);
		}


		/**
		 * @brief 開始 / 終了スケールの値プロバイダを取得
		 * @return 開始 / 終了スケールの値プロバイダ
		 */
		FloatValueProvider& StartScaleX() { return m_startScaleX; }
		FloatValueProvider& EndScaleX() { return m_endScaleX; }
		FloatValueProvider& StartScaleY() { return m_startScaleY; }
		FloatValueProvider& EndScaleY() { return m_endScaleY; }

		/**
		 * @brief イージングタイプを取得
		 * @return イージングタイプ
		 */
		void SetEasing(util::EasingType easing) { m_easingType = easing; }
		/**
		 * @brief XとYを共通値にするかのフラグを設定
		 * @param uniform trueならXとYを同じ値にする、falseなら個別に値を生成する
		 */
		void SetUniform(bool uniform) { m_uniform = uniform; }
		/**
		 * @brief XとYを共通値にするかのフラグを取得
		 * @return uniformの取得
		 */
		bool IsUniform() const { return m_uniform; }

		/**
		 * @brief uniformフラグをtrueにして、開始スケールと終了スケールのX値をY値にも適用する
		 * @details SetUniformScale()で一括設定ができる
		 * @param start 開始スケールの値プロバイダ(X値がY値にも適用される)
		 * @param end 終了スケールの値プロバイダ(X値がY値にも適用される)
		 */
		void SetUniformScale(const FloatValueProvider& start, const FloatValueProvider& end)
		{
			m_uniform = true;
			m_startScaleX = start;
			m_endScaleX = end;
		}

		/**
		 * @brief パーティクルのスケールを寿命に応じて変化させる
		 * @param p 更新するパーティクル
		 * @param rng 乱数生成器
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			float sx, ex, sy, ey;

			if (m_uniform)
			{
				sx = m_startScaleX.ResolveInitial(rng);
				ex = m_endScaleX.ResolveInitial(rng);
				sy = sx;
				ey = ex;
			}
			else
			{
				sx = m_startScaleX.ResolveInitial(rng);
				ex = m_endScaleX.ResolveInitial(rng);;
				sy = m_startScaleY.ResolveInitial(rng);
				ey = m_endScaleY.ResolveInitial(rng);
			}

			/**
			 * @brief パーティクルのスケールカーブXを初期化
			 */
			p.scaleCurveX.Initialize(sx, ex, p.lifeTime, m_easingType, util::LoopMode::Once);
			/**
			 * @brief パーティクルのスケールカーブYを初期化
			 */
			p.scaleCurveY.Initialize(sy, ey, p.lifeTime, m_easingType, util::LoopMode::Once);
			p.scaleCurveX.Play();
			p.scaleCurveY.Play();
			p.hasScaleCurve = true;

			// 初期スケールにも適用しておく。
			p.scaleValue.x = sx;
			p.scaleValue.y = sy;
		}

		/**
		 * @brief パーティクルの更新
		 * @param p 更新するパーティクル
		 * @param deltaTime 前フレームからの経過時間(秒)
		 * @param emitterPos エミッターの位置
		 */
		void OnParticleUpdate(Particle& p, float deltaTime, const Vector3& emitterPos) override
		{
			if (!p.hasScaleCurve) return;

			p.scaleCurveX.Update(deltaTime);
			p.scaleCurveY.Update(deltaTime);
			p.scaleValue.x = p.scaleCurveX.GetCurrentValue();
			p.scaleValue.y = p.scaleCurveY.GetCurrentValue();
		}
	};




	/***********************************************/


	/**
	 * @brief 寿命に応じた回転変化
	 */
	class RotationOverLifeModule : public ParticleModule
	{
	private:
		FloatValueProvider m_startAngle;
		FloatValueProvider m_endAngle;
		util::EasingType m_easingType;


	public:
		RotationOverLifeModule()
			: ParticleModule(EnParticleModuleType::RotationOverLife)
			, m_easingType(util::EasingType::Linear)
		{
			m_startAngle.SetFixed(0.0f);
			m_endAngle.SetFixed(360.0f);
		}

		/**
		 * @brief 開始 / 終了回転の値プロバイダを取得
		 * @return 開始 / 終了回転の値プロバイダ
		 */
		FloatValueProvider& StartAngle() { return m_startAngle; }
		FloatValueProvider& EndAngle() { return m_endAngle; }
		/**
		 * @brief イージングタイプを取得
		 * @return イージングタイプ
		 */
		void SetEasing(util::EasingType type) { m_easingType = type; }

		/**
		 * @brief パーティクルの回転を寿命に応じて変化させる
		 * @param p 更新するパーティクル
		 * @param rng 乱数生成器
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			float s = m_startAngle.ResolveInitial(rng);
			float e = m_endAngle.ResolveInitial(rng);
			p.rotationCurve.Initialize(s, e, p.lifeTime, m_easingType, util::LoopMode::Once);
			p.rotationCurve.Play();
			p.hasRotationCurve = true;
			p.rotationAngle = s;
		}

		/**
		 * @brief パーティクルの更新
		 * @param p 更新するパーティクル
		 * @param deltaTime 前フレームからの経過時間(秒)
		 * @param emitterPos エミッターの位置
		 */
		void OnParticleUpdate(Particle& p, float deltaTime, const Vector3& emitterPos) override
		{
			if (!p.hasRotationCurve) return;
			p.rotationCurve.Update(deltaTime);
			p.rotationAngle = p.rotationCurve.GetCurrentValue();
		}
	};




	/***********************************************/


	/**
	 * @brief 寿命に応じた透明度変化
	 */
	class AlphaOverLifeModule : public ParticleModule
	{
	private:
		FloatValueProvider m_startAlpha;
		FloatValueProvider m_endAlpha;
		util::EasingType m_easingType;


	public:
		AlphaOverLifeModule()
			: ParticleModule(EnParticleModuleType::AlphaOverLife)
			, m_easingType(util::EasingType::Linear)
		{
			m_startAlpha.SetFixed(1.0f);
			m_endAlpha.SetFixed(0.0f);
		}

		/**
		 * @brief 開始 / 終了透明度の値プロバイダを取得
		 * @return 開始 / 終了透明度の値プロバイダ
		 */
		FloatValueProvider& StartAlpha() { return m_startAlpha; }
		FloatValueProvider& EndAlpha() { return m_endAlpha; }

		/**
		 * @brief イージングタイプを取得
		 * @return イージングタイプ
		 */
		void SetEasing(util::EasingType type) { m_easingType = type; }

		/**
		 * @brief パーティクルの透明度を寿命に応じて変化させる
		 * @param p 更新するパーティクル
		 * @param rng 乱数生成器
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			float s = m_startAlpha.ResolveInitial(rng);
			float e = m_endAlpha.ResolveInitial(rng);
			p.alphaCurve.Initialize(s, e, p.lifeTime, m_easingType, util::LoopMode::Once);
			p.alphaCurve.Play();
			p.hasAlphaCurve = true;
			p.color.w = s;
		}

		/**
		 * @brief パーティクルの更新
		 * @param p 更新するパーティクル
		 * @param deltaTime 前フレームからの経過時間(秒)
		 * @param emitterPos エミッターの位置
		 */
		void OnParticleUpdate(Particle& p, float deltaTime, const Vector3& emitterPos) override
		{
			if (!p.hasAlphaCurve)return;

			p.alphaCurve.Update(deltaTime);
			p.color.w = p.alphaCurve.GetCurrentValue();
		}
	};




	/***********************************************/


	/**
	 * @brief 寿命に応じた速度変化
	 */
	class SpeedOverLifeModule : public ParticleModule
	{
	private:
		FloatValueProvider m_startMultiplier;
		FloatValueProvider m_endMultiplier;
		util::EasingType m_easingType;


	public:
		SpeedOverLifeModule()
			: ParticleModule(EnParticleModuleType::SpeedOverLife)
			, m_easingType(util::EasingType::Linear)
		{
			m_startMultiplier.SetFixed(1.0f);
			m_endMultiplier.SetFixed(0.0f);
		}

		/**
		 * @brief 開始 / 終了速度の倍率の値プロバイダを取得
		 * @return 開始 / 終了速度の倍率の値プロバイダ
		 */
		FloatValueProvider& StartMultiplier() { return m_startMultiplier; }
		FloatValueProvider& EndMultiplier() { return m_endMultiplier; }

		/**
		 * @brief イージングタイプを取得
		 * @return イージングタイプ
		 */
		void SetEasing(util::EasingType type) { m_easingType = type; }

		/**
		 * @brief パーティクルの速度を寿命に応じて変化させる
		 * @param p 更新するパーティクル
		 * @param rng 乱数生成器
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			float s = m_startMultiplier.ResolveInitial(rng);
			float e = m_endMultiplier.ResolveInitial(rng);
			p.speedCurveX.Initialize(s, e, p.lifeTime, m_easingType, util::LoopMode::Once);
			p.speedCurveY.Initialize(s, e, p.lifeTime, m_easingType, util::LoopMode::Once);
			p.speedCurveX.Play();
			p.speedCurveY.Play();
			p.hasSpeedCurve = true;


			// 初期速度を基準にカーブで計算したマルチプライヤをかけて現在の速度にする。
			p.velocity.x = p.initialVelocity.x * p.speedCurveX.GetCurrentValue();
			p.velocity.y = p.initialVelocity.y * p.speedCurveY.GetCurrentValue();
		}

		/**
		 * @brief パーティクルの更新
		 * @param p 更新するパーティクル
		 * @param deltaTime 前フレームからの経過時間(秒)
		 * @param emitterPos エミッターの位置
		 */
		void OnParticleUpdate(Particle& p, float deltaTime, const Vector3& emitterPos) override
		{
			if (!p.hasSpeedCurve)return;

			p.speedCurveX.Update(deltaTime);
			p.speedCurveY.Update(deltaTime);
		}
	};




	/***********************************************/


	/**
	 * @brief 加速度モジュール(重力等)
	 */
	class AccelerationModule : public ParticleModule
	{
	private:
		Vector3 m_acceleration;


	public:
		AccelerationModule()
			: ParticleModule(EnParticleModuleType::Acceleration)
			, m_acceleration(Vector3::Zero)
		{}

		/**
		 * @brief 加速度の設定
		 * @param accel 加速度ベクトル
		 */
		void SetAcceleration(const Vector3& accel) { m_acceleration = accel; }
		/**
		 * @brief 加速度の取得
		 * @return 加速度ベクトル
		 */
		const Vector3& GetAcceleration() const { return m_acceleration; }

		/**
		 * @brief パーティクルの更新
		 * @param p 更新するパーティクル
		 * @param deltaTime 前フレームからの経過時間(秒)
		 * @param emitterPos エミッターの位置
		 */
		void OnParticleUpdate(Particle& p, float deltaTime, const Vector3& emitterPos) override
		{
			p.velocity.x += m_acceleration.x * deltaTime;
			p.velocity.y += m_acceleration.y * deltaTime;
			p.velocity.z += m_acceleration.z * deltaTime;
		}
	};




	/***********************************************/


	/**
	 * @brief 速度減衰モジュール
	 */
	class VelocityDampingModule : public ParticleModule
	{
	private:
		/** 0.0~1.0(1.0で即停止、0.0で減衰なし) */
		float m_damping;


	public:
		VelocityDampingModule()
			: ParticleModule(EnParticleModuleType::VelocityDamping)
			, m_damping(0.0f)
		{}

		/**
		 * @brief 速度減衰を設定
		 * @param damping 0.0~1.0の値
		 */
		void SetDamping(float d) { m_damping = util::clamp<float>(d, 0.0f, 1.0f); }
		/**
		 * @brief 速度減衰を取得
		 * @return 速度減衰の値
		 */
		float GetDamping() const { return m_damping; }

		/**
		 * @brief パーティクルの更新
		 * @param p 更新するパーティクル
		 * @param deltaTime 前フレームからの経過時間(秒)
		 * @param emitterPos エミッターの位置
		 */
		void OnParticleUpdate(Particle& p, float deltaTime, const Vector3& emitterPos) override
		{
			float factor = 1.0f - m_damping * deltaTime;
			if (factor < 0.0f) factor = 0.0f;
			p.velocity.x *= factor;
			p.velocity.y *= factor;
			p.velocity.z *= factor;
		}
	};




	/***********************************************/


	/**
	 * @brief 寿命に応じた収束
	 */
	class ConvergenceOverLifeModule : public ParticleModule
	{
	private:
		FloatValueProvider m_startRatio;		// 収束開始時の割合(0.0で収束なし、1.0で最初から収束)
		FloatValueProvider m_endRatio;			// 収束終了時の割合(0.0で収束なし、1.0で完全に収束)
		util::EasingType  m_easingType;


	public:
		ConvergenceOverLifeModule()
			: ParticleModule(EnParticleModuleType::ConvergenceOverLife)
		{
			m_startRatio.SetFixed(0.0f);
			m_endRatio.SetFixed(1.0f);
			m_easingType = util::EasingType::Linear;
		}

		virtual ~ConvergenceOverLifeModule() {}

		/**
		 * @brief 開始 / 終了収束の割合の値プロバイダを取得
		 * @return 開始 / 終了収束の割合の値プロバイダ
		 */
		FloatValueProvider& StartRatio() { return m_startRatio; }
		FloatValueProvider& EndRatio() { return m_endRatio; }

		/**
		 * @brief イージングタイプの設定
		 * @param type イージングタイプ
		 */
		void SetEasing(util::EasingType type) { m_easingType = type; }

		/**
		 * @brief パーティクルの収束を寿命に応じて変化させる
		 * @param p 更新するパーティクル
		 * @param rng 乱数生成器
		 */
		void OnParticleSpawn(Particle& p, std::mt19937& rng) override
		{
			float s = m_startRatio.ResolveInitial(rng);
			float e = m_endRatio.ResolveInitial(rng);
			// 初期化。
			p.convergenceCurve.Initialize(s, e, p.lifeTime, m_easingType, util::LoopMode::Once);
			p.convergenceCurve.Play();
			p.hasConvergenceCurve = true;
		}

		/**
		 * @brief パーティクルの更新
		 * @param p 更新するパーティクル
		 * @param deltaTime 前フレームからの経過時間(秒)
		 * @param emitterPos エミッターの位置
		 */
		void OnParticleUpdate(Particle& p, float deltaTime, const Vector3& emitterPos) override
		{
			if (!p.hasConvergenceCurve) return;

			p.convergenceCurve.Update(deltaTime);

			float ratio = p.convergenceCurve.GetCurrentValue();
			if (ratio > 0.0f)
			{
				// 収束力の計算。
				float pullStrength = ratio * deltaTime * PULL_STRENGTH;

				// 引っ張る力が強くなり過ぎないように制限。
				if (pullStrength > 1.0f) pullStrength = 1.0f;

				// 現在位置 += (エミッター位置 - 現在位置) * 収束力。
				p.position.x += (emitterPos.x - p.position.x) * pullStrength;
				p.position.y += (emitterPos.y - p.position.y) * pullStrength;
				p.position.z += (emitterPos.z - p.position.z) * pullStrength;
			}
		}
	};
};