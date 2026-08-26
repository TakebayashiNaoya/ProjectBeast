/**
 * @file ReplayScene.cpp
 * @brief プレイログを再生するリプレイシーン
 * @author 竹林
 */
#include "stdafx.h"
#include "ReplayScene.h"

#include "TitleScene.h"
#include "Source/Camera/CameraController.h"
#include "Source/Camera/CameraManager.h"
#include "Source/Nature/Whirlpool.h"
#include "Source/Actor/Character/Penguin/DaddyPenguin/DaddyPenguin.h"
#include "Source/Actor/Character/Enemy/Enemy.h"
#include "Source/Actor/Character/Penguin/ChildPenguin/ChildPenguin.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Actor/ActorStateMachine.h"
#include "Source/Actor/Character/Enemy/EnemyTypes.h"
#include "Source/Actor/Stage/StageSystem.h"
#include "Source/Nature/Ocean.h"
#include "Source/Graphics/PBRStatus.h"
#include "Source/Manager/InGameUIManager.h"
#include "Source/Manager/TimeManager.h"
#include "Source/UI/InGameTimer/InGameTimerMenu.h"
#include "Source/UI/RemainingChild/RemainingChildMenu.h"
#include "Source/GameLog/LogCompression.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>


namespace app
{
	namespace
	{
		/**
		 * @brief 1倍速再生時に1秒あたり何tick進めるかのフォールバック値
		 * @details 記録データの "t" は TimeManager の残り時間（カウントダウン）を
		 *          そのまま使っており単調増加しないため、再生の時間軸には使えない。
		 *          "frame"（tick連番、確実に1ずつ増える）を直接タイムライン単位として使う。
		 *          実際の1秒あたりのtick数（記録レート）はログごとに異なる
		 *          （LOG_TICK_INTERVALは過去に0.1秒間隔→毎フレームと変更された経緯があり、
		 *          このLOG_TICK_INTERVAL自体もログごとに違いうる）ため、本来はこの固定値ではなく
		 *          m_effectiveTicksPerSecond（LoadSession()でログ自身から自動算出）を使う。
		 *          この定数はログが短すぎて算出できない場合のフォールバックとしてのみ使う。
		 */
		constexpr float DEFAULT_TICKS_PER_SECOND = 10.0f;

		/** InGameSceneBase.cpp の SKY_CUBE_SCALE と同じ値 */
		const Vector3 SKY_CUBE_SCALE = Vector3(1000.0f, 800.0f, 1000.0f);

		/** json配列 [x,y,z] を Vector3 に変換する */
		Vector3 JsonToV3(const nlohmann::json& j)
		{
			return Vector3(j[0].get<float>(), j[1].get<float>(), j[2].get<float>());
		}

		/** json配列の指定indexの値を取得する。配列でない/範囲外ならデフォルト値を返す */
		template <typename T>
		T ArrGet(const nlohmann::json& arr, size_t index, T defaultValue)
		{
			return (arr.is_array() && arr.size() > index) ? arr[index].get<T>() : defaultValue;
		}

		/**
		 * @brief json配列の指定indexの要素をVector3として取得する
		 * @details JsonToV3(j[index])を直接呼ぶと、配列の要素数が足りない壊れたログ
		 *          （破損した.cmpファイル等）に対して範囲外アクセスの未定義動作になりうるため、
		 *          ArrGet同様に範囲チェックしてから読む
		 */
		Vector3 ArrGetV3(const nlohmann::json& arr, size_t index, const Vector3& defaultValue)
		{
			return (arr.is_array() && arr.size() > index) ? JsonToV3(arr[index]) : defaultValue;
		}

		/**
		 * @brief jsonオブジェクトの指定キーの要素をVector3として取得する
		 * @details 旧フォーマット（オブジェクト形式）用。j[key]をconst jsonに対して直接呼ぶと
		 *          キーが存在しない場合に例外を投げるため、contains()で確認してから読む
		 */
		Vector3 ObjGetV3(const nlohmann::json& j, const char* key, const Vector3& defaultValue)
		{
			return j.contains(key) ? JsonToV3(j[key]) : defaultValue;
		}

		/**
		 * @brief bears/penguins/whirlpools配列内の1エンティティのidだけを取り出す
		 * @details 新旧フォーマット両対応（新: 配列の先頭要素がid／旧: {"id":...}）。
		 *          tick1側でtick0と同じidの要素を探す処理で、フルパースせず軽く使う
		 */
		int GetEntityId(const nlohmann::json& j, int defaultId)
		{
			return j.is_array() ? ArrGet<int>(j, 0, defaultId) : j.value("id", defaultId);
		}

		/**
		 * @brief json配列の指定indexの値を真偽値として取得する
		 * @details GameLogManagerはログサイズ削減のため in_formation/is_alive を
		 *          JSONのtrue/false（4〜5文字）ではなく0/1（1文字）の数値で書き出すようになった。
		 *          nlohmann::jsonは型に厳格で、数値を素の get<bool>() で読むと例外になるため、
		 *          数値・真偽値のどちらで記録されていても読めるようにする
		 */
		bool ArrGetBool(const nlohmann::json& arr, size_t index, bool defaultValue)
		{
			if (!arr.is_array() || arr.size() <= index) return defaultValue;
			const auto& v = arr[index];
			if (v.is_boolean()) return v.get<bool>();
			if (v.is_number()) return v.get<int>() != 0;
			return defaultValue;
		}

		/** @brief 親ペンギン1体分の記録データ */
		struct ParentFields
		{
			Vector3 pos;
			float rotY = 0.0f;
			std::string state = "Idle";
		};

		/**
		 * @brief 親ペンギンの記録データをパースする（新旧フォーマット両対応）
		 * @details 新: [pos, rot_y, state]／旧: {"pos":..., "rot_y":..., "state":...}
		 *          新フォーマットはGameLogManagerがファイルサイズ削減のため導入したもの
		 *          （キー名を毎回書き出す代わりに固定順の配列で記録する）。
		 */
		ParentFields ParseParent(const nlohmann::json& j)
		{
			ParentFields f;
			if (j.is_array())
			{
				f.pos = ArrGetV3(j, 0, Vector3(0.0f, 0.0f, 0.0f));
				f.rotY = ArrGet<float>(j, 1, 0.0f);
				f.state = ArrGet<std::string>(j, 2, "Idle");
			}
			else
			{
				f.pos = ObjGetV3(j, "pos", Vector3(0.0f, 0.0f, 0.0f));
				f.rotY = j.value("rot_y", 0.0f);
				f.state = j.value("state", "Idle");
			}
			return f;
		}

