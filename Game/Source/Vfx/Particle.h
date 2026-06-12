/**
 * @file Particle.h
 * @brief 個別パーティクルのデータ
 * @author 忽那
 */
#pragma once
#include "Source/Util/Curve.h"


namespace app
{
	/**
	 * @brief 1つのパーティクルが持つランタイムデータ
	 */
	struct Particle
	{
		/** 生存フラグ */
		bool isAlive;
		
		/** 寿命 */
		float lifeTime;
		float age;
		
		/** トランスフォーム系 */
		Vector3 position;
		Vector3 velocity;
		Vector3 acceleration;
		Vector3 scaleValue;
		util::FloatCurve scaleCurveX;
		util::FloatCurve scaleCurveY;
		bool hasScaleCurve;

		float rotationAngle;			// Z軸回転角度(degree)
		float angularVelocity;			// 回転速度(degree/sec)
		util::FloatCurve rotationCurve;
		bool hasRotationCurve;
		
		/** カラー */
		Vector4 color;
		util::FloatCurve alphaCurve;
		bool hasAlphaCurve;
		
		/** 速度カーブ */
		util::FloatCurve speedCurveX;
		util::FloatCurve speedCurveY;
		bool hasSpeedCurve;


		/**
		 * @brief 構造体を初期化するコンストラクタ
		 */
		Particle()
			: isAlive(false)
			, lifeTime(0.0f)
			, age(0.0f)
			, position(Vector3::Zero)
			, velocity(Vector3::Zero)
			, acceleration(Vector3::Zero)
			, scaleValue(Vector3::One)
			, hasScaleCurve(false)
			, rotationAngle(0.0f)
			, angularVelocity(0.0f)
			, hasRotationCurve(false)
			, color(Vector4::One)
			, hasAlphaCurve(false)
			, hasSpeedCurve(false)
		{}


		/**
		 * @brief 正規化した寿命割合を取得する(0.0f ~ 1.0f)
		 * @return 寿命の割合(0.0fから1.0fまでの寿命が尽きるまで)
		 */
		float GetNormalizedAge() const
		{
			if (lifeTime <= 0.0f) return 1.0f;
			return util::clamp<float>(age / lifeTime, 0.0f, 1.0f);
		}
	};
}
