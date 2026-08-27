/**
 * @file InGameSceneBase.cpp
 * @brief インゲームシーン基底クラス
 */
#include "stdafx.h"
#include "InGameSceneBase.h"
#include "ResultScene.h"
#include "Source/Core/ParameterManager.h"
#include "Source/Graphics/PBRStatus.h"

#include "Source/Sound/SoundManager.h"

#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Enemy/EnemyController.h"
#include "Source/Actor/Character/Enemy/EnemyManager.h"
#include "Source/Actor/Character/Enemy/EnemyStateMachine.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguin.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinManager.h"
#include "Source/Actor/Character/penguin/childPenguin/ChildPenguinStateMachine.h"
#include "Source/Actor/Character/penguin/daddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/penguin/daddyPenguin/DaddyPenguinController.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Camera/CameraController.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Noise/NoiseManager.h"
#include "Source/Util/JsonConverter.h"

#include "Source/Manager/BattleManager.h"
#include "Source/Manager/FeverTimeManager.h"
#include "Source/Manager/IglooManager.h"
#include "Source/Manager/InGameUIManager.h"
#include "Source/Manager/ScoreManager.h"
#include "Source/Manager/TimeManager.h"
#include "Source/UI/Fever/FeverIconMenu.h"

#include "Source/Achivement/AchievementManager.h"

#include "Source/UI/Menus/CountDownMenu.h"
#include "Source/UI/Menus/FinishMenu.h"
#include "Source/UI/Menus/PauseScreenMenu.h"
#include "Source/UI/Menus/SoundOptionMenu.h"
#include "Source/UI/Menus/TutorialMenu.h"

#include "Graphics/Camera/SubCameraManager.h"
#include "Source/Scene/SceneManager.h"
#include "TitleScene.h"

#include <thread>

#include "Source/Effect/DecalManager.h"
#include "Source/Nature/Ocean.h"
#include "Source/Nature/WhirlpoolManager.h"


namespace
{
	const Vector3 SKY_CUBE_SCALE = Vector3(1000.0f, 800.0f, 1000.0f);

	/** シーン遷移までの待機時間（秒） */
	constexpr float SCENE_TRANSITION_WAIT_TIME = 0.5f;

	/** FINISH演出に合わせたBGMフェードアウト時間（秒） */
	constexpr float FINISH_BGM_FADE_DURATION = 3.0f;

	/** 足跡デカールのロード中ウォームアップで1フレームに初期化する枚数。
	 *  1枚8ms前後かかるため、ロード画面のアニメが固まらない程度に刻む */
	constexpr int DECAL_PREWARM_PER_FRAME = 6;

	/** ミニマップアイコンのロード中分割生成で1フレームに作る個数。
	 *  アイコンはフィーバー予約分も含め数百個あり、一括生成は1秒以上フレームが止まる */
	constexpr int MAP_ICON_INIT_PER_FRAME = 16;

	/** ロードフェーズ名（LoadPhase の並び順と一致させること。ロード時間トレース用） */
	constexpr const char* LOAD_PHASE_NAMES[] = {
		"None", "Stage", "StageWait", "DecalPrewarm", "Daddy",
		"Children", "Enemy", "Camera", "Ocean", "MapIcon", "Done"
	};

	/** ロードの1フレームがこの時間を超えたらトレースへ記録する（ミリ秒）。
	 *  ローディングアイコンはフレームが回らないと止まるため、犯人特定に使う */
	constexpr double LOAD_TICK_LOG_THRESHOLD_MS = 50.0;

	//============================================//
	// 衝撃演出のつまみ（BattleManager::NotifyImpact の受け口で使う）
	//
	// 揺れは 2026-08-25 の酔い対策で全体を一段下げてある。
	// 揺れ自体も滑らかなノイズ方式（GameCamera::StartShake）に変更済み。
	//============================================//

	/** 咆哮の画面演出を出す最大距離。
	 *  遠くの無関係な咆哮で画面がボケると理不尽なため、ここより遠い咆哮は無視する */
	constexpr float ROAR_EFFECT_MAX_DISTANCE = 900.0f;
	/** 咆哮のラジアルブラーの長さ・ピークまでの時間・最小強度（秒／秒／0〜1） */
	constexpr float ROAR_BLUR_DURATION = 1.0f;
	constexpr float ROAR_BLUR_ATTACK_TIME = 0.3f;
	constexpr float ROAR_BLUR_MIN_STRENGTH = 0.3f;
	/** 咆哮の画面揺れ（至近時の最大振幅・秒）。ラジアルブラーも重なるため特に控えめ */
	constexpr float ROAR_SHAKE_STRENGTH = 6.0f;
	constexpr float ROAR_SHAKE_DURATION = 0.4f;

	/** かまくら崩壊の画面揺れ（振幅・秒） */
	constexpr float IGLOO_BREAK_SHAKE_STRENGTH = 10.0f;
	constexpr float IGLOO_BREAK_SHAKE_DURATION = 0.4f;

	/** 弾き返しのヒットストップ（時間倍率・実時間秒）と画面揺れ（振幅・秒） */
	constexpr float NULLIFY_HITSTOP_SCALE = 0.4f;
	constexpr float NULLIFY_HITSTOP_DURATION = 0.15f;
	constexpr float NULLIFY_SHAKE_STRENGTH = 8.0f;
	constexpr float NULLIFY_SHAKE_DURATION = 0.25f;

	/** ウルト発動のヒットストップ（時間倍率・実時間秒） */
	constexpr float ULT_HITSTOP_SCALE = 0.4f;
	constexpr float ULT_HITSTOP_DURATION = 0.3f;
	/** ウルト発動のラジアルブラー（強度・ピークまでの時間・長さ）。強さは咆哮の半分程度に抑える */
	constexpr float ULT_BLUR_STRENGTH = 0.5f;
	constexpr float ULT_BLUR_ATTACK_TIME = 0.12f;
	constexpr float ULT_BLUR_DURATION = 0.5f;
	/** ウルト発動のパンチイン（詰める割合・秒） */
	constexpr float ULT_PUNCH_IN_AMOUNT = 0.07f;
	constexpr float ULT_PUNCH_IN_DURATION = 0.3f;

