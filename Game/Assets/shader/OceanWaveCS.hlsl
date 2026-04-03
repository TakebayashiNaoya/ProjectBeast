/*!
 * @brief 波高さ計算コンピュートシェーダー。
 * @details Ocean.fx の CalcWaveOffset() と同じ数式で
 *          グリッド全頂点の波面Yを RWStructuredBuffer に書き出す。
 *          スレッドグループ: (8, 8, 1)、ディスパッチ: (8, 8, 1) = 合計64x64スレッド
 */

////////////////////////////////////////////////
// 定数バッファ。
////////////////////////////////////////////////
cbuffer WaveCb : register(b0)
{
    float waveScroll;       // 波のスクロール値
    float wave1Amplitude;   // 波①の振幅
    float wave1Frequency;   // 波①の空間周波数
    float wave2Amplitude;   // 波②の振幅
    float wave2Frequency;   // 波②の空間周波数
    float gridHalfSize;     // グリッド半辺長（= GRID_SIZE / 2）
    float cellSize;         // セルサイズ（= GRID_SIZE / GRID_DIVISION）
    int   numVertsPerRow;   // 1行あたりの頂点数（= GRID_DIVISION + 1）
}

////////////////////////////////////////////////
// 出力バッファ。
// インデックス z * numVertsPerRow + x に波面Yを書き出す。
////////////////////////////////////////////////
RWStructuredBuffer<float> g_waveHeightOut : register(u0);

////////////////////////////////////////////////
// 波のオフセット計算。
// Ocean.fx の CalcWaveOffset() と同じ数式。
////////////////////////////////////////////////
float CalcWaveOffset(float worldX, float worldZ)
{
    // 波①：方向(1, 0)、speed倍率1.0
    float phase1  = worldX * wave1Frequency + waveScroll * 1.0;
    float offset1 = wave1Amplitude * sin(phase1);

    // 波②：方向(0.6, 0.8)、speed倍率1.7
    float phase2  = (worldX * 0.6 + worldZ * 0.8) * wave2Frequency + waveScroll * 1.7;
    float offset2 = wave2Amplitude * sin(phase2);

    return offset1 + offset2;
}

////////////////////////////////////////////////
// エントリーポイント。
// スレッドID(ix, iz) が担当する頂点のワールドXZを求め
// 波高さを計算してバッファに書き出す。
////////////////////////////////////////////////
[numthreads(8, 8, 1)]
void CSMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    const uint ix = dispatchThreadID.x;
    const uint iz = dispatchThreadID.y;

    // numVertsPerRow = GRID_DIVISION + 1 なので
    // ix / iz が GRID_DIVISION を超える場合は範囲外
    if (ix >= (uint)numVertsPerRow || iz >= (uint)numVertsPerRow)
    {
        return;
    }

    // 頂点のワールドXZ座標を算出する
    // CreateGridMesh() と同じ計算式
    const float worldX = -gridHalfSize + cellSize * (float)ix;
    const float worldZ = -gridHalfSize + cellSize * (float)iz;

    // 波高さを計算してバッファに書き出す
    const uint index             = iz * (uint)numVertsPerRow + ix;
    g_waveHeightOut[index]       = CalcWaveOffset(worldX, worldZ);
}
