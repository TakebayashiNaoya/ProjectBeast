// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;    // ワールド行列
    float4x4 mView;     // ビュー行列
    float4x4 mProj;     // プロジェクション行列
};

//スキニング用の頂点データをひとまとめ。
struct SSkinVSIn
{
    int4   Indices : BLENDINDICES0;     // 影響するボーンの番号（最大4本）
    float4 Weights : BLENDWEIGHT0;      // 各ボーンの影響度（合計1.0になる）
};

// 頂点シェーダーへの入力（1頂点のデータ）
struct SVSIn  
{
    float4 pos      : POSITION;      // 頂点の座標（モデル座標系）
    float3 normal   : NORMAL;        // 頂点の法線ベクトル
    float3 tangent  : TANGENT;       // 接ベクトル（法線マップ用）
    float3 biNormal : BINORMAL;      // 従ベクトル（法線マップ用）
    float2 uv       : TEXCOORD0;     // UV座標（テクスチャのどこを使うか）
    SSkinVSIn skinVert;              // スキニング用データ
    uint instanceID : SV_InstanceID; // インスタンシング描画時のID
};

// ピクセルシェーダーへの入力（頂点シェーダーの出力がここに来る）
struct SPSIn  
{
    float4 pos      : SV_POSITION;   // スクリーン座標系に変換済みの座標
    float3 normal   : NORMAL;        // ワールド空間に変換済みの法線
    float3 tangent  : TANGENT;       // ワールド空間に変換済みの接ベクトル
    float3 biNormal : BINORMAL;      // ワールド空間に変換済みの従ベクトル
    float2 uv       : TEXCOORD0;     // UV座標（そのまま引き継ぐ）
    float3 worldPos : TEXCOORD1;     // ワールド座標（ライティング計算用）
};

// ピクセルシェーダーからの出力（GBufferに書き込まれる）
struct SPSOut  
{
    float4 albedo  : SV_Target0;  // アルベド（色） → GBuffer[0]に書き込み
    float4 normal  : SV_Target1;  // 法線           → GBuffer[1]に書き込み
    float  specPow : SV_Target2;  // スペキュラ強度  → GBuffer[2]に書き込み
};

//シェーダーリソース
Texture2D<float4> g_albedo      : register(t0);  // アルベドマップ（モデルの色テクスチャ）
Texture2D<float4> g_normalMap   : register(t1);  // 法線マップ（凹凸情報）
Texture2D<float4> g_specularMap : register(t2);  // スペキュラマップ（光沢情報）
StructuredBuffer<float4x4> g_boneMatrix       : register(t3);   // ボーン行列（スキニング用）
StructuredBuffer<float4x4> g_worldMatrixArray : register(t10);  // インスタンス用ワールド行列
sampler g_sampler : register(s0);  // サンプラー（テクスチャをどう読むかの設定）

float3 CalcNormal(SPSIn psIn);

//スキン行列を計算する。
float4x4 CalcSkinMatrix(SSkinVSIn skinVert)
{
    float4x4 skinning = 0;   // 結果行列（ゼロ初期化）
    float w = 0.0f;          // ウェイトの合計（正規化確認用）

    [unroll]  // ループを展開してGPU最適化
    for (int i = 0; i < 3; i++)
    {
        // ボーン行列 × ウェイト を加算していく（加重平均）
        skinning += g_boneMatrix[skinVert.Indices[i]] * skinVert.Weights[i];
        w += skinVert.Weights[i];  // ウェイト合計を積算
    }

    // 4本目のウェイトは「1.0 - 残り3本の合計」で求める（精度節約のため）
    skinning += g_boneMatrix[skinVert.Indices[3]] * (1.0f - w);

    return skinning;
}

//モデル用の頂点シェーダーのエントリーポイント
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
            m = g_worldMatrixArray[vsIn.instanceID]; //インスタンスIDに対応するワールド行列を取得。
        }
        else
        {
            m = mWorld;
        }

    }

    psIn.pos = mul(m, vsIn.pos); // モデルの頂点をワールド座標系に変換
    psIn.worldPos = psIn.pos;
    float4 viewPos = mul(mView, psIn.pos); // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, viewPos); // カメラ座標系からスクリーン座標系に変換
    
    //法線、接ベクトル、従ベクトルをワールド空間に変換する。
    //平行移動を無視するために、3x3行列に変換してから乗算する。
    float3x3 m3x3 = (float3x3) m;
    psIn.normal = normalize(mul(m3x3, vsIn.normal));
    psIn.tangent = normalize(mul(m3x3, vsIn.tangent));
    psIn.biNormal = normalize(mul(m3x3, vsIn.biNormal));

    psIn.uv = vsIn.uv;
    
    // 法線や接ベクトル、従ベクトルがNaNになっていたら0ベクトルにする。
    if (any(isnan(psIn.tangent)))
    {
        psIn.tangent.xyz = 0;
    }
    
    if (any(isnan(psIn.biNormal)))
    {
        psIn.biNormal.xyz = 0;
    }
    
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
//モデル用のピクセルシェーダーのエントリーポイント
SPSOut PSMainCore(SPSIn psIn, bool isShadowReciever)
{
    //GBufferに出力
    SPSOut psOut;
    
    //アルベドカラーの抽出
    psOut.albedo = g_albedo.Sample(g_sampler, psIn.uv);
    clip(psOut.albedo.a - 0.2f); // ピクセルキル
    psOut.albedo.w = psIn.pos.z / psIn.pos.w;

        
    psOut.normal.xyz = CalcNormal(psIn);
    //psOut.normal.xyz = normalize(psOut.normal.xyz);

    
    
    psOut.specPow = g_specularMap.Sample(g_sampler, psIn.uv); //スペキュラ強度はとりあえず1.0fで固定。
        
    // シャドウレシーバーかどうかを判定するフラグをw成分に格納する。
    //法線マップのｗは使わないので、ここに格納する。
    if (isShadowReciever == true)
    {
        psOut.normal.w = 1.0f;
    }
    else
    {
        psOut.normal.w = 0.0f;
    }
    
    return psOut;
}


SPSOut PSMainShadowReciever(SPSIn psIn) : SV_Target0
{
    return PSMainCore(psIn, true);
}

SPSOut PSMain(SPSIn psIn) : SV_Target0
{
    return PSMainCore(psIn, false);
}

//////関数/////////////////////////////////////////////////////////////////////

// 法線マップから法線を計算する関数
float3 CalcNormal(SPSIn psIn)
{
    float3 normalMap = g_normalMap.Sample(g_sampler, psIn.uv).xyz;

    // sRGB で読み込まれているので、リニア→sRGB 変換で元の値に戻す
    normalMap = pow(normalMap, 1.0f / 2.2f);

    normalMap = (normalMap - 0.5f) * 2.0f;

    float3 normal;
    normal = (psIn.tangent * normalMap.x) 
           + (psIn.biNormal * normalMap.y) 
           + (psIn.normal * normalMap.z);

    normal = normalize(normal);
    normal = (normal / 2.0f) + 0.5f;

    return normal;
}
