/*!
 * @brief シンプルなモデルシェーダー。
 */

////////////////////////////////////////////////
// 構造体
////////////////////////////////////////////////
// スキニング用の頂点データをひとまとめ。
struct SSkinVSIn
{
    int4 Indices : BLENDINDICES0;
    float4 Weights : BLENDWEIGHT0;
};
// 頂点シェーダーへの入力。
struct SVSIn
{
    float4 pos : POSITION;
    float2 uv : TEXCOORD0;
    SSkinVSIn skinVert;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
};
// ピクセルシェーダーへの入力。
struct SPSIn
{
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float4 posRefCamViewProj : TEXCOORD3;
    //float4 refClip : TEXCOORDn;
};

// ディレクションライト構造体。
struct DirectionLight
{
    float3   direction;
    float    padding0;
    float3   color;
    float    padding1;
    float4x4 mLVP;
};

// ポイントライト構造体。
struct PointLight
{
    float3 position;
    int    isUsed;
    float3 color;
    float  range;
    float3 positionInView;
    float  padding;
};

// スポットライト構造体。
struct SpotLight
{
    float3 position;
    int    isUsed;
    float3 color;
    float  range;
    float3 direction;
    float  angle;
    float3 positionInView;
    float  padding;
};

// 半球ライト構造体。
struct HemisphereLight
{
    float3 groundColor;
    float  padding0;
    float3 skyColor;
    float  padding1;
    float3 groundNormal;
    float  padding2;
};

// ライトの構造体。
struct Light
{
    DirectionLight  directionLight;
    PointLight      pointLight[32];
    SpotLight       spotLight[32];
    HemisphereLight hemisphereLight;
    int    usedPointLightCount;
    float3 cameraEyePos;
    int    usedSpotLightCount;
    float3 ambientColor;
};

////////////////////////////////////////////////
// 定数バッファ。
////////////////////////////////////////////////
// モデル用の定数バッファ。
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

// 海用の定数バッファー。
cbuffer OceanCb : register(b1)
{
    Light light;
    float baseReflectance;
    float waveScroll;		// 頂点移動用スクロール値
    float textureScroll;	// テクスチャスクロール用スクロール値

    // 波パラメータ
    float wave1Amplitude;   // 波①の振幅
    float wave1Frequency;   // 波①の空間周波数
    float wave2Amplitude;   // 波②の振幅
    float wave2Frequency;   // 波②の空間周波数
}

////////////////////////////////////////////////
// グローバル変数。
////////////////////////////////////////////////
Texture2D<float4> g_albedo : register(t0);
//StructuredBuffer<float4x4> g_boneMatrix : register(t3);
sampler g_sampler : register(s0);
Texture2D<float4> g_normalMap : register(t1);
Texture2D<float4> g_specularMap : register(t2);
Texture2D<float4> g_refLect : register(t10);
Texture2D<float4> g_shadowMap : register(t11);

////////////////////////////////////////////////
// 関数宣言。
////////////////////////////////////////////////
float3 CalcLigFromDrectionLight(SPSIn psIn, float3 normal);
float3 CalcLambertDiffuse(float3 lightDirection, float3 lightColor, float3 normal);
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal, float2 uv);
float2 CalcReflectUV(float4 clip);
float ComputeFresnel(float3 normal, float3 viewDir, float baseReflectance);
float2 DistortUVByNormal(float2 uv, float3 normal, float distortionStrength);
float3 ComputeNomal(SPSIn psIn, float2 uv, float scroll);
float CalcShadowPow(float3 worldPos);

////////////////////////////////////////////////
// 波のオフセット計算。
// waveScroll を時間軸とし、2本のsin波を重ねてY方向オフセットを返す。
// 波①：正面方向(1,0)、波②：斜め方向(0.6, 0.8)。
// 各波のspeed倍率は波①=1.0、波②=1.7（ハードコード）。
////////////////////////////////////////////////
float CalcWaveOffset(float3 worldPos)
{
    // 波①
    float2 dir1 = float2(1.0, 0.0);
    float phase1 = dot(dir1, worldPos.xz) * wave1Frequency + waveScroll * 1.0;
    float offset1 = wave1Amplitude * sin(phase1);

    // 波②
    float2 dir2 = float2(0.6, 0.8);
    float phase2 = dot(dir2, worldPos.xz) * wave2Frequency + waveScroll * 1.7;
    float offset2 = wave2Amplitude * sin(phase2);

    return offset1 + offset2;
}

/// <summary>
//スキン行列を計算する。
/// </summary>
// float4x4 CalcSkinMatrix(SSkinVSIn skinVert)
// {
//     float4x4 skinning = 0;
//     float w = 0.0f;
// 	[unroll]
//     for (int i = 0; i < 3; i++)
//     {
//         skinning += g_boneMatrix[skinVert.Indices[i]] * skinVert.Weights[i];
//         w += skinVert.Weights[i];
//     }

//     skinning += g_boneMatrix[skinVert.Indices[3]] * (1.0f - w);

//     return skinning;
// }