	/**
	 * @brief BGMのWAVをバックグラウンドで先読みしてOSのファイルキャッシュへ乗せる
	 * @details インゲームBGM(16MB)は初回再生の同期ロードでロード画面末尾を約1.3秒止め、
	 *          フィーバーBGM(30MB)はフィーバー開始をカクつかせる。
	 *          ロード開始と同時に純粋なファイル読みだけを別スレッドで済ませておけば、
	 *          本番の同期ロードはキャッシュヒットになり一瞬で終わる。
	 *          エンジンには一切触れないのでスレッド安全。プロセスにつき1回でよい
	 */
	void PrefetchBgmFiles()
	{
		static bool s_isPrefetched = false;
		if (s_isPrefetched) return;
		s_isPrefetched = true;

		std::thread([] {
			const char* paths[] = {
				"Assets/sound/BGM/inGame.wav",
				"Assets/sound/BGM/FeverTime.wav",
			};
			std::vector<char> buffer;
			for (const char* path : paths)
			{
				FILE* fp = nullptr;
				if (fopen_s(&fp, path, "rb") != 0 || fp == nullptr) continue;
				fseek(fp, 0, SEEK_END);
				const long size = ftell(fp);
				fseek(fp, 0, SEEK_SET);
				if (size > 0)
				{
					buffer.resize(static_cast<size_t>(size));
					fread(buffer.data(), 1, buffer.size(), fp);
				}
				fclose(fp);
			}
		}).detach();
	}

	/** ステージ紹介動画の撮影モード（環境変数 BEAST_SHOWCASE=Easy|Normal|Hard）が有効かどうか。
	 *  UIを消してショーケースカメラで周回し、一定時間で自動終了する */
	bool IsShowcaseEnabled()
	{
		char buf[16];
		size_t len = 0;
		return getenv_s(&len, buf, sizeof(buf), "BEAST_SHOWCASE") == 0 && len > 0;
	}

	/** 撮影モードの自動終了時間（秒）。ループ検出用に2周分（66秒）撮る余白を含む。
	 *  環境変数 BEAST_SHOWCASE_TIME（秒）で上書きできる（観察デバッグ用に長くする等） */
	float GetShowcaseQuitTime()
	{
		char buf[16];
		size_t len = 0;
		if (getenv_s(&len, buf, sizeof(buf), "BEAST_SHOWCASE_TIME") == 0 && len > 0)
		{
			const float t = static_cast<float>(atof(buf));
			if (t > 0.0f) return t;
		}
		return 90.0f;
	}

	/** 親ペンギンのスポーン時に地面から浮かせる高さ。
	 *  カウントダウン中は物理が止まるため、ほぼ接地した高さに置く */
	constexpr float DADDY_SPAWN_GROUND_OFFSET = 2.0f;

	// デバイスロスト調査用の一時計測。VRAM使用量を Logs/vram_trace.txt へ追記する。
	// ゲームループの周回でVRAMが積み上がっていないかをステージの開始・終了時点で比較する
	void LogVramUsage(const char* tag)
	{
		double usageMB = 0.0;
		double budgetMB = 0.0;
		g_graphicsEngine->QueryVideoMemoryMB(usageMB, budgetMB);

		char buf[512];
		sprintf_s(buf, "[VRAM] %-16s usage %.1f MB / budget %.1f MB heaps %d\n",
			tag, usageMB, budgetMB, nsK2EngineLow::g_numDescriptorHeapLive);
		OutputDebugStringA(buf);

		FILE* fp = nullptr;
		fopen_s(&fp, "Logs/vram_trace.txt", "a");
		if (fp)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			fprintf(fp, "%04d-%02d-%02d %02d:%02d:%02d %s",
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, buf);
			fclose(fp);
		}
	}
}

namespace app
{
	InGameSceneBase::InGameSceneBase()
	{}


	InGameSceneBase::~InGameSceneBase()
	{
#ifdef DEBUG
		// デバッグ描画を止めてから破棄する（破棄済みShapeへのアクセスを防ぐ）
		nsBeastEngine::nsCollision::PhysicsWorld::Get().DisableDrawDebugWireFrame();
#endif
		/** UIManager（デストラクタでBattleManagerのfunctionもリセットされる） */
		InGameUIManager::DestroyInstance();

		/** アクター */
		delete m_daddyPenguin;
		actor::EnemyManager::DestroyInstance();
		actor::ChildPenguinManager::DestroyInstance();

		/** デカールの後始末。StageSystem（地形）の破棄より前に行うこと。
		 *  デカールは地形ハイトマップのSRVを持っているため、地形破棄後に
		 *  描画されると解放済みテクスチャをGPUが読んでデバイスハングする */
		effect::DecalManager::Get().OnStageChanged();

		actor::StageSystem::DestroyInstance();
		nature::WhirlpoolManager::DestroyInstance();
		actor::IglooManager::DestroyInstance();

		/** PBRステータス */
		graphics::PBRStatus::DestroyInstance();

		DeleteGO(m_skyCube);
		nature::Ocean::DestroyInstance();

		/** マネージャー */
		BattleManager::DestroyInstance();
		ScoreManager::DestroyInstance();
		TimeManager::DestroyInstance();
		FeverTimeManager::DestroyInstance();

		/** プレイログ。最後まで遊んだ場合は書き出し時に破棄済みだが、
		 *  途中でタイトルへ戻った場合はここまで残っている。破棄しないと
		 *  CreateInstance() が既存インスタンスを再利用してしまい、
		 *  中断したプレイのログが次のプレイへ合流してしまう */
		GameLogManager::DestroyInstance();

		/** シーン破棄時にサブカメラを強制停止する（タイトル遷移後に残らないよう） */
		nsBeastEngine::SubCameraManager::Get().ForceEnd();

		/** タイトルへ戻る場合は ResultScene を経由しないため、ここで破棄する。
		 *  リザルト遷移の場合は ResultScene::~ResultScene が担当する。 */
		if (m_goTitle) {
			if (app::achievement::AchievementManager::GetInstance()) {
				app::achievement::AchievementManager::DestroyInstance();
			}
		}

		/** 2周目以降のために GameCamera の登録を解除する。
		 *  次の LoadPhase::Camera で新しいインスタンスを Register できるようにする。 */
		camera::CameraManager::Get().Unregister(camera::GameCamera::ID());
		camera::CameraManager::Get().Unregister(camera::ShowcaseCamera::ID());

		// デバイスロスト調査：シーン破棄後のVRAM使用量を記録する
		// （D3D12オブジェクトは1フレーム遅延解放なので、実際の解放は次フレーム反映）
		LogVramUsage("scene end");
	}


