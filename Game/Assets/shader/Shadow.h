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
 * @brief シャドウアクネを防ぐための深度バイアス
 * @details 自分自身の深度と比較して誤って影と判定される（縞模様が出る）のを防ぐ。
 *          大きくしすぎると影が実際の位置からずれる（ピーターパン現象）。
 */
static const float SHADOW_DEPTH_BIAS = 0.0015f;

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
 * @param outRate    光が当たっている割合の出力先（0.0=完全な影 1.0=影なし）
 * @return このカスケードの範囲内で判定できたならtrue
 */
bool TryCalcShadowRateInCascade(
    float3 worldPos,
    float4x4 lvpMatrix,
    Texture2D<float4> shadowMap,
    sampler shadowSmp,
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
    float depthFromLight = lvpPos.z - SHADOW_DEPTH_BIAS;
    float texelOffset = 1.0f / SHADOW_MAP_SIZE;

    // 3x3のPCFで平均を取り、輪郭のジャギを緩和する
    float lightRate = 0.0f;
    [unroll]
    for (int y = -1; y <= 1; y++)
    {
        [unroll]
        for (int x = -1; x <= 1; x++)
        {
            float2 uv = shadowUV + float2(x, y) * texelOffset;
            float shadowMapDepth = shadowMap.Sample(shadowSmp, uv).r;
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

    if (TryCalcShadowRateInCascade(worldPos, lvp0, sm0, shadowSmp, rate)) { return rate; }
    if (TryCalcShadowRateInCascade(worldPos, lvp1, sm1, shadowSmp, rate)) { return rate; }
    if (TryCalcShadowRateInCascade(worldPos, lvp2, sm2, shadowSmp, rate)) { return rate; }

    // どのカスケードの範囲にも入らない（影を出す距離より遠い）
    return 1.0f;
}

#endif // _SHADOW_H_
