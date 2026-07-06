/**
 * @file DebugWindow.h
 * @brief ImGuiデバッグウィンドウの一元管理クラス
 * @details 各クラスから描画関数を登録することで、
 *          1つのウィンドウに全員のデバッグUIをまとめて表示する
 * @author 立山
 */
#pragma once
#include "imgui.h"
#include <algorithm>
#include <any>
#include <cassert>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>


namespace app
{
	/**
	 * @brief ImGuiデバッグウィンドウのマネージャー（シングルトン）
	 *
	 * 【使い方】
	 *  ① 自分のクラスのコンストラクタ（またはInit）で Register() を呼ぶ
	 *  ② ラムダ式の中に、表示したいImGuiウィジェットを書く
	 *  ③ デストラクタで Unregister() を呼ぶ（オブジェクトが破棄されても安全）
	 *
	 * 【例】
	 *  // Player.cpp
	 *  Player::Player()
	 *  {
	 *      app::DebugWindow::Get().Register("プレイヤー", [this]()
	 *      {
	 *          ImGui::SliderFloat("移動速度", &m_moveSpeed, 0.0f, 500.0f);
	 *          ImGui::DragFloat3("座標",     &m_position.x);
	 *          ImGui::Text("HP: %d",          m_hp);
	 *      });
	 *  }
	 *
	 *  Player::~Player()
	 *  {
	 *      app::DebugWindow::Get().Unregister("プレイヤー");
	 *  }
	 */
	class DebugWindow
	{
	public:
		/** デバッグ描画関数の型 */
		using DebugFunc = std::function<void()>;

		// -----------------------------------------------
		//  シングルトンアクセス
		// -----------------------------------------------
		static DebugWindow& Get()
		{
			static DebugWindow instance;
			return instance;
		}

		// -----------------------------------------------
		//  登録 / 解除
		// -----------------------------------------------

		/**
		 * @brief デバッグ表示の登録
		 * @param label セクション名（CollapsingHeaderのタイトルになる）
		 * @param func  ImGuiウィジェットを書いたラムダ式
		 * @note  同じlabelを複数回登録しないよう注意する
		 */
		void Register(const std::string& label, DebugFunc func)
		{
			m_entries.push_back({ label, std::move(func) });
		}

		/**
		 * @brief デバッグ表示の登録解除
		 * @param label 解除するセクション名
		 * @note  オブジェクトのデストラクタで呼ぶとメモリ安全
		 */
		void Unregister(const std::string& label)
		{
			auto it = std::remove_if(
				m_entries.begin(), m_entries.end(),
				[&](const DebugEntry& e) { return e.label == label; }
			);
			m_entries.erase(it, m_entries.end());
		}

		/**
		 * @brief 全登録をクリアする
		 * @note  シーン遷移時などにSceneManagerから呼ぶと便利
		 */
		void Clear()
		{
			m_entries.clear();
		}

		// -----------------------------------------------
		//  値の固定（Override）機能
		// -----------------------------------------------

		/**
		 * @brief 「固定」チェックボックス付きウィジェット
		 * @details チェックをONにすると値が固定され、ゲーム側がリセットしても維持される
		 *          チェックOFF時はゲームの現在値をリアルタイム表示する（操作不可）
		 * @tparam T            固定・表示する値の型
		 * @param key           一意なキー（"クラス名/変数名" の形式を推奨）
		 * @param currentValue  現在のゲーム側の値（固定OFF時に表示に使う）
		 * @param drawWidget    値を描画するImGuiウィジェット関数
		 * @note ラムダ式の中で呼ぶ。対応する TryGetOverride() を Update() で呼ぶこと
		 */
		template<typename T>
		void Override(const std::string& key, const T& currentValue,
			const std::function<void(T&)>& drawWidget)
		{
			auto& entry = GetOrAddEntry<T>(key);

			// 「固定」チェックボックス（ON=値をロック、OFF=ゲーム値をリアルタイム表示）
			std::string checkId = std::string(u8"固定##") + key;
			ImGui::Checkbox(checkId.c_str(), &entry.enabled);
			ImGui::SameLine();

			if (entry.enabled)
			{
				// 固定ON：ウィジェットで自由に操作できる
				drawWidget(entry.value);
			}
			else
			{
				// 固定OFF：ゲームの現在値を表示するだけ（グレーアウトで操作不可）
				entry.value = currentValue;
				ImGui::BeginDisabled();
				drawWidget(entry.value);
				ImGui::EndDisabled();
			}
		}


