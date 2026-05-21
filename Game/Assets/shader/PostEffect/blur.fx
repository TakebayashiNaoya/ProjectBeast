/*!
 * @brief 横・縦ブラー（平均ブラー・ガウシアンブラー共用）
 * @details 重みテーブルに均等値を設定すれば平均ブラー、
 *          ガウス関数で計算した値を設定すればガウシアンブラーになる
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

/*!
 * @brief ブラー用重みテーブル
 */
cbuffer BlurCb : register(b1)
{
    float4 weights[2]; // weights[0].xyzw = [0]〜[3], weights[1].xyzw = [4]〜[7]
};

struct VSInputBlur
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

/*!
 * @brief 横ブラー用頂点シェーダーの出力
 */
struct PSInputXBlur
{
    float4 pos    : SV_POSITION;
    float2 uv[8]  : TEXCOORD0;
};

/*!
 * @brief 縦ブラー用頂点シェーダーの出力
 */
struct PSInputYBlur
{
    float4 pos    : SV_POSITION;
    float2 uv[8]  : TEXCOORD0;
};

Texture2D<float4> g_texture : register(t0);
sampler Sampler : register(s0);

/*!
 * @brief 横ブラー用頂点シェーダー
 * @details テクセルサイズ分ずつ横にオフセットしたUVを8サンプル分計算する
 */
PSInputXBlur VSXBlur(VSInputBlur In)
{
    PSInputXBlur psIn;
    psIn.pos = mul(mvp, In.pos);

    // テクスチャのサイズからテクセルサイズを算出する
    float width;
    float height;
    g_texture.GetDimensions(width, height);
    float texelSizeX = 1.0f / width;

    // 基準テクセルを中心に左右4サンプルずつのUVを計算する
    for (int i = 0; i < 8; i++)
    {
        psIn.uv[i] = In.uv + float2(texelSizeX * float(i), 0.0f);
    }
    return psIn;
}

/*!
 * @brief 縦ブラー用頂点シェーダー
 * @details テクセルサイズ分ずつ縦にオフセットしたUVを8サンプル分計算する
 */
PSInputYBlur VSYBlur(VSInputBlur In)
{
    PSInputYBlur psIn;
    psIn.pos = mul(mvp, In.pos);

    // テクスチャのサイズからテクセルサイズを算出する
    float width;
    float height;
    g_texture.GetDimensions(width, height);
    float texelSizeY = 1.0f / height;

    // 基準テクセルを中心に上下4サンプルずつのUVを計算する
    for (int i = 0; i < 8; i++)
    {
        psIn.uv[i] = In.uv + float2(0.0f, texelSizeY * float(i));
    }
    return psIn;
}

/*!
 * @brief ブラー用ピクセルシェーダー（横・縦共用）
 * @details 8サンプルを重みテーブルで加重平均する
 */
float4 PSBlur(PSInputXBlur In) : SV_Target0
{
    // weightsをfloat配列として扱うためにfloat4[2]から展開する
    float w[8];
    w[0] = weights[0].x;
    w[1] = weights[0].y;
    w[2] = weights[0].z;
    w[3] = weights[0].w;
    w[4] = weights[1].x;
    w[5] = weights[1].y;
    w[6] = weights[1].z;
    w[7] = weights[1].w;

    float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 8; i++)
    {
        color += g_texture.Sample(Sampler, In.uv[i]) * w[i];
    }
    return color;
}
