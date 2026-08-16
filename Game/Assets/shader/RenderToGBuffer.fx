/*!
 * @brief GBuffer書き込み用シェーダー
 */

// モデル用の定数バッファー
// MeshParts::SConstantBufferのレイアウトと一致させること
// (mWorld / mView / mProj / mulColor)
cbuffer ModelCb : register(b0)
{
    float4x4 mWorld;    // ワールド行列
    float4x4 mView;     // ビュー行列
    float4x4 mProj;     // プロジェクション行列
    float4   mulColor;  // 乗算カラー (RGBA, 1.0f=変更なし)
};

// スキニング用の頂点データをひとまとめ。
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
    float3 worldPos : TEXCOORD1;     // ワールド座標（ライティング計算・ディザリング判定用）
};

// ピクセルシェーダーからの出力（GBufferに書き込まれる）
// metaricSmoothMap のレイアウト:
//   r = metallic
//   g = dirLightScale（ディレクションライト強度倍率）
//   b = ambientScale（環境光強度倍率）
//   a = smooth
struct SPSOut
{
    float4 albedo           : SV_Target0;  // アルベド（色）  → GBuffer[0]に書き込み
    float4 normal           : SV_Target1;  // 法線            → GBuffer[1]に書き込み
    float4 metaricSmoothMap : SV_Target2;  // PBRパラメータ   → GBuffer[2]に書き込み
};

// シェーダーリソース
Texture2D<float4> g_albedo            : register(t0);    // アルベドマップ（モデルの色テクスチャ）
Texture2D<float4> g_normalMap         : register(t1);    // 法線マップ（凹凸情報）
Texture2D<float4> g_metallicSmoothMap : register(t2);    // メタリックスムースマップ（rにmetallic、aにsmooth）
StructuredBuffer<float4x4> g_boneMatrix       : register(t3);    // ボーン行列（スキニング用）
StructuredBuffer<float4x4> g_worldMatrixArray : register(t10);   // インスタンス用ワールド行列
sampler g_sampler : register(s0);  // サンプラー（テクスチャをどう読むかの設定）

// PBR補正パラメータ定数バッファ（b2）
// C++側 PBRParam（ModelRender.h）と一致させること
cbuffer PBRParamCb : register(b2)
{
    float dirLightScale;   // ディレクションライト強度倍率
    float ambientScale;    // 環境光強度倍率
    float metallicOffset;  // metallicオフセット
    float smoothOffset;    // smoothオフセット
};

// ディザリング用の定数バッファ（b3）
// C++側 SDitherCb（OcclusionDitherManager.h）と一致させること
cbuffer DitherCb : register(b3)
{
    float3 cameraWorldPos;  // カメラのワールド座標
    float  cylinderRadius;  // 円柱判定の半径（ワールド空間単位）
    float3 targetWorldPos;  // プレイヤーのワールド座標
    float  depthBias;       // 前後判定のオフセット
    float  ditherStrength;  // ディザリング強度（0.0f=オフ, 0.5f=50%透過, 1.0f=最大）
    float3 ditherPad;       // パディング
};

// モデル単位ディザリング用の定数バッファ（b4）
// C++側 SModelDitherCb（ModelRender.h）と一致させること
// OcclusionDitherManager（b3）によるカメラ遮蔽ディザリングとは独立して動作する
cbuffer ModelDitherCb : register(b4)
{
    float  modelDitherAlpha;  // モデル単位の透過率（0.0f=オフ, 1.0f=完全消去）
    float3 modelDitherPad;    // パディング
};

// Bayer 4x4 ディザリングパターン
// 値が小さいほど先に消える。
static const int g_bayerPattern[4][4] =
{
    {  0, 32,  8, 40 },
    { 48, 16, 56, 24 },
    { 12, 44,  4, 36 },
    { 60, 28, 52, 20 },
};

// 関数宣言
float3 CalcNormal(SPSIn psIn);

// スキン行列を計算する。
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

// モデル用の頂点シェーダーのエントリーポイント
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
            m = g_worldMatrixArray[vsIn.instanceID]; // インスタンスIDに対応するワールド行列を取得。
        }
        else
        {
            m = mWorld;
        }
    }

    psIn.pos = mul(m, vsIn.pos); // モデルの頂点をワールド座標系に変換
    psIn.worldPos = psIn.pos.xyz; // ワールド座標を保持する（射影変換前に取得すること）
    float4 viewPos = mul(mView, psIn.pos); // ワールド座標系からカメラ座標系に変換
    psIn.pos = mul(mProj, viewPos); // カメラ座標系からスクリーン座標系に変換

    // 法線、接ベクトル、従ベクトルをワールド空間に変換する。
    // 平行移動を無視するために、3x3行列に変換してから乗算する。
    float3x3 m3x3 = (float3x3) m;
    psIn.normal   = normalize(mul(m3x3, vsIn.normal));
    psIn.tangent  = normalize(mul(m3x3, vsIn.tangent));
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

