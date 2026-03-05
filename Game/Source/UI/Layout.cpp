/**
 * @file Layout.cpp
 * @brief UIのレイアウト管理
 * @author 忽那
 */
#include "stdafx.h"
#include "Layout.h"
#include <fstream>
#include <sys/stat.h>
#include <Windows.h>
#include "Source/Util/CRC32.h"

namespace
{
	/** カラーに使う用の値 */
	constexpr float COLOR_VALUE = 255.0f;


	/**
	 * @brief UTF-8(JSONの文字列)をShift-JIS(Windowsアプリ用)に変換する
	 */
	std::wstring Utf8ToShiftJis(const std::string& utf8Str)
	{
		if (utf8Str.empty())return std::wstring();
		/** @brief UTF-8をUnicode(UTF-16)に変換*/
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}
}



/**
 * @brief パース関連
 */
Vector3 ParseVector3(const nlohmann::json& arr)
{
	return Vector3(
		arr[0].get<float>(),
		arr[1].get<float>(),
		arr[2].get<float>()
	);
}


Vector4 ParseVector4(const nlohmann::json& arr)
{
	return Vector4(
		arr[0].get<float>(),
		arr[1].get<float>(),
		arr[2].get<float>(),
		arr[3].get<float>()
	);
}


Vector4 ParseColor(const nlohmann::json& arr)
{
	return Vector4(
		arr[0].get<float>() / COLOR_VALUE,
		arr[1].get<float>() / COLOR_VALUE,
		arr[2].get<float>() / COLOR_VALUE,
		arr[3].get<float>() / COLOR_VALUE
	);
}


Quaternion ParseRotation(const float rotation)
{
	Quaternion q;
	q.SetRotationDegZ(rotation);
	return q;
}


/**
 * @brief 初期化関連
 */
template<typename T>
void InitializeUIParts(T* parts, const nlohmann::json& item)
{
	K2_ASSERT(false, "未実装\n");
}


void InitializeUIParts(app::ui::UIIcon* image, const nlohmann::json& item)
{
	const std::string assetName = item["asset"].get<std::string>();
	const float w = item["width"].get<float>();
	const float h = item["height"].get<float>();
	const Vector3 position = ParseVector3(item["position"]);
	const Vector3 scale = ParseVector3(item["scale"]);
	const Quaternion rotation = ParseRotation(item["rotation"].get<float>());
	const Vector4 color = ParseColor(item["color"]);


	image->Initialize(assetName.c_str(), w, h);
	image->m_transform.m_localTransform.m_position = position;
	image->m_transform.m_localTransform.m_scale = scale;
	image->m_transform.m_localTransform.m_rotation = rotation;
	image->m_color = color;
}


namespace app
{
	namespace ui
	{
		void Layout::Update()
		{
			m_menu->Update();

#ifdef APP_ENABLE_LAYOUT_HOTRELOAD
			/** ホットリロードチェック */
			struct stat st;
			if (stat(m_filePath.c_str(), &st) == 0)
			{
				if (m_lastUpdateTime != st.st_mtime)
				{
					m_lastUpdateTime = st.st_mtime;
					Reload();
				}
			}
#endif //APP_ENABLE_LAYOUT_HOTRELOAD
		}


		void Layout::Render(RenderContext& rc)
		{
			m_menu->Render(rc);
		}


		void Layout::Reload()
		{
			std::ifstream file(m_filePath);
			if (!file.is_open())return;
			
			nlohmann::json j;
			file >> j;

			/** すでにMenuやCanvasがある場合は作り直しを行う */
			if (m_menu->GetCanvas() == nullptr)
			{
				m_menu->SetCanvas(new UICanvas());
			}

			auto* canvas = m_menu->GetCanvas();
			auto& elements = j["canvas"]["elements"];

			for (auto& item : elements)
			{
				std::string type = item["type"];
				std::string name = item["name"];

				/** すでに存在するUIならパラメーター更新のみ */
				const uint32_t key = Hash32(name.c_str());
				if (m_menu->HasUI(key))
				{
					m_menu->UnregisterUI(key);
					canvas->RemoveUI(key);
				}
				auto* ui = CreateUI(canvas, type, key, item);
				m_menu->RegisterUI(key, ui);
			}
			m_menu->IntializeLogic();
		}


		UIBase* Layout::CreateUI(UICanvas* canvas, const std::string& type, const uint32_t key, nlohmann::json& item)
		{
			if (type == "UIIcon")
			{
				canvas->CreateUI<UIIcon>(key);
				auto* image = canvas->FindUI<UIIcon>(key);
				InitializeUIParts(image, key);
				return image;
			}
			return nullptr;
		}
	}
}