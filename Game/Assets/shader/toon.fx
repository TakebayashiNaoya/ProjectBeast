/*!
 * @brief トゥーンシェーダー
 * @details
 *   法線とライト方向の内積を取り、閾値以下なら影色を乗算するシンプルなトゥーン実装。
 *   フォワードレンダリングパスで描画する。GBufferへは書き込まない。
 */

// モデル用の定数バッファ（b0）
// MeshParts::SConstantBufferのレイアウトと一致させること
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;    // ワールド行列
    float4x4 mView;     // ビュー行列
    float4x4 mProj;     // プロジェクション行列
    float4   mulColor;  // 乗算カラー（RGBA, 1.0f=変更なし）
};

// ライト用の定数バッファ（b1）
// C++側 Light構造体（SceneLight.h）と一致させること
struct SDirectionLight
{
    float3   direction;  // ライトの方向
    float    padding0;
    float3   color;      // ライトの色
    float    padding1;
    float4x4 LVP;        // ライトビュープロジェクション行列
};

cbuffer LightCb : register(b1)
{
    SDirectionLight directionLight;   // ディレクションライト
    float3          cameraPosition;   // カメラの位置
    float           padding0;
    float3          ambientLightColor;// 環境光の色
    float           padding1;
    float3          rimLightColor;    // リムライトの色
    float           padding2;
    float4x4        mViewProjInv;     // カメラのビュープロジェクション逆行列
};

// トゥーンパラメータの定数バッファ（b2）
// C++側 SToonCb（ModelRender.h）と一致させること
// 段階は明るい側から順に並べる（threshold[0] > threshold[1] > threshold[2] > threshold[3]）
// stepCount で実際に使う段階数を指定する（1〜4）
cbuffer ToonCb : register(b2)
{
    float4 shadowThresholds;  // 各段階の閾値（x=段階1, y=段階2, z=段階3, w=段階4）
    float4 shadowColorRates;  // 各段階の暗さ係数（x=段階1, y=段階2, z=段階3, w=段階4）
    int    stepCount;         // 実際に使う段階数（1〜4）
    float3 toonPad;           // パディング
};

// スキニング用の頂点データ
struct SSkinVSIn
{
    int4   Indices : BLENDINDICES0;     // 影響するボーンの番号（最大4本）
    float4 Weights : BLENDWEIGHT0;      // 各ボーンの影響度
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos      : POSITION;         // モデル座標系の頂点座標
    float3 normal   : NORMAL;           // 頂点の法線ベクトル
    float3 tangent  : TANGENT;          // 接ベクトル
    float3 biNormal : BINORMAL;         // 従ベクトル
    float2 uv       : TEXCOORD0;        // UV座標
    SSkinVSIn skinVert;                 // スキニング用データ
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos      : SV_POSITION;      // スクリーン座標
    float3 normal   : NORMAL;           // ワールド空間の法線
    float2 uv       : TEXCOORD0;        // UV座標
};

// シェーダーリソース
Texture2D<float4> g_albedo    : register(t0);   // アルベドマップ
StructuredBuffer<float4x4> g_boneMatrix : register(t3);  // ボーン行列
sampler g_sampler : register(s0);               // サンプラー

// スキン行列を計算する
float4x4 CalcSkinMatrix(SSkinVSIn skinVert)
{
    float4x4 skinning = 0;
    float w = 0.0f;

    [unroll]
    for (int i = 0; i < 3; i++)
    {
        skinning += g_boneMatrix[skinVert.Indices[i]] * skinVert.Weights[i];
        w += skinVert.Weights[i];
    }
    skinning += g_boneMatrix[skinVert.Indices[3]] * (1.0f - w);

    return skinning;
}

// 頂点シェーダー（スキンなし）
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;

    float4 worldPos = mul(mWorld, vsIn.pos);
    float4 viewPos  = mul(mView,  worldPos);
    psIn.pos        = mul(mProj,  viewPos);

    // 法線をワールド空間に変換する（平行移動を除くため w=0）
    psIn.normal = normalize(mul((float3x3)mWorld, vsIn.normal));

    psIn.uv = vsIn.uv;

    return psIn;
}

// 頂点シェーダー（スキンあり）
SPSIn VSMainSkin(SVSIn vsIn)
{
    SPSIn psIn;

    float4x4 skinMatrix = CalcSkinMatrix(vsIn.skinVert);
    float4   skinPos    = mul(skinMatrix, vsIn.pos);

    // スキン行列にはワールド変換が含まれているため、mWorldは掛けない
    float4 viewPos  = mul(mView, skinPos);
    psIn.pos        = mul(mProj, viewPos);

    // 法線はスキン行列で変換する（mWorldは不要）
    psIn.normal = normalize(mul((float3x3)skinMatrix, vsIn.normal));

    psIn.uv = vsIn.uv;

    return psIn;
}

// ピクセルシェーダー
float4 PSMain(SPSIn psIn) : SV_Target0
{
    // アルベドカラーをサンプリングする
    float4 albedo = g_albedo.Sample(g_sampler, psIn.uv);
    albedo *= mulColor;

    // アルファが低いピクセルを破棄する
    clip(albedo.a - 0.2f);

    // 法線とライト方向の内積を取る
    // ライト方向は「光が来る方向」なので逆方向にして計算する
    float dotNL = dot(psIn.normal, -directionLight.direction);

    // 閾値を明るい側から順に比較し、最初に一致した段階の係数を適用する
    // stepCount で使用する段階数を制限する（最大4段階）
    float thresholds[4] = { shadowThresholds.x, shadowThresholds.y, shadowThresholds.z, shadowThresholds.w };
    float colorRates[4] = { shadowColorRates.x, shadowColorRates.y, shadowColorRates.z, shadowColorRates.w };

    int count = clamp(stepCount, 1, 4);
    for (int i = 0; i < count; i++)
    {
        if (dotNL <= thresholds[i])
        {
            albedo.rgb *= colorRates[i];
            break;
        }
    }

    return albedo;
}
