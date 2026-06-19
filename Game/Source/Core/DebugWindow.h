/**
 * @file DebugWindow.h
 * @brief ImGuiデバッグウィンドウの一元管理クラス
 * @details 各クラスから描画関数を登録することで、
 *          1つのウィンドウに全員のデバッグUIをまとめて表示する
 */
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <algorithm>
#include "imgui.h"

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
        //  描画（毎フレーム Application::Render から呼ぶ）
        // -----------------------------------------------

        /**
         * @brief デバッグウィンドウを描画する
         * @details Application::Render() の先頭で呼ぶこと
         */
        void Render()
        {
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
                    // セクション名をクリックすると開閉できる
                    if (ImGui::CollapsingHeader(entry.label.c_str()))
                    {
                        ImGui::Indent();     // 少し右にずらして見やすく
                        entry.func();        // ← ここで各自のWidgetが呼ばれる
                        ImGui::Unindent();
                    }
                }
            }

            ImGui::End();
        }

    private:
        DebugWindow() = default;

        struct DebugEntry
        {
            std::string label;
            DebugFunc   func;
        };

        std::vector<DebugEntry> m_entries;
    };

} // namespace app
