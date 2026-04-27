/*!
 * @brief ディファードライティングシェーダー
 */


///////////////////////////////////////
// 定数
///////////////////////////////////////

static const int MAX_POINT_LIGHT = 32;
static const int MAX_SPOT_LIGHT  = 32;

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
    float4x4 mViewProjInv;       // offset: 128
};


///////////////////////////////////////
// テクスチャ
///////////////////////////////////////

Texture2D<float4> g_albedoTexture   : register(t0);	// アルベド
Texture2D<float4> g_normalTexture   : register(t1);	// 法線
Texture2D<float4> g_specularTexture : register(t2);	// スペキュラ

sampler g_sampler : register(s0);


///////////////////////////////////////
// 関数
///////////////////////////////////////

/*!
 * @brief UV座標とスクリーン空間のZ値からワールド座標を計算する
 * @param uv          UV座標
 * @param zInScreen   スクリーン空間のZ値
 * @param mViewProjInv ビュープロジェクション行列の逆行列
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
 * @brief ランバート拡散反射を計算する
 * @param lightDirection ライトの方向
 * @param lightColor     ライトの色
 * @param normal         法線
 */
float3 CalcLambertDiffuse(float3 lightDirection, float3 lightColor, float3 normal)
{
	float t = dot(normal, lightDirection);
	t *= -1.0f;
	t = saturate(t);
	return lightColor * t;
}

/*!
 * @brief フォン鏡面反射を計算する
 * @param lightDirection ライトの方向
 * @param lightColor     ライトの色
 * @param worldPos       ワールド座標
 * @param normal         法線
 */
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal)
{
	float3 lightIncidentDir = -lightDirection;
	float3 reflectDir = reflect(lightIncidentDir, normal);

	float3 toViewDir = normalize(cameraPosition - worldPos);

	float t = dot(reflectDir, toViewDir);
	t = max(0.0f, t);
	t = pow(t, 10.0f);

	return lightColor * t;
}

/*!
 * @brief ディレクションライトを計算する
 * @param normal   法線
 * @param worldPos ワールド座標
 */
float3 CalcDirectionLight(float3 normal, float3 worldPos)
{
    float3 diffuse = CalcLambertDiffuse(dirLightDirection, dirLightColor, normal);
    // スペキュラを一時的に除く
    return diffuse;
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
    float3 normal = g_normalTexture.Sample(g_sampler, In.uv).xyz;
    normal = (normal * 2.0f) - 1.0f;

    // ワールド座標を復元
    float3 worldPos = CalcWorldPosFromUVZ(In.uv, albedo.w, mViewProjInv);

    // ディレクションライトを計算
    float3 lig = CalcDirectionLight(normal, worldPos);

    // 環境光を加算
    lig += ambientLightColor;

    // 最終カラー
    float4 finalColor = albedo;
    finalColor.xyz *= lig;
    return finalColor;
}