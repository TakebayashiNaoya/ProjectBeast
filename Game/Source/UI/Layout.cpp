/**
 * @file Layout.cpp
 * @brief UIのレイアウト管理
 * @author 忽那
 */
#include "stdafx.h"
#include "Layout.h"

#include "Source/Util/CRC32.h"

#include <fstream>
#include <sys/stat.h>
#include <Windows.h>


namespace
{
	/**
	 * @brief UTF-8(JSONの文字列)をShift-JIS(Windowsアプリ用)に変換する
	 */
	std::wstring Utf8ToShiftJis(const std::string& utf8Str)
	{
		if (utf8Str.empty())return std::wstring();
		/** @brief UTF-8をUnicode(UTF-16)に変換*/
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], static_cast<int>(utf8Str.size()), NULL, 0);
		std::wstring wstrTo(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &utf8Str[0], (int)utf8Str.size(), &wstrTo[0], size_needed);
		return wstrTo;
	}
}



/**
 * @brief パース関連
 */
Quaternion ParseRotation(const float rotation)
{
	// 回転が無効な値の場合は単位クォータニオン初期値を返す
	if (fabsf(rotation - app::util::JsonConverter::InvalidFloat) < FLT_EPSILON)
	{
		return Quaternion::Identity;
	}

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


void InitializeUIParts(app::ui::UICircleGauge* gauge, const nlohmann::json& item)
{
	const std::string asset = app::util::JsonConverter::ToString(item, "asset");
	const std::string fx = app::util::JsonConverter::ToString(item, "fx");
	const float w = app::util::JsonConverter::ToFloat(item, "width");
	const float h = app::util::JsonConverter::ToFloat(item, "height");
	const Vector3 position = app::util::JsonConverter::ToVector3(item, "position");
	const Vector3 scale = app::util::JsonConverter::ToVector3(item, "scale");
	const Quaternion rotation = ParseRotation(app::util::JsonConverter::ToFloat(item, "rotation"));
	const Vector2 pivot = app::util::JsonConverter::ToVector2(item, "pivot");
	const Vector4 gaugeColor = app::util::JsonConverter::ToVector4(item, "gaugeColor");
	const Vector4 bgColor = app::util::JsonConverter::ToVector4(item, "bgColor");
	const float innerRadius = app::util::JsonConverter::ToFloat(item, "innerRadius");
	const float outerRadius = app::util::JsonConverter::ToFloat(item, "outerRadius");


	gauge->Initialize(asset.c_str(), fx.c_str(), w, h, position, scale, rotation, pivot, gaugeColor, bgColor, innerRadius, outerRadius);
	gauge->m_transform.m_localTransform.m_position = position;
	gauge->m_transform.m_localTransform.m_scale = scale;
	gauge->m_transform.m_localTransform.m_rotation = rotation;
	gauge->m_color = gaugeColor;
	gauge->m_pivot = pivot;
}


void InitializeUIParts(app::ui::UIIcon* image, const nlohmann::json& item)
{
	const std::string assetName = app::util::JsonConverter::ToString(item, "asset");
	const float w = app::util::JsonConverter::ToFloat(item, "width");
	const float h = app::util::JsonConverter::ToFloat(item, "height");
	const Vector3 position = app::util::JsonConverter::ToVector3(item, "position");
	const Vector3 scale = app::util::JsonConverter::ToVector3(item, "scale");
	const Quaternion rotation = ParseRotation(app::util::JsonConverter::ToFloat(item, "rotation"));
	const Vector4 color = app::util::JsonConverter::ToVector4(item, "color");


	image->Initialize(assetName.c_str(), w, h, position, scale, rotation, color);
	image->m_transform.m_localTransform.m_position = position;
	image->m_transform.m_localTransform.m_scale = scale;
	image->m_transform.m_localTransform.m_rotation = rotation;
	image->m_color = color;
}


void InitializeUIParts(app::ui::UIGauge* gauge, const nlohmann::json& item)
{
	const std::string asset = app::util::JsonConverter::ToString(item, "asset");
	const float w = app::util::JsonConverter::ToFloat(item, "width");
	const float h = app::util::JsonConverter::ToFloat(item, "height");
	const Vector3 position = app::util::JsonConverter::ToVector3(item, "position");
	const Vector3 scale = app::util::JsonConverter::ToVector3(item, "scale");
	const Quaternion rotation = ParseRotation(app::util::JsonConverter::ToFloat(item, "rotation"));
	const Vector4 color = app::util::JsonConverter::ToVector4(item, "color");
	const Vector2 pivot = app::util::JsonConverter::ToVector2(item, "pivot");


	gauge->Initialize(asset.c_str(), w, h, position, scale, rotation, color, pivot);
	gauge->m_transform.m_localTransform.m_position = position;
	gauge->m_transform.m_localTransform.m_scale = scale;
	gauge->m_transform.m_localTransform.m_rotation = rotation;
	gauge->m_color = color;
	gauge->m_pivot = pivot;
}


void InitializeUIParts(app::ui::UIButton* button, const nlohmann::json& item)
{
	const std::string asset = app::util::JsonConverter::ToString(item, "asset");
	const float w = app::util::JsonConverter::ToFloat(item, "width");
	const float h = app::util::JsonConverter::ToFloat(item, "height");
	const Vector3 position = app::util::JsonConverter::ToVector3(item, "position");
	const Vector3 scale = app::util::JsonConverter::ToVector3(item, "scale");
	const Quaternion rotation = ParseRotation(app::util::JsonConverter::ToFloat(item, "rotation"));
	const Vector4 color = app::util::JsonConverter::ToVector4(item, "color");


	button->Initialize(asset.c_str(), w, h, position, scale, rotation, color);
	button->m_transform.m_localTransform.m_position = position;
	button->m_transform.m_localTransform.m_scale = scale;
	button->m_transform.m_localTransform.m_rotation = rotation;
	button->m_color = color;
}


void InitializeUIParts(app::ui::UIText* text, const nlohmann::json& item)
{
	const std::string content    = app::util::JsonConverter::ToString(item, "text");
	const float       fontSize   = app::util::JsonConverter::ToFloat(item, "fontSize",   1.0f);
	const float       fontScaleX = app::util::JsonConverter::ToFloat(item, "fontScaleX", 1.0f) * fontSize;
	const float       fontScaleY = app::util::JsonConverter::ToFloat(item, "fontScaleY", 1.0f) * fontSize;
	const Vector3     position   = app::util::JsonConverter::ToVector3(item, "position");
	const Vector3     scale      = app::util::JsonConverter::ToVector3(item, "scale", false, Vector3::One);
	const Quaternion  rotation   = ParseRotation(app::util::JsonConverter::ToFloat(item, "rotation", app::util::JsonConverter::InvalidFloat));
	const Vector4     color      = app::util::JsonConverter::ToVector4(item, "color");
	const Vector2     pivot      = app::util::JsonConverter::ToVector2(item, "pivot");

	text->SetText(content);
	text->SetScale(Vector2(fontScaleX, fontScaleY));
	text->m_transform.m_localTransform.m_position = position;
	text->m_transform.m_localTransform.m_scale = scale;
	text->m_transform.m_localTransform.m_rotation = rotation;
	text->m_color = color;
	text->m_pivot = pivot;
}


void InitializeUIParts(app::ui::UIVideo* video, const nlohmann::json& item)
{
	app::ui::UIVideoInitData data;
	data.clipPath = app::util::JsonConverter::ToString(item, "clipPath");
	data.width = app::util::JsonConverter::ToFloat(item, "width", 1920.0f);
	data.height = app::util::JsonConverter::ToFloat(item, "height", 1080.0f);
	data.fps = app::util::JsonConverter::ToFloat(item, "fps", 24.0f);
	data.loop = app::util::JsonConverter::ToBool(item, "loop", false);
	data.autoPlay = app::util::JsonConverter::ToBool(item, "autoPlay", true);

	const Vector3    position = app::util::JsonConverter::ToVector3(item, "position", false, Vector3::Zero);
	const Vector3    scale    = app::util::JsonConverter::ToVector3(item, "scale",    false, Vector3::One);
	const Quaternion rotation = ParseRotation(app::util::JsonConverter::ToFloat(item, "rotation", app::util::JsonConverter::InvalidFloat));
	const Vector4    color    = app::util::JsonConverter::ToVector4(item, "color", false, Vector4::White);

	video->Initialize(data);
	video->m_transform.m_localTransform.m_position = position;
	video->m_transform.m_localTransform.m_scale = scale;
	video->m_transform.m_localTransform.m_rotation = rotation;
	video->m_color = color;
}


void InitializeUIParts(app::ui::UIDigit* digit, const nlohmann::json& item)
{
	const std::string asset = app::util::JsonConverter::ToString(item, "asset");
	const int digitNumber = app::util::JsonConverter::ToInt(item, "digit");
	const int number = app::util::JsonConverter::ToInt(item, "number");
	const float w = app::util::JsonConverter::ToFloat(item, "width");
	const float h = app::util::JsonConverter::ToFloat(item, "height");
	const Vector3 position = app::util::JsonConverter::ToVector3(item, "position");
	const Vector3 scale = app::util::JsonConverter::ToVector3(item, "scale");
	const Quaternion rotation = ParseRotation(app::util::JsonConverter::ToFloat(item, "rotation"));


	digit->Initialize(asset.c_str(), digitNumber, number, w, h, position, scale, rotation);
	digit->SetNumber(number);
	digit->m_transform.m_localTransform.m_position = position;
	digit->m_transform.m_localTransform.m_scale = scale;
	digit->m_transform.m_localTransform.m_rotation = rotation;
	digit->m_color = Vector4::White;
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
			if (app::util::JsonConverter::CheckFileModified(m_filePath, m_lastUpdateTime))
			{
				// 更新日時を最新に上書きしてからReload
				m_lastUpdateTime = app::util::JsonConverter::GetFileLastWriteTime(m_filePath.c_str());
				Reload();
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
			m_menu->InitializeLogic();
		}


		UIBase* Layout::CreateUI(UICanvas* canvas, const std::string& type, const uint32_t key, nlohmann::json& item)
		{
			if (type == "UIIcon")
			{
				canvas->CreateUI<UIIcon>(key);
				auto* image = canvas->FindUI<UIIcon>(key);
				InitializeUIParts(image, item);
				return image;
			}
			if (type == "UIGauge")
			{
				canvas->CreateUI<UIGauge>(key);
				auto* gauge = canvas->FindUI<UIGauge>(key);
				InitializeUIParts(gauge, item);
				return gauge;
			}
			if (type == "UIButton")
			{
				canvas->CreateUI<UIButton>(key);
				auto* button = canvas->FindUI<UIButton>(key);
				InitializeUIParts(button, item);
				return button;
			}
			if (type == "UIDigit")
			{
				canvas->CreateUI<UIDigit>(key);
				auto* digit = canvas->FindUI<UIDigit>(key);
				InitializeUIParts(digit, item);
				return digit;
			}
			if (type == "UICircleGauge")
			{
				canvas->CreateUI<UICircleGauge>(key);
				auto* cirGauge = canvas->FindUI<UICircleGauge>(key);
				InitializeUIParts(cirGauge, item);
				return cirGauge;
			}
			if (type == "UIVideo")
			{
				canvas->CreateUI<UIVideo>(key);
				auto* video = canvas->FindUI<UIVideo>(key);
				InitializeUIParts(video, item);
				return video;
			}
			if (type == "UIText")
			{
				canvas->CreateUI<UIText>(key);
				auto* text = canvas->FindUI<UIText>(key);
				InitializeUIParts(text, item);
				return text;
			}
			return nullptr;
		}
	}
}