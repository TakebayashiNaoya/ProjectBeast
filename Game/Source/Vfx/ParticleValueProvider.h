/**
 * @file ParticleValueProvider.h
 * @brief パーティクルパラメーターの値を提供するプロバイダ
 */
#pragma once
#include "Source/Util/Curve.h"
#include "Source/Util/RandomDevice.h"
#include <string>


namespace app
{
	/**
	 * @brief 値供給モード
	 * @detail
	 * Fixed: 固定値
	 * Random: 最小~最大の範囲でランダムな値
	 * Curve: 開始値~終了値をイージングで補間
	 * RandomCurve: ランダムで決定した開始値~終了値をイージングで補間
	 */
	enum class EnValueMode
	{
		Fixed,
		Random,
		Curve,
		RandomCurve
	};


	/**
	 * @brief float用の値プロバイダ
	 */
	class FloatValueProvider
	{
	private:
		EnValueMode m_valueMode;
		float m_fixedValue;
		float m_minValue;
		float m_maxValue;
		float m_startValue;
		float m_endValue;
		util::EasingType m_easingType;


	public:
		FloatValueProvider()
			: m_valueMode(EnValueMode::Fixed)
			, m_fixedValue(0.0f)
			, m_minValue(0.0f)
			, m_maxValue(0.0f)
			, m_startValue(0.0f)
			, m_endValue(0.0f)
			, m_easingType(util::EasingType::Linear)
		{}


		/**
		 * @brief 固定値を設定
		 * @param value 固定値
		 */
		void SetFixed(float value)
		{
			m_valueMode = EnValueMode::Fixed;
			m_fixedValue = value;
		}


		/**
		 * @brief ランダム値の範囲を設定
		 * @param min 最小値
		 * @param max 最大値
		 */
		void SetRandom(float minValue, float maxValue)
		{
			m_valueMode = EnValueMode::Random;
			m_minValue = minValue;
			m_maxValue = maxValue;
		}


		/**
		 * @brief カーブの開始値、終了値、イージングタイプを設定
		 * @param start 開始値
		 * @param end 終了値
		 * @param easing イージングタイプ(デフォルトは線形補間)
		 */
		void SetCurve(float startValue, float endValue, util::EasingType easingType = util::EasingType::Linear)
		{
			m_valueMode = EnValueMode::Curve;
			m_startValue = startValue;
			m_endValue = endValue;
			m_easingType = easingType;
		}

		/**
		 * @brief カーブの最小開始値、最大開始値、最小終了値、最大終了値、イージングタイプを設定
		 * @param minStart 最小開始値
		 * @param maxStart 最大開始値
		 * @param minEnd 最小終了値
		 * @param maxEnd 最大終了値
		 * @param easing イージングタイプ(デフォルトは線形補間)
		 */
		void SetRandomCurve(float minStart, float maxStart, float minEnd, float maxEnd, util::EasingType easingType = util::EasingType::Linear)
		{
			m_valueMode = EnValueMode::RandomCurve;
			m_minValue = minStart;
			m_maxValue = maxStart;
			m_startValue = minStart;
			m_endValue = maxEnd;
			m_easingType = easingType;
		}


	public:
		/**
		 * @brief 値供給モードを取得
		 * @return 値供給モード
		 */
		EnValueMode GetValueMode() const { return m_valueMode; }
		/**
		 * @brief イージングタイプを取得
		 * @return イージングタイプ
		 */
		util::EasingType GetEasingType() const { return m_easingType; }
		/**
		 * @brief 固定値を取得
		 * @return 固定値
		 */
		float GetFixedValue() const { return m_fixedValue; }
		/**
		 * @brief ランダム値の最小値を取得
		 * @return ランダム値の最小値
		 */
		float GetMinValue() const { return m_minValue; }
		/**
		 * @brief ランダム値の最大値を取得
		 * @return ランダム値の最大値
		 */
		float GetMaxValue() const { return m_maxValue; }
		/**
		 * @brief カーブの開始値を取得
		 * @return カーブの開始値
		 */
		float GetStartValue() const { return m_startValue; }
		/**
		 * @brief カーブの終了値を取得
		 * @return カーブの終了値
		 */
		float GetEndValue() const { return m_endValue; }


	public:
		/**
		 * @brief 初期値を決定する(パーティクル生成時に呼ぶ)
		 * @param rng 乱数生成器(シード値)
		 * @return 初期値
		 */
		float ResolveInitial() const
		{
			switch (m_valueMode)
			{
			case EnValueMode::Fixed:
			{
				return m_fixedValue;
			}
			case EnValueMode::Random:
			{
				// 指定された範囲内で等確率に値を生成するための分布。
				return util::RandomDevice::Random(m_minValue, m_maxValue);
			}
			case EnValueMode::Curve:
			{
				return m_startValue;
			}
			case EnValueMode::RandomCurve:
			{
				return util::RandomDevice::Random(m_minValue, m_maxValue);
			}
			}
			return m_fixedValue; // デフォ値
		}