		/** @brief シロクマ1体分の記録データ */
		struct BearFields
		{
			int id = -1;
			Vector3 pos;
			float rotY = 0.0f;
			std::string state = "Idle";
		};

		/** @brief シロクマの記録データをパースする（新: [id, pos, rot_y, state, sleep_timer]／旧: オブジェクト形式） */
		BearFields ParseBear(const nlohmann::json& j)
		{
			BearFields f;
			if (j.is_array())
			{
				f.id = ArrGet<int>(j, 0, -1);
				f.pos = ArrGetV3(j, 1, Vector3(0.0f, 0.0f, 0.0f));
				f.rotY = ArrGet<float>(j, 2, 0.0f);
				f.state = ArrGet<std::string>(j, 3, "Idle");
			}
			else
			{
				f.id = j.value("id", -1);
				f.pos = ObjGetV3(j, "pos", Vector3(0.0f, 0.0f, 0.0f));
				f.rotY = j.value("rot_y", 0.0f);
				f.state = j.value("state", "Idle");
			}
			return f;
		}

		/** @brief 子ペンギン1体分の記録データ */
		struct PenguinFields
		{
			int id = -1;
			std::string type = "Serious";
			Vector3 pos;
			float rotY = 0.0f;
			std::string state = "Idle";
			bool inFormation = false;
			bool isAlive = true;
		};

		/** @brief 子ペンギンの記録データをパースする（新: [id, type, pos, rot_y, state, in_formation, is_alive]／旧: オブジェクト形式） */
		PenguinFields ParsePenguin(const nlohmann::json& j)
		{
			PenguinFields f;
			if (j.is_array())
			{
				f.id = ArrGet<int>(j, 0, -1);
				f.type = ArrGet<std::string>(j, 1, "Serious");
				f.pos = ArrGetV3(j, 2, Vector3(0.0f, 0.0f, 0.0f));
				f.rotY = ArrGet<float>(j, 3, 0.0f);
				f.state = ArrGet<std::string>(j, 4, "Idle");
				f.inFormation = ArrGetBool(j, 5, false);
				f.isAlive = ArrGetBool(j, 6, true);
			}
			else
			{
				f.id = j.value("id", -1);
				f.type = j.value("type", "Serious");
				f.pos = ObjGetV3(j, "pos", Vector3(0.0f, 0.0f, 0.0f));
				f.rotY = j.value("rot_y", 0.0f);
				f.state = j.value("state", "Idle");
				f.inFormation = j.value("in_formation", false);
				f.isAlive = j.value("is_alive", true);
			}
			return f;
		}

		/** @brief 渦潮1体分の記録データ */
		struct WhirlpoolFields
		{
			int id = -1;
			Vector3 pos;
			float scaleXZ = 1.0f;
		};

		/** @brief 渦潮の記録データをパースする（新: [id, pos, state, scale_xz]／旧: オブジェクト形式） */
		WhirlpoolFields ParseWhirlpool(const nlohmann::json& j)
		{
			WhirlpoolFields f;
			if (j.is_array())
			{
				f.id = ArrGet<int>(j, 0, -1);
				f.pos = ArrGetV3(j, 1, Vector3(0.0f, 0.0f, 0.0f));
				f.scaleXZ = ArrGet<float>(j, 3, 1.0f);
			}
			else
			{
				f.id = j.value("id", -1);
				f.pos = ObjGetV3(j, "pos", Vector3(0.0f, 0.0f, 0.0f));
				f.scaleXZ = j.value("scale_xz", 1.0f);
			}
			return f;
		}

		/** @brief カメラ1件分の記録データ */
		struct CameraFields
		{
			Vector3 pos;
			Vector3 target;
			float fov = 60.0f;
		};

		/** @brief カメラの記録データをパースする（新: [pos, target, fov]／旧: オブジェクト形式） */
		CameraFields ParseCamera(const nlohmann::json& j)
		{
			CameraFields f;
			if (j.is_array())
			{
				f.pos = ArrGetV3(j, 0, Vector3(0.0f, 0.0f, 0.0f));
				f.target = ArrGetV3(j, 1, Vector3(0.0f, 0.0f, 0.0f));
				f.fov = ArrGet<float>(j, 2, 60.0f);
			}
			else
			{
				f.pos = ObjGetV3(j, "pos", Vector3(0.0f, 0.0f, 0.0f));
				f.target = ObjGetV3(j, "target", Vector3(0.0f, 0.0f, 0.0f));
				f.fov = j.value("fov", 60.0f);
			}
			return f;
		}

		/** GameLogManager が記録したタイプ名文字列を EnChildPenguinType に変換する */
		actor::EnChildPenguinType ParseChildPenguinType(const std::string& typeStr)
		{
			if (typeStr == "Serious") return actor::EnChildPenguinType::Serious;
			if (typeStr == "Clingy")  return actor::EnChildPenguinType::Clingy;
			if (typeStr == "Naughty") return actor::EnChildPenguinType::Naughty;
			if (typeStr == "Clumsy")  return actor::EnChildPenguinType::Clumsy;
			if (typeStr == "Caring")  return actor::EnChildPenguinType::Caring;
			return actor::EnChildPenguinType::Serious;
		}

		/**
		 * @brief ペンギン（親・子共通、PenguinStateMachine::GetStateNameForLog()）の
		 *        記録stateから対応するアニメーションクリップ番号を返す
		 * @details PenguinIState.cpp の各ステートEnter()が実際に呼ぶPlayAnimation()と対応させている
		 */
		int PenguinAnimIndexForState(const std::string& state)
		{
			using AnimID = actor::EnPenguinAnimationID;
			if (state == "Run")         return static_cast<int>(AnimID::MoveRun);
			if (state == "Sneak")       return static_cast<int>(AnimID::MoveWalk);
			if (state == "Jump")        return static_cast<int>(AnimID::JumpWalking);
			if (state == "SlideStart")  return static_cast<int>(AnimID::SlideStart);
			if (state == "Slide")       return static_cast<int>(AnimID::Sliding);
			if (state == "SlideEnd")    return static_cast<int>(AnimID::StandUp);
			if (state == "Swim")        return static_cast<int>(AnimID::MoveSwim);
			if (state == "InWhirlpool") return static_cast<int>(AnimID::MoveSwim);
			if (state == "Dying")       return static_cast<int>(AnimID::DeathFaceDown);
			if (state == "Dead")        return static_cast<int>(AnimID::DeathFaceDown);
			// "Idle" / "Damaged"（Damagedは実ゲームでも専用アニメが無く直前の姿勢を継続する）/ 未知の値
			return static_cast<int>(AnimID::IdleStanding);
		}