// モデル用のピクセルシェーダーのエントリーポイント
SPSOut PSMainCore(SPSIn psIn, bool isShadowReciever)
{
    // ディザリング強度が0より大きい場合のみ判定する
    if (ditherStrength > 0.0f)
    {
        // カメラからターゲット（プレイヤー）へのベクトルと長さを求める
        float3 camToTarget    = targetWorldPos - cameraWorldPos;
        float  camToTargetLen = length(camToTarget);

        // ターゲットが非常に近い場合はゼロ除算を避けるためスキップする
        if (camToTargetLen > 0.001f)
        {
            // カメラからターゲットへの正規化済み方向ベクトル
            float3 camToTargetDir = camToTarget / camToTargetLen;

            // カメラからフラグメントへのベクトル
            float3 camToFrag = psIn.worldPos - cameraWorldPos;

            // フラグメントを視線方向に射影した長さを求める
            // これはフラグメントがカメラからどれだけ「前方」にあるかを示す
            float projLen = dot(camToFrag, camToTargetDir);

            // ① フラグメントがカメラとターゲットの間にあるか判定する
            // depthBiasはカメラ側の余裕（プレイヤーのモデルのめり込み防止）
            bool isBetween = (projLen >= depthBias) && (projLen < camToTargetLen);

            if (isBetween)
            {
                // ② 視線（カメラ→ターゲット方向）の円錐内にあるか判定する
                // 視線からフラグメントへの最短距離 = |camToFrag × camToTargetDir|
                // camToTargetDirは正規化済みなので分母は1
                float3 crossVec  = cross(camToFrag, camToTargetDir);
                float  distToRay = length(crossVec);

                // 円錐の外側半径：カメラに近いほど小さく、ターゲットに近いほど広がる
                float t         = projLen / camToTargetLen;
                float outerCone = cylinderRadius * t;
                // 内側半径：外側の70%をフェード開始点とする
                // 70〜100%の範囲でsmoothstepによるグラデーションをかける
                float innerCone = outerCone * 0.7f;

                // 外側の円錐を超えていたらディザリングしない
                if (distToRay < outerCone)
                {
                    // ③ distToRayをinnerCone〜outerConeでsmoothstepして不透明度alphaを求める
                    // distToRay < innerCone → alpha = 0.0f（最大透過）
                    // distToRay > outerCone → alpha = 1.0f（不透明）
                    // その間は滑らかに補間する
                    float alpha = smoothstep(innerCone, outerCone, distToRay);

                    // ditherStrengthで全体の透過量をスケールする
                    // clip(alpha - threshold) で threshold > alpha のピクセルを破棄する
                    // ditherStrengthが小さいほど全体的に透けにくくなる
                    int x = (int)fmod(psIn.pos.x, 4.0f);
                    int y = (int)fmod(psIn.pos.y, 4.0f);
                    float bayerValue = (float)g_bayerPattern[y][x] / 64.0f;

                    // threshold = ditherStrength * (1.0f - alpha) とすることで
                    // 中心付近（alpha=0）はditherStrength相当の透過率になり
                    // 外縁付近（alpha=1）はthreshold=0で破棄されなくなる
                    float threshold = ditherStrength * (1.0f - alpha);
                    clip(bayerValue - threshold);
                }
            }
        }
    }

    // モデル単位ディザリング判定（b4）
    // ditherStrength（カメラ遮蔽）とは独立して動作する
    if (modelDitherAlpha > 0.0f)
    {
        int x = (int)fmod(psIn.pos.x, 4.0f);
        int y = (int)fmod(psIn.pos.y, 4.0f);
        float bayerValue = (float)g_bayerPattern[y][x] / 64.0f;
        clip(bayerValue - modelDitherAlpha);
    }

    // GBufferに出力
    SPSOut psOut;

    // アルベドカラーの抽出と乗算カラーの適用
    float4 albedoSample = g_albedo.Sample(g_sampler, psIn.uv);
    psOut.albedo = float4(albedoSample.rgb * mulColor.rgb, albedoSample.a);
    clip(psOut.albedo.a - 0.2f); // ピクセルキル
    // ピクセルシェーダーのSV_POSITIONは、zにNDC深度(0〜1)がすでに入っている。
    // wにはクリップ空間のw（視点からの距離）が入るため、zをwで割ってはいけない。
    // 割ってしまうとワールド座標の復元が破綻する（影の判定が成立しなくなる）。
    psOut.albedo.w = psIn.pos.z;

    psOut.normal.xyz = normalize(CalcNormal(psIn));

    // メタリックスムースマップをサンプリングする
    float4 metaricSmooth = g_metallicSmoothMap.Sample(g_sampler, psIn.uv);

    // PBRパラメータをGBufferに書き込む
    // r = metallic（補正値加算）
    // g = dirLightScale（ライト倍率）
    // b = ambientScale（環境光倍率）
    // a = smooth（補正値加算）
    psOut.metaricSmoothMap.r = saturate(metaricSmooth.r + metallicOffset);
    psOut.metaricSmoothMap.g = dirLightScale;
    psOut.metaricSmoothMap.b = ambientScale;
    psOut.metaricSmoothMap.a = saturate(metaricSmooth.a + smoothOffset);

    // シャドウレシーバーかどうかを判定するフラグをw成分に格納する。
    // 法線マップのｗは使わないので、ここに格納する。
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

///////////////////////////////////////////////////////////////////////////////
// 関数

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
