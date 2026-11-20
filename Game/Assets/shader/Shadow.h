/*!
 * @brief 影を受ける側の共通処理（カスケードシャドウマップ）
 * @details ディファードライティング・海・渦潮など、影を受けるシェーダーが共通で使う。
 *          シャドウマップとサンプラーはシェーダーごとにレジスタ番号が違うため、
 *          グローバル変数を直接参照せず引数で受け取る。
 * @details シャドウマップを書き込む側は shadowMap.fx / ShadowMap.cpp。
 */

#ifndef _SHADOW_H_
#define _SHADOW_H_

/**
 * @brief カスケードの数
 * @details ShadowMap.h の NUM_SHADOW_CASCADES と必ず一致させること
 */
#define NUM_SHADOW_CASCADES 3

/** シャドウマップ1枚あたりの解像度。ShadowMap.cpp の SHADOW_MAP_SIZE と一致させること */
static const float SHADOW_MAP_SIZE = 2048.0f;

/**
 * @brief シャドウアクネを防ぐための深度バイアス（海・渦潮用。面の傾きを使わない CalcShadowRate() が使う）
 * @details 自分自身の深度と比較して誤って影と判定される（縞模様が出る）のを防ぐ。
 *          大きくしすぎると影が実際の位置からずれる（ピーターパン現象）。
 * @details ライトのクリップ空間の深度（0〜1）に対する値なので、ワールドでの大きさは
 *          カスケードの深度範囲に比例して変わる。面の傾きを渡せる受け手は CalcShadowRateWithSlope() を使う。
 */
static const float SHADOW_DEPTH_BIAS = 0.0015f;

/**
 * @brief 傾きバイアスに使うtanの上限
 * @details 光とほぼ平行な面ではtanが発散し、バイアスが際限なく大きくなる。
 *          そこまで傾いた面は SHADOW_TERMINATOR_WIDTH で影へ寄せるので、ここで打ち切る。
 *          tan=10 は光との角度でおよそ84度。
 */
static const float SHADOW_SLOPE_TAN_MAX = 10.0f;

/**
 * @brief 光がすれすれに当たる面を影へ寄せる幅（面と光の向きの内積）
 * @details 内積がこの値より小さい面は、シャドウマップの結果に関わらず徐々に影にする。
 *          光とほぼ平行な面はシャドウマップ上で数テクセルの細い帯にしか写らず、
 *          バイアスをどれだけ足しても正しく判定できない（三角形単位のまだら・ちらつきになる）。
 *          裏向きの面（内積が負）は常に影になる。
 *          大きくすると低ポリのモデルで明暗の境界がポリゴン状に見えやすくなる。
 */
static const float SHADOW_TERMINATOR_WIDTH = 0.15f;

/**
 * @brief カスケードの端で参照を打ち切る割合
 * @details UVが0〜1のちょうど端だとPCFの参照が範囲外へはみ出すため、
 *          少し内側で「このカスケードでは判定できない」と扱って次のカスケードへ送る。
 */
static const float CASCADE_UV_MARGIN = 0.02f;

/*!
 * @brief 1つのカスケードで影の割合を求める
 * @details ワールド座標をライト空間へ変換し、シャドウマップの深度と比較する。
 *          3x3のPCFで周囲も参照して輪郭を柔らかくしている。
 * @param worldPos   ワールド座標
 * @param lvpMatrix  このカスケードのライトビュープロジェクション行列
 * @param shadowMap  このカスケードのシャドウマップ
 * @param shadowSmp  シャドウマップをサンプリングするサンプラー
 * @param depthBias  深度バイアス（ライトのクリップ空間の深度0〜1に対する値）
 * @param outRate    光が当たっている割合の出力先（0.0=完全な影 1.0=影なし）
 * @return このカスケードの範囲内で判定できたならtrue
 */