		/**
		 * @brief シロクマ（EnemyStateMachine::GetStateNameForLog()）の記録stateから
		 *        対応するアニメーションクリップ番号を返す
		 * @details EnemyIState.cpp の各ステートEnter()が実際に呼ぶPlayAnimation()と対応させている
		 *          （水中判定は記録していないため、陸上時のアニメーションのみを使う）
		 */
		int BearAnimIndexForState(const std::string& state)
		{
			using AnimID = EnEnemyAnimationType;
			if (state == "Chase")  return static_cast<int>(AnimID::Run);
			if (state == "Attack") return static_cast<int>(AnimID::Attack);
			if (state == "Roar")   return static_cast<int>(AnimID::Buff);
			if (state == "Stun")   return static_cast<int>(AnimID::Stun);
			if (state == "Search") return static_cast<int>(AnimID::BackWalk);
			if (state == "Walk")   return static_cast<int>(AnimID::Walk);
			if (state == "Swim")   return static_cast<int>(AnimID::Swim);
			if (state == "Sleep")  return static_cast<int>(AnimID::Sleep);
			if (state == "Return") return static_cast<int>(AnimID::Walk);
			// "Idle" / "Jump"（専用アニメ無し）/ 未知の値
			return static_cast<int>(AnimID::Idle);
		}
	}


	ReplayScene::ReplayScene()
	{
		ScanSessions();
	}


	ReplayScene::~ReplayScene()
	{
		camera::CameraManager::Get().Unregister(camera::ReplayCamera::ID());
		UnloadBackground();
		g_renderingEngine->UnregisterNatureObject(this);
		InGameUIManager::DestroyInstance();
		TimeManager::DestroyInstance();
	}


	bool ReplayScene::Start()
	{
		// リプレイ再生中はこのカメラで撮影した視点を使う
		m_replayCamera = std::make_shared<camera::ReplayCamera>();
		camera::CameraManager::Get().Register(camera::ReplayCamera::ID(), m_replayCamera);
		camera::CameraManager::Get().SwitchCamera(camera::ReplayCamera::ID(), 0.0f);

		// 渦潮（nature::Whirlpool）はGBuffer/ライティング/フォワードパスの後という
		// 専用タイミングでしか正しく描画されないため、Nature Objectとして登録する。
		// 背景（Ocean）読み込み前の暫定登録。LoadBackground()でOceanの後ろに登録し直す
		// （Nature Objectは登録順に描画されるため、渦潮をOceanより後＝海面の上に描く必要がある）
		g_renderingEngine->RegisterNatureObject(this);

		// HUD（タイマー・救助数のみ）。BattleManager等は生成せず、値は再生側から直接設定する
		InGameUIManager::CreateInstance();
		InGameUIManager::GetInstance()->Initialize();

		// InGameTimerMenu::Update()がTimeManager::GetInstance().GetMaxTime()を無条件参照するため、
		// 生成だけしておく（Update()はBattleManagerに依存するので呼ばない。最大値はLoadSession()で設定）
		TimeManager::CreateInstance();

		return true;
	}


	void ReplayScene::Update()
	{
		if (g_pad[0]->IsTrigger(enButtonB))
		{
			m_backToTitle = true;
		}

		if (m_backgroundLoaded)
		{
			actor::StageSystem::GetInstance()->Update();
			nature::Ocean::GetInstance()->Update();
		}

		InGameUIManager::GetInstance()->UpdateTimerAndScoreOnly();

		if (!m_ticks.empty())
		{
			if (m_isPlaying)
			{
				float newTime = m_playbackTime + g_gameTime->GetFrameDeltaTime() * m_playbackSpeed * m_effectiveTicksPerSecond;
				const float maxTime = GetMaxPlaybackTime();

				// 終端・先頭に到達したら止める（早送り・巻き戻し両対応）
				if (newTime >= maxTime)
				{
					newTime = maxTime;
					m_isPlaying = false;
				}
				else if (newTime <= 0.0f)
				{
					newTime = 0.0f;
					m_isPlaying = false;
				}

				SeekToTime(newTime);
			}

			// 再生中でなくても（一時停止中・タイムラインをドラッグした直後も）
			// 現在位置を毎フレーム反映する
			ApplyCurrentTick();
		}

		// カメラ情報が無いログでは、一時停止中も自由に見回せるよう毎フレーム更新する
		if (!m_hasCameraData)
		{
			UpdateFallbackCamera();
		}
	}


	void ReplayScene::PauseUpdate()
	{}


	void ReplayScene::Render(RenderContext& rc)
	{
		DrawUI();

		if (m_backgroundLoaded)
		{
			actor::StageSystem::GetInstance()->Render(rc);
		}

		if (m_parentActor)
		{
			m_parentActor->RenderWrapper(rc);
		}

		for (size_t i = 0; i < m_bearActors.size(); i++)
		{
			if (m_bearSlotActive[i]) m_bearActors[i]->RenderWrapper(rc);
		}

		for (size_t i = 0; i < m_penguinActors.size(); i++)
		{
			if (m_penguinSlotActive[i]) m_penguinActors[i]->RenderWrapper(rc);
		}

		// 渦潮はINatureObject版のRender(rc, view)（下記）で、GBuffer/ライティング/
		// フォワードパスの後という正しいタイミングに描画される

		// HUD（タイマー・救助数のみ）
		InGameUIManager::GetInstance()->RenderTimerAndScoreOnly(rc);
	}


	void ReplayScene::Render(RenderContext& rc, const nsBeastEngine::RenderViewContext& view)
	{
		for (size_t i = 0; i < m_whirlpoolModels.size(); i++)
		{
			if (m_whirlpoolSlotActive[i]) m_whirlpoolModels[i]->Render(rc, view);
		}
	}


	bool ReplayScene::RequesutScene(uint32_t& id, float& waitTime)
	{
		if (m_backToTitle)
		{
			id = TitleScene::ID();
			waitTime = 0.5f;
			return true;
		}
		return false;
	}


	void ReplayScene::ScanSessions()
	{
		m_sessions.clear();

		const std::filesystem::path logsDir = "Logs";
		if (!std::filesystem::exists(logsDir)) return;

		for (const auto& entry : std::filesystem::directory_iterator(logsDir))
		{
			if (!entry.is_directory()) continue;

			SessionEntry session;
			session.id = entry.path().filename().string();

			std::ifstream ifs(entry.path() / "session.json");
			if (ifs)
			{
				const nlohmann::json j = nlohmann::json::parse(ifs, nullptr, false);
				if (!j.is_discarded())
				{
					session.stage       = j.value("stage", "");
					session.durationSec = j.value("duration_sec", 0.0f);

					if (j.contains("result"))
					{
						session.rescued = j["result"].value("rescued", 0);
						session.score   = j["result"].value("score", 0.0f);
					}
				}
			}

			m_sessions.push_back(std::move(session));
		}

		// session_id は "%Y-%m-%d_%H-%M-%S" 形式なので文字列の降順ソート＝新しい順になる
		std::sort(m_sessions.begin(), m_sessions.end(),
			[](const SessionEntry& a, const SessionEntry& b) { return a.id > b.id; });
	}