		/**
		 * @brief 固定されている値を取得する
		 * @tparam T       値の型
		 * @param key      取得するエントリのキー
		 * @param outValue 取得した値の出力先
		 * @return 固定がONで値を取得できた場合 true、そうでない場合 false
		 */
		template<typename T>
		bool TryGetOverride(const std::string& key, T& outValue) const
		{
			auto it = m_overrides.find(key);
			if (it == m_overrides.end())
			{
				return false;
			}

			const auto* entry = std::any_cast<OverrideEntry<T>>(&it->second);
			// nullptrになるのは同じkeyを別の型で使ってしまった場合（呼び出し側のバグ）
			assert(entry != nullptr && u8"DebugWindow::TryGetOverride : keyに対応する型が一致しません");
			if (!entry || !entry->enabled)
			{
				return false;
			}

			outValue = entry->value;
			return true;
		}


		// -----------------------------------------------
		//  描画（毎フレーム Application::Render から呼ぶ）
		// -----------------------------------------------
		/**
		 * @brief デバッグウィンドウの表示/非表示を切り替えるメニュー項目を描画する
		 * @details プロジェクトに共通のメインメニューバーがすでにある場合は、
		 *          そのBeginMainMenuBar()/EndMainMenuBar()の中からこの関数だけ呼べばよい
		 */
		void DrawMenuItems()
		{
			if (ImGui::BeginMenu(u8"デバッグ"))
			{
				ImGui::MenuItem(u8"ウィンドウを表示", nullptr, &m_visible);
				ImGui::EndMenu();
			}
		}


		/**
		 * @brief デバッグウィンドウを描画する
		 * @details Application::Render() の先頭で呼ぶこと
		 *          マウスの中クリックでウィンドウの表示/非表示を切り替えられる
		 */
		void Render()
		{
			// ── 中クリックで表示/非表示を切り替え ──────────────
			if (ImGui::GetIO().MouseClicked[ImGuiMouseButton_Middle])
			{
				m_visible = !m_visible;
			}

			// チェックが外れているときはウィンドウ本体を描画しない
			if (!m_visible)
			{
				return;
			}

			// ── 常時表示する小さいメニューバー ──────────────────────
			if (ImGui::BeginMainMenuBar())
			{
				DrawMenuItems();
				ImGui::EndMainMenuBar();
			}

			// ── 初期位置・サイズ（左上）──────────────────────
			// ImGuiCond_FirstUseEver : imgui.ini に保存記録が無い「本当に初回」のときだけ適用される。
			// 一度でもユーザーが動かす／リサイズすると、以後は imgui.ini 側の値が優先される。
			ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(600.0f, 500.0f), ImGuiCond_FirstUseEver);

			ImGui::Begin(u8"デバッグ");

			// ── 共通情報（常に表示）──────────────────────
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Separator();

			// ── 各自が登録したセクション ─────────────────
			if (m_entries.empty())
			{
				ImGui::TextDisabled(u8"（登録なし）");
			}
			else
			{
				for (auto& entry : m_entries)
				{
					if (ImGui::CollapsingHeader(entry.label.c_str()))
					{
						ImGui::Indent();
						entry.func();
						ImGui::Unindent();
					}
				}
			}

			ImGui::End();
		}


	private:
		/** @brief シングルトンのためコンストラクタを非公開とする */
		DebugWindow() = default;


		/** @brief デバッグウィンドウの1エントリ（セクション名と描画関数のペア） */
		struct DebugEntry
		{
			std::string label;
			DebugFunc   func;
		};

		std::vector<DebugEntry> m_entries;

		bool m_visible = true;

		/**
		 * @brief 固定（Override）機能用エントリ
		 * @tparam T 固定する値の型
		 */
		template<typename T>
		struct OverrideEntry
		{
			T    value{};          // 固定する値
			bool enabled = false;  // 固定ON/OFF
		};

		// キーごとに型が異なるエントリを1つのmapにまとめて保持するため std::any を使う
		std::unordered_map<std::string, std::any> m_overrides;


		/**
		 * @brief キーに対応する OverrideEntry を取得または新規生成する
		 * @tparam T  エントリの値の型
		 * @param key エントリのキー
		 * @return 該当する OverrideEntry の参照
		 */
		template<typename T>
		OverrideEntry<T>& GetOrAddEntry(const std::string& key)
		{
			auto it = m_overrides.find(key);
			if (it == m_overrides.end())
			{
				it = m_overrides.emplace(key, OverrideEntry<T>{}).first;
			}
			return *std::any_cast<OverrideEntry<T>>(&it->second);
		}
	};

} // namespace app
