/*!
 * @brief ディファードライティングシェーダー
 */


///////////////////////////////////////
// 定数
///////////////////////////////////////

static const int   MAX_POINT_LIGHT = 32;
static const int   MAX_SPOT_LIGHT  = 32;
static const float PI              = 3.1415926f; // π

// リムライトの計算に使用する定数
static const float RIM_LIGHT_EXPONENT = 10.0f;   // リムライトの輪郭の鋭さ（指数）
static const float RIM_LIGHT_STRENGTH = 0.5f;   // リムライトの強度倍率

///////////////////////////////////////
// 構造体
///////////////////////////////////////

// 頂点シェーダーへの入力
struct VSInput
{
	float4 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

// ピクセルシェーダーへの入力
struct PSInput
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

// ディレクションライト
struct SDirectionLight
{
    float3   direction;
    float    padding0;
    float3   color;
    float    padding1;
    // mLVP is reserved for shadow (currently unused)
    float4x4 mLVP;
};

// ポイントライト
struct SPointLight
{
	float3 position;		// 位置
	int    isUsed;			// 使用中かどうか
	float3 color;			// 色
	float  range;			// 影響範囲
	float3 positionInView;	// カメラ空間での座標
	float  padding;
};

// スポットライト
struct SSpotLight
{
	float3 position;		// 位置
	int    isUsed;			// 使用状況
	float3 color;			// 色
	float  range;			// 影響範囲
	float3 direction;		// 向き
	float  angle;			// 射出角度
	float3 positionInView;	// カメラ空間での座標
	float  padding;
};

// 半球ライト
struct SHemisphereLight
{
	float3 groundColor;		// 地面の色
	float  padding0;
	float3 skyColor;		// 空の色
	float  padding1;
	float3 groundNormal;	// 地面の法線
	float  padding2;
};


///////////////////////////////////////
// 定数バッファ（Lightと同じレイアウト）
///////////////////////////////////////

cbuffer cb : register(b0)
{
	float4x4 mvp;
	float4   mulColor;
};

cbuffer LightCB : register(b1)
{
    float3   dirLightDirection;  // offset:   0
    float    pad0;               // offset:  12
    float3   dirLightColor;      // offset:  16
    float    pad1;               // offset:  28
    float4x4 dirLightLVP;        // offset:  32
    float3   cameraPosition;     // offset:  96
    float    pad2;               // offset: 108
    float3   ambientLightColor;  // offset: 112
    float    pad3;               // offset: 124
    float3   rimLightColor;      // offset: 128
    float    pad4;               // offset: 140
    float4x4 mViewProjInv;       // offset: 144
};


///////////////////////////////////////
// テクスチャ
///////////////////////////////////////

Texture2D<float4> g_albedoTexture        : register(t0);	// アルベド
Texture2D<float4> g_normalTexture        : register(t1);	// 法線
// metaricSmoothTexture のレイアウト:
//   r = metallic
//   g = dirLightScale（ディレクションライト強度倍率）
//   b = ambientScale（環境光強度倍率）
//   a = smooth
Texture2D<float4> g_metaricSmoothTexture : register(t2);	// メタリックスムース
Texture2D<float4> g_shadowMap            : register(t3);	// シャドウマップ（ライト空間の深度）

// レジスタ番号は C++ 側の EnDeferredLightingSrv（RenderViewContext.h）と一致させること。
// RenderingEngine::InitDeferredLightingSprite() がその順番でテクスチャを設定している。

sampler g_sampler : register(s0);


///////////////////////////////////////
// シャドウ
///////////////////////////////////////

/** シャドウマップの解像度。ShadowMap.cpp の SHADOW_MAP_SIZE と一致させること */
static const float SHADOW_MAP_SIZE = 2048.0f;

/**
 * @brief シャドウアクネを防ぐための深度バイアス
 * @details 自分自身の深度と比較して誤って影と判定される（縞模様が出る）のを防ぐ。
 *          大きくしすぎると影が実際の位置からずれる（ピーターパン現象）。
 */
static const float SHADOW_DEPTH_BIAS = 0.0015f;

/*!
 * @brief 影になっている割合を計算する
 * @details ワールド座標をライト空間へ変換し、シャドウマップの深度と比較する。
 *          3x3のPCFで周囲も参照して輪郭を柔らかくしている。
 * @param worldPos ワールド座標
 * @return 光が当たっている割合（0.0=完全な影 1.0=影なし）
 */
float CalcShadowRate(float3 worldPos)
{
    // ライトから見たクリップ空間へ変換する
    float4 posInLVP = mul(dirLightLVP, float4(worldPos, 1.0f));
    float3 lvpPos = posInLVP.xyz / posInLVP.w;

    // クリップ空間(-1〜1)からUV(0〜1)へ変換する。yは上下が逆になる
    float2 shadowUV = float2(0.5f, -0.5f) * lvpPos.xy + 0.5f;

    // シャドウマップの範囲外は影を落とさない
    // 範囲は注視点まわりに限定しているため、遠景は必ずここに来る
    if (shadowUV.x < 0.0f || shadowUV.x > 1.0f ||
        shadowUV.y < 0.0f || shadowUV.y > 1.0f ||
        lvpPos.z < 0.0f || lvpPos.z > 1.0f)
    {
        return 1.0f;
    }

    // 自分の深度がシャドウマップの深度より奥なら影
    float depthFromLight = lvpPos.z - SHADOW_DEPTH_BIAS;
    float texelOffset = 1.0f / SHADOW_MAP_SIZE;

    // 3x3のPCFで平均を取り、輪郭のジャギを緩和する
    float lightRate = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            float2 uv = shadowUV + float2(x, y) * texelOffset;
            float shadowMapDepth = g_shadowMap.Sample(g_sampler, uv).r;
            lightRate += (depthFromLight <= shadowMapDepth) ? 1.0f : 0.0f;
        }
    }

    return lightRate / 9.0f;
}


