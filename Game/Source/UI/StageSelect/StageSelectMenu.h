/**
 * @file StageSelectMenu.h
 * @brief ステージ選択画面のメニュークラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/StageSelect/StageInfoPanel.h"
#include "Source/UI/Modules/Input/UIInputController.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief ステージ選択肢の列挙体
		 */
		enum class EnStageChoices : uint8_t
		{
			Easy,
			Normal,
			Hard,
			Max,
		};


		/**
		 * @brief ステージ選択画面のメニュークラス
		 */
		class StageSelectMenu : public MenuBase
		{
		public:
			StageSelectMenu();
			~StageSelectMenu() override;


		private:
			void InitializeLogic() override final;

			void Update() override final;


		public:
			/** @brief ステージが選択されたかどうかを設定する */
			inline void SetIsSelected(const bool isSelected) { m_isSelected = isSelected; }


		public:
			/** @brief 選択中のステージを取得する */
			inline EnStageChoices GetSelectingStage() const { return m_selectingStage; }
			/** @brief ステージが選択されたかどうかを取得する */
			inline bool IsSelected() const { return m_isSelected; }
			/** @brief 選択後の演出（ズーム＋白フェード）が終了したかどうかを取得する */
			inline bool IsFinishedSelectAnimation() const
			{
				return m_isSelected && m_selectEffectTimer >= m_param.selectZoomDuration;
			}
			/** @brief ステージ選択状態をリセットする */
			void Reset();



		private:
			/** @brief ステージ選択状態を更新する */
			void UpdateSelecting();
			/** @brief ステージ選択肢の選択状態を更新する */
			void UpdateSelected();



		private:
			/**
			 * @brief 選択中だけ見せるパーツの表示/非表示を反映する
			 * @param isShow 表示するかどうか
			 * @details 状態が変わった時にだけ呼ぶ（毎フレーム呼ばない）。
			 *          表示するパーツは m_selectingParts と情報パネル。映像と白フラッシュは含まない。
			 */
			void ApplyVisibility(const bool isShow);
			/**
			 * @brief 位置を更新する
			 */
			void UpdateIcons();
			/**
			 * @brief UIパーツを取得する
			 */
			void GetUIParts();
			/**
			 * @brief アニメーションを設定する
			 */
			void SetAnimations(const uint32_t animationKey);
			/**
			 * @brief JSONからメニューパラメーターを読み込む
			 */
			void LoadMenuParam();

			/**
			 * @brief 選択確定後の演出（画面中央へズームイン＋白フェード）を更新する
			 * @details メニュー類は演出の開始と同時に隠し、ステージ映像だけをズームさせる。
			 *          座標系が画面中央原点なので、位置とスケールに同じ倍率を掛けるだけで
			 *          中央へのズームインになる。
			 *          白が満ちたあとは既存のシーンフェード（暗転）へつながる。
			 */
			void UpdateSelectEffect();

			/**
			 * @brief ズーム対象の基準位置・スケールを保存する（演出開始時に1回）
			 */
			void CaptureZoomBase();


		private:
			/** ステージ選択状態 */
			enum class EnStageSelectState : uint8_t
			{
				Selecting,
				Selected,
			};

			/**
			 * @brief ステージ選択状態を切り替える
			 * @details 状態によって見せるパーツが変わるので、次のUpdateで表示を反映させる
			 */
			void SetState(const EnStageSelectState state);


			/** JSONから読み込むメニューパラメーター */
			struct StageSelectParam
			{
				float   inputInterval = 0.2f;
				float   inputThreshold = 0.5f;
				float   selectZoomDuration = 0.6f;      /** 選択確定演出の長さ（秒） */
				float   selectZoomScale = 2.2f;         /** ズームの最終倍率 */
				float   selectWhiteFadeDuration = 0.35f; /** 白フェードの長さ（秒・演出の末尾に重ねる） */
				float   cursorBlinkDuration = 0.5f;
				Vector4 cursorBlinkStartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				Vector4 cursorBlinkEndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
				/** ステージごとの背景映像パス（Easy/Normal/Hard 順）*/
				std::array<std::string, static_cast<uint8_t>(EnStageChoices::Max)> stageVideoPaths = {};
			};


			/** 選択確定演出のズーム対象と基準値 */
			struct ZoomTarget
			{
				UIBase* m_ui = nullptr;                     /** 対象のUI */
				Vector3 m_basePosition;                     /** 演出開始時の位置 */
				Vector3 m_baseScale = Vector3::One;         /** 演出開始時のスケール */
				Vector2 m_baseFontScale = { 1.0f, 1.0f };   /** テキストの場合のフォントスケール */
			};


		private:
			/** ステージ選択状態 */
			EnStageSelectState m_state = EnStageSelectState::Selecting;
			/** 選択中のステージ選択肢 */
			EnStageChoices m_selectingStage = EnStageChoices::Easy;


			/**
			 * 選択中だけ見せるパーツ（背景・見出し・選択肢・ボタン・カーソルなど）
			 * 選択が確定すると、まとめて隠れる。
			 */
			std::vector<UIBase*> m_selectingParts;
			/** 表示/非表示の反映が必要か（初期化直後と状態が変わった時に立てる） */
			bool m_isVisibilityDirty = true;

			/** ステージ選択肢のバブル（カーソルの位置合わせの基準） */
			std::array<UIIcon*, static_cast<uint8_t>(EnStageChoices::Max)> m_bubbleIcons = {};
			/** 選択カーソルのフレーム */
			UIIcon* m_cursorFrame = nullptr;
			/** 選択カーソルのフレームの背景 */
			UIIcon* m_cursorFrameBG = nullptr;

			/** 選択中ステージの情報パネル */
			StageInfoPanel m_infoPanel;

			/** ステージ背景映像 */
			UIVideo* m_stagePreviewVideo = nullptr;
			/** 選択確定演出の白フラッシュアイコン */
			UIIcon* m_selectFlashIcon = nullptr;
			/** 直前のステージ選択（映像・情報パネルの切り替え検出用）*/
			EnStageChoices m_prevSelectingStage = EnStageChoices::Max;

			/** カーソル移動ポップの残り時間（秒） */
			float m_cursorPopTimer = 0.0f;
			/** 選択確定演出の経過時間（秒） */
			float m_selectEffectTimer = 0.0f;
			/** ズーム対象と基準値 */
			std::vector<ZoomTarget> m_zoomTargets;
			/** ズーム基準値を保存済みか */
			bool m_isZoomBaseCaptured = false;
			/** 選択されたかどうか */
			bool m_isSelected = false;
			/** JSONから読み込んだメニューパラメーター */
			StageSelectParam m_param;


			/** 左右入力の判定 */
			AxisInputDetector m_horizontalInputDetector;
		};
	}
}


