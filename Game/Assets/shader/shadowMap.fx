/*!
 * @brief シャドウマップ書き込み用シェーダー
 * @details ライトから見た深度をR32_FLOATのレンダリングターゲットへ書き込む。
 *          頂点レイアウトと定数バッファはRenderToGBuffer.fxと同じものを使うため、
 *          同じtkmモデルをそのまま描画できる。
 *          カメラ行列の代わりにライトのビュー・プロジェクション行列が渡される。
 */

// モデル用の定数バッファー
// MeshParts::SConstantBufferのレイアウトと一致させること
// (mWorld / mView / mProj / mulColor)
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;    // ワールド行列
    float4x4 mView;     // ライトのビュー行列
    float4x4 mProj;     // ライトのプロジェクション行列
    float4   mulColor;  // 乗算カラー（このシェーダーでは未使用）
};

// スキニング用の頂点データをひとまとめ。
struct SSkinVSIn
{
    int4   Indices : BLENDINDICES0;     // 影響するボーンの番号（最大4本）
    float4 Weights : BLENDWEIGHT0;      // 各ボーンの影響度（合計1.0になる）
};

// 頂点シェーダーへの入力
// RenderToGBuffer.fx と同じレイアウトにすること
struct SVSIn
{
    float4 pos      : POSITION;      // 頂点の座標（モデル座標系）
    float3 normal   : NORMAL;        // 頂点の法線ベクトル
    float3 tangent  : TANGENT;       // 接ベクトル
    float3 biNormal : BINORMAL;      // 従ベクトル
    float2 uv       : TEXCOORD0;     // UV座標
    SSkinVSIn skinVert;              // スキニング用データ
    uint instanceID : SV_InstanceID; // インスタンシング描画時のID
};

// ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos   : SV_POSITION;   // ライトのクリップ空間座標
    float  depth : TEXCOORD0;     // ライト空間での深度（0〜1）
};

StructuredBuffer<float4x4> g_boneMatrix       : register(t3);    // ボーン行列（スキニング用）
StructuredBuffer<float4x4> g_worldMatrixArray : register(t10);   // インスタンス用ワールド行列

// スキン行列を計算する。
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

    // 4本目のウェイトは「1.0 - 残り3本の合計」で求める
    skinning += g_boneMatrix[skinVert.Indices[3]] * (1.0f - w);

    return skinning;
}

// 頂点シェーダーの本体
SPSIn VSMainCore(SVSIn vsIn, uniform bool hasSkin, uniform bool isEnableInstancingDraw)
{
    SPSIn psIn;
    float4x4 m;
    if (hasSkin)
    {
        m = CalcSkinMatrix(vsIn.skinVert);
    }
    else
    {
        if (isEnableInstancingDraw)
        {
            m = g_worldMatrixArray[vsIn.instanceID];
        }
        else
        {
            m = mWorld;
        }
    }

    // ワールド空間へ変換してから、ライトのビュー・プロジェクションを掛ける
    float4 worldPos = mul(m, vsIn.pos);
    float4 viewPos = mul(mView, worldPos);
    psIn.pos = mul(mProj, viewPos);

    // ライト空間での深度を0〜1で保持する
    // 直交投影なのでwは1になるが、透視投影へ変更した場合にも対応できるよう除算しておく
    psIn.depth = psIn.pos.z / psIn.pos.w;

    return psIn;
}

// スキンなしメッシュ用の頂点シェーダーのエントリー関数。
SPSIn VSMain(SVSIn vsIn)
{
    return VSMainCore(vsIn, false, false);
}
// スキンありメッシュの頂点シェーダーのエントリー関数。
SPSIn VSMainSkin(SVSIn vsIn)
{
    return VSMainCore(vsIn, true, false);
}
// インスタンシングありスキンなしメッシュ用の頂点シェーダーのエントリー関数。
SPSIn VSInstancingMain(SVSIn vsIn)
{
    return VSMainCore(vsIn, false, true);
}
// インスタンシングありスキンありメッシュの頂点シェーダーのエントリー関数。
SPSIn VSSkinInstancingMain(SVSIn vsIn)
{
    return VSMainCore(vsIn, true, true);
}

// ピクセルシェーダーのエントリーポイント
// ライト空間の深度をそのまま書き込む
float4 PSMain(SPSIn psIn) : SV_Target0
{
    return float4(psIn.depth, 0.0f, 0.0f, 1.0f);
}
