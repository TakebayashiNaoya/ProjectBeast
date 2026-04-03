/**
 * @file Achivement.h
 * @brief アチーブメントに関するクラス
 * @author 藤谷
 */
#pragma once
#include "Json/json.hpp"


namespace app
{
	namespace achievement
	{
		/**
		 * @brief アチーブメントの基底クラス
		 */
		class AchievementBase : public Noncopyable
		{
		public:
			/**
			 * @brief 初期化処理
			 */
			void Init(const nlohmann::json& json);


		public:
			/**
			 * @brief アチーブメントの名前を取得
			 * @return アチーブメントの名前を取得
			 */
			inline const std::string& GetName() const { return m_name; }
			/**
			 * @brief アチーブメントの説明を取得
			 * @return アチーブメントの説明を取得
			 */
			inline const std::string& GetDescription() const { return m_description; }
			/**
			  * @brief アチーブメントのIDを取得
			  * @return アチーブメントのIDを取得
			  */
			inline uint32_t GetID() const { return m_id; }
			/**
			 * @brief アチーブメントが達成されているかどうかを設定
			 */
			inline void SetIsAchieved(const bool achieved) { m_isAchieved = achieved; }
			/**
			 * @brief アチーブメントが達成されているかどうかを取得
			 * @return アチーブメントが達成されているかどうかを取得
			 */
			inline bool IsAchieved() const { return m_isAchieved; }
			/**
			 * @brief アチーブメントが達成された時間を取得
			 * @return アチーブメントが達成された時間を取得(秒)
			 */
			inline uint32_t GetAchievedTime() const { return m_achievedTime; }


		public:
			/**
			 * @brief 更新処理
			 */
			virtual void Update() = 0;


		public:
			AchievementBase();
			virtual ~AchievementBase();


		public:
			inline int GetIndex() const { return m_index; }
			inline void SetIndex(int index) { m_index = index; }

			inline const std::string& GetSpriteName() const { return m_spriteName; }


		protected:
			/**
			 * @brief アチーブメントを初期化
			 * @param name アチーブメントの名前
			 * @param description アチーブメントの説明
			 * @param id アチーブメントのID
			 */
			void InitAchievementBase(const nlohmann::json& json);


			/**
			 * @brief 派生先の条件用変数を初期化する関数
			 * @todo 派生先の条件用変数を初期化する関数を実装する
			 */
			virtual void InitAchievementImpl(const nlohmann::json& json) = 0;


		protected:
			/** アチーブメントの達成条件を満たしているかどうかを判定する関数 */
			std::function<bool()> m_conditionFunc;
			/** アチーブメントの達成を管理するフラグ群 */
			std::vector<bool> m_flags;


			/** アチーブメントの名前 */
			std::string m_name;
			/** アチーブメントの説明 */
			std::string m_description;
			/** アチーブメントのID */
			uint32_t m_id;
			/** アチーブメントが達成されているかどうか */
			bool m_isAchieved;
			/** アチーブメントが達成された時間(秒) */
			uint32_t m_achievedTime;

			int m_index = -1;
			/** 画像の名前 */
			std::string m_spriteName;
		};




		/*************************************************/


		/**
		 * @brief カウンタータイプのアチーブメントクラス
		 * @details カウンターアチーブメントは、特定の条件を満たす回数をカウントし、その回数が目標値に達したときに達成されるアチーブメントです。
		 */
		class CounterAchievement : public AchievementBase
		{
		public:
			/**
			 * @brief カウンターアチーブメントの現在の値を取得
			 * @return カウンターアチーブメントの現在の値を取得
			 */
			inline uint32_t GetCurrentValue() const { return m_currentValue; }
			/**
			 * @brief カウンターアチーブメントの目標の値を取得
			 * @return カウンターアチーブメントの目標の値を取得
			 */
			inline uint32_t GetTargetValue() const { return m_targetValue; }


		public:
			/** 更新処理 */
			void Update() override final;


		public:
			CounterAchievement();
			~CounterAchievement() override;


		private:
			void InitAchievementImpl(const nlohmann::json& json) override final;


		protected:
			/** カウンターアチーブメントの現在の値 */
			uint32_t m_currentValue;
			/** カウンターアチーブメントの目標の値 */
			uint32_t m_targetValue;
		};




		/*************************************************/


		/**
		 * @brief ロケーションタイプのアチーブメントクラス
		 * @details ロケーションアチーブメントは、特定の位置に到達したときに達成されるアチーブメントです。
		 */
		class LocationAchievement : public AchievementBase
		{
		public:
			LocationAchievement();
			~LocationAchievement() override;


			void Update()override final;


		private:
			void InitAchievementImpl(const nlohmann::json& json) override final;


		private:
			/** 目標の位置 */
			Vector3 m_targetLocation;
			/** 達成するための距離の閾値 */
			float m_enableDistance;
		};




		/*************************************************/


		/**
		 * @brief 条件判定タイプのアチーブメントクラス
		 * @details条件判定アチーブメントは、条件を満たした瞬間に達成されるアチーブメントです。
		 */
		class ConditionAchievement :public AchievementBase
		{
		public:
			ConditionAchievement();
			~ConditionAchievement()override;


			/**
			 * @brief 達成条件
			 */
			void SetCondition(std::function<bool()>condition) { m_conditionFunc = condition; }


			void Update()override final;


		private:
			void InitAchievementImpl(const nlohmann::json& json) override final;
		};




		/*************************************************/


		/**
		 * @brief イベントタイプのアチーブメントクラス
		 * @details イベントアチーブメントは、特定の処理が実行された瞬間に達成されるアチーブメントです。
		 */
		class EventAchievement : public AchievementBase
		{
		public:
			EventAchievement();
			~EventAchievement() override;

			/** 更新処理：何もしない（イベント駆動のため） */
			void Update() override final {}

			/**
			 * @brief 外部から達成を通知する
			 */
			void Unlock();

		private:
			void InitAchievementImpl(const nlohmann::json& json) override final {}
		};


		/*************************************************/

		/**
		 * @brief レコード（記録）タイプのアチーブメントクラス
		 * @details レコードアチーブメントは、ゲーム中の最大値や最高記録を継続して更新・保持するためのアチーブメントです。
		 */
		class RecordAchievement : public AchievementBase
		{
		public:
			RecordAchievement();
			~RecordAchievement() override;

			/** 更新処理：何もしない（イベント駆動で数値を更新するため） */
			void Update() override final {}

			/**
			 * @brief 記録を更新する（現在の記録より大きければ上書き）
			 * @param value 新しい記録値
			 */
			void UpdateRecord(uint32_t value);

			/** 現在の最大記録を取得 */
			inline uint32_t GetRecordValue() const { return m_recordValue; }

		private:
			void InitAchievementImpl(const nlohmann::json& json) override final {}

		private:
			uint32_t m_recordValue; // 保持する記録（今回の場合は最大マーキング数）
		};
	}
}

