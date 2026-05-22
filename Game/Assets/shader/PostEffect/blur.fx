/*!
 * @brief 横・縦ブラー（平均ブラー・ガウシアンブラー共用）
 * @details 重みテーブルに均等値を設定すれば平均ブラー、
 *          ガウス関数で計算した値を設定すればガウシアンブラーになる。
 *          中心テクセル1サンプル＋左右（または上下）対称7サンプルの
 *          合計15サンプルで加重平均する。
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

/*!
 * @brief ブラー用重みテーブル
 * @details C++側 GaussianBlur::SBlurCb::weights[2]（Vector4×2）と一致させること。
 *          weights[0].x    = 中心テクセルの重み
 *          weights[0].yzw  = オフセット1〜3の重み
 *          weights[1].xyzw = オフセット4〜7の重み
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
 * @details uv[0]     = 中心テクセルのUV
 *          uv[1..7]  = +方向オフセットのUV（-方向は PS 内で対称サンプル）
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
 * @details uv[0]を中心、uv[1..7]を+X方向へオフセットしたUVとして渡す。
 *          PS内で-X方向にも対称サンプルを行う。
 */
PSInputBlur VSXBlur(VSInputBlur In)
{
    PSInputBlur psIn;
    psIn.pos = mul(mvp, In.pos);

    // テクスチャのサイズからテクセルサイズを算出する
    uint width;
    uint height;
    g_texture.GetDimensions(width, height);
    float texelSizeX = 1.0f / (float)width;

    // uv[0]は中心テクセル
    psIn.uv[0] = In.uv;

    // uv[1..7]は+X方向へオフセット（PS内で-X方向にも対称サンプルする）
    for (int i = 1; i < 8; i++)
    {
        psIn.uv[i] = In.uv + float2(texelSizeX * float(i), 0.0f);
    }

    return psIn;
}

/*!
 * @brief 縦ブラー用頂点シェーダー
 * @details uv[0]を中心、uv[1..7]を+Y方向へオフセットしたUVとして渡す。
 *          PS内で-Y方向にも対称サンプルを行う。
 */
PSInputBlur VSYBlur(VSInputBlur In)
{
    PSInputBlur psIn;
    psIn.pos = mul(mvp, In.pos);

    // テクスチャのサイズからテクセルサイズを算出する
    uint width;
    uint height;
    g_texture.GetDimensions(width, height);
    float texelSizeY = 1.0f / (float)height;

    // uv[0]は中心テクセル
    psIn.uv[0] = In.uv;

    // uv[1..7]は+Y方向へオフセット（PS内で-Y方向にも対称サンプルする）
    for (int i = 1; i < 8; i++)
    {
        psIn.uv[i] = In.uv + float2(0.0f, texelSizeY * float(i));
    }

    return psIn;
}

/*!
 * @brief ブラー用ピクセルシェーダー（横・縦共用）
 * @details 中心テクセルを weights[0].x で1回サンプル。
 *          オフセット1〜7を対応する重みで±方向それぞれサンプルして加算。
 *          合計15サンプルで加重平均する。
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

    // 中心テクセルをサンプルする（1回）
    float4 color = g_texture.Sample(Sampler, In.uv[0]) * w[0];

    // オフセット1〜7を±方向それぞれサンプルして加算する（左右または上下対称）
    // uv[i]は+方向、2*uv[0]-uv[i]は-方向（中心を軸に反転）
    for (int i = 1; i < 8; i++)
    {
        color += g_texture.Sample(Sampler, In.uv[i])               * w[i];
        color += g_texture.Sample(Sampler, 2.0f * In.uv[0] - In.uv[i]) * w[i];
    }

    return color;
}
