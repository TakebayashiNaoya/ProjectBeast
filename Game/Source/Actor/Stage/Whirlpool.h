/**
 * @file Whirlpool.h
 * @brief 渦潮のクラス
 * @author 藤谷
 */
#pragma once
#include "IStage.h"
#include "Source/Util/Curve.h"
#include "WhirlpoolPowerSystem.h"


namespace app
{
	namespace actor
	{
		/**
		 * @brief 渦潮のクラス
		 */
		class Whirlpool : public IStageObject
		{
		public:
			/**
			 * @brief 渦潮の状態
			 */
			enum class EnWhirlpoolState : uint8_t
			{
				Bigger,
				Smaller,
				Stay,
				ModelLoading,
				Num,
				None = Num
			};


		public:
			/**
			 * @brief 渦潮の状態を取得
			 * @return 渦潮の状態
			 */
			EnWhirlpoolState GetState() const { return m_state; }
			/**
			 * @brief 渦潮のインデックスを取得
			 * @return 渦潮のインデックス
			 */
			uint8_t GetIndex() const { return m_index; }
			/**
			 * @brief 渦潮のインデックスを設定
			 * @param index 渦潮のインデックス
			 */
			void SetIndex(const uint8_t index) { m_index = index; }


		public:
			void Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			Whirlpool();
			virtual ~Whirlpool() override = default;


		private:
			/**
			 * @brief 渦潮の状態遷移を行う関数
			 */
			void StateMachine();


		private:
			/** 渦潮の拡大カーブ */
			app::util::Vector3Curve m_scaleBigger;
			/** 渦潮の縮小カーブ */
			app::util::Vector3Curve m_scaleSmaller;
			/** 渦潮のインデックス */
			uint8_t m_index;
			/** 渦潮の状態 */
			EnWhirlpoolState m_state;
			/** 渦潮のタイマー */
			float m_timer;
			/** 渦潮の引き寄せ、押し出しを管理するクラス */
			std::unique_ptr<WhirlpoolPowerSytem> m_whirlpoolPowerSystem;

		};
	}
}