bool TryCalcShadowRateInCascade(
    float3 worldPos,
    float4x4 lvpMatrix,
    Texture2D<float4> shadowMap,
    sampler shadowSmp,
    float depthBias,
    out float outRate)
{
    outRate = 1.0f;

    // ライトから見たクリップ空間へ変換する
    float4 posInLVP = mul(lvpMatrix, float4(worldPos, 1.0f));
    float3 lvpPos = posInLVP.xyz / posInLVP.w;

    // クリップ空間(-1〜1)からUV(0〜1)へ変換する。yは上下が逆になる
    float2 shadowUV = float2(0.5f, -0.5f) * lvpPos.xy + 0.5f;

    // このカスケードの範囲外なら判定できない。呼び出し側が次のカスケードを試す
    if (shadowUV.x < CASCADE_UV_MARGIN || shadowUV.x > 1.0f - CASCADE_UV_MARGIN ||
        shadowUV.y < CASCADE_UV_MARGIN || shadowUV.y > 1.0f - CASCADE_UV_MARGIN ||
        lvpPos.z < 0.0f || lvpPos.z > 1.0f)
    {
        return false;
    }

    // 自分の深度がシャドウマップの深度より奥なら影
    float depthFromLight = lvpPos.z - depthBias;
    float texelOffset = 1.0f / SHADOW_MAP_SIZE;

    // 3x3のPCFで平均を取り、輪郭のジャギを緩和する
    // 呼び出し側の分岐の中で使われるため、勾配に頼らない SampleLevel で引く（ミップは1枚なので結果は同じ）
    float lightRate = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            float2 uv = shadowUV + float2(x, y) * texelOffset;
            float shadowMapDepth = shadowMap.SampleLevel(shadowSmp, uv, 0).r;
            lightRate += (depthFromLight <= shadowMapDepth) ? 1.0f : 0.0f;
        }
    }

    outRate = lightRate / 9.0f;
    return true;
}

/*!
 * @brief 影になっている割合を計算する（カスケード対応）
 * @details 手前のカスケードから順に試し、範囲内に入った最初のもので判定する。
 *          手前ほど狭い範囲を覆っていて精細なため、必ず0番から試すこと。
 *          カメラからの距離でカスケードを選ぶ方式もあるが、
 *          この方式なら受け手側がカメラ座標を持っていなくても使える。
 * @param worldPos   ワールド座標
 * @param lvp0〜lvp2 各カスケードのライトビュープロジェクション行列
 * @param sm0〜sm2   各カスケードのシャドウマップ
 * @param shadowSmp  シャドウマップをサンプリングするサンプラー
 * @return 光が当たっている割合（0.0=完全な影 1.0=影なし）
 */
float CalcShadowRate(
    float3 worldPos,
    float4x4 lvp0, float4x4 lvp1, float4x4 lvp2,
    Texture2D<float4> sm0, Texture2D<float4> sm1, Texture2D<float4> sm2,
    sampler shadowSmp)
{
    float rate = 1.0f;

    if (TryCalcShadowRateInCascade(worldPos, lvp0, sm0, shadowSmp, SHADOW_DEPTH_BIAS, rate)) { return rate; }
    if (TryCalcShadowRateInCascade(worldPos, lvp1, sm1, shadowSmp, SHADOW_DEPTH_BIAS, rate)) { return rate; }
    if (TryCalcShadowRateInCascade(worldPos, lvp2, sm2, shadowSmp, SHADOW_DEPTH_BIAS, rate)) { return rate; }

    // どのカスケードの範囲にも入らない（影を出す距離より遠い）
    return 1.0f;
}

/*!
 * @brief 面の傾きに応じた深度バイアスを求める
 * @details バイアスをワールドの長さで「テクセル長 ×（一律分 + 傾き分）」として決め、
 *          このカスケードのクリップ空間の深度に換算する。
 *          光が斜めに当たる面は、1テクセルの中で深度が「テクセル長 × tan(光との角度)」だけ変わる。
 *          PCFで隣のテクセルを引くと、その差で自分自身を影と誤判定するため、傾きに比例して深くする。
 * @param lvpMatrix  このカスケードのライトビュープロジェクション行列（直交投影）
 * @param slopeTan   光との角度のtan（SHADOW_SLOPE_TAN_MAX で打ち切り済み）
 * @param biasParam  x=一律分（テクセル数） y=傾き分の倍率
 * @return 深度バイアス（ライトのクリップ空間の深度0〜1に対する値）
 */
float CalcSlopeScaledDepthBias(float4x4 lvpMatrix, float slopeTan, float2 biasParam)
{
    // 直交投影なので、行列の各行の長さが「ワールド1あたりのクリップ空間の変化量」になる。
    // x は幅（-1〜1 の2）をテクセル数で割ったものが1テクセル、z はそのまま深度の変化量
    float texelWorldSize = 2.0f / (length(lvpMatrix[0].xyz) * SHADOW_MAP_SIZE);
    float depthPerWorld  = length(lvpMatrix[2].xyz);
    return texelWorldSize * (biasParam.x + biasParam.y * slopeTan) * depthPerWorld;
}

