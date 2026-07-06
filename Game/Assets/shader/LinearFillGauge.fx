/*
 * @file LinearFillGauge.fx
 * @brief 縦方向の塗り分けゲージシェーダー
 * @author 竹林
 */


/**
 * @brief 座標変換と色の乗算に使う2つの値
 * 		  エンジン共通(b0)
 */
cbuffer cb : register(b0){
	float4x4 mvp;		// モデル・ビュー・プロジェクション行列
	float4 mulColor;	// 乗算カラー(RGBA)
};


/**
 * @brief 直線ゲージ専用パラメーター(b1)
 *        毎フレームC++から書き換える値
 *        paddingがないと値がずれ、正しく動かない為
 *        padding0~2で空きを埋めて、16バイトに境界をそろえる
 */
cbuffer LinearFillCb : register(b1)
{
	float  g_fillAmount; // 塗りつぶし割合(0.0 ~ 1.0)。下から上へ塗られる
	float  g_padding0;    // アライメント用(空き)
	float  g_padding1;    // アライメント用(空き)
	float  g_padding2;    // アライメント用(空き)
	float4 g_baseColor;   // 未充填部分の色
	float4 g_fillColor;   // 充填部分の色
}


/** VSInput : シェーダーの受け取りを行う構造体 */
struct VSInput{
	float4 pos : POSITION;  // 頂点の3D座標(x,y,z,w)
	float2 uv  : TEXCOORD0; // UV座標(0~1の位置情報)
};


/** PSInput : シェーダーの値を渡す・受け取りを行う構造体 */
struct PSInput{
	float4 pos : SV_POSITION; // 変換済みの画面座標
	float2 uv  : TEXCOORD0;   // UV座標(そのまま)
};

/** カラーテクスチャ */
Texture2D<float4> colorTexture : register(t0);
sampler Sampler : register(s0);

/** 頂点シェーダー : 「どこに表示するか」を決めるもの */
PSInput VSMain(VSInput In)
{
	PSInput psIn;
	psIn.pos = mul( mvp, In.pos ); // 座標変換
	psIn.uv = In.uv;               // UVはそのまま
	return psIn;
}


float4 PSMain( PSInput In ) : SV_Target0
{
	/** アンチエイリアスの幅(境界を少しぼかす) */
	static const float AA = 0.02f;

	// テクスチャのアルファ(アイコンの形)をそのまま活かす。
	// スケールで引き伸ばさないので、アイコン自体は歪まない。
	float texAlpha = colorTexture.Sample(Sampler, In.uv).a;

	// UV.y は 0(画像の上端) ~ 1(画像の下端)。
	// 下端から g_fillAmount の割合だけ塗りつぶす。
	float fillLine = 1.0f - g_fillAmount;
	float fillMask = smoothstep(fillLine - AA, fillLine + AA, In.uv.y);

	// fillMask = 0 → g_baseColor(未充填) / fillMask = 1 → g_fillColor(充填済み)
	float4 tint = lerp(g_baseColor, g_fillColor, fillMask);

	float4 color = float4(tint.rgb, tint.a * texAlpha);
	return color * mulColor;
}