///////////////////////////////////////
// 関数
///////////////////////////////////////

/*!
 * @brief UV座標とスクリーン空間のZ値からワールド座標を計算する
 * @param uv           UV座標
 * @param zInScreen    スクリーン空間のZ値
 * @param viewProjInv  ビュープロジェクション行列の逆行列
 */
float3 CalcWorldPosFromUVZ(float2 uv, float zInScreen, float4x4 viewProjInv)
{
	float3 screenPos;
	// 0~1のUV座標を-1~1のスクリーン座標に変換
	screenPos.xy = (uv * float2(2.0f, -2.0f)) + float2(-1.0f, 1.0f);
	// スクリーン空間のZ値をセット
	screenPos.z = zInScreen;

	// 逆行列を使ってワールド座標に変換
	float4 worldPos = mul(viewProjInv, float4(screenPos, 1.0f));
	worldPos.xyz /= worldPos.w;
	return worldPos.xyz;
}

/*!
 * @brief ベックマン分布を計算する
 * @param m  マイクロファセットの粗さ
 * @param t  法線とハーフベクトルの内積
 */
float Beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
    return D;
}

/*!
 * @brief フレネル反射率を計算する（Schlick近似）
 * @param f0  垂直入射時のフレネル反射率
 * @param u   視線とハーフベクトルの内積
 */
float SpcFresnel(float f0, float u)
{
    return f0 + (1.0f - f0) * pow(1.0f - u, 5.0f);
}

/*!
 * @brief Cook-Torranceモデルの鏡面反射を計算する
 * @param L        光源に向かうベクトル
 * @param V        視点に向かうベクトル
 * @param N        法線ベクトル
 * @param metallic 金属度
 */
float CookTorranceSpecular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;

    // 金属度を垂直入射の時のフレネル反射率として扱う
    float f0 = metallic;

    // ハーフベクトルを求める
    float3 H = normalize(L + V);

    // 各種ベクトルの内積を計算する
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));

    // D項をベックマン分布を用いて計算する
    float D = Beckmann(microfacet, NdotH);

    // F項をSchlick近似を用いて計算する
    float F = SpcFresnel(f0, VdotH);

    // G項を求める
    float G = min(1.0f, min(2.0f * NdotH * NdotV / VdotH, 2.0f * NdotH * NdotL / VdotH));

    // m項を求める
    float m = PI * NdotV * NdotH;

    return max(F * D * G / m, 0.0f);
}

/*!
 * @brief フレネル反射を考慮した拡散反射を計算する（ディズニーベース）
 * @param N  法線
 * @param L  光源に向かうベクトル
 * @param V  視線に向かうベクトル
 */
float CalcDiffuseFromFresnel(float3 N, float3 L, float3 V)
{
    float3 H = normalize(L + V);

    // 粗さは0.5で固定
    float roughness = 0.5f;

    float energyBias   = lerp(0.0f, 0.5f,         roughness);
    float energyFactor = lerp(1.0f, 1.0f / 1.51f, roughness);

    float dotLH = saturate(dot(L, H));
    float Fd90  = energyBias + 2.0f * dotLH * dotLH * roughness;

    float dotNL = saturate(dot(N, L));
    float FL    = (1.0f + (Fd90 - 1.0f) * pow(1.0f - dotNL, 5.0f));

    float dotNV = saturate(dot(N, V));
    float FV    = (1.0f + (Fd90 - 1.0f) * pow(1.0f - dotNV, 5.0f));

    return (FL * FV * energyFactor);
}

/*!
 * @brief 視線方向ベースのリムライトを計算する
 * @details 視線ベクトルと法線の内積が小さい（輪郭付近）ほど強くなる。
 *          視点が変わるとリムの位置も変わる、カメラ基準のリムライト。
 * @param normal  法線ベクトル（ワールド空間）
 * @param toEye   サーフェイスから視点に向かうベクトル（正規化済み）
 * @return リムライトの反射光
 */
