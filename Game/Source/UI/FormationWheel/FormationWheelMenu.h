/**
 * @file FormationWheelMenu.h
 * @brief 陣形切り替え(LB/RB)とウルト発動可否(LT/RT)をアイコンで表示するクラス
 * @author 竹林
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinTypes.h"
#include "Source/UI/Modules/InGameStartingAnimLogic/InGameStartingAnimLogic.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief 陣形/ウルトのボタン操作を示すMenuクラス
		 * @details
		 *   現在の陣形アイコンを中央に大きく表示し、その左右に
		 *   LB/RBで切り替わる陣形アイコンを並べる。
		 *   陣形切り替え中は各アイコンが横にスライドするアニメーションを行う
		 *   (演出中はChildPenguinManager側で入力がロックされる)。
		 *   LT/RTアイコンはウルトが発動可能な間のみ通常色、それ以外はグレーアウトする。
		 *   見た目の座標・サイズ・色はFormationWheelTuning.jsonからホットリロードで調整できる。
		 */
		class FormationWheelMenu : public MenuBase
		{
		public:
			FormationWheelMenu();
			~FormationWheelMenu() override = default;
			void Update() override;
			void InitializeLogic() override;


		private:
			/** 陣形アイコンのスライド演出込みの表示更新 */
			void UpdateFormationIcons();

			/** ウルト発動可否に応じてLT/RTアイコンの色を切り替える */
			void UpdateUltIconColor();

			/** 陣形レベルと次のレベルアップまでの隊員数ゲージの表示を更新する */
			void UpdateLevelDisplay();

			/**
			 * @brief 陣形切り替え開始時に、各陣形種別の遷移前スロット位置を確定する
			 * @param oldType 切り替え前の陣形種別
			 * @param newType 切り替え後の陣形種別
			 */
			void BeginTransition(actor::EnFormationType oldType, actor::EnFormationType newType);

			/**
			 * @brief スロット位置(連続値)からX座標を算出する
			 * @param slot スロット位置
			 * @return X座標
			 */
			float CalculateSlotX(float slot) const;
			/**
			 * @brief スロット位置(連続値)からアイコンサイズを算出する（中央=大、隣接=小）
			 * @param slot スロット位置
			 * @return アイコンサイズ
			 */
			float CalculateSlotSize(float slot) const;
			/**
			 * @brief スロット位置(連続値)からアルファ値を算出する（中央=不透明、離れるほど透明）
			 * @param slot スロット位置
			 * @return アルファ値
			 */
			float CalculateSlotAlpha(float slot) const;

			/**
			 * @brief ドクンドクンと拡縮するパルス演出の倍率を計算する
			 * @param isPulsing 演出中かどうか(falseなら等倍を返す)
			 * @return 拡縮倍率(等倍=1.0を中心に振動する)
			 */
			float CalculatePulseScale(bool isPulsing) const;

			/**
			 * @brief チューニングJSONを読み込む（初回読み込み）
			 * @details FormationWheelTuning.jsonから見た目パラメーターを読み込む
			 */
			void LoadTuning();
			/**
			 * @brief チューニングJSONが変更されていれば再読み込みする
			 * @param dt 前フレームからの経過時間
			 */
			void ReloadTuningIfChanged(float dt);


		private:
			/** 前フレームの陣形種別（切り替え開始検知・遷移前スロット計算に使用） */
			actor::EnFormationType m_lastFrameType = actor::EnFormationType::Circle;
			/** 前フレームで切り替え演出中だったか（開始エッジ検知用） */
			bool m_wasSwitching = false;
			/** 切り替え方向（+1: 次へ / -1: 前へ） */
			int m_direction = 1;
			/** 各陣形種別(EnFormationTypeの値)ごとの遷移前スロット位置（-2〜2） */
			int m_fromSlot[4] = { 0, 1, 2, -1 };

			/** FormationWheelTuning.json からホットリロードされる見た目パラメーター */
			float m_centerX = 640.0f;  /** 中央X座標 */
			float m_rowY = -80.0f;  /** 中央Y座標 */
			float m_slotSpacing = 80.0f;   /** スロット間のX座標間隔 */
			float m_currentSize = 80.0f;   /** 中央アイコンのサイズ */
			float m_sideSize = 50.0f;   /** 隣接アイコンのサイズ */
			float m_sideAlpha = 0.78f;   /** 隣接アイコンのアルファ値 */
			Vector3 m_iconColor = Vector3(255.0f, 255.0f, 255.0f);   /** アイコンのRGB色 */
			float m_pulseAmplitude = 0.15f;   /** パルス演出の振幅(等倍1.0に対する割合) */
			float m_pulseSpeed = 6.0f;    /** パルス演出の速さ(ラジアン/秒) */

			/** パルス演出用の経過時間(常時加算し続ける) */
			float m_pulseTimer = 0.0f;

#if defined(APP_DEBUG)
			time_t m_tuningLastWriteTime = 0;    /** チューニングJSONの最終更新日時 */
			float  m_tuningReloadTimer = 0.0f; /** チューニングJSONの変更チェック用タイマー */
#endif

			/** ゲーム開始時に画面外右からスライドインさせる演出 */
			InGameStartingAnimLogic m_startingAnimLogic;
		};
	}
}
