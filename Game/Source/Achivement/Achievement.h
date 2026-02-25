/**
 * @file Achivement.h
 * @brief アチーブメントに関するクラス
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace achievement
	{
		/**
		 * @brief アチーブメントの基底クラス
		 */
		class Achievement : public Noncopyable
		{
		public:
			/**
			 * @brief アチーブメントを初期化
			 * @param name アチーブメントの名前
			 * @param description アチーブメントの説明
			 * @param id アチーブメントのID
			 * @param conditionFunc アチーブメントの達成条件を満たしているかどうかを判定する関数
			 */
			void InitializeAchievement(
				const std::string& name
				, const std::string& description
				, uint32_t id
				, std::function<bool()> conditionFunc
			);


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
			 * @details アチーブメントの達成フラグを更新
			 */
			void Update();


		public:
			Achievement();
			virtual ~Achievement();


		protected:
			/**< アチーブメントの達成条件を満たしているかどうかを判定する関数 */
			std::function<bool()> m_conditionFunc;


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
		};




		/*************************************************/


		/**
		 * @brief カウンタータイプのアチーブメントクラス
		 * @details カウンターアチーブメントは、特定の条件を満たす回数をカウントし、その回数が目標値に達したときに達成されるアチーブメントです。
		 */
		class CounterAchievement : public Achievement
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
			CounterAchievement();
			~CounterAchievement() override;


		protected:
			/** カウンターアチーブメントの現在の値 */
			uint32_t m_currentValue;
			/** カウンターアチーブメントの目標の値 */
			uint32_t m_targetValue;
		};




		/*************************************************/


		class LocationAchievement : public Achievement
		{
		public:
			LocationAchievement();
			~LocationAchievement() override;


		private:
			std::vector<bool> m_flags;
		};
	}
}

