/*!
 * @brief 地形 GBuffer 書き込みシェーダー
 *        スプラットマップ（RGB）で 3 種のマテリアルをブレンドして GBuffer に出力する。
 *
 * レジスタ対応:
 *   b0 : ModelCb
 *   b1 : TerrainCb        （TerrainObject::TerrainCb）
 *   b2 : PBRParamCb
 *   b3 : DitherCb
 *   b4 : ModelDitherCb
 *   t10: splatmap          [0]
 *   t11: snow BaseColor    [1]
 *   t12: grass BaseColor   [2]
 *   t13: rock BaseColor    [3]
 *   t14: (未使用)          [4]
 *   t15: snow Normal       [5]
 *   t16: snow Roughness    [6]
 *   t17: glass Normal      [7]
 *   t18: glass Roughness   [8]
 *   t19: rock Normal       [9] 
 *   t20: rock Roughness    [10]
 */


// -----------------------------------------------------------------------
// 定数バッファ
// -----------------------------------------------------------------------

/**
 * 全てのオブジェクトで共通の定数バッファ。
 */
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;   // モデル   → ワールド変換行列
    float4x4 mView;    // ワールド → ビュー変換行列
    float4x4 mProj;    // ビュー   → クリップ変換行列
    float4   mulColor; // アルベドに乗算する色（白なら変化なし）
};

/**
 * 地形オブジェクト専用の定数バッファ。
 */
cbuffer TerrainCb : register(b1)
{
    float terrainHalfWidth;    // 地形のX方向半幅（UV計算に使用）
    float terrainHalfDepth;    // 地形のZ方向半幅（UV計算に使用）
    float terrainAlbedoScale;  // アルベドに乗算するスケール
    float padding;             // パディング
};

/**
 * PBR パラメータの定数バッファ。
 */
cbuffer PBRParamCb : register(b2)
{
    float dirLightScale;   // ディレクショナルライトの影響度を調整するスケール
    float ambientScale;    // アンビエントの影響度を調整するスケール
    float metallicOffset;  // メタリック値に加算するオフセット（-1.0 ～ 1.0）
    float smoothOffset;    // スムース値に加算するオフセット（-1.0 ～ 1.0）
};

/**
 * ディザリングのパラメータを格納する定数バッファ。
 */
cbuffer DitherCb : register(b3)
{
    float3 cameraWorldPos;  // カメラのワールド座標（ディザリングの距離減衰に使用）
    float  cylinderRadius;  // ディザリングの距離減衰で使用する円柱の半径
    float3 targetWorldPos;  // ディザリングの距離減衰で使用するターゲットのワールド座標
    float  depthBias;       // ディザリングの距離減衰で使用する深度バイアス
    float  ditherStrength;  // ディザリングの強さ（0.0 ～ 1.0）
    float3 ditherPad;       // パディング
};

/**
 * モデル単位のディザリングのパラメータを格納する定数バッファ。
 */
cbuffer ModelDitherCb : register(b4)
{
    float  modelDitherAlpha;  // モデル全体のディザリングのしきい値（0.0 ～ 1.0）
    float3 modelDitherPad;    // パディング
};


// -----------------------------------------------------------------------
// テクスチャ・サンプラー
// -----------------------------------------------------------------------

Texture2D<float4> g_splatmap       : register(t10);  // スプラットマップ (R=雪, G=草, B=岩)
Texture2D<float4> g_snowBase       : register(t11);  // 雪のベースカラー
Texture2D<float4> g_grassBase      : register(t12);  // 草のベースカラー
Texture2D<float4> g_rockBase       : register(t13);  // 岩のベースカラー
// t14: αは未使用
Texture2D<float4> g_snowNormal     : register(t15);  // 雪のノーマルマップ
Texture2D<float4> g_snowRoughness  : register(t16);  // 雪のラフネスマップ
Texture2D<float4> g_grassNormal    : register(t17);  // 草のノーマルマップ
Texture2D<float4> g_grassRoughness : register(t18);  // 草のラフネスマップ
Texture2D<float4> g_rockNormal     : register(t19);  // 岩のノーマルマップ
Texture2D<float4> g_rockRoughness  : register(t20);  // 岩のラフネスマップ

