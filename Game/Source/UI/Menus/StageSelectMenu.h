/**
 * @file StageSelectMenu.h
 * @brief ステージ選択画面のメニュークラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/Modules/Input/UICursorSelector.h"
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
			Tutorial,
			Max,
		};


		/**
		 * @brief ボタンの種類
		 * @detail もどる、決定、選択
		 */
		enum class EnStageButtonTypes : uint8_t
		{
			Back,
			Decide,
			Select,
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
			/** @brief ステージが選択されたかどうかを取得する */
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
			 * @brief 描画フラグを更新する
			 */
			void UpdateDrawFlag();
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
			 * @brief 選択中ステージの情報パネル（制限時間・クマ数・渦潮数・記録）を更新する
			 * @details クマ数と渦潮数は配置JSONから読むので、ステージを再生成しても
			 *          表示が自動で追従する。チュートリアル選択中は非表示。
			 */
			void UpdateStageInfo();

			/**
			 * @brief ステージ情報（クマ数・渦潮数）を配置JSONから読み込む（初回のみ）
			 */
			void LoadStageInfoIfNeeded();

			/**
			 * @brief 選択確定後の演出（画面中央へズームイン＋白フェード）を更新する
			 * @details 座標系が画面中央原点なので、可視パーツの位置とスケールに
			 *          同じ倍率を掛けるだけで中央へのズームインになる。
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

			/** ステージ情報（Easy/Normal/Hardの3ステージ分） */
			static constexpr int STAGE_INFO_NUM = 3;
			/** 配置JSONから読んだクマの頭数 */
			int m_stageBearCounts[STAGE_INFO_NUM] = { 0, 0, 0 };
			/** 配置JSONから読んだ渦潮の数 */
			int m_stageWhirlCounts[STAGE_INFO_NUM] = { 0, 0, 0 };
			/** ステージ情報を読み込み済みか */
			bool m_isStageInfoLoaded = false;


			/** JSONから読み込むメニューパラメーター */
			struct StageSelectParam
			{
				float   inputInterval = 0.2f;
				float   inputThreshold = 0.5f;
				float   selectZoomDuration = 0.6f;      /** 選択確定演出の長さ（秒） */
				float   selectZoomScale = 2.2f;         /** ズームの最終倍率 */
				float   selectWhiteFadeDuration = 0.35f; /** 白フェードの長さ（秒・演出の末尾に重ねる） */
				float   tutorialCursorScaleX = 400.0f / 280.0f;
				float   cursorBlinkDuration = 0.5f;
				Vector4 cursorBlinkStartColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
				Vector4 cursorBlinkEndColor = Vector4(1.0f, 1.0f, 1.0f, 0.0f);
				/** ステージごとの背景映像パス（Easy/Normal/Hard/Tutorial 順）*/
				std::array<std::string, static_cast<uint8_t>(EnStageChoices::Max)> stageVideoPaths = {};
			};


			/** ステージ選択肢のデータ構造体 */
			struct StageChoicesData
			{
				/** テキスト */
				UIText* m_text;
				/** バブルアイコン */
				UIIcon* m_bubbleIcon;


				StageChoicesData();
				~StageChoicesData() = default;
			};




			/*****************************************************/


			/** ステージ選択のデータ構造体 */
			struct StageButtonData
			{
				/** ボタンアイコン */
				UIIcon* m_button;
				/** テキスト */
				UIText* m_text;


				StageButtonData();
				~StageButtonData() = default;
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
			EnStageSelectState m_state;
			/** 選択中のステージ選択肢 */
			EnStageChoices m_selectingStage;


			/** 背景アイコン */
			UIIcon* m_bgIcon;
			/** "ステージセレクト"のテキスト */
			UIText* m_stageSelectText;
			/** "ステージセレクト"の背景アイコン */
			UIIcon* m_stageSelectTextBGIcon;

			/** ステージ選択肢 */
			std::array<StageChoicesData, static_cast<uint8_t>(EnStageChoices::Max)> m_choices;

			/** ステージ選択画面のボタン */
			std::array<StageButtonData, static_cast<uint8_t>(EnStageButtonTypes::Max)> m_buttons;
			/** ボタンの背景アイコン */
			UIIcon* m_buttonBGIcon;

			/** 選択カーソルのフレーム */
			UIIcon* m_cursorFrame;
			/** 選択カーソルのフレームの背景 */
			UIIcon* m_cursorFrameBG;

			/** ステージ背景映像 */
			UIVideo* m_stagePreviewVideo;
			/** 選択確定演出の白フラッシュアイコン */
			UIIcon* m_selectFlashIcon;
			/** 直前のステージ選択（映像切り替え検出用）*/
			EnStageChoices m_prevSelectingStage;

			/** 選択入力のインターバル */
			float m_selectInputInterval;
			/** カーソル移動ポップの残り時間（秒） */
			float m_cursorPopTimer = 0.0f;
			/** 選択確定演出の経過時間（秒） */
			float m_selectEffectTimer = 0.0f;
			/** ズーム対象と基準値 */
			std::vector<ZoomTarget> m_zoomTargets;
			/** ズーム基準値を保存済みか */
			bool m_isZoomBaseCaptured = false;
			/** 選択されたかどうか */
			bool m_isSelected;
			/** JSONから読み込んだメニューパラメーター */
			StageSelectParam m_param;


			AxisInputDetector m_verticalInputDetector;
			AxisInputDetector m_horizontalInputDetector;
			CursorIndexSelector m_cursorSelector;
		};
	}
}


