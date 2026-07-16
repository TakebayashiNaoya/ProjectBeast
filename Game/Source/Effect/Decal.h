/**
 * @file Decal.h
 * @brief 投影デカール（地形の凹凸に沿った足跡）1枚分の本体クラス
 * @details 呼び出しのたびに、地形の高さに沿った小さな格子メッシュを組み立てて描画する。
 *          直接使わず、DecalManager::Get().SpawnFootprint(...) 経由で生成すること。
 */
#pragma once
#include "Resource/ModelResource.h"
#include <memory>
#include <string>
#include <vector>

namespace app
{
	namespace effect
	{
		/** @brief デカールの見た目種別（地面によって変えたい場合に使用） */
		enum class DecalKind
		{
			SnowFootprint,
			GrassFootprint,
			RockFootprint,
		};

		/**
		 * @brief 投影デカール1枚分の本体クラス
		 */
		class Decal
		{
		public:
			Decal() = default;
			~Decal() = default;

			Decal(const Decal&) = delete;
			Decal& operator=(const Decal&) = delete;
			Decal(Decal&&) noexcept = default;
			Decal& operator=(Decal&&) noexcept = default;

			/**
			 * @brief 軽い準備処理（ModelRenderの確保と、自分専用バンクキーの決定のみ）
			 * @param slotIndex プール内での自分のスロット番号（バンクキーの一意化に使う）
			 */
			void Prepare(int slotIndex);

			/**
			 * @brief 地形の高さに沿った格子メッシュを再構築し、表示を開始する
			 * @param gridPositions  格子頂点のワールド座標（gridResolution×gridResolution個、行優先で格納）
			 * @param gridResolution 1辺あたりの頂点数（例: 5なら5x5=25頂点）
			 * @param texture        貼り付けるテクスチャ
			 * @param color          テクスチャに掛けるティント色
			 */
			void SetupProjected(
				const std::vector<Vector3>& gridPositions,
				int gridResolution,
				nsK2EngineLow::Texture* texture,
				const Vector4& color
			);

			/**
			 * @brief 更新処理（寿命管理）
			 * @return 生存していれば true、寿命切れなら false
			 */
			bool Update(float deltaTime);

			/** @brief 描画処理 */
			void Render(RenderContext& rc);

			/** @brief このデカールが表示中かどうか */
			bool IsActive() const { return m_isActive; }

			/** @brief 残り表示時間を取得する（Manager側での使い回し判定用） */
			float GetRemainingLife() const { return m_remainingLife; }

			/** @brief 寿命とフェードアウト時間を設定する */
			void SetLife(float lifeSeconds, float fadeOutSeconds)
			{
				m_remainingLife = lifeSeconds;
				m_fadeOutSeconds = fadeOutSeconds;
			}

		private:
			/** 定数バッファ(b2)用の構造体 */
			struct cbDecal
			{
				float alpha = 1.0f;
				float padding[3] = { 0.0f, 0.0f, 0.0f };
			};

			/** @brief 格子頂点からTkmFileを組み立て、自分専用のバンクキーに登録する */
			void BuildGridMesh(const std::vector<Vector3>& gridPositions, int gridResolution);

		private:
			std::unique_ptr<nsBeastEngine::ModelRender> m_modelRender;
			cbDecal m_cb;

			/** @brief このスロット専用のTkmFileバンクキー（"decal_patch_0"など） */
			std::string m_tkmKey;

			float m_remainingLife = 0.0f;
			float m_fadeOutSeconds = 0.5f;
			bool  m_isActive = false;
		};
	}
}
