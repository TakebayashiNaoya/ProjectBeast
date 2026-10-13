/**
 * @file HomingParticleRender.cpp
 * @brief ターゲット座標へ向かって移動するパーティクルレンダラー
 */
#include "stdafx.h"
#include "HomingParticleRender.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"


namespace app
{
	namespace
	{
		/** jsonファイルパス */
		constexpr const char* JSON_PATH = "Assets/parameter/UI/vfx/remainParticleEffect/remainEffectFlash.json";
		/** テクスチャファイルパス */
		constexpr const char* TEXTURE_PATH = "Assets/effect/vfx/Texture/crossFlashTexture.dds";
		/** 幅 */
		constexpr float WIDTH = 80.0f;
		/** 高さ */
		constexpr float HEIGHT = 80.0f;
		/** 持続時間 */
		constexpr float DURATION = 1.0f;
		/** 同時に飛ばすパーティクルの最大数 */
		constexpr int MAX_PARTICLE_COUNT = 5;
	}


	HomingParticleRender::HomingParticleInfo::HomingParticleInfo()
		: curve()
		, startPosition(Vector3::Zero)
		, targetActor(nullptr)
		, effectRender(nullptr)
	{}


	HomingParticleRender::HomingParticleRender()
		: m_goalPosition(Vector3::Zero)
		, m_particleCount(0)
		, m_maxParticles(MAX_PARTICLE_COUNT)
	{}


	void HomingParticleRender::Initialize()
	{
		// リストのメモリ確保だけを行う。
		m_homingParticles.reserve(m_particleCount);

		// パーティクルエフェクトレンダーの生成は重い（JSON読み込み・スプライト初期化）ため、
		// 入隊のたびに作るとその瞬間だけ処理が重くなる。最大数ぶんをここで作っておき使い回す。
		m_freeEffectRenders.reserve(m_maxParticles);
		for (int i = 0; i < m_maxParticles; ++i)
		{
			auto effectRender = std::make_unique<ParticleEffectRender>();
			effectRender->Init(
				JSON_PATH,
				TEXTURE_PATH,
				WIDTH,
				HEIGHT
			);
			m_freeEffectRenders.push_back(std::move(effectRender));
		}
	}


	void HomingParticleRender::AddTarget(actor::ChildPenguin* target)
	{
		K2_ASSERT(!m_goalPosition.IsEqual(Vector3::Zero), "目標座標未設定");

		// すでにホーミングパーティクルが最大数存在している（空きが無い）場合は、これ以上追加しない。
		if (m_homingParticles.size() >= m_maxParticles || m_freeEffectRenders.empty()) return;

		auto info = std::make_unique<HomingParticleInfo>();

		info->targetActor = target;

		// 空いているレンダーを取り出し、生成直後と同じ状態（停止・パーティクル無し）に戻して使う。
		info->effectRender = std::move(m_freeEffectRenders.back());
		m_freeEffectRenders.pop_back();
		info->effectRender->Reset();

		// 画面上の座標を取得
		Vector2 screenPosition = Vector2::Zero;
		CameraSystem::Get().GetMainCamera()
			.CalcScreenPositionFromWorldPosition(
				screenPosition, target->GetTransform().m_position
			);
		// Z座標は0にしておく（UI上での描画なので）
		info->startPosition = Vector3(screenPosition.x, screenPosition.y, 0.0f);

		info->curve.Initialize(
			info->startPosition,
			m_goalPosition,
			DURATION,
			util::EasingType::EaseInOut,
			util::LoopMode::Once
		);

		// パーティクルの初期位置を設定する。
		info->effectRender->SetPosition(info->startPosition);

		// カーブの再生を開始する。
		info->curve.Play();
		// パーティクルの再生を開始する。
		info->effectRender->Play();

		m_homingParticles.push_back(std::move(info));
	}


	void HomingParticleRender::Render(RenderContext& rc)
	{
		for (auto& info : m_homingParticles)
		{
			info->effectRender->Draw(rc);
		}
	}


	void HomingParticleRender::Update()
	{
		const float deltaTime = g_gameTime->GetFrameDeltaTime();

		// イテレーターを使って、安全に削除しながらループする。
		for (auto it = m_homingParticles.begin(); it != m_homingParticles.end();)
		{
			auto& info = *it;
			info->curve.Update(deltaTime);

			// パーティクルの位置をカーブの現在値に更新する。
			const Vector3 currentCenterPos = info->curve.GetCurrentValue();
			// 古い位置を取得。
			const Vector3 oldPos = info->effectRender->GetEmitter().GetPosition();
			// 位置の差分を計算。
			Vector3 deltaPos = currentCenterPos - oldPos;

			// ParticleEffectRenderのエミッターに平行移動を加える。
			// detail: SetPosition()で直接座標を設定すると、エミッターの内部状態がリセットされてしまうため、AddPositionOffset()で差分を加えるようにした。
			info->effectRender->GetEmitter().AddPositionOffset(deltaPos);
			info->effectRender->Update(deltaTime);

			if (!info->curve.IsPlaying())
			{
				// 飛び終わったレンダーは使い回すために戻す。
				m_freeEffectRenders.push_back(std::move(info->effectRender));
				// eraseは削除した次の要素のイテレーターを返すので、安全にループできる。
				it = m_homingParticles.erase(it);
			}
			else
			{
				// まだ再生中なら、次の要素に進む。
				++it;
			}
		}
	}
}