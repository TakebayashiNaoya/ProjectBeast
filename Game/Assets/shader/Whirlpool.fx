/**
 * @file Whirlpool.fx
 * @brief 渦潮描画シェーダー
 * @author 竹林
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
cbuffer WhirlpoolCb : register(b1)
{
	float uvRotation;	// UV回転角度（ラジアン）
	float3 padding;
};

// アルベドマップ（t0）
Texture2D<float4> albedoMap : register(t0);

// サンプラー（s0）
SamplerState albedoSampler : register(s0);


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
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
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

	return color;
}
