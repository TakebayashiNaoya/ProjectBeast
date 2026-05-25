/**
 * @file InGameStartingAnimLogic.h
 * @brief ゲーム開始時のアニメーションロジッククラス
 * @author 藤谷
 */
#pragma once


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class MenuBase;
		class UIBase;


		class InGameStartingAnimLogic : public Noncopyable
		{
		public:
			InGameStartingAnimLogic();
			~InGameStartingAnimLogic() = default;


		public:
			/**
			 * @brief 初期化
			 * @details 現在はアイコンと数字のみ使用しているので、実装を省く
			 * @param menu メニュークラスのポインタ
			 * @param iconNames アイコンUIの名前
			 * @param digitNames 数字UIの名前
			 * @param startOffset 開始位置のオフセット
			 * @param duration アニメーションの持続時間
			 */
			void Initialize(
				MenuBase* menu,
				const std::vector<std::string> iconNames,
				const std::vector<std::string> digitNames,
				const Vector3 startOffset,
				float duration = 1.0f
			);

			/**
			 * @brief ゲーム開始時のアニメーションを更新する
			 */
			void Update();
			/** @brief UIパーツを更新する */
			void UpdateUIParts();


		public:
			/** @brief ゲーム開始時のアニメーションが開始したかどうか */
			bool IsAnimationStarted() const { return m_animState != AnimState::NotStarted; }
			/** @brief ゲーム開始時のアニメーションが終了したかどうか */
			bool IsAnimationFinished() const { return m_animState == AnimState::Finished; }


		private:
			size_t m_totalSize;
			std::vector<std::string> m_iconNames;
			std::vector<std::string> m_digitNames;
			/** UIパーツのリスト */
			std::vector<UIBase*> m_uiParts;
			/** メニュークラスのポインタ */
			MenuBase* m_menu;
			/** 開始位置のオフセット */
			Vector3 m_startOffset;

			/** ゲーム開始時のアニメーションの状態 */
			enum class AnimState : uint8_t
			{
				NotStarted,
				Playing,
				Finished
			};
			/** ゲーム開始時のアニメーションの状態 */
			AnimState m_animState;
		};
	}
}