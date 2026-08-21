/**
 * @file DecalManager.h
 * @brief デカールを管理するクラス
 * @author 立山
 */
#pragma once
#include "Decal.h"
#include <array>
#include <vector>

namespace app { namespace actor { class TerrainObject; } }

namespace app {
	namespace effect {
		class DecalManager : public Noncopyable {
		public:
			static DecalManager& Get() { static DecalManager instance; return instance; }

			/**
			 * @brief 足跡デカールを1つ生成する
			 * @param position 生成位置
			 * @param yawRadian Y軸回転（ラジアン）
			 * @param kind デカールの種類
			 * @param autoDetectSurface trueの場合、地形サーフェスに応じてkindとcolorを自動判定する
			 */
			void SpawnFootprint(const Vector3& position, float yawRadian, DecalKind kind = DecalKind::SnowFootprint,
				float size = DEFAULT_FOOTPRINT_SIZE, float lifeSeconds = DEFAULT_LIFE_SECONDS,
				float fadeOutSeconds = DEFAULT_FADE_OUT_SECONDS, const Vector4& color = DEFAULT_FOOTPRINT_COLOR,
				bool autoDetectSurface = true, int priority = DEFAULT_PRIORITY);

			void Update();
			void Render(RenderContext& rc);


		private:
			DecalManager() = default;
			~DecalManager() = default;

			/**
			 * @brief テクスチャ・デカール配列・共有メッシュの初期化を行う
			 * @details 初回呼び出し時のみ実行され、以降は何もしない
			 */
			void EnsureInited();

			/**
			 * @brief デカール種別に対応するテクスチャを取得する
			 * @param kind デカールの種類
			 * @return 対応するテクスチャへのポインタ
			 */
			nsK2EngineLow::Texture* GetTextureForKind(DecalKind kind);

			/** @brief 地面へのレイキャスト結果 */
			struct GroundHitInfo { bool isHit = false; Vector3 position; Vector3 normal = Vector3::Up; };

			/**
			 * @brief 指定座標の真下に向けて地面へレイキャストする
			 * @param fromPosXZ レイの発射基準となるXZ座標（Yは内部でオフセットして調整）
			 * @return ヒット結果（位置・法線・ヒットフラグ）
			 */
			GroundHitInfo RaycastGround(const Vector3& fromPosXZ) const;

			/**
			 * @brief 現在のステージ地形から、デカール描画に必要な凹凸判定用パラメータを構築する
			 * @return 地形の高さ情報（ハイトマップ・サイズ・スケール等）
			 */
			TerrainHeightInfo BuildTerrainHeightInfo() const;

			/**
			 * @brief 負荷計測用のベンチマークを1フレーム進める
			 * @details 環境変数 DECAL_BENCH=1 のときだけ動く。決まった位置・決まった間隔で
			 *          足跡を出し、「1種類だけ出す区間」と「種類を混ぜる区間」を同じ実行の中で
			 *          並べることで、種類の切り替えにいくらかかっているのかを切り分ける。
			 *          プレイの内容に左右されないよう、AIではなくここが自分で足跡を出す
			 */
			void UpdateBenchmark();


			/**
			 * 種類ごとのプールの枚数。
			 *
			 * 以前は全種類で1本の64枚プールを共有していたため、スロットが別の種類に
			 * 使い回されるたびに Decal::Spawn が ModelRender::InitFromLoaded を
			 * 呼び直していた。実測では1回あたり約10ms、Normalステージ118秒で314回。
			 * 種類ごとにプールを分けると m_kind != kind が成立しなくなり、
			 * 作り直しはスロットごとの初回1回だけになる。
			 *
			 * 枚数を24にしたのは実測から。同時に生きているデカールは全種類合わせて
			 * 平均6.9枚・最大46枚で、生存時間は1秒しかない。24枚あれば1種類に
			 * 偏った瞬間でも足りる。空き待ちで捨てた回数は DecalProfiler の
			 * spawnNoSlot / evictions で確認できる
			 */
			static constexpr int MAX_DECAL_NUM_PER_KIND = 24;

			/** 子ペンギン（優先度0）が種類ごとのプールで使ってよい上限。従来の 40/64 と同じ比率 */
			static constexpr int MAX_CHILD_DECAL_NUM_PER_KIND = 15;
			static constexpr float RAY_START_HEIGHT = 50.0f;
			static constexpr float RAY_MAX_DISTANCE = 200.0f;
			static constexpr float PROJECTED_SURFACE_OFFSET = 1.0f;

			static constexpr float DEFAULT_FOOTPRINT_SIZE = 40.0f;
			static constexpr float DEFAULT_LIFE_SECONDS = 8.0f;
			static constexpr float DEFAULT_FADE_OUT_SECONDS = 5.0f;
			static inline const Vector4 DEFAULT_FOOTPRINT_COLOR = { 1.0f, 1.0f, 1.0f, 1.0f };
			static constexpr int DEFAULT_PRIORITY = 1;

			/** 種類ごとのデカールのプール。添字は DecalKind をintにしたもの */
			std::array<std::vector<Decal>, DECAL_KIND_NUM> m_decalPools;

			nsK2EngineLow::Texture m_snowFootprintTex;
			nsK2EngineLow::Texture m_grassFootprintTex;
			nsK2EngineLow::Texture m_rockFootprintTex;
			nsK2EngineLow::Texture m_bearFootprintTex;

			bool m_isInited = false;

			//------------------------------------------------
			// 負荷計測用のベンチマーク（DECAL_BENCH=1 のときだけ使う）
			//------------------------------------------------
			/** ベンチマークのフェーズ */
			enum class BenchPhase {
				Warmup,     //!< 地形のロード待ち・暖機。足跡は出さない
				SingleKind, //!< 1種類だけを出す。InitFromLoaded は最初の1回しか通らない
				MixedKind,  //!< 種類を毎回変える。毎回 InitFromLoaded を通る
				AutoDetect, //!< 実際と同じくスプラットマップから種類を決める
				Finished,   //!< 計測終了
			};

			bool       m_isBenchEnabled = false;
			BenchPhase m_benchPhase = BenchPhase::Warmup;
			int        m_benchFrameInPhase = 0;
			int        m_benchFramesPerPhase = 600;
			float      m_benchSpawnsPerSecond = 12.0f;
			float      m_benchSpawnAccumulator = 0.0f;
			int        m_benchSpawnCounter = 0;
			bool       m_isBenchQuitOnFinish = false;
		};
	}
}