/// <summary>
/// 頂点シェーダーのコア関数。
/// </summary>
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;

    float4 worldPos = mul(mWorld, vsIn.pos);

    // WPO：Y方向にsin波オフセットを加算する
    float wpoCenter = CalcWaveOffset(worldPos.xyz);
    worldPos.y += wpoCenter;

    // 差分近似で法線を再計算する
    // 隣接点のXZ座標にepsilonだけずらした位置のWPOを求め、
    // 接線・従線ベクトルを構築して外積から法線を得る
    const float epsilon = 0.1;

    float3 neighborX = worldPos.xyz + float3(epsilon, 0.0, 0.0);
    neighborX.y = (worldPos.y - wpoCenter) + CalcWaveOffset(neighborX);

    float3 neighborZ = worldPos.xyz + float3(0.0, 0.0, epsilon);
    neighborZ.y = (worldPos.y - wpoCenter) + CalcWaveOffset(neighborZ);

    // X方向・Z方向の接線ベクトルを求め、外積で法線を得る
    float3 tangentX  = normalize(neighborX - worldPos.xyz);
    float3 tangentZ  = normalize(neighborZ - worldPos.xyz);
    // cross(tangentX, tangentZ) でY成分が正の上向き法線になる
    float3 waveNormal = normalize(cross(tangentX, tangentZ));

    psIn.worldPos = worldPos.xyz;
    psIn.pos = mul(mView, worldPos);
    psIn.pos = mul(mProj, psIn.pos);

    psIn.uv = vsIn.uv;

    // 法線はWPOから再計算した値を使う
    // tangentとbiNormalはwaveNormalから再構築する
    float3x3 m3x3    = (float3x3)mWorld;
    float3 baseTangent = normalize(mul(m3x3, vsIn.tangent));
    psIn.normal   = waveNormal;
    psIn.tangent  = normalize(baseTangent - dot(baseTangent, waveNormal) * waveNormal);
    psIn.biNormal = normalize(cross(waveNormal, psIn.tangent));

    return psIn;
}

/// <summary>
/// ピクセルシェーダーのエントリー関数。
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    float2 uvScaled = psIn.uv * float2(20.0, 20.0);

    float3 ligDirection = light.directionLight.direction;

    float3 normal = ComputeNomal(psIn, uvScaled, textureScroll);	// waveScroll → textureScroll
    float refMapDistortionStrength = 0.2;
    float albedoDistortionStrength = 0.3;

    float2 albedoUv = uvScaled + normal.xz * albedoDistortionStrength;
    float4 albedoColor = g_albedo.Sample(g_sampler, albedoUv);

    float3 directionLight = CalcLigFromDrectionLight(psIn, normal);
    float3 lig = directionLight;

    float flesnel = ComputeFresnel(normal, normalize(light.cameraEyePos - psIn.worldPos), baseReflectance);

    float4 litColor = albedoColor;
    litColor.xyz += lig;

    float4 finalColor;
    finalColor = litColor;

    return finalColor;
}

//////////////////////////////////////////////////////////////////////////////////
//Lambert拡散反射を計算
//////////////////////////////////////////////////////////////////////////////////
float3 CalcLambertDiffuse(float3 lightDirection, float3 lightColor, float3 normal)
{
    normal = normalize(normal);
    lightDirection = normalize(lightDirection);

    float t = dot(normal, lightDirection);
    t *= -1.0f;
    if (t < 0.0f)
    {
        t = 0.0f;
    }

    return lightColor * t;
}

//////////////////////////////////////////////////////////////////////////////////
//phong鏡面反射を計算
//////////////////////////////////////////////////////////////////////////////////
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal, float2 uv)
{
    float3 refVec = reflect(lightDirection, normal);
    float3 toEye = light.cameraEyePos - worldPos;
    toEye = normalize(toEye);

    float t = dot(refVec, toEye);
    t = max(0.0f, t);
    t = pow(t, 10.0f);

    float specPower = g_specularMap.Sample(g_sampler, uv).r;

    float3 specularLig = lightColor * t;

    return specularLig;
}

//////////////////////////////////////////////////////////////////////////////////
//ディレクションライトを計算
//////////////////////////////////////////////////////////////////////////////////
float3 CalcLigFromDrectionLight(SPSIn psIn, float3 normal)
{
    float3 diffDirection = CalcLambertDiffuse(
		light.directionLight.direction, light.directionLight.color, normal);
    float3 specDirection = CalcPhongSpecular(
		light.directionLight.direction, light.directionLight.color, psIn.worldPos, normal, psIn.uv);

    return diffDirection + specDirection;
}

//////////////////////////////////////////////////////////////////////////////////
//反射カメラのuvを計算
//////////////////////////////////////////////////////////////////////////////////
float2 CalcReflectUV(float4 clip)
{
    float w = max(abs(clip.w), 1e-6);
    float invW = rcp(w);
    float2 uv = clip.xy * invW * 0.5f + 0.5f;
    uv.y = 1.0f - uv.y;
    return uv;
}

float ComputeFresnel(float3 normal, float3 viewDir, float baseReflectance)
{
    float cosTheta = saturate(dot(normalize(normal), -normalize(viewDir)));
    float angleFactor = pow(1.0f - cosTheta, 5.0f);
    float remainingReflectance = 1 - baseReflectance;

    return baseReflectance + remainingReflectance * angleFactor;
}

float2 DistortUVByNormal(float2 uv, float3 normal, float distortionStrength)
{
    float2 distortion = normal.xz * distortionStrength;
    return uv + distortion;
}

float3 ComputeNomal(SPSIn psIn, float2 uv, float scroll)
{
    float3 localNormal = g_normalMap.Sample(g_sampler, uv + scroll).xyz;
    localNormal = normalize(localNormal);
    localNormal = (localNormal - 0.5f) * 2.0f;

    float3 normal = psIn.normal;
    normal = psIn.tangent * localNormal.x + psIn.biNormal * localNormal.y + normal * localNormal.z;
    normal = normalize(normal);
    return normal;
}

float CalcShadowPow(float3 worldPos)
{
    float shadowPow = 1.0f;
    return shadowPow;
}