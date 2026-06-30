/**
 * @file FormationController.h
 * @brief 陣形の切り替えと座標計算を管理するコントローラー
 * @author 竹林
 */
#pragma once
#include <array>
#include <functional>
#include <memory>
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguinTypes.h"


namespace app
{
	namespace actor
	{
		class IFormation;


		/**
		 * @brief 陣形コントローラー
		 * @details
		 *   IFormation の各実装を保持し、現在の陣形への切り替えと
		 *   座標計算の委譲を担う。レベル管理もここで行う。
		 *   ChildPenguinManager のメンバーとして所有される。
		 */
		class FormationController
		{
		public:
			FormationController();
			~FormationController();


			/**
			 * @brief 陣形座標を計算する（現在の陣形に委譲）
			 * @details レベルが上昇した場合は m_onLevelUp を呼び出す。
			 * @param center         親ペンギンの座標
			 * @param forward        親ペンギンの前方向（正規化済み）
			 * @param out            計算結果を書き込むベクター（事前にclearしておくこと）
			 * @param count          位置を生成するスロット数（m_outerRadius の算出に使用）
			 * @param countForLevel  レベル判定に使うフォロワー数。-1 の場合は count を使う
			 */
			void CalculatePositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int count,
				int countForLevel = -1
			);

			/**
			 * @brief 現在充填中のリングの全スロット座標を計算する（表示専用・状態を変化させない）
			 * @details m_outerRadius を元の値に復元するため、レベルアップ判定もしない。
			 * @param center   親ペンギンの座標
			 * @param forward  親ペンギンの前方向（正規化済み）
			 * @param out      計算結果を書き込むベクター（事前にclearしておくこと）
			 * @param occupied 現在のフォロワー数（空きスロットのフィルタリングに使用）
			 */
			void CalculateNextLevelPositions(
				const Vector3& center,
				const Vector3& forward,
				std::vector<Vector3>& out,
				int occupied
			);

			/**
			 * @brief 陣形を切り替える
			 * @param type 切り替え先の陣形
			 */
			void SwitchFormation(EnFormationType type);

			/**
			 * @brief 現在の陣形種別を取得する
			 */
			EnFormationType GetCurrentType() const { return m_currentType; }

			/**
			 * @brief 移動速度倍率を取得する
			 */
			float GetSpeedMultiplier() const;

			/**
			 * @brief 最外半径を取得する（CalculatePositions後に有効）
			 */
			float GetOuterRadius()  const;

			/**
			 * @brief 入隊判定半径を取得する（最外半径 + 入隊マージン）
			 */
			float GetJoinRadius()   const;

			/**
			 * @brief 陣形レベルを取得する（フォロワー数 / FOLLOWERS_PER_LEVEL）
			 */
			int GetFormationLevel() const { return m_formationLevel; }

			/**
			 * @brief レベルアップ時のコールバックを設定する
			 * @param callback 引数: 新しいレベル
			 */
			void SetOnLevelUp(std::function<void(int)> callback) { m_onLevelUp = std::move(callback); }


		private:
			static constexpr int FOLLOWERS_PER_LEVEL = 9;  /** レベルアップに必要なフォロワー数 */

			std::array<
				std::unique_ptr<IFormation>,
				static_cast<size_t>(EnFormationType::Num)> m_formations;	/** 陣形の種類ごとのインスタンスを保持する配列 */

			IFormation*     m_currentFormation = nullptr;					/** 現在の陣形インスタンスへのポインタ */
			EnFormationType m_currentType      = EnFormationType::Circle;	/**< 現在の陣形種別 */
			int             m_formationLevel   = 0;							/** 現在の陣形レベル */

			std::function<void(int)> m_onLevelUp;  /**< レベルアップ時のコールバック */
		};
	}
}
