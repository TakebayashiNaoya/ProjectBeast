#pragma once
#include <string>
#include <unordered_map>
#include <map>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cstdio>

namespace nsK2EngineLow {
	/// <summary>
	/// 【計測用】GPU専用メモリ（VRAM）を使うリソースの台帳。
	/// テクスチャ・レンダリングターゲットを作ったときにサイズと名前を記録し、VRAMの内訳を書き出す。
	/// 頂点・インデックス・定数・ストラクチャバッファは UPLOAD ヒープ（共有メモリ）なので対象外。
	/// Effekseer や DirectXTK などが内部で作るものは記録されない（DXGIの合計との差＝台帳外として出す）。
	/// </summary>
	class GpuMemoryLedger {
	public:
		/// <summary>
		/// リソースを記録する。
		/// </summary>
		/// <param name="resource">リソース</param>
		/// <param name="category">種類（"Texture" など）</param>
		/// <param name="name">名前（ファイル名など）</param>
		void Add(ID3D12Resource* resource, const char* category, const std::string& name)
		{
			if (resource == nullptr) return;
			const D3D12_RESOURCE_DESC desc = resource->GetDesc();
			ID3D12Device* device = nullptr;
			if (FAILED(resource->GetDevice(IID_PPV_ARGS(&device))) || device == nullptr) return;
			const D3D12_RESOURCE_ALLOCATION_INFO info = device->GetResourceAllocationInfo(0, 1, &desc);
			device->Release();

			std::lock_guard<std::mutex> lock(m_mutex);
			m_entries[resource] = Entry{ category, name, info.SizeInBytes };
		}
		/// <summary>
		/// 記録を消す（リソースを解放するとき）。
		/// </summary>
		void Remove(ID3D12Resource* resource)
		{
			if (resource == nullptr) return;
			std::lock_guard<std::mutex> lock(m_mutex);
			m_entries.erase(resource);
		}
		/// <summary>
		/// 内訳を Logs/vram_breakdown.txt に追記する。
		/// </summary>
		/// <param name="tag">書き出すきっかけ</param>
		/// <param name="dxgiLocalUsageMB">DXGIで調べたGPU専用メモリの使用量（台帳外の量を出すため）</param>
		void WriteReport(const char* tag, const double dxgiLocalUsageMB)
		{
			constexpr double BYTES_PER_MB = 1024.0 * 1024.0;
			constexpr size_t MAX_NAMES = 60;

			struct Total { uint64_t bytes = 0; int count = 0; };
			std::map<std::string, Total> byCategory;
			std::map<std::string, Total> byName;
			uint64_t totalBytes = 0;
			{
				std::lock_guard<std::mutex> lock(m_mutex);
				for (const auto& pair : m_entries) {
					const Entry& entry = pair.second;
					totalBytes += entry.bytes;
					byCategory[entry.category].bytes += entry.bytes;
					byCategory[entry.category].count++;
					Total& total = byName[entry.category + " " + entry.name];
					total.bytes += entry.bytes;
					total.count++;
				}
			}
			std::vector<std::pair<std::string, Total>> names(byName.begin(), byName.end());
			std::sort(names.begin(), names.end(),
				[](const auto& a, const auto& b) { return a.second.bytes > b.second.bytes; });

			CreateDirectoryA("Logs", nullptr);
			FILE* fp = nullptr;
			if (fopen_s(&fp, "Logs/vram_breakdown.txt", "a") != 0 || fp == nullptr) return;
			SYSTEMTIME time;
			GetLocalTime(&time);
			fprintf(fp, "==== %04d-%02d-%02d %02d:%02d:%02d [%s] DXGI local %.0f MB / tracked %.0f MB / untracked %.0f MB\n",
				time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, (tag != nullptr) ? tag : "",
				dxgiLocalUsageMB, totalBytes / BYTES_PER_MB, dxgiLocalUsageMB - totalBytes / BYTES_PER_MB);
			for (const auto& pair : byCategory) {
				fprintf(fp, "  [category] %-16s %8.1f MB (%d)\n", pair.first.c_str(), pair.second.bytes / BYTES_PER_MB, pair.second.count);
			}
			for (size_t i = 0; i < names.size() && i < MAX_NAMES; ++i) {
				fprintf(fp, "  %8.1f MB x%-4d %s\n", names[i].second.bytes / BYTES_PER_MB, names[i].second.count, names[i].first.c_str());
			}
			fclose(fp);
		}

	private:
		struct Entry {
			std::string category;
			std::string name;
			uint64_t bytes = 0;
		};
		std::unordered_map<ID3D12Resource*, Entry> m_entries;
		std::mutex m_mutex;
	};

	/// <summary>
	/// 【計測用】台帳の取得（ヘッダーだけで使えるよう関数内の static にしている）。
	/// </summary>
	inline GpuMemoryLedger& GetGpuMemoryLedger()
	{
		static GpuMemoryLedger s_ledger;
		return s_ledger;
	}

	/// <summary>
	/// 【計測用】ワイド文字列のパスを台帳の名前用の文字列にする（ASCII以外は '?'）。
	/// </summary>
	inline std::string ToLedgerName(const wchar_t* path)
	{
		std::string name;
		if (path == nullptr) return name;
		for (const wchar_t* p = path; *p != L'\0'; ++p) {
			name += (*p < 0x80) ? static_cast<char>(*p) : '?';
		}
		return name;
	}
}