	bool InGameSceneBase::Start()
	{
		// デバイスロスト調査：ステージ開始時点のVRAM使用量を記録する
		char vramTag[128];
		sprintf_s(vramTag, "start %s", GetStageJsonPath());
		LogVramUsage(vramTag);

		/** マネージャー生成 */
		app::core::ParameterManager::CreateInstance();
		BattleManager::CreateInstance();
		ScoreManager::CreateInstance();
		TimeManager::CreateInstance();
		FeverTimeManager::CreateInstance();

		/** ステージ固有の制限時間を設定する */
		/** 撮影・観察モード中は制限時間を実質無効化する
		 *  （観察が長引いてもFINISHで打ち切られないように） */
		TimeManager::GetInstance().SetMaxTime(IsShowcaseEnabled() ? 6000.0f : GetTimeLimit());
		TimeManager::GetInstance().ResetTime();

		/** リザルトでのハイスコア保存先として、プレイ中のステージ名を控えておく */
		ScoreManager::SetLastPlayedStage(GetStageName());

		/** BGMのWAV先読みを開始する（ロード末尾とフィーバー開始の同期ロード対策） */
		PrefetchBgmFiles();

		/** フィーバータイムの設定を読み込む */
		FeverTimeManager::GetInstance()->Start(GetFeverParameterJsonPath());

		app::achievement::AchievementManager::CreateInstance();
		app::achievement::AchievementManager::GetInstance()->Start(GetAchievementJsonPath());

		GameLogManager::CreateInstance();

		/** PBRStatus生成 */
		graphics::PBRStatus::CreateInstance();

		/** アクター系シングルトン生成 */
		actor::StageSystem::CreateInstance();
		actor::ChildPenguinManager::CreateInstance();
		actor::EnemyManager::CreateInstance();
		actor::IglooManager::CreateInstance();

		/** UIManager生成（Layoutの生成はDaddyPenguin生成後のInitializeで行う） */
		InGameUIManager::CreateInstance();
		InGameUIManager::GetInstance()->Initialize();

		/** ロードフェーズ開始 */
		m_loadPhase = LoadPhase::Stage;
		m_childIndex = 0;

		return true;
	}