		/**
		 * @brief カーブの終了値を決定する(RandomCurve用)
		 * @param rng 乱数生成器(シード値)
		 * @return カーブの終了値
		 */
		float ResolveEnd(std::mt19937& rng) const
		{
			if (m_valueMode == EnValueMode::RandomCurve)
			{
				return util::RandomDevice::Random(m_startValue, m_endValue);
			}
			return m_endValue;
		}


		/**
		 * @brief カーブ系かどうか
		 * @return CurveかRandomCurveならtrue、それ以外はfalse
		 */
		bool IsCurve() const
		{
			return m_valueMode == EnValueMode::Curve || m_valueMode == EnValueMode::RandomCurve;
		}
	};




	/***********************************************/


	/**
	 * @brief Vector3用の値プロバイダ
	 */
	class Vector3ValueProvider
	{
	private:
		FloatValueProvider m_x;
		FloatValueProvider m_y;
		FloatValueProvider m_z;
		bool m_uniform; // trueならxy(z)を同じランダム値にする。


	public:
		Vector3ValueProvider()
			: m_uniform(false)
		{}


	public:
		//===========================================//
		//	書き換え専用							 //
		//===========================================//
		/**
		 * @brief X値プロバイダを書き換える用
		 * @return X値プロバイダ
		 */
		FloatValueProvider& X() { return m_x; }
		/**
		 * @brief Y値プロバイダを書き換える用
		 * @return Y値プロバイダ
		 */
		FloatValueProvider& Y() { return m_y; }
		/**
		 * @brief Z値プロバイダを書き換える用
		 * @return Z値プロバイダ
		 */
		FloatValueProvider& Z() { return m_z; }


		//===========================================//
		//	読み取り専用							 //
		//===========================================//
		/**
		 * @brief X値プロバイダを読み取る
		 * @return X値プロバイダ
		 */
		const FloatValueProvider& X() const { return m_x; }
		/**
		 * @brief Y値プロバイダを読み取る
		 * @return Y値プロバイダ
		 */
		const FloatValueProvider& Y() const { return m_y; }
		/**
		 * @brief Z値プロバイダを読み取り
		 * @return Z値プロバイダ
		 */
		const FloatValueProvider& Z() const { return m_z; }

		/**
		 * @brief Uniformフラグを設定
		 * @param uniform trueならxy(z)を同じランダム値にする、falseなら個別にランダム値を生成する
		 */
		void SetUniform(bool uniform) { m_uniform = uniform; }

		/**
		 * @brief Uniformフラグを取得
		 * @return uniformフラグの取得
		 */
		bool IsUniform() const { return m_uniform; }


	public:
		/**
		 * @brief 固定値を設定
		 * @param value 固定値
		 */
		void SetFixed(const Vector3& value)
		{
			m_x.SetFixed(value.x);
			m_y.SetFixed(value.y);
			m_z.SetFixed(value.z);
		}

		/**
		 * @brief ランダム値の範囲を設定
		 * @param min 最小値
		 * @param max 最大値
		 */
		void SetRandom(const Vector3& min, const Vector3& max)
		{
			m_x.SetRandom(min.x, max.x);
			m_y.SetRandom(min.y, max.y);
			m_z.SetRandom(min.z, max.z);
		}

		/**
		 * @brief カーブの開始値、終了値、イージングタイプを設定
		 * @param start カーブの開始値
		 * @param end カーブの終了値
		 * @param easing イージングタイプ(デフォルトは線形補間)
		 */
		void SetCurve(const Vector3& start, const Vector3& end, util::EasingType easing = util::EasingType::Linear)
		{
			m_x.SetCurve(start.x, end.x, easing);
			m_y.SetCurve(start.y, end.y, easing);
		}

		/**
		 * @brief uniformがtrueのとき、Xプロバイダの結果をもとにYプロバイダにもコピーをする(2DなのでZは独立)
		 * @param rng 乱数生成器(シード値)
		 */
		Vector3 ResolveInitial() const
		{
			float xVal = m_x.ResolveInitial();
			float yVal;
			if (m_uniform)
			{
				yVal = xVal; // xと同じ値にするため
			}
			else
			{
				yVal = m_y.ResolveInitial();
			}
			float zVal = m_z.ResolveInitial();
			return Vector3(xVal, yVal, zVal);
		}
	};
}
