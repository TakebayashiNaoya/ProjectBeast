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
 * @details C++側 GaussianBlur::SBlurCb::weights[2]（Vector4×2）と一致させること。
 *          HLSLのcbuffer内のfloat配列は各要素が16バイト境界にアライメントされるため、
 *          float4[2]で受け取り、PS内でfloat[8]として展開して使用する。
 *          weights[0].xyzw = weights[0]〜[3]
 *          weights[1].xyzw = weights[4]〜[7]
 */
cbuffer BlurCb : register(b1)
{
    float4 weights[2]; // 重みテーブル（float4×2 = 8サンプル分）
};

struct VSInputBlur
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

/*!
 * @brief ブラー用ピクセルシェーダーの入力（横・縦共用）
 */
struct PSInputBlur
{
    float4 pos   : SV_POSITION;
    float2 uv[8] : TEXCOORD0;
};

Texture2D<float4> g_texture : register(t0);
sampler Sampler : register(s0);

/*!
 * @brief 横ブラー用頂点シェーダー
 * @details テクセルサイズ分ずつ横にオフセットしたUVを8サンプル分計算する
 */
PSInputBlur VSXBlur(VSInputBlur In)
{
    PSInputBlur psIn;
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
PSInputBlur VSYBlur(VSInputBlur In)
{
    PSInputBlur psIn;
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
 * @details float4[2]の重みテーブルをfloat[8]として展開し、
 *          8サンプルを加重平均する
 */
float4 PSBlur(PSInputBlur In) : SV_Target0
{
    // float4[2]をfloat[8]として展開する
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
