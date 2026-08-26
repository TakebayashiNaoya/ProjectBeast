/*!
 * @brief アウトラインシェーダー（背面法線押し出し方式）
 * @details
 *   頂点をクリップ空間で法線方向に押し出し、前面をカリングすることで
 *   背面のみを単色で描画し輪郭線を表現する。
 *   C++側で D3D12_CULL_MODE_FRONT を設定すること。
 */

// モデル用の定数バッファ（b0）
// MeshParts::SConstantBufferのレイアウトと一致させること
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;    // ワールド行列
    float4x4 mView;     // ビュー行列
    float4x4 mProj;     // プロジェクション行列
    float4   mulColor;  // 乗算カラー（未使用だがレイアウト一致のために定義）
};

// アウトラインパラメータの定数バッファ（b2）
// C++側 SOutlineCb（ModelRender.h）と一致させること
cbuffer OutlineCb : register(b2)
{
    float  outlineWidth;    // 輪郭線の太さ（法線押し出し量）
    float3 outlinePad;      // パディング
    float4 outlineColor;    // 輪郭線の色（RGBA）
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
    float3 tangent  : TANGENT;          // 接ベクトル（未使用・構造一致用）
    float3 biNormal : BINORMAL;         // 従ベクトル（未使用・構造一致用）
    float2 uv       : TEXCOORD0;        // UV座標（未使用・構造一致用）
    SSkinVSIn skinVert;                 // スキニング用データ
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION;           // スクリーン座標
};

// ボーン行列
StructuredBuffer<float4x4> g_boneMatrix : register(t3);

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
// クリップ空間で法線方向に頂点を押し出して輪郭線を作る
SPSIn VSMain(SVSIn vsIn)
{
    SPSIn psIn;

    float4 worldPos = mul(mWorld, vsIn.pos);
    float4 viewPos  = mul(mView,  worldPos);
    float4 clipPos  = mul(mProj,  viewPos);

    // 法線をクリップ空間に変換する（XY成分のみ使用）
    float3 worldNormal = normalize(mul((float3x3)mWorld, vsIn.normal));
    float4 clipNormal  = mul(mProj, mul(mView, float4(worldNormal, 0.0f)));

    // クリップ空間のXY方向に押し出す（Wで正規化することで透視変換の影響を除く）。
    // ただしスクリーン幅一定のままだと遠距離でモデルが輪郭線に飲み込まれるため、
    // ワールド単位の上限（OUTLINE_MAX_WORLD_WIDTH）でクランプして、
    // 遠くでは輪郭が細くなるようにする
    const float OUTLINE_MAX_WORLD_WIDTH = 6.0f;
    float widthNdc = min(outlineWidth, OUTLINE_MAX_WORLD_WIDTH * mProj._m00 / max(clipPos.w, 0.001f));
    clipPos.xy += normalize(clipNormal.xy) * widthNdc * clipPos.w;

    psIn.pos = clipPos;

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
    float4 clipPos  = mul(mProj, viewPos);

    // 法線はスキン行列で変換する（mWorldは不要）
    float3 skinNormal = normalize(mul((float3x3)skinMatrix, vsIn.normal));
    float4 clipNormal = mul(mProj, mul(mView, float4(skinNormal, 0.0f)));

    // クリップ空間のXY方向に押し出す（VSMainと同じく遠距離はワールド単位でクランプ）
    const float OUTLINE_MAX_WORLD_WIDTH = 6.0f;
    float widthNdc = min(outlineWidth, OUTLINE_MAX_WORLD_WIDTH * mProj._m00 / max(clipPos.w, 0.001f));
    clipPos.xy += normalize(clipNormal.xy) * widthNdc * clipPos.w;

    psIn.pos = clipPos;

    return psIn;
}

// ピクセルシェーダー（輪郭線の色を単色で出力する）
float4 PSMain(SPSIn psIn) : SV_Target0
{
    return outlineColor;
}