sampler g_sampler : register(s0);  // 全てのテクスチャで共通のサンプラー


// -----------------------------------------------------------------------
// 構造体
// -----------------------------------------------------------------------

/**
 * 頂点シェーダーの入力構造体。
 */
struct SSkinVSIn
{
    int4   Indices : BLENDINDICES0;  // ボーンインデックス
    float4 Weights : BLENDWEIGHT0;   // ボーンウェイト
};

/**
 * 頂点シェーダーの入力構造体（スキニングなし）。
 */
struct SVSIn
{
    float4    pos        : POSITION;       // モデル空間の頂点位置
    float3    normal     : NORMAL;         // モデル空間の法線
    float3    tangent    : TANGENT;        // モデル空間の接線
    float3    biNormal   : BINORMAL;       // モデル空間の従法線
    float2    uv         : TEXCOORD0;      // UV座標
    SSkinVSIn skinVert;                    // スキニング用の頂点データ（スキニングなしの場合は未使用）
    uint      instanceID : SV_InstanceID;  // インスタンスID（インスタンシングで使用、スキニングなしの場合は未使用）
};

/**
 * 頂点シェーダーの入力構造体（スキニングあり）。
 */
struct SPSIn
{
    float4 pos      : SV_POSITION;  // クリップ空間の頂点位置（シェーダー内で計算して書き込む）
    float3 normal   : NORMAL;       // ワールド空間の法線（シェーダー内で計算して書き込む）
    float3 tangent  : TEXCOORD2;    // ワールド空間の接線（シェーダー内で計算して書き込む）
    float3 biNormal : TEXCOORD3;    // ワールド空間の従法線（シェーダー内で計算して書き込む）
    float2 uv       : TEXCOORD0;    // UV座標
    float3 worldPos : TEXCOORD1;    // ワールド空間の頂点位置（シェーダー内で計算して書き込む）
};

/**
 * ピクセルシェーダーの出力構造体。
 */
struct SPSOut
{
    float4 albedo           : SV_Target0;  // アルベド (RGB) と NDC 深度 (A)
    float4 normal           : SV_Target1;  // 法線 (RGB) とメタリック値 (A)
    float4 metaricSmoothMap : SV_Target2;  // メタリック値 (R)、スムース値 (A)、その他のマップを格納するための余裕のある構成
};


// -----------------------------------------------------------------------
// Bayer 4x4 ディザリング
// -----------------------------------------------------------------------

static const int g_bayerPattern[4][4] =
{
    {  0, 32,  8, 40 },
    { 48, 16, 56, 24 },
    { 12, 44,  4, 36 },
    { 60, 28, 52, 20 },
};


// -----------------------------------------------------------------------
// PBR ヘルパー関数
// -----------------------------------------------------------------------

/**
 * ノーマルマップをタンジェント空間からワールド空間に変換する（RenderToGBuffer.fx と同じ処理）
 */
float3 SampleNormal(Texture2D normalTex, float2 uv, float3x3 tbn)
{
    float3 normalMap = normalTex.Sample(g_sampler, uv).xyz;
    //normalMap = pow(normalMap, 1.0f / 2.2f);   // sRGB → linear（RenderToGBuffer.fx と同じ）
    normalMap = (normalMap - 0.5f) * 2.0f;
    return normalize(mul(normalMap, tbn));
}

/**
 * ラフネスマップをサンプルして Smooth 値（1 - Roughness）を返す
 */
float SampleSmooth(Texture2D roughnessTex, float2 uv)
{
    return 1.0f - roughnessTex.Sample(g_sampler, uv).r;
}


