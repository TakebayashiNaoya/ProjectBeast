/**
 * @file IFormationPassive.h
 * @brief 陣形パッシブ効果のインターフェース・基底・抽象デコレーター
 * @author 竹林
 */
#pragma once
#include <memory>


namespace app
{
	namespace actor
	{
		/**
		 * @brief 陣形パッシブ効果のインターフェース
		 * @details 常時発動する陣形固有の効果を定義する。
		 */
		class IFormationPassive
		{
		public:
			virtual ~IFormationPassive() = default;

			/** @brief 渦潮耐性を持つか */
			virtual bool HasWhirlpoolResistance() const { return false; }

			/**
			 * @brief 移動速度倍率を返す
			 * @param level 現在の陣形レベル
			 */
			virtual float GetSpeedMultiplier(int level) const { return 1.0f; }
		};


		/** @brief パッシブなし（デコレーターチェーンの末端） */
		class BasePassive : public IFormationPassive {};


		/**
		 * @brief パッシブデコレーターの抽象基底
		 * @details オーバーライドしないメソッドは自動的に m_wrapped に委譲される。
		 */
		class FormationPassiveDecorator : public IFormationPassive
		{
		protected:
			std::unique_ptr<IFormationPassive> m_wrapped;

		public:
			explicit FormationPassiveDecorator(std::unique_ptr<IFormationPassive> w)
				: m_wrapped(std::move(w))
			{}

			bool  HasWhirlpoolResistance() const override { return m_wrapped->HasWhirlpoolResistance(); }
			float GetSpeedMultiplier(int level) const override { return m_wrapped->GetSpeedMultiplier(level); }
		};
	}
}
