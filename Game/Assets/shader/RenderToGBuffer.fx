// モデル用の定数バッファー
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProj;
};

//スキニング用の頂点データをひとまとめ。
struct SSkinVSIn
{
    int4 Indices : BLENDINDICES0;
    float4 Weights : BLENDWEIGHT0;
};

// 頂点シェーダーへの入力
struct SVSIn
{
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv : TEXCOORD0;
    SSkinVSIn skinVert; //スキン用のデータ。
    uint instanceID : SV_InstanceID;

};

//ピクセルシェーダーへの入力
struct SPSIn
{
    float4 pos : SV_POSITION; //座標。
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 biNormal : BINORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

// ピクセルシェーダーからの出力
struct SPSOut
{
    float4 albedo : SV_Target0; // アルベド
    float4 normal : SV_Target1; // 法線
    float specPow : SV_Target2; // スペキュラ強度
};

//シェーダーリソース
Texture2D<float4> g_albedo : register(t0); //アルベドマップ
Texture2D<float4> g_normalMap : register(t1); //法線マップにアクセスするための変数。
Texture2D<float4> g_specularMap : register(t2); //スペキュラマップにアクセスするための変数。
StructuredBuffer<float4x4> g_boneMatrix : register(t3); //ボーン行列。
StructuredBuffer<float4x4> g_worldMatrixArray : register(t10); //ワールド行列の配列。インスタンシング描画の際に有効。


//サンプラーステート
sampler g_sampler : register(s0);

float3 CalcNormal(SPSIn psIn);

//スキン行列を計算する。
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
    psOut.normal.xyz = normalize(psOut.normal.xyz);

    
    
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
    // 法線マップからタンジェントスペースの法線をサンプリングする
    float3 normalMap = g_normalMap.Sample(g_sampler, psIn.uv).xyz;
    normalMap = (normalMap - 0.5f) * 2.0f;
    
    float3 normal = float3(1.0f, 1.0f, 1.0f);

    // タンジェントスペースの法線をワールドスペースに変換する
    normal = (psIn.tangent * normalMap.x) + (psIn.biNormal * normalMap.y) + (psIn.normal * normalMap.z);

    //// 出力は0～1に丸められてしまいマイナスの値が失われてしまうので-1～1を0～1に変換する
    normal = (normal / 2.0f) + 0.5f;

    return normal;
}
