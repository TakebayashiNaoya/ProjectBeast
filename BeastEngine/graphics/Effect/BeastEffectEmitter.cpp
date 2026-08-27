/**
 * @file BeastEffectEmitter.cpp
 * @brief フラスタムカリング対応のエフェクトエミッタークラスの実装
 */
#include "BeastEnginePreCompile.h"
#include "graphics/effect/BeastEffectEmitter.h"


namespace nsBeastEngine
{
	void BeastEffectEmitter::Update()
	{
		m_effect.Update();

		if (!IsExist())
		{
			DeleteGO(this);
		}
	}


	void BeastEffectEmitter::SetVisible(const bool visible)
	{
		EffectEngine::GetInstance()->SetShown(m_effect.GetHandle(), visible);
	}


	bool BeastEffectEmitter::IsExist() const
	{
		return EffectEngine::GetInstance()->IsExist(m_effect.GetHandle());
	}
} // namespace nsBeastEngine