	void ReplayScene::LoadSession(const std::string& sessionId)
	{
		m_ticks.clear();
		m_lastLoadError.clear();

		// GameLogManagerは書き出し時にログ全体を圧縮して ticks.jsonl.cmp として保存する
		// （数十MBになる非圧縮JSONLをそのまま置かないため）。旧セッションは非圧縮の
		// ticks.jsonl のまま残っているため、そちらも読めるようにフォールバックする
		const std::string compressedPath = "Logs/" + sessionId + "/ticks.jsonl.cmp";
		const std::string plainPath = "Logs/" + sessionId + "/ticks.jsonl";

		std::string jsonl;
		if (std::filesystem::exists(compressedPath))
		{
			std::ifstream ifs(compressedPath, std::ios::binary);
			if (!ifs)
			{
				m_lastLoadError = u8"ticks.jsonl.cmp を開けませんでした";
				return;
			}

			const std::vector<uint8_t> compressed(
				(std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

			// 圧縮ファイル自体が空（0tickで記録されたセッション）なのと、中身はあるのに
			// 展開に失敗した（壊れたファイル）のは区別する。後者だけ明確な読み込み失敗として扱う
			if (!compressed.empty())
			{
				jsonl = DecompressLogData(compressed);
				if (jsonl.empty())
				{
					m_lastLoadError = u8"ticks.jsonl.cmp の展開に失敗しました（ファイルが壊れている可能性があります）";
					return;
				}
			}
		}
		else
		{
			std::ifstream ifs(plainPath);
			if (!ifs)
			{
				m_lastLoadError = u8"ticks.jsonl を開けませんでした";
				return;
			}

			std::ostringstream oss;
			oss << ifs.rdbuf();
			jsonl = oss.str();
		}

		std::istringstream jsonlStream(jsonl);
		std::string line;
		while (std::getline(jsonlStream, line))
		{
			if (line.empty()) continue;

			nlohmann::json tick = nlohmann::json::parse(line, nullptr, false);
			if (tick.is_discarded()) continue;

			// spawn/despawnイベント行は座標情報を持たず再生に使えないため除外する。
			// "type":"tick" の行だけを残すことで、"frame" が確実に連番になる
			if (tick.value("type", "") != "tick") continue;

			m_ticks.push_back(std::move(tick));
		}

		m_loadedSessionId = sessionId;

		// カメラ情報が1件でも記録されているログか（古いログには無い場合がある）
		m_hasCameraData = std::any_of(m_ticks.begin(), m_ticks.end(),
			[](const nlohmann::json& t) { return t.contains("camera"); });

		// このログの実際の記録レート（tick/秒）をログ自身から算出する。
		// tickの"t"（TimeManagerの残り時間）は実時間で1秒に1減るカウントダウンなので、
		// 先頭tickと末尾tickの"t"の差 ＝ 実際に経過した秒数として使える
		// （session.jsonのdurationは、記録経路によっては0のまま保存されることがあり信頼できない）
		m_effectiveTicksPerSecond = DEFAULT_TICKS_PER_SECOND;
		if (m_ticks.size() >= 2)
		{
			const float tFirst = m_ticks.front().value("t", 0.0f);
			const float tLast = m_ticks.back().value("t", 0.0f);
			const float frameFirst = static_cast<float>(m_ticks.front().value("frame", 0));
			const float frameLast = static_cast<float>(m_ticks.back().value("frame", 0));
			const float elapsedRealSeconds = tFirst - tLast;
			const float elapsedFrames = frameLast - frameFirst;
			if (elapsedRealSeconds > 0.1f && elapsedFrames > 0.0f)
			{
				m_effectiveTicksPerSecond = elapsedFrames / elapsedRealSeconds;
			}
		}

		// 再生状態をリセット
		m_isPlaying = false;
		m_playbackTime = 0.0f;
		m_currentTickIndex = 0;
		std::fill(m_bearSlotActive.begin(), m_bearSlotActive.end(), false);
		std::fill(m_penguinSlotActive.begin(), m_penguinSlotActive.end(), false);
		std::fill(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), false);

		// このセッションのステージに合わせて背景を読み込む
		const auto it = std::find_if(m_sessions.begin(), m_sessions.end(),
			[&](const SessionEntry& s) { return s.id == sessionId; });
		if (it != m_sessions.end() && !it->stage.empty())
		{
			LoadBackground(it->stage);
		}

		// 救助数（分子）・総数（分母）はどちらもApplyCurrentTick()が毎tickの
		// penguins[].in_formation / is_alive からライブ算出する（実際のScoreManagerの
		// 総数も出現・死亡のたびに増減するため、最終値の固定表示ではなく再現する）

		// InGameTimerMenuの時計表示（残り時間の割合）が使う最大値。session.jsonには
		// 記録されていないため、ログ全体で観測された"t"の最大値（≒ステージ開始直後の値）を使う
		{
			float maxTimeSeen = 0.0f;
			for (const auto& tick : m_ticks)
			{
				maxTimeSeen = max(maxTimeSeen, tick.value("t", 0.0f));
			}
			TimeManager::GetInstance().SetMaxTime(max(maxTimeSeen, 1.0f));
		}
	}


	void ReplayScene::DrawUI()
	{
		ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(500.0f, 400.0f), ImGuiCond_FirstUseEver);
		ImGui::Begin(u8"リプレイ");

		if (m_loadedSessionId.empty())
		{
			ImGui::Text(u8"再生するログを選択してください");

			if (!m_lastLoadError.empty())
			{
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), u8"読み込みエラー: %s", m_lastLoadError.c_str());
			}

			ImGui::Separator();

			if (m_sessions.empty())
			{
				ImGui::TextDisabled(u8"（Logs/ にセッションが見つかりません）");
			}

