/*!
 * @brief トーンマップ（HDR → LDR 変換）
 * @details メインレンダリングターゲットのHDR値を0〜1の範囲へ圧縮する。
 *          方式ごとにピクセルシェーダーを用意しており、
 *          C++側の EnToneMapType で使用するエントリーポイントを切り替える。
 * @details 各方式は「RGBベース」と「輝度ベース」の2通りで適用できる。
 *          RGBベースは各チャンネルを独立に圧縮するため、明るいチャンネルほど
 *          強く潰れて3チャンネルが互いに近づき、明部の色が白へ抜ける。
 *          輝度ベースは色比を保ったまま輝度だけを圧縮するので彩度が残る。
 */

cbuffer cb : register(b0)
{
    float4x4 mvp;       // MVP行列
    float4 mulColor;    // 乗算カラー
};

/*!
 * @brief トーンマップパラメーター
 * @details C++側の ToneMap::SToneMapCb と一致させること
 */
cbuffer ToneMapCb : register(b1)
{
    float exposure;          // 露出倍率
    float whitePoint;        // 白とみなす輝度（PSToneMapReinhardExtendedのみ使用）
    float applyGamma;        // sRGBエンコードを行うか（0:しない 1:する）
    float isLuminanceBased;  // 輝度ベースで適用するか（0:RGBベース 1:輝度ベース）
};

/** 輝度を求める重み（人間の視覚特性に合わせた係数） */
static const float3 LUMINANCE_WEIGHT = float3(0.2125f, 0.7154f, 0.0721f);

/** ゼロ除算を避けるための下限値 */
static const float EPSILON = 0.00001f;

struct VSInput
{
    float4 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

Texture2D<float4> mainRenderTargetTexture : register(t0); // メインRTのテクスチャ
sampler Sampler : register(s0);

/*!
 * @brief 頂点シェーダー
 */
PSInput VSMain(VSInput In)
{
    PSInput psIn;
    psIn.pos = mul(mvp, In.pos);
    psIn.uv = In.uv;
    return psIn;
}

/////////////////////////////////////////////////////////
// 共通処理
/////////////////////////////////////////////////////////
/*!
 * @brief 露出を適用したHDRカラーを取得する
 */
float3 SampleExposedColor(float2 uv)
{
    float3 color = mainRenderTargetTexture.Sample(Sampler, uv).rgb;
    return color * exposure;
}

/*!
 * @brief 輝度を取得する
 */
float GetLuminance(float3 color)
{
    return max(dot(color, LUMINANCE_WEIGHT), EPSILON);
}

/*!
 * @brief RGBベースと輝度ベースを切り替えて結果を選ぶ
 * @param exposedColor 露出適用後のHDRカラー
 * @param curvedRgb    各チャンネルにカーブを適用した結果
 * @param curvedLum    輝度にカーブを適用した結果
 * @details 輝度ベースでは色比 exposedColor/luminance を保ったまま、
 *          輝度だけを圧縮後の値へ置き換える。
 *          彩度の高い明るい色ではチャンネルが1.0を超えることがあるため、
 *          最終的に ToOutputColor() でクランプしている。
 */
float3 SelectToneMapResult(float3 exposedColor, float3 curvedRgb, float curvedLum)
{
    if (isLuminanceBased > 0.5f)
    {
        return exposedColor * (curvedLum / GetLuminance(exposedColor));
    }
    return curvedRgb;
}

/*!
 * @brief トーンマップ結果を出力カラーへ変換する
 * @details applyGamma が有効ならsRGBエンコードを掛ける。
 *          バックバッファが非sRGBフォーマットのため、シェーダー側で補正する必要がある。
 */
float4 ToOutputColor(float3 color)
{
    color = saturate(color);
    if (applyGamma > 0.5f)
    {
        color = pow(color, 1.0f / 2.2f);
    }
    return float4(color, 1.0f);
}

/////////////////////////////////////////////////////////
// 各方式のカーブ
// スカラーにも使えるよう、輝度は x.xxx の形で渡して .x を取る
/////////////////////////////////////////////////////////
/*!
 * @brief Reinhardのカーブ
 */
float3 CurveReinhard(float3 x)
{
    return x / (1.0f + x);
}

/*!
 * @brief ホワイトポイント付きReinhardのカーブ
 * @details whitePoint の値がちょうど1.0になるよう正規化されている
 */
float3 CurveReinhardExtended(float3 x)
{
    float white = max(whitePoint, EPSILON);
    return (x * (1.0f + x / (white * white))) / (1.0f + x);
}

/*!
 * @brief ACESフィルミックのカーブ
 * @details Krzysztof Narkowicz による近似式
 */
float3 CurveACES(float3 x)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return (x * (a * x + b)) / (x * (c * x + d) + e);
}

