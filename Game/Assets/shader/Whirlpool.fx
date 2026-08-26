/**
 * @file Whirlpool.fx
 * @brief 渦潮描画シェーダー
 */

// 共通定数バッファ（b0）
cbuffer CommonCb : register(b0)
{
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;
	float4   mulColor;
};

// 渦潮定数バッファ（b1）
// C++側 Whirlpool::SWhirlpoolConstantBuffer と一致させること
cbuffer WhirlpoolCb : register(b1)
{
	float    uvRotation;		// UV回転角度（ラジアン）
	float3   padding;
	float4x4 lvpMatrix0;		// ライトビュープロジェクション行列（近景）
	float4x4 lvpMatrix1;		// ライトビュープロジェクション行列（中景）
	float4x4 lvpMatrix2;		// ライトビュープロジェクション行列（遠景）
	float    shadowAmbientRate;	// 影の中で明るさを何割残すか
	float3   padding2;
};

// アルベドマップ（t0）
Texture2D<float4> albedoMap : register(t0);
// キャラクターや地形の影を渦潮に映すためのカスケードシャドウマップ
// 渦潮自身は影を落とさない（キャスターには登録していない）
// C++側 Whirlpool::InitDescriptorHeap() の登録順と一致させること
Texture2D<float4> shadowMap0 : register(t1);
Texture2D<float4> shadowMap1 : register(t2);
Texture2D<float4> shadowMap2 : register(t3);

// サンプラー（s0）
SamplerState albedoSampler : register(s0);

// 影の判定は Shadow.h の CalcShadowRate() を使う（海・ディファードと共通）
#include "Shadow.h"


// 頂点シェーダーの入力
struct VSInput
{
	float4 pos      : POSITION;
	float3 normal   : NORMAL;
	float3 tangent  : TANGENT;
	float3 biNormal : BINORMAL;
	float2 uv       : TEXCOORD0;
	int4   indices  : BLENDINDICES;
	float4 weights  : BLENDWEIGHT;
};

// ピクセルシェーダーの入力（頂点シェーダーの出力）
struct PSInput
{
	float4 pos      : SV_POSITION;
	float2 uv       : TEXCOORD0;
	float3 worldPos : TEXCOORD1;	// 影の判定に使うワールド座標
};


/**
 * @brief 頂点シェーダー
 * @details TRS変換のみ。Y変形はCPU側（Whirlpool::UpdateVertexHeights）で処理済み。
 */
PSInput VSMain(VSInput input)
{
	PSInput output;

	float4 worldPos = mul(mWorld, input.pos);
	float4 viewPos  = mul(mView,  worldPos);
	output.pos      = mul(mProj,  viewPos);
	output.uv       = input.uv;
	output.worldPos = worldPos.xyz;

	return output;
}


/**
 * @brief ピクセルシェーダー
 * @details UVを中心基点で回転させてサンプリングすることで渦が回って見える。
 */
float4 PSMain(PSInput input) : SV_Target
{
	// UVを中心（0.5, 0.5）基点で回転させる
	float2 centeredUv = input.uv - float2(0.5f, 0.5f);

	float sinVal, cosVal;
	sincos(uvRotation, sinVal, cosVal);

	float2 rotatedUv = float2(
		centeredUv.x * cosVal - centeredUv.y * sinVal,
		centeredUv.x * sinVal + centeredUv.y * cosVal
	);

	float2 finalUv = rotatedUv + float2(0.5f, 0.5f);

	float4 color = albedoMap.Sample(albedoSampler, finalUv);

	// mulColorを乗算する
	color *= mulColor;

	// キャラクターや地形の影を受ける（渦潮自身は影を落とさない）
	// 渦潮はライティングを行わないため、影の割合をそのまま暗さとして掛ける。
	// 濃さはデバッグUIから調整できる
	float shadowRate = CalcShadowRate(
		input.worldPos,
		lvpMatrix0, lvpMatrix1, lvpMatrix2,
		shadowMap0, shadowMap1, shadowMap2,
		albedoSampler);
	color.rgb *= lerp(shadowAmbientRate, 1.0f, shadowRate);

	return color;
}
