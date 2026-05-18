/*!
 * @brief 海のシェーダー
 */

////////////////////////////////////////////////
// 構造体
////////////////////////////////////////////////
// 頂点シェーダーへの入力。
struct SVSIn
{
    float4 pos      : POSITION;
    float2 uv       : TEXCOORD0;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 biNormal : BINORMAL;
};

// ピクセルシェーダーへの入力。
struct SPSIn
{
    float4 pos      : SV_POSITION;
    float2 uv       : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 normal   : NORMAL;
    float3 tangent  : TANGENT;
    float3 biNormal : BINORMAL;
};

// ディレクションライト構造体。
// C++側 SDirectionLight（SceneLight.h）と一致させること。
struct DirectionLight
{
    float3   direction;
    float    padding0;
    float3   color;
    float    padding1;
    float4x4 mLVP;
};

// ライトの構造体。
// C++側 Light（SceneLight.h）と完全に一致させること。
struct Light
{
    DirectionLight directionLight;    // SDirectionLight と一致（96バイト）
    float3         cameraPosition;    // m_cameraPosition（12バイト）
    float          padding0;          // パディング（4バイト）
    float3         ambientLightColor; // m_ambientLightColor（12バイト）
    float          padding1;          // パディング（4バイト）
    float4x4       mViewProjInv;      // m_mViewProjInv（64バイト）
};

////////////////////////////////////////////////
// 定数バッファ。
////////////////////////////////////////////////
// 共通定数バッファ（b0）。
// C++側 OceanMesh::SCommonConstantBufferと一致させること
// (mWorld / mView / mProj / mulColor)
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
    float4   mulColor;
};

// 海用の定数バッファー（b1）。
// C++側 Ocean::OceanConstantBufferと一致させること。
cbuffer OceanCb : register(b1)
{
    Light light;
    float baseReflectance;
    float waveScroll;       // 頂点移動用スクロール値
    float textureScroll;    // テクスチャスクロール用スクロール値
    float wave1Amplitude;   // 波①の振幅
    float wave1Frequency;   // 波①の空間周波数
    float wave2Amplitude;   // 波②の振幅
    float wave2Frequency;   // 波②の空間周波数
    float specularPower;    // スペキュラのPhong指数（大きいほどハイライトが絞られる）
    float specularScale;    // スペキュラ強度の倍率（0.0で照り返しを消せる）
    float ambientScale;     // 海専用アンビエント強度倍率（他オブジェクトに影響しない）
    float padding0;
};

////////////////////////////////////////////////
// グローバル変数。
//////////////////////////////
Texture2D<float4> g_albedo      : register(t0);
Texture2D<float4> g_normalMap   : register(t1);
Texture2D<float4> g_specularMap : register(t2);
sampler           g_sampler     : register(s0);

////////////////////////////////////////////////
// 関数宣言。
////////////////////////////////////////////////
float3 CalcLigFromDrectionLight(SPSIn psIn, float3 normal);
float3 CalcLambertDiffuse(float3 lightDirection, float3 lightColor, float3 normal);
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal, float2 uv);
float  ComputeFresnel(float3 normal, float3 viewDir, float baseReflectance);
float3 ComputeNormal(SPSIn psIn, float2 uv, float scroll);

////////////////////////////////////////////////
// 波のオフセット計算。
// waveScroll を時間軸とし、2本のsin波を重ねてY方向オフセットを返す。
// 波①：正面方向(1,0)、波②：斜め方向(0.6, 0.8)。
// 各波のspeed倍率は波①=1.0、波②=1.7（ハードコード）。
////////////////////////////////////////////////
float CalcWaveOffset(float3 worldPos)
{
    // 波①
    float2 dir1    = float2(1.0, 0.0);
    float  phase1  = dot(dir1, worldPos.xz) * wave1Frequency + waveScroll * 1.0;
    float  offset1 = wave1Amplitude * sin(phase1);

    // 波②
    float2 dir2    = float2(0.6, 0.8);
    float  phase2  = dot(dir2, worldPos.xz) * wave2Frequency + waveScroll * 1.7;
    float  offset2 = wave2Amplitude * sin(phase2);

    return offset1 + offset2;
}