/*!
 * @brief Uncharted2のフィルミックカーブの素の形
 * @details John Hable によるカーブ。CurveUncharted2 から呼び出す
 */
float3 Uncharted2Raw(float3 x)
{
    const float A = 0.15f;  // ショルダーの強さ
    const float B = 0.50f;  // 線形部の強さ
    const float C = 0.10f;  // 線形部の角度
    const float D = 0.20f;  // トゥの強さ
    const float E = 0.02f;  // トゥの分子
    const float F = 0.30f;  // トゥの分母

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

/*!
 * @brief Uncharted2のカーブ
 * @details カーブ適用後に白基準で正規化し、白がきちんと1.0になるようにしている
 */
float3 CurveUncharted2(float3 x)
{
    // 露光補正の基準値。大きいほど明るい側に階調が寄る
    const float EXPOSURE_BIAS = 2.0f;
    // 白とみなす入力値
    const float LINEAR_WHITE = 11.2f;

    return Uncharted2Raw(x * EXPOSURE_BIAS) / Uncharted2Raw(LINEAR_WHITE.xxx);
}

/////////////////////////////////////////////////////////
// 露出のみ（比較用の基準）
/////////////////////////////////////////////////////////
/*!
 * @brief 露出を掛けてクランプするだけのピクセルシェーダー
 * @details 圧縮カーブが無いので輝度ベースにしても結果は変わらない。
 *          トーンマップ無しとの差を確認するための基準として用意している
 */
float4 PSToneMapExposure(PSInput In) : SV_Target0
{
    return ToOutputColor(SampleExposedColor(In.uv));
}

/////////////////////////////////////////////////////////
// Reinhard
/////////////////////////////////////////////////////////
/*!
 * @brief Reinhardトーンマップ
 */
float4 PSToneMapReinhard(PSInput In) : SV_Target0
{
    float3 c = SampleExposedColor(In.uv);
    float  l = GetLuminance(c);
    return ToOutputColor(SelectToneMapResult(c, CurveReinhard(c), CurveReinhard(l.xxx).x));
}

/////////////////////////////////////////////////////////
// Reinhard（ホワイトポイント付き）
/////////////////////////////////////////////////////////
/*!
 * @brief ホワイトポイント付きReinhardトーンマップ
 * @details 通常のReinhardと違い、最も明るい部分をきちんと白まで持ち上げられる
 */
float4 PSToneMapReinhardExtended(PSInput In) : SV_Target0
{
    float3 c = SampleExposedColor(In.uv);
    float  l = GetLuminance(c);
    return ToOutputColor(SelectToneMapResult(c, CurveReinhardExtended(c), CurveReinhardExtended(l.xxx).x));
}

/////////////////////////////////////////////////////////
// ACESフィルミック
/////////////////////////////////////////////////////////
/*!
 * @brief ACESフィルミックトーンマップ
 */
float4 PSToneMapACES(PSInput In) : SV_Target0
{
    float3 c = SampleExposedColor(In.uv);
    float  l = GetLuminance(c);
    return ToOutputColor(SelectToneMapResult(c, CurveACES(c), CurveACES(l.xxx).x));
}

/////////////////////////////////////////////////////////
// Uncharted2フィルミック
/////////////////////////////////////////////////////////
/*!
 * @brief Uncharted2フィルミックトーンマップ
 * @details 暗部が締まりコントラストが出る
 */
float4 PSToneMapUncharted2(PSInput In) : SV_Target0
{
    float3 c = SampleExposedColor(In.uv);
    float  l = GetLuminance(c);
    return ToOutputColor(SelectToneMapResult(c, CurveUncharted2(c), CurveUncharted2(l.xxx).x));
}
