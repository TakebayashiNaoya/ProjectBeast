#pragma once

namespace nsK2EngineLow {
	/// <summary>
	/// CPUが毎フレーム書き込むGPUリソース（定数バッファ・ストラクチャバッファ・ディスクリプタなど）の面数。
	/// 「フレーム（GPUが前のフレームを描いている間に次のフレームを書くための2面）」×
	/// 「ビュー（同じフレーム内でメインビューとサブビューが別の内容を書くための2面）」= 4面。
	/// どの面を使うかは GraphicsEngine::GetResourceSlot() で決まる（フレーム番号 × 2 + ビュー番号）。
	/// </summary>
	static const int NUM_GPU_RESOURCE_SLOTS = 4;
	/// <summary>
	/// 1フレームあたりのビューの数（メインビュー・サブビュー）。
	/// </summary>
	static const int NUM_GPU_RESOURCE_VIEWS = 2;
}
