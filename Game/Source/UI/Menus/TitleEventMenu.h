/**
 * @file TitleEventMenu.h
 * @brief タイトルの動的処理クラス
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/Modules/Input/UICursorSelector.h"
#include "Source/UI/Modules/Input/UIInputController.h"

#include "Source/Vfx/ParticleEffectRender.h"


namespace app
{
	namespace ui
	{
		class TitleEventMenu : public MenuBase
		{
		public:
			TitleEventMenu();
			~TitleEventMenu();
			void Update()override;
			void InitializeLogic()override;

			/**
			 * @brief 選択されている項目の見た目を変更させる
			 * @details 選択中の文字を大きくし（カーソル移動直後はポップさせる）、それ以外の文字は半透明にする
			 */
			void SelectVisual();

			/**
			 * @brief 現在選択されているキーを取得する
			 * @return 現在選択されているキーの取得
			 */
			uint32_t GetSelectKey()const;

			/**
			 * @brief 描画の設定。
			 * @param isDraw 描画するかどうかのフラグ。
			 */
			void SetDraw(bool isDraw) { m_isDraw = isDraw; }

			/**
			 * @brief UIパーツの取得
			 */
			void GetUIParts();

			/**
			 * @brief 描画フラグの更新
			 */
			void UpdateDrawFlag();

			/**
			 * @brief タイトルの環境演出（音符・雪・ロゴの弾み）の更新
			 * @details 静止画のタイトルに命を入れるレイヤー。
			 *          音符が湧き上がり、雪が降り、ロゴが拍で弾む。
			 */
			void UpdateAmbient();

			/**
			 * @brief 音符の発生範囲をロゴの位置と大きさに合わせる
			 * @details ロゴを真ん中として左側と右側の帯（ロゴの中心より上）に分け、ロゴに被らない近くのランダムな位置から発生させる
			 */
			void UpdateMusicalNoteSpawnArea();

			/**
			 * @brief 指定の位置に音符を出すと、出ている音符と重なるか
			 * @param localPosition ロゴ中心（エミッター）からの相対座標
			 * @return 左右どちらかのグループの音符と重なるならtrue
			 */
			bool IsMusicalNoteOverlapped(const Vector2& localPosition);

			/**
			 * @brief 音符のパーティクルエフェクトを描画
			 * @param rc 描画コンテキスト
			 */
			void Render(RenderContext& rc) override;


		private:
			/**
			 * @brief 項目の文字のJSON上の大きさと色を記録する（まだ記録していないときだけ）
			 * @return 全ての項目の記録が済んでいればtrue
			 */
			bool CaptureEventBaseVisual();


		public:
			/**
			 * @brief イベントの種類のenum
			 * @details Start:スタート、Sound:サウンドオプション、Rule:ルール説明、End:おわり
			 */
			enum class EnEventType : uint8_t
			{
				Start,
				Sound,
				Rule,
				End,
				Num
			};


		private:
			GamePad* m_gamePad;
			EnEventType m_selectIndex;
			bool m_isStickNeutral;
			bool m_isSelect;
			bool m_isDraw;


			/** 背景アイコン */
			UIIcon* m_bgIcon;
			/** ロゴアイコン */
			UIIcon* m_rogoIcon;
			/** 選択中の項目の横に出す音符アイコン */
			UIIcon* m_selectNoteIcon;
			/** 選択中の項目の下に出す下線アイコン */
			UIIcon* m_selectUnderlineIcon;
			/** イベントテキスト */
			std::array<UIText*, static_cast<uint8_t>(EnEventType::Num)> m_eventIcon;
			/** イベントテキストのJSON上の大きさ（選択演出の基準値） */
			std::array<Vector2, static_cast<uint8_t>(EnEventType::Num)> m_eventBaseScales;
			/** イベントテキストのJSON上の色（選択演出の基準値） */
			std::array<Vector4, static_cast<uint8_t>(EnEventType::Num)> m_eventBaseColors;
			/** イベントテキストの基準値を記録済みか */
			bool m_isEventBaseVisualCaptured = false;

			/** コントローラの入力制御 */
			AxisInputDetector m_axisInputDetector;
			/** カーソル移動時の制御 */
			CursorIndexSelector m_cursorSelector;

			/** 音符のパーティクルエフェクト（[0]:ロゴの左側の帯用、[1]:ロゴの右側の帯用。それぞれJSONのmaxParticles個ずつ出す） */
			std::array<ParticleEffectRender, 2> m_musicalNoteEffectRenders;
			/** 音符エフェクトを初期化済みか */
			bool m_isMusicalNoteEffectInitialized;

			/** 環境演出（音符・雪・ロゴ弾み）用の経過時間（秒） */
			float m_ambientTimer = 0.0f;
			/** カーソル移動ポップの残り時間（秒） */
			float m_cursorPopTimer = 0.0f;
			/** 音符アイコンの拡縮の経過時間（秒）。選択する項目が変わったら0に戻す */
			float m_selectNotePulseTimer = 0.0f;
			/** ロゴのJSON上の基準スケール（弾み演出の基準値） */
			Vector3 m_rogoBaseScale = Vector3::One;
			/** 基準スケールを取得済みか */
			bool m_isRogoBaseScaleCaptured = false;
		};
	}
}
