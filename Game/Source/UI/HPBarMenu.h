/**
 * @file HPBarMenu.h
 * @brief 親ペンギンのHPバーの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Menu.h"


namespace app
{
	namespace ui
	{
		/** HPバーのタイプ */
		enum class EnHPType : uint8_t
		{
			Gray, Red, Green, Max
		};


		class HPBarIcon
		{
		public:
			HPBarIcon(EnHPType type);
			~HPBarIcon();
			void Update();
			void SetUIIcon(UIIcon* icon);
			UIIcon* GetUIIcon() const { return m_icon; }

			/**
			 * @brief 動くかどうかの取得
			 */
			bool IsMoving()const;


		public:
			/**
			 * @brief HPの設定
			 * @param targetHP 外部から設定する用
			 */
			void SetTargetHP(float targetHP);

			/**
			 * @brief 現在のスケールの取得
			 * @return 現愛のスケールを取得
			 */
			float GetCurrentScale()const { return m_currentScale; }


		private:
			UIIcon* m_icon;
			EnHPType m_type;
			float m_targetHP;
			float m_currentScale;
			float m_initialPosX;
			float m_width;
		};


		class HPBarMenu : public MenuBase
		{
			using HPBarClass = MenuBase;
		public:
			HPBarMenu();
			~HPBarMenu();
			void Update()override;
			void InitializeLogic()override;

			/**
			 * @brief HPのタイプの取得
			 * @return 現在のHPのタイプを取得
			 */
			EnHPType GetType()const { return m_currentHPType; }
			/**
			 * @brief 最大HPの設定
			 * @param maxHp 外部から設定する最大HP
			 */
			void SetMaxHP(int maxHp) { m_maxHP = maxHp; }
			/**
			 * @brief HPの設定
			 * @param targetHP 外部から設定する用
			 */
			void SetTargetHP(float targetHP);
			/**
			 * @brief ダメージの処理
			 * @param damage ダメージ量
			 */
			void Damage(float damage);


			/**
			 * @brief バーが動いているかどうかの取得
			 * @return GreenかRedのHPバーが動いている場合はtrue、そうでない場合はfalseを返す
			 */
			bool IsAnimating()const
			{
				return m_isGreenMoving || m_isRedMoving;
			}
			/**
			 * @brief 緑のバーが動くフラグの取得
			 * @return GreenのHPバーが動いているか
			 */
			bool IsGreenMoving()const { return m_isGreenMoving; }
			/**
			 * @brief 赤いバーが動くフラグの取得
			 * @return RedのHPバーが動いているか
			 */
			bool IsRedMoving()const { return m_isRedMoving; }

			/**
			 * @brief HPが0以下かどうかの取得
			 * @return HPが0以下の場合はtrue、そうでない場合はfalseを返す
			 */
			bool IsLostHP()const { return m_currentHP <= 0.0f; }
			/**
			 * @brief 現在のHPの取得
			 * @return 現在のHPを取得
			 */
			float GetCurrentHP()const { return m_currentHP; }


		private:
			EnHPType m_currentHPType;
			bool m_isGreenMoving;
			bool m_isRedMoving;
			//bool m_prevGreenMoving;
			//bool m_prevRedMoving;

			using Icon = std::unique_ptr<HPBarIcon>;
			using Key = uint32_t;

			std::unordered_map<Key, Icon> m_hpBarIconMap;

			/** 最大のHP */
			int m_maxHP;
			/** 現在のHP */
			int m_currentHP;
			/** 目標HP */
			int m_targetHP;
			/** ダメージ */
			int m_damage;
			float m_damageRatio;
		};
	}
}