// -----------------------------------------------------------------------
// 頂点シェーダー
// -----------------------------------------------------------------------
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;

    float4 worldPos  = mul(mWorld, vsIn.pos);
    psIn.worldPos    = worldPos.xyz;

    float4 viewPos   = mul(mView, worldPos);
    psIn.pos         = mul(mProj, viewPos);

    float3x3 m3      = (float3x3)mWorld;
    psIn.normal      = normalize(mul(m3, vsIn.normal));
    psIn.tangent     = normalize(mul(m3, vsIn.tangent));
    psIn.biNormal    = normalize(mul(m3, vsIn.biNormal));

    psIn.uv          = vsIn.uv;

    return psIn;
}

SPSIn VSMainSkin(SVSIn vsIn)
{
    return VSMain(vsIn);
}

// -----------------------------------------------------------------------
// ピクセルシェーダー
// -----------------------------------------------------------------------
SPSOut PSMain(SPSIn psIn)
{
    // ------ ディザリング ------
    int2  pixelPos  = (int2)psIn.pos.xy;
    int   bayerIdx  = g_bayerPattern[pixelPos.y % 4][pixelPos.x % 4];
    float threshold = (bayerIdx + 0.5f) / 64.0f;

    if (modelDitherAlpha > threshold)
        discard;

    // ------ スプラットマップ UV ------
    // 頂点は 3ds Max Z-up で生成し、エンジンが MakeRotationX(-PI/2) で Y-up に変換する。
    // 変換後: worldPos.z = -(3ds Max Y) なので V は符号反転して計算する。
    float2 splatUV;
    splatUV.x = (psIn.worldPos.x + terrainHalfWidth) / (terrainHalfWidth * 2.0f);
    splatUV.y = (terrainHalfDepth - psIn.worldPos.z) / (terrainHalfDepth * 2.0f);
    splatUV   = saturate(splatUV);

    // ------ ブレンド重み（RGB のみ使用、正規化して合計 = 1.0）------
    float3 weights  = g_splatmap.Sample(g_sampler, splatUV).rgb;
    float  weightSum = weights.r + weights.g + weights.b;
    
    if (weightSum > 0.001f) {
        weights = weights / weightSum; // 全体の合計が1になるように調整する
    }
    else {
        weights = float3(1.0f, 0.0f, 0.0f); // 安全な初期値をセットする
    }

    // ------ TBN 行列（タンジェント空間 → ワールド空間）------
    float3x3 tbn = float3x3(psIn.tangent, psIn.biNormal, psIn.normal);

    // ------ BaseColor ブレンド ------
    float4 albedo = g_snowBase.Sample (g_sampler, psIn.uv) * weights.r
                  + g_grassBase.Sample(g_sampler, psIn.uv) * weights.g
                  + g_rockBase.Sample (g_sampler, psIn.uv) * weights.b;
    albedo.a = 1.0f;

    // ------ Normal ブレンド ------
    float3 snowNorm  = SampleNormal(g_snowNormal,  psIn.uv, tbn);
    float3 grassNorm = SampleNormal(g_grassNormal, psIn.uv, tbn);
    float3 rockNorm  = SampleNormal(g_rockNormal,  psIn.uv, tbn);

    float3 finalNormal = normalize(
        snowNorm  * weights.r +
        grassNorm * weights.g +
        rockNorm  * weights.b
    );

    // ------ Roughness → Smooth ブレンド ------
    float finalSmooth = SampleSmooth(g_snowRoughness,  psIn.uv) * weights.r
                      + SampleSmooth(g_grassRoughness, psIn.uv) * weights.g
                      + SampleSmooth(g_rockRoughness,  psIn.uv) * weights.b;

    // ------ GBuffer 出力 ------
    SPSOut psOut;

    // RenderToGBuffer.fx と同じ: w に NDC 深度を書く（デファードがワールド座標再構築に使う）
    psOut.albedo   = float4(albedo.rgb * mulColor.rgb * terrainAlbedoScale, 1.0f);
    psOut.albedo.w = psIn.pos.z / psIn.pos.w;
    psOut.normal = float4(finalNormal * 0.5f + 0.5f, 1.0f);
    psOut.metaricSmoothMap = float4(
        saturate(0.0f + metallicOffset),
        dirLightScale,
        ambientScale,
        saturate(finalSmooth + smoothOffset)
    );

    return psOut;
}