	void InGameSceneBase::Update()
	{
		/** ステージ紹介動画の撮影モード：ロード完了を合図ファイルで知らせ、
		 *  UIを消して一定時間後に自動終了する（キャプチャスクリプトが合図を待つ） */
		if (IsShowcaseEnabled() && IsLoaded())
		{
			if (!m_isShowcaseStarted)
			{
				m_isShowcaseStarted = true;
				nsBeastEngine::g_renderingEngine->Set2DRenderEnabled(false);

				/** カウントダウンを飛ばしてすぐ世界を動かす。
				 *  物理停止中の「宙に浮いた絵」を映さないため。
				 *  通常の遷移（CountDown終了時）と同じ有効化を行う */
				m_gamePhase = GamePhase::Playing;
				BattleManager::GetInstance().SetIsActive(true);

				FILE* fp = nullptr;
				fopen_s(&fp, "Logs/showcase_ready.txt", "w");
				if (fp)
				{
					fprintf(fp, "%s\n", GetStageName());
					fclose(fp);
				}
			}

			m_showcaseTimer += g_gameTime->GetFrameDeltaTime();
			if (m_showcaseTimer >= GetShowcaseQuitTime())
			{
				PostQuitMessage(0);
				return;
			}
		}

		//------------------------------------------------------------
		// ロードフェーズ
		//------------------------------------------------------------
		// ロード中に1フレームを長時間ブロックしたフェーズを記録する
		// （ローディングアイコンが止まる原因の特定用）
		const LoadPhase tickPhase = m_loadPhase;
		LARGE_INTEGER tickBegin;
		QueryPerformanceCounter(&tickBegin);

		switch (m_loadPhase)
		{
		case LoadPhase::Stage:
		{
			actor::StageSystem::GetInstance()->LoadStageObjectsFromJson(GetStageJsonPath());
			if (const char* terrainPath = GetTerrainJsonPath())
			{
				actor::StageSystem::GetInstance()->InitTerrainFromJson(terrainPath);
			}
			m_loadPhase = LoadPhase::StageWait;
			break;
		}

		case LoadPhase::StageWait:
		{
			/** ステージの非同期モデルロードと物理コリジョン登録が完了するまで待つ */
			auto* system = actor::StageSystem::GetInstance();

			system->Update();
			if (system->IsAllLoaded())
			{
				/** イグルー・クマの巣を実際の地形メッシュの高さへ接地させる
				 *  （配置JSONのYは生成スクリプトの近似値なので、ここで正確な高さに直す） */
				system->SnapObjectsToTerrain();

				// イグルーとクマの巣の数をミニマップへ登録
				const uint8_t iglooCount = system->GetNumbaringObjectCount("igloo");
				const uint8_t bearHomeCount = system->GetNumbaringObjectCount("bearHome");

				InGameUIManager::GetInstance()->SetMiniMapIconNum(ui::EnMiniMapIconType::Igloo, iglooCount);
				InGameUIManager::GetInstance()->SetMiniMapIconNum(ui::EnMiniMapIconType::BearNest, bearHomeCount);

				m_loadPhase = LoadPhase::DecalPrewarm;
			}
			break;
		}

		case LoadPhase::DecalPrewarm:
		{
			/** 足跡デカールの全スロットを今の地形で事前初期化する。
			 *  プレイ中の初回スポーンに任せると1枚8ms前後の初期化がヒッチになるため、
			 *  ロード中に少しずつ済ませる（一括だと約2秒ロード画面が固まる） */
			if (effect::DecalManager::Get().PrewarmPoolsStep(DECAL_PREWARM_PER_FRAME))
			{
				m_loadPhase = LoadPhase::Daddy;
			}
			break;
		}

		case LoadPhase::Daddy:
		{
			m_daddyPenguin = new actor::DaddyPenguin();

			/** スポーン地点を地面へ吸着させる。
			 *  イグルーと同じく、描画メッシュと同一の高さ場（GetHeightAt）を使う。
			 *  物理レイキャストはコリジョン登録タイミングに依存して外れることがあり、
			 *  外れると高いYのままカウントダウン中（物理停止）に宙に浮いて見える */
			Vector3 daddySpawnPos = GetDaddySpawnPos();
			if (auto* terrain = actor::StageSystem::GetInstance()->GetTerrain())
			{
				daddySpawnPos.y = terrain->GetHeightAt(daddySpawnPos) + DADDY_SPAWN_GROUND_OFFSET;
			}
			m_daddyPenguin->SetPosition(daddySpawnPos);
			m_daddyPenguin->StartWrapper();

			// DaddyPenguinをディザリングのプレイヤーターゲットとして登録する
			// DaddyPenguinのモデルはデプス描画の対象になり、遮蔽対象リストには含まれない
			nsBeastEngine::OcclusionDitherManager::Get().SetPlayerTarget(
				&m_daddyPenguin->GetModelRender()
			);


			BattleManager::GetInstance().SetDaddyPenguin(m_daddyPenguin);
			InGameUIManager::GetInstance()->RegisterObservers();
			m_daddyPenguin->GetController()->SetIglooPromptMenu(
				InGameUIManager::GetInstance()->GetIglooPromptMenu()
			);

			m_loadPhase = LoadPhase::Children;
			break;
		}
		case LoadPhase::Children:
		{
			/** ステージ固有の生成設定を取得して一括生成 */
			const PenguinSpawnConfig cfg = GetPenguinConfig();
			actor::ChildPenguinManager::GetInstance()->CreateChildPenguins(
				cfg.serious,
				cfg.clingy,
				cfg.naughty,
				cfg.clumsy,
				cfg.caring,
				cfg.spawnRadius,
				cfg.groundRayStartY
			);

			/** フィーバーで降ってくる分のミニマップアイコン枠をあらかじめ確保する
			 *  （SpawnFromSkyは初期数が0のタイプを選ばないため、0のタイプには足さない） */
			const int feverExtra = FeverTimeManager::GetInstance()->GetFeverDropCount();
			auto* ui = InGameUIManager::GetInstance();
			ui->SetMiniMapIconNum(ui::EnMiniMapIconType::Serious, cfg.serious > 0 ? cfg.serious + feverExtra : 0);
			ui->SetMiniMapIconNum(ui::EnMiniMapIconType::Clingy, cfg.clingy > 0 ? cfg.clingy + feverExtra : 0);
			ui->SetMiniMapIconNum(ui::EnMiniMapIconType::Naughty, cfg.naughty > 0 ? cfg.naughty + feverExtra : 0);
			ui->SetMiniMapIconNum(ui::EnMiniMapIconType::Clumsy, cfg.clumsy > 0 ? cfg.clumsy + feverExtra : 0);
			ui->SetMiniMapIconNum(ui::EnMiniMapIconType::Caring, cfg.caring > 0 ? cfg.caring + feverExtra : 0);

			auto* manager = actor::ChildPenguinManager::GetInstance();
			manager->SetDaddyPenguin(m_daddyPenguin);
			manager->Start();

			/** ステージ上の総ペンギン数をセット */
			const int totalNum = cfg.serious + cfg.clingy + cfg.naughty + cfg.clumsy + cfg.caring;
			ScoreManager::GetInstance().SetTotalCount(totalNum);

			m_loadPhase = LoadPhase::Enemy;
			break;
		}

		case LoadPhase::Enemy:
		{
			nlohmann::json json;
			util::JsonConverter::IsLoadJsonFile(json, GetEnemyJsonPath());
			actor::EnemyManager::GetInstance()->LoadEnemies(json);

			const auto& enemies = actor::EnemyManager::GetInstance()->GetEnemies();

			/** エネミー1体につき探索UIを1つ生成 */
			for (auto* enemy : enemies)
			{
				InGameUIManager::GetInstance()->AddSearchLayout(enemy);
			}

			InGameUIManager::GetInstance()->InitializeReactionSystem(
				actor::EnemyManager::GetInstance()->GetEnemies().size()
			);

			InGameUIManager::GetInstance()->SetMiniMapIconNum(ui::EnMiniMapIconType::Bear, enemies.size());

			m_loadPhase = LoadPhase::Camera;
			break;
		}

		case LoadPhase::Camera:
		{
			camera::CameraSteering::Config config;
			m_cameraSteering.SetConfig(config);
			m_cameraSteering.SetTargetCharacter(m_daddyPenguin);

			/** 2周目以降は Unregister 済みのため、毎回新しいインスタンスを登録できる */
			auto gameCamera = std::make_shared<camera::GameCamera>();
			camera::CameraManager::Get().Register(camera::GameCamera::ID(), gameCamera);
			camera::CameraManager::Get().SwitchCamera(camera::GameCamera::ID());

			/** 衝撃演出（揺れ・ブラー・ヒットストップ）の受け口を用意する。
			 *  カメラの生成後でないと登録しても揺らす相手がいない */
			RegisterImpactObserver();

			/** ステージ紹介動画の撮影モードでは、外周を周回するショーケースカメラに切り替える */
			if (IsShowcaseEnabled())
			{
				auto showcaseCamera = std::make_shared<camera::ShowcaseCamera>();
				camera::CameraManager::Get().Register(camera::ShowcaseCamera::ID(), showcaseCamera);
				camera::CameraManager::Get().SwitchCamera(camera::ShowcaseCamera::ID());
			}

			m_loadPhase = LoadPhase::Ocean;
			break;
		}

		case LoadPhase::Ocean:
		{
			/**
			 * NOTE:SkyCubeは後で生み出す場所を変えるかもしれない。
			 */
			m_skyCube = NewGO<SkyCube>(0);
			m_skyCube->SetType(enSkyCubeType_Clear);
			m_skyCube->SetScale(SKY_CUBE_SCALE);
			m_skyCube->SetLuminance(0.8f);

			nature::Ocean::CreateInstance();
			nature::Ocean::GetInstance()->Start(GetOceanParameterBinaryPath(), GetOceanParameterJsonPath());

			nature::WhirlpoolManager::CreateInstance();
			nature::WhirlpoolManager::GetInstance()->Start(
				GetWhirlpoolPositionsJsonPath(),
				GetWhirlpoolParameterBinaryPath()
			);

			InGameUIManager::GetInstance()->SetMiniMapIconNum(
				ui::EnMiniMapIconType::Whirlpool,
				nature::WhirlpoolManager::GetInstance()->GetWhirlpoolCountMax());

			m_loadPhase = LoadPhase::MapIcon;
			break;
		}

		case LoadPhase::MapIcon:
		{
			/** ミニマップのアイコン生成を分割実行する（一括だと1.3秒フレームが止まる） */
			if (!InGameUIManager::GetInstance()->InitializeMapIconStep(MAP_ICON_INIT_PER_FRAME))
			{
				break;
			}

			m_loadPhase = LoadPhase::Done;

			/** ロード完了 → カウントダウン開始 */
			if (InGameUIManager::GetInstance()->GetCountDownMenu())
			{
				InGameUIManager::GetInstance()->GetCountDownMenu()->SetIsDelay(true);
				SoundManager::Get().PlayBGM(enSoundKind_InGame, 0.5f);
			}

			/** どの配合・どのステージ設定で取ったログなのかをログ自身に残す。
			 *  配合違いのセッションを比較するとき、これが無いとログだけでは区別できない */
			if (auto* lm = GameLogManager::GetInstance())
			{
				const PenguinSpawnConfig cfg = GetPenguinConfig();
				auto* em = actor::EnemyManager::GetInstance();
				auto* fever = FeverTimeManager::GetInstance();
				const uint8_t whirlpoolCount = nature::WhirlpoolManager::GetInstance()->GetWhirlpoolCountMax();
				lm->SetStageConfig({
					{ "serious",          cfg.serious },
					{ "clingy",           cfg.clingy },
					{ "naughty",          cfg.naughty },
					{ "clumsy",           cfg.clumsy },
					{ "caring",           cfg.caring },
					{ "total",            cfg.serious + cfg.clingy + cfg.naughty + cfg.clumsy + cfg.caring },
					{ "spawn_radius",     cfg.spawnRadius },
					{ "time_limit_sec",   GetTimeLimit() },
					{ "bear_count",       em ? static_cast<int>(em->GetEnemies().size()) : 0 },
					{ "whirlpool_count",  static_cast<int>(whirlpoolCount) },
					{ "fever_drop_count", fever ? fever->GetFeverDropCount() : 0 }
				});
			}

			OnLoadComplete();
			break;
		}

		case LoadPhase::Done:
		{
			/** ポーズ終了後の初回フレームでポーズ入場フラグとサブビューをリセットする
			 *  通常ポーズ解除（IsRetry）と異なりチュートリアルポーズはここでリセットされる */
			if (m_isPauseEntered)
			{
				nsBeastEngine::SubCameraManager::Get().SetRenderingBlocked(false);
				m_isPauseEntered = false;
			}
			/** ゲームフェーズへ移譲 */
			UpdateGamePhase();
			break;
		}
		default:
			break;
		}

		if (tickPhase != LoadPhase::Done && tickPhase != LoadPhase::None)
		{
			LARGE_INTEGER tickEnd, tickFreq;
			QueryPerformanceCounter(&tickEnd);
			QueryPerformanceFrequency(&tickFreq);
			const double tickMs =
				1000.0 * (tickEnd.QuadPart - tickBegin.QuadPart) / tickFreq.QuadPart;
			if (tickMs > LOAD_TICK_LOG_THRESHOLD_MS)
			{
				FILE* fp = nullptr;
				if (fopen_s(&fp, "Logs/load_trace.txt", "a") == 0 && fp)
				{
					fprintf(fp, "%s %s: %.0f ms\n",
						GetStageName(), LOAD_PHASE_NAMES[static_cast<int>(tickPhase)], tickMs);
					fclose(fp);
				}
			}
		}
	}


