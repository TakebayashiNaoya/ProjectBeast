/**
 * @file Whirlpool.h
 * @brief 渦潮のクラス
 * @author 藤谷、竹林
 */
#pragma once
#include "IObject.h"
#include "Source/Core/Transform.h"
#include "Source/Util/Curve.h"
#include "WhirlpoolPowerSystem.h"
#include "WhirlpoolParameter.h"
#include "Source/Effect/EffectManager.h"


namespace app
{
	namespace nature
	{
		/**
		 * @brief 渦潮のクラス
		 */
		class Whirlpool : public IObject
		{
		public:
			/**
			 * @brief 渦潮の状態
			 */
			enum class EnWhirlpoolState : uint8_t
			{
				Bigger,
				Smaller,
				Stay,
				Num,
				None = Num
			};

			/** メッシュの半径（ワールド単位）。フラスタムカリングの球半径計算に使用する */
			static constexpr float MESH_RADIUS = 1000.0f;


		private:
			/**
			 * @brief 頂点構造体
			 * @details OceanMeshと同じ頂点レイアウト（Material::InitPipelineState準拠）
			 */
			struct WhirlpoolVertex
			{
				Vector3 pos;		/** 座標 */
				Vector3 normal;		/** 法線 */
				Vector3 tangent;	/** 接線 */
				Vector3 biNormal;	/** 従法線 */
				Vector2 uv;			/** テクスチャ座標 */
				int     indices[4];	/** ボーンインデックス（未使用・ゼロ埋め） */
				Vector4 weights;	/** ボーンウェイト（未使用・ゼロ埋め） */
			};

			/**
			 * @brief 共通定数バッファ（b0）
			 */
			struct SCommonConstantBuffer
			{
				Matrix  mWorld;		/** ワールド行列 */
				Matrix  mView;		/** ビュー行列 */
				Matrix  mProj;		/** プロジェクション行列 */
				Vector4 mulColor;	/** 乗算カラー */
			};

			/**
			 * @brief 拡張定数バッファ（b1）
			 * @details UVスクロール（渦回転）に使用する
			 */
			struct SWhirlpoolConstantBuffer
			{
				float uvRotation;	/** UV回転角度（ラジアン） */
				float padding[3];	/** パディング（16バイトアライン） */
			};


		public:
			/**
			 * @brief 渦潮の状態を取得
			 * @return 渦潮の状態
			 */
			EnWhirlpoolState GetState() const { return m_state; }
			/**
			 * @brief 渦潮のインデックスを取得
			 * @return 渦潮のインデックス
			 */
			uint8_t GetIndex() const { return m_index; }
			/**
			 * @brief 渦潮のインデックスを設定
			 * @param index 渦潮のインデックス
			 */
			void SetIndex(const uint8_t index) { m_index = index; }
			/**
			 * @brief 座標を設定
			 * @param position 座標
			 */
			void SetPosition(const Vector3& position) { m_transform.m_position = position; }
			/**
			 * @brief トランスフォームを取得
			 * @return トランスフォームの参照
			 */
			const core::Transform& GetTransform() const { return m_transform; }
			/**
			 * @brief 最大スケールXZを取得
			 * @return 最大スケールXZ（Bigger完了時のスケール）
			 */
			float GetMaxScaleXZ() const { return m_maxScaleXZ; }


		public:
			void Start() override;
			void Update() override;
			void Render(RenderContext& rc) override;


		public:
			Whirlpool();
			~Whirlpool() = default;


		private:
			/**
			 * @brief 渦潮の状態遷移を行う関数
			 */
			void StateMachine();

			/**
			 * @brief 円形グリッド頂点・インデックスを生成する
			 */
			void CreateCircleMesh();

			/**
			 * @brief シェーダーを初期化する
			 */
			void InitShaders();

			/**
			 * @brief ルートシグネチャを初期化する
			 * @details
			 *   - b0: SCommonConstantBuffer
			 *   - b1: SWhirlpoolConstantBuffer
			 *   - t0: アルベドマップ
			 */
			void InitRootSignature();

			/**
			 * @brief パイプラインステートを初期化する
			 * @details アルファブレンド有効（渦潮テクスチャの円形マスク用）
			 */
			void InitPipelineState();

			/**
			 * @brief ディスクリプタヒープを初期化する
			 */
			void InitDescriptorHeap();

			/**
			 * @brief 毎フレーム頂点バッファのYをOceanの波面に合わせて更新する
			 */
			void UpdateVertexHeights();

			/**
			 * @brief 渦潮エフェクトを再生する
			 */
			void PlayWhirlpoolEffect();

			/**
			 * @brief 渦潮エフェクトを停止する
			 */
			void StopWhirlpoolEffect();

			/**
			 * @brief 渦潮エフェクトのスケールを更新する
			 */
			void UpdateWhirlpoolEffectScale();


		private:
			/**
			 * @brief グリッドの分割設定
			 * @details これらの値を変更することで頂点密度を調整できる
			 */
			static constexpr int   NUM_RINGS = 8;		/** リング数（中心から外周までの分割数） */
			static constexpr int   NUM_SEGMENTS = 32;	/** 円周方向の分割数 */

			/** 頂点データ（CPU側キャッシュ・毎フレーム更新） */
			std::vector<WhirlpoolVertex> m_vertices;

			Texture        m_albedoMap;					/** アルベドマップ */

			VertexBuffer   m_vertexBuffer;				/** 頂点バッファ */
			IndexBuffer    m_indexBuffer;				/** インデックスバッファ */
			int            m_indexCount = 0;			/** インデックス数 */

			Shader* m_vs = nullptr;						/** 頂点シェーダー */
			Shader* m_ps = nullptr;						/** ピクセルシェーダー */

			RootSignature  m_rootSignature;				/** ルートシグネチャ */
			PipelineState  m_pipelineState;				/** パイプラインステート */

			ConstantBuffer m_commonConstantBuffer;		/** 共通定数バッファ（b0） */
			ConstantBuffer m_whirlpoolConstantBuffer;	/** 渦潮定数バッファ（b1） */

			DescriptorHeap m_descriptorHeap;			/** ディスクリプタヒープ */

			/** 渦潮のトランスフォーム */
			core::Transform m_transform;
			/** 渦潮の拡大カーブ */
			app::util::Vector3Curve m_scaleBigger;
			/** 渦潮の縮小カーブ */
			app::util::Vector3Curve m_scaleSmaller;
			/** 渦潮のインデックス */
			uint8_t m_index;
			/** 渦潮の状態 */
			EnWhirlpoolState m_state;
			/** 渦潮のタイマー */
			float m_timer;
			/** UV回転角度（ラジアン）：ピクセルシェーダーに渡してテクスチャを回す */
			float m_uvRotation;
			/** Bigger完了時の最大スケールXZ */
			float m_maxScaleXZ;
			/** 渦潮エフェクトのハンドル */
			EffectHandle m_effectHandle;
			/** 渦潮の引き寄せ、押し出しを管理するクラス */
			std::unique_ptr<WhirlpoolPowerSytem> m_whirlpoolPowerSystem;
		};
	}
}