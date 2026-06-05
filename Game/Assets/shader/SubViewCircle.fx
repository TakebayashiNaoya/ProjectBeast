/**
 * @file SubViewCircle.fx
 * @brief サブビュー（PIP）を円形マスクで描画するスプライトシェーダー
 *
 * レンダーターゲットのアスペクト比（16:9）を考慮したUV距離計算で
 * 真円を生成する。smoothstepで縁をフェザリングしてジャギーを抑える。
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;
    float4   mulColor;
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

Texture2D<float4> colorTexture : register(t0);
sampler Sampler : register(s0);


PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv  = In.uv;
    return psIn;
}


float4 PSMain(PSInput In) : SV_Target0
{
    // レンダーターゲットのアスペクト比（480 / 270 = 16 / 9）
    // X方向をこの比で引き伸ばすことで、UV空間での距離計算が
    // ピクセル空間での真円に対応する
    static const float AR = 480.0f / 270.0f;

    // UVを中心 (0.5, 0.5) 基準の座標に変換
    float2 c = In.uv - 0.5f;
    c.x *= AR;

    // 高さ方向を基準にした正規化距離
    // dist = 0.5 が円の縁（半径 = 高さの半分）
    float dist = length(c);

    // smoothstep でソフトエッジ（縁の前後 2px 分をフェード）
    static const float RADIUS      = 0.5f;
    static const float FEATHER_PX  = 2.0f;
    static const float FEATHER     = FEATHER_PX / 270.0f;

    float alpha = 1.0f - smoothstep(RADIUS - FEATHER, RADIUS + FEATHER, dist);

    // 完全に透明なピクセルは破棄して2D合成コストを下げる
    if (alpha < 0.001f)
    {
        discard;
    }

    float4 color = colorTexture.Sample(Sampler, In.uv) * mulColor;
    // テクスチャのアルファ（RT由来・不定値）ではなく円マスクのアルファで上書きする。
    // ディファードRTのアルファは一部オブジェクトで0になるため乗算すると透明になる。
    color.a = alpha;
    return color;
}