	void InGameSceneBase::RegisterImpactObserver()
	{
		BattleManager::GetInstance().SetOnImpact(
			[](EnImpactType type, const Vector3& worldPosition)
			{
				auto& postEffect = nsBeastEngine::g_renderingEngine->GetPostEffectManager();
				auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(
					camera::GameCamera::ID()
				);

				switch (type)
				{
				case EnImpactType::BearRoar:
				{
					/** プレイヤーから遠い咆哮ほど弱め、一定距離より遠ければ何も出さない */
					Vector3 toDaddy = worldPosition
						- actor::ChildPenguinManager::GetInstance()->GetDaddyPosition();
					toDaddy.y = 0.0f;
					const float distance = toDaddy.Length();
					if (distance > ROAR_EFFECT_MAX_DISTANCE) return;

					const float closeness = 1.0f - (distance / ROAR_EFFECT_MAX_DISTANCE);
					const float blurStrength =
						ROAR_BLUR_MIN_STRENGTH + (1.0f - ROAR_BLUR_MIN_STRENGTH) * closeness;

					postEffect.GetRadialBlur().Start(
						blurStrength, ROAR_BLUR_ATTACK_TIME, ROAR_BLUR_DURATION);
					if (gameCamera)
					{
						gameCamera->StartShake(ROAR_SHAKE_STRENGTH * closeness, ROAR_SHAKE_DURATION);
					}
					break;
				}

				case EnImpactType::IglooBreak:
					if (gameCamera)
					{
						gameCamera->StartShake(IGLOO_BREAK_SHAKE_STRENGTH, IGLOO_BREAK_SHAKE_DURATION);
					}
					break;

				case EnImpactType::BearNullified:
					g_gameTime->StartSlowMotion(NULLIFY_HITSTOP_SCALE, NULLIFY_HITSTOP_DURATION);
					if (gameCamera)
					{
						gameCamera->StartShake(NULLIFY_SHAKE_STRENGTH, NULLIFY_SHAKE_DURATION);
					}
					break;

				case EnImpactType::UltActivate:
					g_gameTime->StartSlowMotion(ULT_HITSTOP_SCALE, ULT_HITSTOP_DURATION);
					postEffect.GetRadialBlur().Start(
						ULT_BLUR_STRENGTH, ULT_BLUR_ATTACK_TIME, ULT_BLUR_DURATION);
					if (gameCamera)
					{
						gameCamera->StartPunchIn(ULT_PUNCH_IN_AMOUNT, ULT_PUNCH_IN_DURATION);
					}
					break;
				}
			}
		);
	}