float3 CalcRimLight(float3 normal, float3 toEye)
{
    // 視線と法線の内積が小さいほど（輪郭付近ほど）リムライトが強くなる
    // saturate でクランプして負値による pow の NaN 発生を防ぐ
    float rimStrength = saturate(1.0f - saturate(dot(normal, toEye)));

    // 指数関数的に変化させて輪郭を鋭くし、強度倍率を乗算する
    rimStrength = pow(rimStrength, RIM_LIGHT_EXPONENT) * RIM_LIGHT_STRENGTH;

    return rimLightColor * rimStrength;
}

/*!
 * @brief PBRベースのディレクションライトを計算する
 * @param normal         法線
 * @param worldPos       ワールド座標
 * @param albedo         アルベドカラー
 * @param metallic       金属度
 * @param smooth         滑らかさ
 * @param dirLightScale  ディレクションライト強度倍率
 * @param ambientScale   環境光強度倍率
 */
float3 CalcDirectionLight(
    float3 normal,
    float3 worldPos,
    float3 albedo,
    float  metallic,
    float  smooth,
    float  dirLightScale,
    float  ambientScale)
{
    float3 toLight = -dirLightDirection;
    float3 toEye   = normalize(cameraPosition - worldPos);

    // 補正済みライトカラーを計算する
    float3 scaledDirLightColor = dirLightColor * dirLightScale;

    // フレネル反射を考慮した拡散反射を計算する
    float diffuseFromFresnel = CalcDiffuseFromFresnel(normal, toLight, toEye);

    // 正規化Lambert拡散反射を求める
    float  NdotL          = saturate(dot(normal, toLight));
    float3 lambertDiffuse = scaledDirLightColor * NdotL / PI;

    // 最終的な拡散反射光を計算する
    float3 diffuse = albedo * diffuseFromFresnel * lambertDiffuse;

    // Cook-Torranceモデルを利用した鏡面反射率を計算する
    float3 spec = CookTorranceSpecular(toLight, toEye, normal, metallic)
                * scaledDirLightColor;

    // 金属度が高ければ鏡面反射はアルベドカラー、低ければ白
    spec *= lerp(float3(1.0f, 1.0f, 1.0f), albedo, metallic);

    // 滑らかさを使って拡散反射光と鏡面反射光を合成する
    // 滑らかさが高ければ拡散反射は弱くなる
    float3 lig = diffuse * (1.0f - smooth) + spec;

    // 影を適用する
    // 環境光より前に掛けることで、影の中でも環境光は残り真っ黒にならない
    lig *= CalcShadowRate(worldPos);

    // 補正済み環境光を加算する
    lig += ambientLightColor * albedo * ambientScale;

    // リムライトを加算する
    lig += CalcRimLight(normal, toEye);

    return lig;
}


///////////////////////////////////////
// 頂点シェーダー
///////////////////////////////////////

PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = mul(mvp, In.pos);
	psIn.uv  = In.uv;
	return psIn;
}


///////////////////////////////////////
// ピクセルシェーダー
///////////////////////////////////////

float4 PSMain(PSInput In) : SV_Target0
{
    // G-Bufferからアルベドカラーをサンプリング
    float4 albedo = g_albedoTexture.Sample(g_sampler, In.uv);

    // G-Bufferから法線をサンプリング
    float3 normalRaw = g_normalTexture.Sample(g_sampler, In.uv).xyz;
    float3 normal    = (normalRaw * 2.0f) - 1.0f;

    // GBufferに何も書き込まれていないピクセル（法線が未書き込み）はライティングをスキップする
    // 未書き込み時は normalRaw = (0,0,0) となるため、生の法線値の長さで判定する
    if (dot(normalRaw, normalRaw) < 0.1f)
    {
        return float4(albedo.rgb, 1.0f);
    }

    // G-BufferからPBRパラメータをサンプリング
    float4 metaricSmooth  = g_metaricSmoothTexture.Sample(g_sampler, In.uv);
    float  metallic       = metaricSmooth.r;
    float  dirLightScale  = metaricSmooth.g;
    float  ambientScale   = metaricSmooth.b;
    float  smooth         = metaricSmooth.a;

    // ワールド座標を復元
    float3 worldPos = CalcWorldPosFromUVZ(In.uv, albedo.w, mViewProjInv);




    // PBRベースのディレクションライトを計算する（補正値込み）
    float3 lig = CalcDirectionLight(normal, worldPos, albedo.rgb, metallic, smooth, dirLightScale, ambientScale);

    // 最終カラー
    float4 finalColor = albedo;
    finalColor.xyz    = lig;
    return finalColor;
}
