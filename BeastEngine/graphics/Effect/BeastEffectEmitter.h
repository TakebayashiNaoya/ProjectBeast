/**
 * @file BeastEffectEmitter.h
 * @brief フラスタムカリング対応のエフェクトエミッタークラス
 */
#pragma once
#include "graphics/effect/EffectEmitter.h"


namespace nsBeastEngine
{
	/**
	 * @brief フラスタムカリング対応のエフェクトエミッタークラス
	 * @details
	 *   k2EngineLow の EffectEmitter を継承し、SetVisible() と IsExist() を追加する。
	 *   SetVisible(false) で Effekseer の描画のみを抑制し、再生は継続する。
	 *   IsExist() で再生継続を判定するため、SetVisible(false) 中も自己削除されない。
	 */
	class BeastEffectEmitter : public nsK2EngineLow::EffectEmitter
	{
	public:
		BeastEffectEmitter() = default;
		~BeastEffectEmitter() override = default;

		/**
		 * @brief 更新処理
		 * @details
		 *   IsPlay()（GetShown ベース）ではなく IsExist() で再生継続を判定する。
		 *   SetVisible(false) でカリングされていても自己削除されない。
		 */
		void Update() override;

		/**
		 * @brief エフェクトの描画可否を設定する
		 * @details
		 *   Effekseer::Manager::SetShown() を呼び出して描画のみを制御する。
		 *   再生自体は継続するため、SetVisible(true) で即座に復元できる。
		 * @param visible trueで描画する、falseで描画を抑制する
		 */
		void SetVisible(const bool visible);

		/**
		 * @brief エフェクトが存在（再生継続）しているか判定する
		 * @details
		 *   Effekseer::Manager::Exists() を使用する。
		 *   GetShown() を使う IsPlay() と異なり、SetVisible(false) の影響を受けない。
		 * @return 再生継続中であればtrue
		 */
		bool IsExist() const;
	};


} // namespace nsBeastEngine