	void InGameSceneBase::UpdateGamePhase()
	{
		/** カメラは常に更新 */
		auto gameCamera = camera::CameraManager::Get().GetController<camera::GameCamera>(
			camera::GameCamera::ID()
		);
		if (gameCamera)
		{
			camera::CameraData data = gameCamera->GetCameraData();
			m_cameraSteering.Update(data, g_gameTime->GetFrameDeltaTime());
			gameCamera->SetState(data);
		}

		/** ディザリングマネージャーは毎フレーム更新して、カメラとプレイヤーの位置を反映させる */
		OcclusionDitherManager::Get().SetPlayerTarget(&m_daddyPenguin->GetModelRender());
		OcclusionDitherManager::Get().Update();

		/** ステージは常に更新 */
		actor::StageSystem::GetInstance()->Update();
		if (nature::Ocean::GetInstance()) {
			nature::Ocean::GetInstance()->Update();
		}
		nature::WhirlpoolManager::GetInstance()->Update();

		switch (m_gamePhase)
		{
			//------------------------------------------------------------
			// カウントダウン
			//------------------------------------------------------------
		case GamePhase::CountDown:
		{
			/** BattleManager にゲーム非アクティブを伝える */
			BattleManager::GetInstance().SetIsActive(false);

			/** AI・入力は動かさないが、描画用の行列更新だけ行う */
			if (m_daddyPenguin) m_daddyPenguin->UpdateModelOnly();
			actor::ChildPenguinManager::GetInstance()->UpdateModelOnly();
			actor::EnemyManager::GetInstance()->UpdateModelOnly();

			/** カウントダウン UI 更新 */
			auto* uiMngr = InGameUIManager::GetInstance();

			uiMngr->UpdateCountDown();

			/** カウントダウン中のSE制御 */
			auto* countDownMenu = uiMngr->GetCountDownMenu();
			if (countDownMenu)
			{
				ui::EnCountDownType currentType = countDownMenu->GetCurrentCountType();

				if (currentType != m_lastCountType)
				{
					switch (currentType)
					{
					case ui::EnCountDownType::Third:
					case ui::EnCountDownType::Second:
					case ui::EnCountDownType::First:
						SoundManager::Get().PlaySE(enSoundKind_CountDown);
						break;

					case ui::EnCountDownType::GO:
						SoundManager::Get().PlaySE(enSoundKind_GameStart);
						break;

					default:
						break;
					}

					m_lastCountType = currentType;
				}
			}

			/** カウントダウン終了判定 */
			if (countDownMenu && countDownMenu->IsCountDownFinished())
			{
				m_gamePhase = GamePhase::Playing;
				BattleManager::GetInstance().SetIsActive(true);

				// カウントダウン終了 → プレイ中のUIに切り替える。
				InGameUIManager::GetInstance()->UpdatePlaying();
			}
			break;
		}

		//------------------------------------------------------------
		// プレイ中
		//------------------------------------------------------------
		case GamePhase::Playing:
		{
			if (g_pad[0]->IsTrigger(enButtonStart))
			{
				SceneManager::GetInstance()->SetPause(true);
			}

			/** プレイヤー・子ペンギン・シロクマ の更新 */
			if (m_daddyPenguin) m_daddyPenguin->UpdateWrapper();
			actor::ChildPenguinManager::GetInstance()->Update();
			actor::EnemyManager::GetInstance()->Update();

			/** インゲームUI 更新 */
			auto* uiMngr = InGameUIManager::GetInstance();
			uiMngr->UpdatePlaying();

			BattleManager::GetInstance().Update();
			TimeManager::GetInstance().Update();

			/** 撮影モード中はフィーバーを発動させない（紹介動画が突然ピンクになるため） */
			if (!IsShowcaseEnabled())
			{
				FeverTimeManager::GetInstance()->Update();
			}

			app::achievement::AchievementManager::GetInstance()->Update();

			/** ログ毎フレームティック */
			if (auto* lm = GameLogManager::GetInstance())
				lm->RecordTick(m_daddyPenguin);

			/** ノイズリストをクリア */
			NoiseManager::GetInstance().ClearNoises();

			OnUpdatePlaying();

			/** 終了判定 */
			if (BattleManager::GetInstance().GetBattleState() == BattleManager::EnBattleState::Finished)
			{
				SoundManager::Get().StopAllSE();

				/** FINISH演出（3秒）に合わせてBGMを徐々にフェードアウトする */
				SoundManager::Get().FadeOutBGM(FINISH_BGM_FADE_DURATION);

				/** フィーバー演出中にラウンドが終わった場合、途中の状態で固まらないよう強制的に消す
				 *  （Finishingフェーズに入るとUpdatePlaying経由のUpdate()が呼ばれなくなるため） */
				if (auto* feverIconMenu = uiMngr->GetFeverIconMenu())
				{
					feverIconMenu->ForceHide();
				}

				/** FINISH 演出開始 */
				auto* finishMenu = uiMngr->GetFinishMenu();
				if (finishMenu) finishMenu->StartFinish();

				m_gamePhase = GamePhase::Finishing;
			}
			break;
		}

		//------------------------------------------------------------
		// FINISH 演出
		//------------------------------------------------------------
		case GamePhase::Finishing:
		{
			/** FINISH UI 更新 */
			auto* uiMngr = InGameUIManager::GetInstance();
			uiMngr->UpdateFinishing();

			/** ホイッスルは演出開始時に1回だけ鳴らす */
			if (!m_isWhistlePlayed)
			{
				SoundManager::Get().PlaySE(enSoundKind_Whistle, 1.0f, false);
				m_isWhistlePlayed = true;
			}

			/** 演出終了 → リザルトへ */
			auto* finishMenu = uiMngr->GetFinishMenu();
			if (finishMenu && finishMenu->IsFinished())
			{
				SoundManager::Get().StopBGM();
				m_nextScene = true;

				const float clearTime = TimeManager::GetInstance().GetCurTime();
				const int   rescued = actor::ChildPenguinManager::GetInstance()->GetRescuedNum();
				ResultScene::SetResult(rescued);

				/** アチーブメント最終判定（FinalCondition 型を評価） */
				float logScore = 0.0f;
				if (auto* am = app::achievement::AchievementManager::GetInstance())
				{
					am->FinalizeAchievements();

					// ResultScene::CalcTotalScore() と同じ式でスコアを算出してログに記録する。
					// プレイヤーがリザルト画面で見る数字とログの数字がずれないよう、式は必ず揃えること
					int achievedCount = 0;
					for (auto* achieve : am->GetAllAchievements())
					{
						if (achieve && achieve->IsAchieved()) achievedCount++;
					}

					/** アチーブメントの成否と各種カウンタをログへ残す。
					 *  「条件が緩すぎて無条件達成」「厳しすぎて達成不能」の判定に使う */
					if (auto* lm = GameLogManager::GetInstance())
					{
						nlohmann::json achievements = nlohmann::json::array();
						for (auto* achieve : am->GetAllAchievements())
						{
							if (achieve == nullptr) continue;
							achievements.push_back({
								{ "name",         achieve->GetName() },
								{ "achieved",     achieve->IsAchieved() },
								{ "achieved_time", achieve->GetAchievedTime() }
							});
						}

						lm->SetResultDetail({
							{ "achieved_count",     achievedCount },
							{ "achievements",       achievements },
							{ "bear_kills",         am->GetBearKillCount() },
							{ "whirlpool_captures", am->GetWhirlpoolCaptureCount() },
							{ "remaining_sec",      clearTime }
						});
					}
					constexpr float SCORE_BASE_MULTIPLIER = 100.0f;   // 救出数の基本スコア倍率
					constexpr float SCORE_PER_ACHIEVEMENT = 2000.0f;  // アチーブメント達成1件ごとの加算スコア

					logScore = static_cast<float>(rescued) * SCORE_BASE_MULTIPLIER
						+ static_cast<float>(achievedCount) * SCORE_PER_ACHIEVEMENT;
				}

				/** ログをファイルへ書き出し
				 *  clearTime は「残り時間」なので、経過時間へ直してから duration として渡す */
				if (auto* lm = GameLogManager::GetInstance())
				{
					const float elapsedTime = GetTimeLimit() - clearTime;
					lm->Flush(GetStageName(), elapsedTime, rescued, logScore);
					GameLogManager::DestroyInstance();
				}
			}
			break;
		}
		}
	}


