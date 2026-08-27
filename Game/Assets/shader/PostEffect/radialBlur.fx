/*!
 * @brief ラジアルブラー（放射状ブラー）
 * @details シロクマの咆哮など「衝撃の瞬間」に、画面中心へ向かって
 *          放射状にサンプリングをずらしてブラーをかける。
 *          強度はC++側（RadialBlur::Render）が毎フレーム減衰させて渡してくる。
 *          strength = 0 のフレームではC++側がパス自体をスキップするので、
 *          このシェーダーは strength > 0 のときしか走らない。
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

/*!
 * @brief ラジアルブラーパラメーター
 * @details C++側の RadialBlur::SRadialBlurCb と一致させること
 */
cbuffer RadialBlurCb : register(b1)
{
    float strength;     // ブラーの強さ（UV空間での最大サンプリング距離。0.0〜0.1程度）
    float3 padding;
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

Texture2D<float4> mainRenderTargetTexture : register(t0); // メインRTのテクスチャ
sampler Sampler : register(s0);

/** サンプリング数。増やすと滑らかになるが重くなる */
static const int NUM_SAMPLES = 12;

/** ブラーの中心（画面中央） */
static const float2 BLUR_CENTER = float2(0.5f, 0.5f);

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

/*!
 * @brief ピクセルシェーダー
 * @details 中心へ向かうベクトルに沿って複数回サンプリングして平均する。
 *          中心から遠いピクセルほどずらし幅が大きくなるので、
 *          画面端が強く流れ、中心（プレイヤー付近）は見やすいまま残る。
 */
float4 PSMain(PSInput In) : SV_Target0
{
    // 中心へ向かうベクトル。中心からの距離に比例してブラーが強くなる
    float2 toCenter = BLUR_CENTER - In.uv;

    float3 color = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < NUM_SAMPLES; i++)
    {
        float t = strength * (float(i) / float(NUM_SAMPLES - 1));
        float2 uv = In.uv + toCenter * t;
        color += mainRenderTargetTexture.Sample(Sampler, uv).rgb;
    }
    color /= float(NUM_SAMPLES);

    return float4(color, 1.0f);
}
