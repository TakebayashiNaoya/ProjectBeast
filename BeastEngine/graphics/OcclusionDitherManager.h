/**
 * @file OcclusionDitherManager.h
 * @brief ディザリングを管理するクラス
 */
#pragma once
#include <list>


namespace nsBeastEngine
{
	class ModelRender;


	/**
	 * @brief ディザリング用の定数バッファ
	 * @details
	 *   RenderToGBuffer.fx の cbuffer DitherCb : register(b3) に対応する。
	 *   16バイト境界を合わせるためにパディングを含む。
	 *   OcclusionDitherManager が所有・更新する。
	 */
	struct SDitherCb
	{
		/** カメラのワールド座標 */
		Vector3 cameraWorldPos = Vector3::Zero;
		/** 円柱判定の半径（ワールド空間単位）。この範囲外のピクセルはディザリングしない */
		float cylinderRadius = 100.0f;
		/** プレイヤーのワールド座標 */
		Vector3 targetWorldPos = Vector3::Zero;
		/** 前後判定のオフセット。正の値でカメラ寄りにわずかに余裕を持たせる */
		float depthBias = 10.0f;
		/** ディザリング強度（0.0f=オフ, 0.5f=50%透過, 1.0f=ほぼ完全消去） */
		float ditherStrength = 0.5f;
		/** パディング（16バイト境界用） */
		float pad[3] = { 0.0f, 0.0f, 0.0f };
	};


	/**
	 * @brief カメラとプレイヤーの間の遮蔽物をディザリング透過させるマネージャー
	 * @details
	 *   毎フレームUpdate()を呼ぶことで、カメラとプレイヤーのワールド座標を更新し、
	 *   登録済みの全ModelRenderにDitherCb(b3)をセットする。
	 *   シェーダー側でカメラ・ターゲット間の円柱判定とBayerディザリングを行う。
	 *   シングルトンとして使用する。
	 */
	class OcclusionDitherManager
	{
	private:
		/**
		 * @brief ディザリング対象の管理エントリ
		 * @details
		 *   Register時に生成され、Unregister時に削除される。
		 *   cbのアドレスをModelRenderのb3に渡すため、
		 *   std::listで管理しアドレスが不変であることを保証する。
		 */
		struct DitherEntry
		{
			/** 照合用のModelRenderポインタ */
			ModelRender* modelRender = nullptr;
			/** このエントリ固有のディザリング用定数バッファ */
			SDitherCb cb;
		};


	public:
		/**
		 * @brief ディザリング対象を登録する
		 * @details
		 *   SetPlayerTarget()で登録済みのModelRenderと同一の場合は
		 *   遮蔽対象リストに追加しない（プレイヤー自身が透過されるのを防ぐ）。
		 *   cbのポインタをModelRenderのb3にセットする。
		 * @param modelRender ディザリングを適用するモデルレンダー
		 * @param ditherStrength ディザリング強度（0.0f=オフ, 0.5f=デフォルト, 1.0f=最大）
		 * @param cylinderRadius 円柱判定の半径（ワールド空間単位）。この範囲外のピクセルはディザリングしない。デフォルトは30.0f。
		 */
		void Register(ModelRender* modelRender, float ditherStrength = 0.5f, float cylinderRadius = 30.0f);

		/**
		 * @brief ディザリング対象の登録を解除する
		 * @param modelRender Register時に渡したモデルレンダー
		 */
		void Unregister(ModelRender* modelRender);

		/**
		 * @brief プレイヤーのModelRenderをターゲットとして設定する
		 * @details
		 *   設定されたModelRenderは遮蔽対象リストに追加されない。
		 *   Register()より先に呼ばれた場合はm_playerModelRender_に保存し、
		 *   後からRegister()が呼ばれても追加しない。
		 *   Register()より後に呼ばれた場合は遮蔽対象リストから除外する。
		 * @param modelRender プレイヤーのモデルレンダー
		 */
		void SetPlayerTarget(ModelRender* modelRender);

		/**
		 * @brief 毎フレーム更新処理
		 * @details
		 *   カメラとプレイヤーのワールド座標を取得してDitherCb(b3)を更新し、
		 *   登録済み全ModelRenderにSetExpandConstantBuffer3()でセットする。
		 */
		void Update();


	private:
		OcclusionDitherManager() = default;
		~OcclusionDitherManager() = default;


	private:
		/**
		 * @brief 登録されたディザリング対象のリスト
		 * @details
		 *   std::listはpush_back後も既存要素のアドレスが不変であることが保証されている。
		 *   DitherEntry::cbのポインタをModelRenderに渡すため、アドレス安定性が必須。
		 */
		std::list<DitherEntry> m_entries;

		/** プレイヤーのModelRender（遮蔽対象リストには含まれない） */
		ModelRender* m_playerModelRender = nullptr;


		/**
		 * シングルトン関連
		 */
	private:
		static OcclusionDitherManager* m_instance;


	public:
		static void Initialize()
		{
			if (m_instance == nullptr)
			{
				m_instance = new OcclusionDitherManager();
			}
		}
		static OcclusionDitherManager& Get() { return *m_instance; }
		static void Finalize()
		{
			if (m_instance)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}
	};
}