/// <summary>
/// 頂点シェーダーのエントリー関数。
/// </summary>
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;

    float4 worldPos = mul(mWorld, vsIn.pos);

    // WPO：Y方向にsin波オフセットを加算する
    float wpoCenter  = CalcWaveOffset(worldPos.xyz);
    worldPos.y      += wpoCenter;

    // 差分近似で法線を再計算する
    // 隣接点のXZ座標にepsilonだけずらした位置のWPOを求め、
    // 接線・従法線ベクトルを構築して外積から法線を得る
    const float epsilon = 0.1;

    float3 neighborX = worldPos.xyz + float3(epsilon, 0.0, 0.0);
    neighborX.y      = (worldPos.y - wpoCenter) + CalcWaveOffset(neighborX);

    float3 neighborZ = worldPos.xyz + float3(0.0, 0.0, epsilon);
    neighborZ.y      = (worldPos.y - wpoCenter) + CalcWaveOffset(neighborZ);

    // X方向・Z方向の接線ベクトルを求め、外積で法線を得る
    float3 tangentX   = normalize(neighborX - worldPos.xyz);
    float3 tangentZ   = normalize(neighborZ - worldPos.xyz);
    float3 waveNormal = normalize(cross(tangentX, tangentZ));

    psIn.worldPos = worldPos.xyz;
    psIn.pos      = mul(mView, worldPos);
    psIn.pos      = mul(mProj, psIn.pos);
    psIn.uv       = vsIn.uv;

    // 法線はWPOから再計算した値を使う
    // tangentとbiNormalはwaveNormalから再構築する
    float3x3 m3x3      = (float3x3) mWorld;
    float3 baseTangent  = normalize(mul(m3x3, vsIn.tangent));
    psIn.normal         = waveNormal;
    psIn.tangent        = normalize(baseTangent - dot(baseTangent, waveNormal) * waveNormal);
    psIn.biNormal       = normalize(cross(waveNormal, psIn.tangent));

    return psIn;
}

/// <summary>
/// ピクセルシェーダーのエントリー関数。
/// </summary>
float4 PSMain(SPSIn psIn) : SV_Target0
{
    float2 uvScaled               = psIn.uv * float2(20.0, 20.0);
    float  albedoDistortionStrength = 0.3;

    float3 normal      = ComputeNormal(psIn, uvScaled, textureScroll);
    float2 albedoUv    = uvScaled + normal.xz * albedoDistortionStrength;
    float4 albedoColor = g_albedo.Sample(g_sampler, albedoUv);

    float3 diff      = CalcLambertDiffuse(
        light.directionLight.direction, light.directionLight.color, normal);
    float3 spec      = CalcPhongSpecular(
        light.directionLight.direction, light.directionLight.color, psIn.worldPos, normal, psIn.uv);
    float3 ambient   = light.ambientLightColor * ambientScale;

    float4 litColor  = albedoColor;
    litColor.xyz    *= saturate(diff + ambient);
    litColor.xyz    += saturate(spec) * lerp(float3(1.0f, 1.0f, 1.0f), albedoColor.xyz, 0.3f);

    return litColor;
}

//////////////////////////////////////////////////////////////////////////////////
// Lambert拡散反射を計算
//////////////////////////////////////////////////////////////////////////////////
float3 CalcLambertDiffuse(float3 lightDirection, float3 lightColor, float3 normal)
{
    normal         = normalize(normal);
    lightDirection = normalize(lightDirection);

    float t = max(0.0f, dot(normal, lightDirection) * -1.0f);

    return lightColor * t;
}

//////////////////////////////////////////////////////////////////////////////////
// Phong鏡面反射を計算
//////////////////////////////////////////////////////////////////////////////////
float3 CalcPhongSpecular(float3 lightDirection, float3 lightColor, float3 worldPos, float3 normal, float2 uv)
{
    float3 refVec = reflect(lightDirection, normal);
    float3 toEye  = normalize(light.cameraPosition - worldPos);

    float t       = pow(max(0.0f, dot(refVec, toEye)), specularPower);
    // ComputeFresnelは内部で -normalize(viewDir) を使うため、
    // toEye（サーフェスから視点への方向）を反転して渡す
    float fresnel = ComputeFresnel(normal, -toEye, baseReflectance);

    return lightColor * t * specularScale * fresnel;
}

//////////////////////////////////////////////////////////////////////////////////
// ディレクションライトを計算
//////////////////////////////////////////////////////////////////////////////////
float3 CalcLigFromDrectionLight(SPSIn psIn, float3 normal)
{
    float3 diff = CalcLambertDiffuse(
        light.directionLight.direction, light.directionLight.color, normal);
    float3 spec = CalcPhongSpecular(
        light.directionLight.direction, light.directionLight.color, psIn.worldPos, normal, psIn.uv);

    return diff + spec;
}

//////////////////////////////////////////////////////////////////////////////////
// フレネル項を計算
//////////////////////////////////////////////////////////////////////////////////
float ComputeFresnel(float3 normal, float3 viewDir, float baseReflectance)
{
    float cosTheta             = saturate(dot(normalize(normal), -normalize(viewDir)));
    float angleFactor          = pow(1.0f - cosTheta, 5.0f);
    float remainingReflectance = 1.0f - baseReflectance;

    return baseReflectance + remainingReflectance * angleFactor;
}

//////////////////////////////////////////////////////////////////////////////////
// 法線マップから法線を計算
//////////////////////////////////////////////////////////////////////////////////
float3 ComputeNormal(SPSIn psIn, float2 uv, float scroll)
{
    float3 localNormal = g_normalMap.Sample(g_sampler, uv + scroll).xyz;
    localNormal        = normalize(localNormal);
    localNormal        = (localNormal - 0.5f) * 2.0f;

    float3 normal = psIn.tangent  * localNormal.x
                  + psIn.biNormal * localNormal.y
                  + psIn.normal   * localNormal.z;

    return normalize(normal);
}