			for (const auto& session : m_sessions)
			{
				char label[256];
				sprintf_s(label, u8"%s  [%s]  %.0f秒  救出%d  score:%.0f",
					session.id.c_str(), session.stage.c_str(),
					session.durationSec, session.rescued, session.score);

				if (ImGui::Selectable(label))
				{
					LoadSession(session.id);
				}
			}
		}
		else
		{
			ImGui::Text(u8"読み込み済み: %s", m_loadedSessionId.c_str());
			ImGui::Text(u8"tick数: %d", static_cast<int>(m_ticks.size()));
			ImGui::Separator();

			// --- 巻き戻し / 再生・一時停止 / 早送り ---
			if (ImGui::Button(u8"<< -5s"))
			{
				SeekToTime(m_playbackTime - 5.0f * m_effectiveTicksPerSecond);
			}
			ImGui::SameLine();

			const float maxPlaybackTime = GetMaxPlaybackTime();
			const bool atEnd = m_playbackTime >= maxPlaybackTime;
			if (!m_isPlaying)
			{
				if (ImGui::Button(atEnd ? u8"再生（最初から）" : u8"再生"))
				{
					// 末尾で止まっている状態から押した場合のみ先頭に戻す。
					// 一時停止からの再開では現在位置をそのまま引き継ぐ
					if (atEnd)
					{
						SeekToTime(0.0f);
					}
					m_isPlaying = true;
				}
			}
			else
			{
				if (ImGui::Button(u8"一時停止"))
				{
					m_isPlaying = false;
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(u8"+5s >>"))
			{
				SeekToTime(m_playbackTime + 5.0f * m_effectiveTicksPerSecond);
			}

			// 再生速度（負値で巻き戻し再生。Update()側でm_playbackSpeedの符号に応じて
			// タイムラインを逆方向に進める）
			// ドラッグ操作だけでは1.0にぴったり戻すのが難しいため、Ctrl+クリックでの
			// 直接数値入力に加えて等倍リセットボタンも用意する
			ImGui::SliderFloat(u8"再生速度", &m_playbackSpeed, -4.0f, 4.0f);
			ImGui::SameLine();
			if (ImGui::Button(u8"x1"))
			{
				m_playbackSpeed = 1.0f;
			}

			// --- タイムライン（好きな位置へシーク可能） ---
			const float maxTimeSeconds = maxPlaybackTime / m_effectiveTicksPerSecond;
			float currentSeconds = m_playbackTime / m_effectiveTicksPerSecond;
			if (ImGui::SliderFloat(u8"タイムライン", &currentSeconds, 0.0f, maxTimeSeconds, u8"%.1f秒"))
			{
				SeekToTime(currentSeconds * m_effectiveTicksPerSecond);
			}
			ImGui::Text(u8"再生時間: %.1f / %.1f 秒相当", m_playbackTime / m_effectiveTicksPerSecond, maxTimeSeconds);

			if (!m_hasCameraData)
			{
				ImGui::Separator();
				ImGui::TextDisabled(u8"※このログにはカメラ情報が記録されていません");

				int mode = static_cast<int>(m_noCameraDataMode);
				bool modeChanged = false;
				modeChanged |= ImGui::RadioButton(u8"親ペンギン追従", &mode, static_cast<int>(NoCameraDataMode::FollowParent));
				ImGui::SameLine();
				modeChanged |= ImGui::RadioButton(u8"インスペクター（自由視点）", &mode, static_cast<int>(NoCameraDataMode::Inspector));

				if (m_noCameraDataMode == NoCameraDataMode::FollowParent)
				{
					ImGui::TextDisabled(u8"右スティック:視点回転（追従したまま回せます）");
				}
				else
				{
					ImGui::TextDisabled(u8"左スティック:移動　右スティック:視点回転　RB1+左スティックY:FOV");
				}

				if (modeChanged)
				{
					m_noCameraDataMode = static_cast<NoCameraDataMode>(mode);
					if (m_noCameraDataMode == NoCameraDataMode::Inspector)
					{
						// 切り替え時に現在のカメラ状態から始める（視点が急に飛ばないように）
						m_inspectorCameraData = camera::CameraManager::Get().GetCurrentCameraData();
					}
				}
			}

			ImGui::Separator();
			if (ImGui::Button(u8"別のログを選び直す"))
			{
				m_loadedSessionId.clear();
				m_ticks.clear();
				m_isPlaying = false;
				std::fill(m_bearSlotActive.begin(), m_bearSlotActive.end(), false);
				std::fill(m_penguinSlotActive.begin(), m_penguinSlotActive.end(), false);
				std::fill(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), false);
			}
		}

		ImGui::Separator();
		ImGui::Text(u8"Bボタンでタイトルへ戻る");

		ImGui::End();
	}


	float ReplayScene::GetMaxPlaybackTime() const
	{
		if (m_ticks.empty()) return 0.0f;
		return static_cast<float>(m_ticks.back().value("frame", 0));
	}


	void ReplayScene::SeekToTime(float playbackTime)
	{
		if (m_ticks.empty())
		{
			m_playbackTime = 0.0f;
			m_currentTickIndex = 0;
			return;
		}

		m_playbackTime = std::clamp(playbackTime, 0.0f, GetMaxPlaybackTime());

		// "frame" <= m_playbackTime を満たす最後のインデックスを二分探索で求める。
		// 早送り・巻き戻し・タイムラインのドラッグなど、非連続な移動からも安全に呼べる
		size_t lo = 0, hi = m_ticks.size() - 1;
		while (lo < hi)
		{
			const size_t mid = lo + (hi - lo + 1) / 2;
			if (m_ticks[mid].value("frame", 0) <= m_playbackTime) lo = mid;
			else hi = mid - 1;
		}
		m_currentTickIndex = lo;
	}


	void ReplayScene::ApplyCurrentTick()
	{
		// 壊れた/想定外の形式のtickデータ（破損した.cmpファイル、手編集されたログ等）に対して
		// nlohmann::jsonが例外（型不一致・必須キー欠落など）を投げても、リプレイシーン全体を
		// クラッシュさせないよう、このフレームの反映だけをスキップして次のtickでの復帰を試みる
		try
		{
		const nlohmann::json& tick0 = m_ticks[m_currentTickIndex];
		const nlohmann::json& tick1 = m_ticks[min(m_currentTickIndex + 1, m_ticks.size() - 1)];

		const float t0 = static_cast<float>(tick0.value("frame", 0));
		const float t1 = static_cast<float>(tick1.value("frame", 0));
		float alpha = (t1 > t0) ? (m_playbackTime - t0) / (t1 - t0) : 0.0f;
		alpha = std::clamp(alpha, 0.0f, 1.0f);

		// ------ タイマー ------
		// tickの"t"はTimeManagerの残り時間をそのまま記録したもの（単調増加しないため
		// 再生の時間軸には使えないが、表示用の値としてはそのまま使える）
		InGameUIManager::GetInstance()->GetTimerMenu()->SetTime(tick0.value("t", 0.0f));

		// ------ 親ペンギン ------
		if (tick0.contains("parent"))
		{
			const ParentFields p0 = ParseParent(tick0["parent"]);
			const ParentFields p1 = tick1.contains("parent") ? ParseParent(tick1["parent"]) : p0;

			Vector3 pos;
			pos.Lerp(alpha, p0.pos, p1.pos);

			if (!m_parentActor)
			{
				m_parentActor = std::make_unique<actor::DaddyPenguin>();
				m_parentActor->StartWrapper();
			}

			Quaternion rot;
			rot.SetRotationDegY(p0.rotY + (p1.rotY - p0.rotY) * alpha);
			m_parentActor->SetPosition(pos);
			m_parentActor->SetRotation(rot);
			m_parentActor->UpdateModelOnly();

			// UpdateModelOnly() はAI/ステートマシンを動かさないため、PlayAnimation()で
			// 自然に切り替わる機会が無い。記録されたstateが変わったとき（またはInit()直後で
			// クリップ0=CommandShoutが自動再生されて止まっているとき）に明示的に切り替える
			auto& parentModel = m_parentActor->GetModelRender();
			if (!parentModel.IsPlayingAnimation() || p0.state != m_parentLastState)
			{
				parentModel.PlayAnimation(PenguinAnimIndexForState(p0.state),
					actor::ActorStateMachine::ANIMATION_INTERPOLATE_TIME);
				m_parentLastState = p0.state;
			}
		}

		// ------ シロクマ ------
		if (tick0.contains("bears"))
		{
			std::fill(m_bearSlotActive.begin(), m_bearSlotActive.end(), false);

			const auto& bears0 = tick0["bears"];
			const auto& bears1 = tick1.contains("bears") ? tick1["bears"] : bears0;

			// tick1側をid→要素のマップにしておく。エンティティ数が多いログで毎回線形探索すると
			// O(N^2)になりフレームレートに影響するため、O(N)で一度だけ引けるようにする
			std::unordered_map<int, const nlohmann::json*> bears1ById;
			bears1ById.reserve(bears1.size());
			for (const auto& cand : bears1)
			{
				bears1ById.emplace(GetEntityId(cand, -2), &cand);
			}

			for (const auto& b0json : bears0)
			{
				const BearFields b0 = ParseBear(b0json);

				// tick1側で同じidを探す。見つからなければ補間せずtick0の値をそのまま使う
				const auto it1 = bears1ById.find(b0.id);
				const BearFields b1 = (it1 != bears1ById.end()) ? ParseBear(*it1->second) : b0;

				const size_t slot = AcquireBearSlot(b0.id);
				m_bearSlotActive[slot] = true;

				Vector3 pos;
				pos.Lerp(alpha, b0.pos, b1.pos);

				Quaternion rot;
				rot.SetRotationDegY(b0.rotY + (b1.rotY - b0.rotY) * alpha);
				m_bearActors[slot]->SetPosition(pos);
				m_bearActors[slot]->SetRotation(rot);
				m_bearActors[slot]->UpdateModelOnly();

				// 親ペンギンと同様、記録されたstateの変化に応じて明示的にアニメーションを切り替える
				auto& bearModel = m_bearActors[slot]->GetModelRender();
				if (!bearModel.IsPlayingAnimation() || b0.state != m_bearLastState[slot])
				{
					bearModel.PlayAnimation(BearAnimIndexForState(b0.state),
						actor::ActorStateMachine::ANIMATION_INTERPOLATE_TIME);
					m_bearLastState[slot] = b0.state;
				}
			}
		}

		// ------ 子ペンギン・救助数・総数 ------
		// 救助数・総数の集計はここで同時に行う（以前は専用ループでParsePenguin()を
		// 呼んでいたが、更新ループでも同じ要素を再度ParsePenguin()しており、
		// 最大200体規模のログで毎フレーム二重にパースするコストがかかっていたため統合した）
		if (tick0.contains("penguins"))
		{
			std::fill(m_penguinSlotActive.begin(), m_penguinSlotActive.end(), false);

			const auto& penguins0 = tick0["penguins"];
			const auto& penguins1 = tick1.contains("penguins") ? tick1["penguins"] : penguins0;

			// 子ペンギンはステージに同時に100〜200体存在することがあり、毎tick線形探索すると
			// O(N^2)になりフレームレートに影響するため、bears同様にid→要素のマップを先に作る
			std::unordered_map<int, const nlohmann::json*> penguins1ById;
			penguins1ById.reserve(penguins1.size());
			for (const auto& cand : penguins1)
			{
				penguins1ById.emplace(GetEntityId(cand, -2), &cand);
			}

			// 実際のScoreManagerは「死亡で総数-1」「出現で総数+1」「救助（隊列入り）では総数は変わらない」
			// という増減をする。tick0の penguins[] には現存する（削除待ちを除く）子ペンギンが
			// 全て記録されているため、is_alive を除いた頭数=総数、in_formation の数=救助数として
			// 毎tickライブに再現できる（session.jsonの最終値ではなく、その時点の値を表示する）
			int liveTotal = 0;
			int liveRescued = 0;

			for (const auto& c0json : penguins0)
			{
				const PenguinFields c0 = ParsePenguin(c0json);

				if (c0.isAlive) // 死亡直後・削除待ちの個体は総数に含めない
				{
					liveTotal++;
					if (c0.inFormation) liveRescued++;
				}

				const auto it1 = penguins1ById.find(c0.id);
				const PenguinFields c1 = (it1 != penguins1ById.end()) ? ParsePenguin(*it1->second) : c0;

				const size_t slot = AcquirePenguinSlot(c0.id, c0.type);
				m_penguinSlotActive[slot] = true;

				Vector3 pos;
				pos.Lerp(alpha, c0.pos, c1.pos);

				Quaternion rot;
				rot.SetRotationDegY(c0.rotY + (c1.rotY - c0.rotY) * alpha);
				m_penguinActors[slot]->SetPosition(pos);
				m_penguinActors[slot]->SetRotation(rot);
				m_penguinActors[slot]->UpdateAtCountDownTime();

				// UpdateAtCountDownTime() は泳ぎ判定以外でアニメーションを切り替えないため、
				// 親ペンギン・シロクマと同様に記録されたstateの変化に応じて明示的に切り替える
				auto& penguinModel = m_penguinActors[slot]->GetModelRender();
				if (!penguinModel.IsPlayingAnimation() || c0.state != m_penguinLastState[slot])
				{
					penguinModel.PlayAnimation(PenguinAnimIndexForState(c0.state),
						actor::ActorStateMachine::ANIMATION_INTERPOLATE_TIME);
					m_penguinLastState[slot] = c0.state;
				}
			}

			auto* remainingChildMenu = InGameUIManager::GetInstance()->GetRemainingChildMenu();
			remainingChildMenu->SetTotalNum(liveTotal);
			remainingChildMenu->SetChildNum(liveRescued);
		}

		// ------ 渦潮 ------
		if (tick0.contains("whirlpools"))
		{
			std::fill(m_whirlpoolSlotActive.begin(), m_whirlpoolSlotActive.end(), false);

			// Update()（StateMachine）を呼ばないためUV回転は自前で進める
			// （MasterWhirlpoolParameterのuvRotationSpeedのデフォルト値と同じ1.5rad/秒を使う）
			// 一時停止・シーク中は他の見た目と同様に止めておく（再生中のみ進める）
			if (m_isPlaying)
			{
				m_whirlpoolUvRotation += g_gameTime->GetFrameDeltaTime() * m_playbackSpeed * 1.5f;
			}

			const auto& wp0 = tick0["whirlpools"];
			const auto& wp1 = tick1.contains("whirlpools") ? tick1["whirlpools"] : wp0;

			std::unordered_map<int, const nlohmann::json*> wp1ById;
			wp1ById.reserve(wp1.size());
			for (const auto& cand : wp1)
			{
				wp1ById.emplace(GetEntityId(cand, -2), &cand);
			}

			for (const auto& w0json : wp0)
			{
				const WhirlpoolFields w0 = ParseWhirlpool(w0json);

				const auto it1 = wp1ById.find(w0.id);
				const WhirlpoolFields w1 = (it1 != wp1ById.end()) ? ParseWhirlpool(*it1->second) : w0;

				const size_t slot = AcquireWhirlpoolSlot(m_whirlpoolSlotIds, m_whirlpoolSlotActive, m_whirlpoolSlotIndexById, w0.id);
				m_whirlpoolSlotActive[slot] = true;

				Vector3 pos;
				pos.Lerp(alpha, w0.pos, w1.pos);

				m_whirlpoolModels[slot]->SetPosition(pos);
				m_whirlpoolModels[slot]->SetScaleXZ(w0.scaleXZ + (w1.scaleXZ - w0.scaleXZ) * alpha);
				m_whirlpoolModels[slot]->SetUvRotation(m_whirlpoolUvRotation);
			}
		}

		// ------ カメラ ------
		if (tick0.contains("camera") && m_replayCamera)
		{
			const CameraFields cam0 = ParseCamera(tick0["camera"]);
			const CameraFields cam1 = tick1.contains("camera") ? ParseCamera(tick1["camera"]) : cam0;

			camera::CameraData data;
			data.position.Lerp(alpha, cam0.pos, cam1.pos);
			data.target.Lerp(alpha, cam0.target, cam1.target);
			data.up = Vector3::Up;
			data.fov = cam0.fov + (cam1.fov - cam0.fov) * alpha;

			m_replayCamera->SetState(data);
		}
		}
		catch (const nlohmann::json::exception&)
		{
			// このtickの反映はスキップする（ゴースト・カメラ・HUDは直前の状態のまま維持される）
		}
	}


	void ReplayScene::UpdateFallbackCamera()
	{
		if (!m_replayCamera) return;

		if (m_noCameraDataMode == NoCameraDataMode::FollowParent)
		{
			if (!m_parentActor) return;

			// 右スティックでターゲット（親ペンギン）中心にオフセットを回転させる
			const float rotX = g_pad[0]->GetRStickXF() * 0.05f;
			const float rotY = g_pad[0]->GetRStickYF() * 0.05f;
			if (rotX != 0.0f || rotY != 0.0f)
			{
				Quaternion yRotation;
				yRotation.SetRotationY(rotX);
				yRotation.Apply(m_followCameraOffset);

				Vector3 rightDir;
				rightDir.Cross(Vector3::Up, m_followCameraOffset);
				rightDir.Normalize();

				Quaternion xzRotation;
				xzRotation.SetRotation(rightDir, rotY);
				xzRotation.Apply(m_followCameraOffset);

				// 上下角度をクランプ（真上・真下まで回り込みすぎないように）
				const float length = m_followCameraOffset.Length();
				const float maxAngle = Math::DegToRad(80.0f);
				const float minAngle = Math::DegToRad(-30.0f);
				const float maxY = sinf(maxAngle) * length;
				const float minY = sinf(minAngle) * length;
				m_followCameraOffset.y = std::clamp(m_followCameraOffset.y, minY, maxY);

				const float xzLenSq = length * length - m_followCameraOffset.y * m_followCameraOffset.y;
				const float xzLen = (xzLenSq > 0.0f) ? sqrtf(xzLenSq) : 0.0f;
				Vector3 xzDir(m_followCameraOffset.x, 0.0f, m_followCameraOffset.z);
				if (xzDir.LengthSq() > FLT_EPSILON)
				{
					xzDir.Normalize();
				}
				else
				{
					xzDir.Set(0.0f, 0.0f, -1.0f);
				}
				m_followCameraOffset.x = xzDir.x * xzLen;
				m_followCameraOffset.z = xzDir.z * xzLen;
			}

			// ターゲット自体は毎フレーム親ペンギンの現在座標に追従する
			const Vector3& targetPos = m_parentActor->GetTransform().m_position;
			camera::CameraData data;
			data.target = targetPos + Vector3(0.0f, 60.0f, 0.0f);
			data.position = data.target + m_followCameraOffset;
			data.up = Vector3::Up;

			m_replayCamera->SetState(data);
		}
		else
		{
			UpdateInspectorCamera();
		}
	}


	void ReplayScene::UpdateInspectorCamera()
	{
		if (!m_replayCamera) return;

		// fov調整
		if (g_pad[0]->IsPress(enButtonRB1))
		{
			const float value = g_pad[0]->GetLStickYF();
			m_inspectorCameraData.fov += value * 0.05f;
			m_replayCamera->SetState(m_inspectorCameraData);
			return;
		}

		// 左スティックで平行移動
		{
			Vector3 inputDirection;
			inputDirection.x = g_pad[0]->GetLStickXF();
			inputDirection.z = g_pad[0]->GetLStickYF();

			Vector3 forward = CameraSystem::Get().GetMainCamera().GetForward();
			Vector3 right = CameraSystem::Get().GetMainCamera().GetRight();
			forward.y = 0.0f;
			right.y = 0.0f;

			right *= inputDirection.x;
			forward *= inputDirection.z;

			Vector3 direction = right + forward;
			direction.Normalize();
			direction.Scale(10.0f);

			m_inspectorCameraData.position += direction;
			m_inspectorCameraData.target += direction;
		}

		// 右スティックでターゲット中心に回転
		{
			const float rotX = g_pad[0]->GetRStickXF() * 0.05f;
			const float rotY = g_pad[0]->GetRStickYF() * 0.05f;

			Quaternion yRotation;
			yRotation.SetRotationY(rotX);
			Vector3 toVector = m_inspectorCameraData.position - m_inspectorCameraData.target;
			yRotation.Apply(toVector);

			Vector3 rightDir;
			rightDir.Cross(Vector3::Up, toVector);
			rightDir.Normalize();

			Quaternion xzRotation;
			xzRotation.SetRotation(rightDir, rotY);
			xzRotation.Apply(toVector);

			const float length = toVector.Length();
			const float maxAngle = Math::DegToRad(85.0f);
			const float minAngle = Math::DegToRad(-85.0f);
			const float maxY = sinf(maxAngle) * length;
			const float minY = sinf(minAngle) * length;

			toVector.y = std::clamp(toVector.y, minY, maxY);

			const float xzLenSq = length * length - toVector.y * toVector.y;
			const float xzLen = (xzLenSq > 0.0f) ? sqrtf(xzLenSq) : 0.0f;

			Vector3 xzDir;
			xzDir.Set(toVector.x, 0.0f, toVector.z);
			if (xzDir.LengthSq() > FLT_EPSILON)
			{
				xzDir.Normalize();
			}
			else
			{
				xzDir.Set(0.0f, 0.0f, -1.0f);
			}

			toVector.x = xzDir.x * xzLen;
			toVector.z = xzDir.z * xzLen;

			m_inspectorCameraData.position = m_inspectorCameraData.target + toVector;
		}

		m_replayCamera->SetState(m_inspectorCameraData);
	}


	void ReplayScene::LoadBackground(const std::string& stageName)
	{
		if (m_backgroundLoaded && m_loadedStageName == stageName) return;

		UnloadBackground();

		// InGameSceneBase の各ステージシーン（例: NormalInGameScene）が返すパスと同じ命名規則
		const std::string stageJsonPath  = "Assets/parameter/stage/StageObject_" + stageName + ".json";
		const std::string terrainJsonPath = "Assets/parameter/stage/TerrainConfig_" + stageName + ".json";
		const std::string oceanJsonPath  = "Assets/parameter/nature/oceanParameter_" + stageName + ".json";
		const std::string oceanBinPath   = "Assets/parameter/nature/oceanParameter_" + stageName + ".bin";

		// StageSystem・Oceanが内部でPBRパラメータ・ParameterManagerを参照するため生成しておく
		graphics::PBRStatus::CreateInstance();

		actor::StageSystem::CreateInstance();
		actor::StageSystem::GetInstance()->LoadStageObjectsFromJson(stageJsonPath);
		actor::StageSystem::GetInstance()->InitTerrainFromJson(terrainJsonPath);

		m_skyCube = NewGO<SkyCube>(0);
		m_skyCube->SetType(enSkyCubeType_Clear);
		m_skyCube->SetScale(SKY_CUBE_SCALE);
		m_skyCube->SetLuminance(0.8f);

		nature::Ocean::CreateInstance();
		nature::Ocean::GetInstance()->Start(oceanBinPath.c_str(), oceanJsonPath.c_str());

		// Nature Objectは登録順に描画される。渦潮用のReplayScene自身の登録がOceanより先に
		// 済んでいると、海面が渦潮の上から描かれて隠れてしまうため、Oceanの後ろに登録し直す
		g_renderingEngine->UnregisterNatureObject(this);
		g_renderingEngine->RegisterNatureObject(this);

		m_backgroundLoaded = true;
		m_loadedStageName = stageName;
	}


	void ReplayScene::UnloadBackground()
	{
		if (!m_backgroundLoaded) return;

		DeleteGO(m_skyCube);
		m_skyCube = nullptr;

		nature::Ocean::DestroyInstance();
		actor::StageSystem::DestroyInstance();
		graphics::PBRStatus::DestroyInstance();

		m_backgroundLoaded = false;
		m_loadedStageName.clear();
	}


	size_t ReplayScene::AcquireBearSlot(int id)
	{
		// 長時間のログでは同時に存在するシロクマ・子ペンギン数が多くなり、毎tick・毎エンティティで
		// 線形探索するとO(N^2)になってフレームレートに影響するため、id→スロット番号をO(1)で引く
		if (const auto it = m_bearSlotIndexById.find(id); it != m_bearSlotIndexById.end())
		{
			return it->second;
		}

		auto enemy = std::make_unique<actor::Enemy>();
		enemy->StartWrapper();
		m_bearActors.push_back(std::move(enemy));
		m_bearSlotIds.push_back(id);
		m_bearSlotActive.push_back(false);
		m_bearLastState.push_back("");

		const size_t slot = m_bearSlotIds.size() - 1;
		m_bearSlotIndexById.emplace(id, slot);
		return slot;
	}


	size_t ReplayScene::AcquirePenguinSlot(int id, const std::string& typeStr)
	{
		if (const auto it = m_penguinSlotIndexById.find(id); it != m_penguinSlotIndexById.end())
		{
			return it->second;
		}

		auto penguin = std::make_unique<actor::ChildPenguin>();
		penguin->SetChildPenguinType(ParseChildPenguinType(typeStr));
		penguin->StartWrapper();
		m_penguinActors.push_back(std::move(penguin));
		m_penguinSlotIds.push_back(id);
		m_penguinSlotActive.push_back(false);
		m_penguinLastState.push_back("");

		const size_t slot = m_penguinSlotIds.size() - 1;
		m_penguinSlotIndexById.emplace(id, slot);
		return slot;
	}


	size_t ReplayScene::AcquireWhirlpoolSlot(std::vector<int>& slotIds, std::vector<bool>& slotActive,
		std::unordered_map<int, size_t>& slotIndexById, int id)
	{
		if (const auto it = slotIndexById.find(id); it != slotIndexById.end())
		{
			return it->second;
		}

		auto whirlpool = std::make_unique<nature::Whirlpool>();
		whirlpool->StartWrapper();
		// リプレイではUpdate()（状態遷移）を呼ばないため、Start()が自動再生したエフェクトを
		// ここで明示的に止める。止めないと二度と止まらずセッション中の全渦潮分が蓄積してしまう
		whirlpool->StopEffect();
		m_whirlpoolModels.push_back(std::move(whirlpool));
		slotIds.push_back(id);
		slotActive.push_back(false);

		const size_t slot = slotIds.size() - 1;
		slotIndexById.emplace(id, slot);
		return slot;
	}
}