	void InGameSceneBase::PauseUpdate()
	{
		/**
		 * フェードアウト中など、実際にポーズボタンが押されていないのに
		 * PauseUpdate() が呼ばれた場合は、ポーズ用の処理をスキップする
		 */
		if (!SceneManager::GetInstance()->IsPause())
		{
			return;
		}

		/** ポーズ開始フレームに1回だけ全SEを停止し、サブビューを非表示にする
		 *  チュートリアルポーズを含むすべてのポーズ種別に適用するため
		 *  OnPauseUpdate() の前に実行する */
		if (!m_isPauseEntered)
		{
			SoundManager::Get().StopAllSE();
			SoundManager::Get().PlaySE(enSoundKind::enSoundKind_ButtonEnter);
			nsBeastEngine::SubCameraManager::Get().SetRenderingBlocked(true);
			m_isPauseEntered = true;
		}

		/** ポーズ中もアチーブメントJSONのホットリロードとUI反映を行う */
		if (auto* am = app::achievement::AchievementManager::GetInstance())
		{
			am->CheckHotReload();
		}
		if (auto* uiMngr = InGameUIManager::GetInstance())
		{
			uiMngr->UpdateAchievementHotReload();
		}

		/** 派生クラスが独自ポーズを処理する場合は通常ポーズをスキップ */
		if (OnPauseUpdate()) return;

		switch (m_pauseState)
		{
			//------------------------------------------------------------
			// ポーズメニュー
			//------------------------------------------------------------
		case PauseState::Pause:
		{
			auto* uiMngr = InGameUIManager::GetInstance();
			auto* pauseMenu = uiMngr->GetPauseMenu();
			if (!pauseMenu) break;

			uiMngr->UpdatePause();

			/** ゲームに戻る */
			if (pauseMenu->IsRetry())
			{
				pauseMenu->IsRetry(false);
				nsBeastEngine::SubCameraManager::Get().SetRenderingBlocked(false);
				SceneManager::GetInstance()->SetPause(false);
				SoundManager::Get().PlaySE(enSoundKind::enSoundKind_ButtonBack);
				/** ポーズ解除時にフラグをリセットする */
				m_isPauseEntered = false;
			}
			/** サウンドオプションへ */
			else if (pauseMenu->IsSound())
			{
				pauseMenu->IsSound(false);
				SoundManager::Get().PlaySE(enSoundKind::enSoundKind_ButtonEnter);
				m_pauseState = PauseState::SoundOption;
			}
			/** ルール画面へ */
			else if (pauseMenu->IsRule())
			{
				pauseMenu->IsRule(false);
				auto* tutorialMenu = uiMngr->GetTutorialMenu();
				if (tutorialMenu)
				{
					tutorialMenu->SetClosed(false);
					tutorialMenu->InitializeLogic();
				}
				SoundManager::Get().PlaySE(enSoundKind::enSoundKind_ButtonEnter);
				m_pauseState = PauseState::Tutorial;
			}
			/** タイトルへ戻る */
			else if (pauseMenu->IsGoTitle())
			{
				pauseMenu->IsGoTitle(false);
				SceneManager::GetInstance()->SetPause(false);
				SoundManager::Get().PlaySE(enSoundKind::enSoundKind_ButtonBack);
				/** ポーズ解除時にフラグをリセットする */
				m_isPauseEntered = false;
				m_goTitle = true;
			}
			break;
		}

		//------------------------------------------------------------
		// サウンドオプション（ポーズ中のサブ画面）
		//------------------------------------------------------------
		case PauseState::SoundOption:
		{
			auto* uiMngr = InGameUIManager::GetInstance();
			auto* soundOptionMenu = uiMngr->GetSoundOptionMenu();
			if (soundOptionMenu)
			{
				soundOptionMenu->Update();
			}

			/** Bボタンでポーズ画面に戻る */
			if (g_pad[0]->IsTrigger(enButtonB))
			{
				SoundManager::Get().PlaySE(enSoundKind_ButtonBack);
				m_pauseState = PauseState::Pause;
			}
			break;
		}

		//------------------------------------------------------------
		// チュートリアル画面（ポーズ中のサブ画面）
		//------------------------------------------------------------
		case PauseState::Tutorial:
		{
			auto* uiMngr = InGameUIManager::GetInstance();
			auto* tutorialMenu = uiMngr->GetTutorialMenu();
			if (tutorialMenu)
			{
				tutorialMenu->Update();

				/** TutorialMenu 内で Bボタンが押されたら閉じる */
				if (tutorialMenu->IsClosed())
				{
					tutorialMenu->SetClosed(false);
					SoundManager::Get().PlaySE(enSoundKind_ButtonBack);
					m_pauseState = PauseState::Pause;
				}
			}
			break;
		}
		}
	}


