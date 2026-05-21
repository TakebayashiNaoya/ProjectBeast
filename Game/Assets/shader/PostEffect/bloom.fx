/*!
 * @brief ブルーム（輝度抽出・通常ブルーム合成）
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

/*!
 * @brief 輝度抽出パラメーター（輝度抽出スプライト専用）
 */
cbuffer LuminanceCb : register(b1)
{
    float luminanceThreshold; // 輝度抽出のしきい値
    float3 luminancePad;      // パディング
};

/*!
 * @brief ブルーム合成パラメーター（合成スプライト専用）
 */
cbuffer BloomFinalCb : register(b1)
{
    float bloomIntensity; // ブルームの強度
    float3 bloomPad;      // パディング
};

struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

Texture2D<float4> mainRenderTargetTexture : register(t0); // 輝度抽出用：メインRTのテクスチャ
Texture2D<float4> g_bokeTexture           : register(t0); // 通常ブルーム合成用：ボケテクスチャ
sampler Sampler : register(s0);

/*!
 * @brief 頂点シェーダー
 */
PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

/////////////////////////////////////////////////////////
// 輝度抽出用
/////////////////////////////////////////////////////////
/*!
 * @brief 輝度抽出ピクセルシェーダー
 * @details luminanceThreshold を超える明るさのピクセルのみを出力する
 */
float4 PSSamplingLuminance(PSInput In) : SV_Target0
{
    float4 color = mainRenderTargetTexture.Sample(Sampler, In.uv);

    // 輝度を計算する（人間の視覚特性に合わせた係数）
    float t = dot(color.xyz, float3(0.2125f, 0.7154f, 0.0721f));

    // しきい値未満のピクセルを破棄する
    clip(t - luminanceThreshold);

    return color;
}

/////////////////////////////////////////////////////////
// 通常ブルーム合成用
/////////////////////////////////////////////////////////
/*!
 * @brief 通常ブルーム合成ピクセルシェーダー
 * @details 1枚のボケテクスチャを bloomIntensity 倍して加算合成する
 */
float4 PSBloomFinalNormal(PSInput In) : SV_Target0
{
    float4 color = g_bokeTexture.Sample(Sampler, In.uv);
    color.rgb *= bloomIntensity;
    color.a = 1.0f;
    return color;
}
