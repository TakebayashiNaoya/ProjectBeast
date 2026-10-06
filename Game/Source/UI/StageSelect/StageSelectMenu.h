/**
 * @file StageSelectMenu.h
 * @brief ステージ選択画面のメニュークラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/StageSelect/StageInfoPanel.h"
#include "Source/UI/Modules/Input/UIInputController.h"
#include "Source/Util/Curve.h"

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

		private:
			/**
			 * @brief 選択中だけ見せるパーツの表示/非表示を反映する
			 * @param isShow 表示するかどうか
			 * @details 状態が変わった時にだけ呼ぶ（毎フレーム呼ばない）。
			 *          表示するパーツは m_selectingParts と情報パネル。映像と白フラッシュは含まない。
			 */
			void ApplyVisibility(const bool isShow);
			/**
			 * @brief 左右の矢印の見た目（入力ポップ・端での減光）を更新する
			 */
			void UpdateIcons();

			/**
			 * @brief 難易度テキストの切り替え演出を開始する
			 * @param stageIndex 切り替え先のステージ番号（EnStageChoices の並び）
			 * @details 選択中のテキストが拡大 → 拡大したまま次のテキストへ差し替え → 縮小して元の大きさへ戻る。
			 *          演出中にさらに切り替わった場合は、現在の大きさから拡大し直して最新の難易度へ差し替える。
			 */
			void StartStageNameAnimation(const int stageIndex);
			/**
			 * @brief 難易度テキストの切り替え演出を更新する
			 */
			void UpdateStageNameAnimation();
			/**
			 * @brief 難易度テキストの切り替え演出を止めて、元の大きさへ戻す
			 */
			void ResetStageNameAnimation();
			/**
			 * @brief UIパーツを取得する
			 */
			void GetUIParts();
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

			/**
			 * @brief 映像・情報パネルの切り替え演出（暗転して戻る）を開始する
			 * @details 映像が暗くなり、真っ暗になった瞬間に映像と情報パネルを差し替え、
			 *          少しズームしながら明るく戻る。差し替え先は常に最新の m_selectingStage なので、
			 *          暗くしている最中にさらに切り替わっても、そのまま最新のステージへ差し替わる。
			 *          明るく戻している最中に切り替わった場合は、今の明るさから暗くし直す。
			 */
			void StartSwitchTransition();
			/**
			 * @brief 映像・情報パネルの切り替え演出を更新する
			 */
			void UpdateSwitchTransition();
			/**
			 * @brief 選択中のステージの映像と情報パネルへ差し替える
			 */
			void ApplyStageContent();
			/**
			 * @brief 切り替え演出の明るさを映像・情報パネルへ反映する
			 * @param brightness 明るさ（1.0で通常、0.0で真っ暗）
			 */
			void ApplySwitchLook(const float brightness);
			/**
			 * @brief 切り替え演出を終わらせ、通常の見た目へ戻す
			 * @details 差し替え前だった場合は、先に最新のステージへ差し替える。
			 *          決定演出の開始前（ズームの基準値が暗転途中の値にならないように）と Reset で使う。
			 */
			void FinishSwitchTransition();

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

			/** ステージ切り替え演出の段階 */
			enum class EnSwitchPhase : uint8_t
			{
				Idle,	 /** 演出していない */
				FadeOut, /** 暗くしている（真っ暗になったら中身を差し替える） */
				FadeIn,	 /** 差し替え後、明るく戻している */
			};

			/** JSONから読み込むメニューパラメーター */
			struct StageSelectParam
			{
				float inputInterval = 0.2f;
				float inputThreshold = 0.5f;
				float selectZoomDuration = 0.6f;	   /** 選択確定演出の長さ（秒） */
				float selectZoomScale = 2.2f;		   /** ズームの最終倍率 */
				float selectWhiteFadeDuration = 0.35f; /** 白フェードの長さ（秒・演出の末尾に重ねる） */
				float arrowDisabledAlpha = 0.3f;	   /** 端のステージで、動かせない側の矢印を薄くする時のα */
				float switchOutDuration = 0.12f;	   /** ステージ切り替え時、映像が真っ暗になるまでの時間（秒） */
				float switchInDuration = 0.25f;		   /** ステージ切り替え時、映像が明るく戻るまでの時間（秒） */
				float switchZoomScale = 0.05f;		   /** 真っ暗の時に映像を拡大する量（1.0からの増分） */
				float stageNamePopScale = 1.35f;	   /** 難易度テキストの切り替え時、拡大のピーク倍率（基準の大きさに対する倍率） */
				float stageNameExpandDuration = 0.08f; /** 難易度テキストの拡大にかける時間（秒） */
				float stageNameShrinkDuration = 0.18f; /** 難易度テキストの縮小にかける時間（秒） */
				/** ステージごとの表示名（Easy/Normal/Hard 順）*/
				std::array<std::string, static_cast<uint8_t>(EnStageChoices::Max)> stageNames = {};
				/** ステージごとの背景映像パス（Easy/Normal/Hard 順）*/
				std::array<std::string, static_cast<uint8_t>(EnStageChoices::Max)> stageVideoPaths = {};
			};

			/** 選択確定演出のズーム対象と基準値 */
			struct ZoomTarget
			{
				UIBase* m_ui = nullptr;					/** 対象のUI */
				Vector3 m_basePosition;					/** 演出開始時の位置 */
				Vector3 m_baseScale = Vector3::One;		/** 演出開始時のスケール */
				Vector2 m_baseFontScale = {1.0f, 1.0f}; /** テキストの場合のフォントスケール */
			};

		private:
			/** ステージ選択状態 */
			EnStageSelectState m_state;
			/** 選択中のステージ選択肢 */
			EnStageChoices m_selectingStage;

			/**
			 * 選択中だけ見せるパーツ（背景・ステージ名の帯・矢印・ボタン案内など）
			 * 選択が確定すると、まとめて隠れる。
			 */
			std::vector<UIBase*> m_selectingParts;
			/** 表示/非表示の反映が必要か（初期化直後と状態が変わった時に立てる） */
			bool m_isVisibilityDirty;

			/** ステージ切り替えの矢印（左右） */
			UIIcon* m_leftArrow;
			UIIcon* m_rightArrow;
			/** 選択中のステージ名（左上の帯の文字） */
			UIText* m_stageNameText;

			/** 難易度テキスト切り替え演出の段階 */
			enum class EnStageNamePhase : uint8_t
			{
				Idle,
				Expanding,
				Shrinking,
			};
			/** 難易度テキスト切り替え演出の段階 */
			EnStageNamePhase m_stageNamePhase;
			/** 難易度テキストの現在の倍率（段階ごとに初期化し直す） */
			util::FloatCurve m_stageNameCurve;
			/** 難易度テキストの基準フォントスケール（JSONで設定された大きさ） */
			Vector2 m_stageNameBaseScale;
			/** 拡大のあとで差し替える難易度のステージ番号 */
			int m_stageNameTargetIndex;

			/** 選択中ステージの情報パネル */
			StageInfoPanel m_infoPanel;

			/** ステージ背景映像 */
			UIVideo* m_stagePreviewVideo;
			/** 選択確定演出の白フラッシュアイコン */
			UIIcon* m_selectFlashIcon;
			/** 直前のステージ選択（映像・情報パネルの切り替え検出用）*/
			EnStageChoices m_prevSelectingStage;

			/** ステージ切り替え演出の段階 */
			EnSwitchPhase m_switchPhase;
			/** ステージ切り替え演出の明るさ（1.0で通常、0.0で真っ暗）の補間カーブ */
			util::FloatCurve m_switchCurve;

			/** 矢印の入力ポップの残り時間（秒）。左・右の順 */
			std::array<float, 2> m_arrowPopTimers;
			/** 選択確定演出の経過時間（秒） */
			float m_selectEffectTimer;
			/** ズーム対象と基準値 */
			std::vector<ZoomTarget> m_zoomTargets;
			/** ズーム基準値を保存済みか */
			bool m_isZoomBaseCaptured;
			/** 選択されたかどうか */
			bool m_isSelected;
			/** JSONから読み込んだメニューパラメーター */
			StageSelectParam m_param;

			/** 左右入力の判定 */
			AxisInputDetector m_horizontalInputDetector;
		};
	}
}