	void InGameSceneBase::Render(RenderContext& rc)
	{
		actor::StageSystem::GetInstance()->Render(rc);

		if (m_daddyPenguin) m_daddyPenguin->RenderWrapper(rc);
		actor::ChildPenguinManager::GetInstance()->Render(rc);
		actor::EnemyManager::GetInstance()->Render(rc);

		/** ポーズ中の描画 */
		if (SceneManager::GetInstance()->IsPause())
		{
			if (!OnPauseRender(rc))
			{
				/** 通常ポーズ画面の描画（派生クラスが処理しない場合） */
				auto* uiMngr = InGameUIManager::GetInstance();
				switch (m_pauseState)
				{
				case PauseState::Pause:
					uiMngr->RenderPause(rc);
					break;
				case PauseState::SoundOption:
					uiMngr->RenderSoundOption(rc);
					break;
				case PauseState::Tutorial:
					uiMngr->RenderTutorial(rc);
					break;
				}
			}
			return;
		}

		/** UI 描画（ポーズ中はここに来ない） */
		if (m_loadPhase == LoadPhase::Done)
		{
			auto* uiMngr = InGameUIManager::GetInstance();
			switch (m_gamePhase)
			{
			case GamePhase::CountDown:
				uiMngr->RenderCountDown(rc);
				break;
			case GamePhase::Playing:
				uiMngr->RenderPlaying(rc);
				OnRenderPlaying(rc);
				break;
			case GamePhase::Finishing:
				uiMngr->RenderFinishing(rc);
				break;
			}
		}
	}


	bool InGameSceneBase::RequesutScene(uint32_t& id, float& waitTime)
	{
		/** タイトルへ戻る */
		if (m_goTitle)
		{
			SoundManager::Get().StopAllSE();
			EffectManager::Get().StopAllEffect();

			// 環境音などを全て止めた直後に、タイトルへ戻る決定音を鳴らす
			SoundManager::Get().PlaySE(enSoundKind_ButtonEnter);

			id = TitleScene::ID();
			waitTime = SCENE_TRANSITION_WAIT_TIME;
			return true;
		}
		/** リザルトへ */
		if (m_nextScene)
		{
			SoundManager::Get().StopAllSE();
			EffectManager::Get().StopAllEffect();
			id = ResultScene::ID();
			waitTime = SCENE_TRANSITION_WAIT_TIME;
			return true;
		}
		return false;
	}
}
