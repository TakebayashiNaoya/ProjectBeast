/*
 * @file CircleGauge.fx
 * @brief 円形ゲージシェーダー
 */


/** 
 * @brief 座標変換と色の乗算に使う2つの値
 * 		  エンジン共通(b0)
 */
cbuffer cb : register(b0){
	float4x4 mvp;		// モデル・ビュー・プロジェクション行列
	float4 mulColor;	// 乗算カラー(RGMA)
};


/** 
 * @brief ゲージ専用パラメーター(b1)
 *        マイフレームC++から書き換える値
 *        paddingがないと値がずれ、正しく動かない為
 *        padding0~3で空きを埋めて、16バイトに境界をそろえる
 */
cbuffer Gaugecb : register(b1)
{
	float g_startProgress; // 開始位置(0.0 ~ 1.0)
	float g_endProgress;   // 終了位置(0.0 ~ 1.0)
	float g_innerRadius;   // 内径(UV座標)
	float g_outerRadius;   // 外径(UV座標)
	float g_rotationAngle; // 回転オフセット(ラジアン)
	float g_padding0;      // アライメント用(空き)
	float g_padding1;      // アライメント用(空き)
	float g_padding2;      // アライメント用(空き)
	float4 g_gaugeColor;   // ゲージの色
	float4 g_bgColor;      // 背景の色 RGBA
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
	psIn.pos = mul( mvp, In.pos ); // 座標変換(mul:行列とベクトルの掛け算を行う関数)
	psIn.uv = In.uv;               // UVはそのまま
	return psIn;
}


float4 PSMain( PSInput In ) : SV_Target0
{
	// 円周率(一応下7桁のみ)
	static const float PI = 3.141592625f;
	/** 
	 * @brief アンチエイリアスの幅(ほんの少しぼかす)
	 *        エイリアシングと呼ばれるピクセル間で起こるギザギザ(ジャギー)が起こらない様に滑らかにする処理のこと
	 */
	static const float AA = 0.005f;


	// UVを中心原点(-0.5 ~ 0.5)に変換
	float2 uv = In.uv - float2( 0.5f, 0.5f);
	// length(uv)(中心からの距離を計算)
	float r = length(uv);
	// r < innerRadius → 穴の中(描画しない)
	// innerRadius <= r <= outRadius → 円環(ゲージを対象としている)
	// r > outRadius → 外側(描画しない)


	// stemp()を使った場合(ギザギザ)
	// 境界がカクカクしてジャーぎーが出る
	// smoothstep()を使った場合(なめらか) → 円環マスクと呼ばれるもの
	// 境界が0.005fピクセル幅でぼけてキレイになる
	float ringMask = smoothstep(g_innerRadius - AA, g_innerRadius + AA, r)  // 内側エッジ
				   * smoothstep(g_outerRadius + AA, g_outerRadius - AA, r); // 外側エッジ
	
	// 円環の完全外側は破棄
	// discardなし 65,536ピクセル全部で角度計算を実行してしまう
	// discardあり 円環部分のみ(計算が半分で済む)
	if(ringMask <= 0.0f)
	{
		/**
		  * @brief discard: そのピクセルの処理を即座に終了する
		  *                 色は出力されずに「透明」になる
		  *				    外側のピクセルを捨てることで高速化できる
		  */
		discard;
	}
	
	// ピクセルの角度を求める。
	// atan2(y,x)角度(ラジアン)を求める関数。
	// 0時が基準。結果は-π ～ πの範囲。-3.14~3.14の数字で返ってくる。
	float angle = atan2( uv.x, uv.y) - g_rotationAngle;

	// frac()角度を0 ~ 1に正規化する
	// ① angle / (2π) → -0.5f~+0.5fに圧縮(1周 = 1.0)
	// ② + 0.5f → 0.0 ~ 1.0fにシフト
	// ③ frac() → 確実に0 ~ 1の範囲に折り返す
	float normalizedAngle = frac((angle / (2.0f * PI)) + 0.5f);
	
	float gaugeMask;

	// step()範囲内か判定する
	if(g_startProgress <= g_endProgress)
	{
		// 通常ケース(AND判定)
		// 今回は折り返しは必要ないのでAND判定を行う。
		// ※折り返しの場合はOR判定を行う。
		gaugeMask = step(g_startProgress, normalizedAngle) // start ~ 1.0 の部分
				  * step(normalizedAngle, g_endProgress);  // 0.0 ~ 1.0 の部分
	}
	else
	{
		// 折り返しケース。(OR判定)
		gaugeMask=max(
			step(g_startProgress,normalizedAngle),
			step(normalizedAngle,g_endProgress)
		);
	}

	// 色を選んで最終出力を行う
	// ゲージ色か背景色かを最後に決める
	float4 color = lerp(g_gaugeColor,g_bgColor,gaugeMask);
	// gaugeMask = 0 → g_bgColor(背景色)
	// gaugeMask = 1 → g_gaugeColor(ゲージ色)
	color.a *= ringMask; // 円環エッジをフェードアウト(アンチエイリアス)

	return color * mulColor; // 最終出力(乗算カラーを掛けて返す)
}