/*!
 * @brief 面の傾きを使って、影になっている割合を計算する（カスケード対応）
 * @details CalcShadowRate() との違いは2つ。
 *          ・深度バイアスを面の傾きとカスケードのテクセル長から決める（CalcSlopeScaledDepthBias）
 *          ・光がすれすれに当たる面と裏向きの面を影へ寄せる（SHADOW_TERMINATOR_WIDTH）
 *          面の向きはGBufferパス（RenderToGBuffer.fx）で求めたものを渡す。
 *          背景はポリゴンそのものの向き、キャラクター（スキンあり）は補間した頂点法線を使っている。理由は同ファイル参照。
 * @param worldPos   ワールド座標
 * @param lvp0〜lvp2 各カスケードのライトビュープロジェクション行列
 * @param sm0〜sm2   各カスケードのシャドウマップ
 * @param shadowSmp  シャドウマップをサンプリングするサンプラー
 * @param geomNdotL  ポリゴンの面の向きと、光源へ向かう方向の内積（-1〜1）
 * @param biasParam  x=一律分（テクセル数） y=傾き分の倍率
 * @param outCascade 判定に使ったカスケードの番号の出力先（どれの範囲にも入らなければ-1。デバッグ表示用）
 * @return 光が当たっている割合（0.0=完全な影 1.0=影なし）
 */
float CalcShadowRateWithSlope(
    float3 worldPos,
    float4x4 lvp0, float4x4 lvp1, float4x4 lvp2,
    Texture2D<float4> sm0, Texture2D<float4> sm1, Texture2D<float4> sm2,
    sampler shadowSmp,
    float geomNdotL,
    float2 biasParam,
    out int outCascade)
{
    // 光との角度のtan。裏向き・すれすれの面は下の終端処理で影にするので、ここでは打ち切るだけでよい
    float cosTheta = max(geomNdotL, 1e-3f);
    float slopeTan = min(sqrt(saturate(1.0f - cosTheta * cosTheta)) / cosTheta, SHADOW_SLOPE_TAN_MAX);

    float rate = 1.0f;
    outCascade = -1;
    if (TryCalcShadowRateInCascade(worldPos, lvp0, sm0, shadowSmp, CalcSlopeScaledDepthBias(lvp0, slopeTan, biasParam), rate)) { outCascade = 0; }
    else if (TryCalcShadowRateInCascade(worldPos, lvp1, sm1, shadowSmp, CalcSlopeScaledDepthBias(lvp1, slopeTan, biasParam), rate)) { outCascade = 1; }
    else if (TryCalcShadowRateInCascade(worldPos, lvp2, sm2, shadowSmp, CalcSlopeScaledDepthBias(lvp2, slopeTan, biasParam), rate)) { outCascade = 2; }

    // 影を出す距離より遠い（どのカスケードにも入らない）場所は影なしのままにする。
    // 終端処理まで掛けると、影の届く距離の境目で壁の明るさが急に変わってしまう
    if (outCascade < 0)
    {
        return 1.0f;
    }

    // 光がすれすれに当たる面・裏向きの面を影へ寄せる
    return min(rate, saturate(geomNdotL / SHADOW_TERMINATOR_WIDTH));
}

/*!
 * @brief 地形のベイク影で、光が当たっている割合を求める
 * @details 地形はロード時に「影の高さマップ」をベイクしており（TerrainObject）、カスケードには描いていない。
 *          マップには「この高さより下は地形の影」というワールドYが入っている。
 * @param worldPos       ワールド座標
 * @param shadowHeights  影の高さマップ（x=影の高さ, y=地面の高さ）
 * @param smp            サンプラー（線形補間）
 * @param uvScaleOffset  ワールドXZ→UV（x=X倍率, y=Xオフセット, z=Z倍率, w=Zオフセット）
 * @param param          x=境界をぼかす高さの幅, y=バイアス, z=有効なら1
 * @return 光が当たっている割合（0.0=完全な影 1.0=影なし）
 */
float CalcTerrainBakedShadowRate(
    float3 worldPos,
    Texture2D<float2> shadowHeights,
    sampler smp,
    float4 uvScaleOffset,
    float4 param)
{
    if (param.z < 0.5f)
    {
        return 1.0f;
    }

    float2 uv = float2(
        worldPos.x * uvScaleOffset.x + uvScaleOffset.y,
        worldPos.z * uvScaleOffset.z + uvScaleOffset.w);
    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
    {
        return 1.0f;
    }

    float2 heights = shadowHeights.SampleLevel(smp, uv, 0);
    // 影を受ける点の高さは、本来の地面より下にはしない。
    // 遠くの地形はLODで頂点を間引いており、崖などでは粗いメッシュの面が本来の地面より低い所を通る。
    // そのまま比べると、本来の地面より低い点だけ影になって縞模様が出る
    float receiverY = max(worldPos.y, heights.y);
    // 影の高さより上なら光が当たる。境界は param.x の幅でぼかす
    return saturate((receiverY + param.y - heights.x) / param.x + 0.5f);
}

#endif // _SHADOW_H_
