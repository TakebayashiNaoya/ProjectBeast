/**
 * @file UIParts.cpp
 * @brief UIPartsのパーツ群
 * @author 忽那
 */
#include "stdafx.h"
#include "UIParts.h"

namespace app
{
	namespace ui
	{
		UIImage::UIImage()
		{}


		UIImage::~UIImage()
		{}


		void UIImage::Update()
		{}


		void UIImage::Render(RenderContext& rc)
		{}





		/***************************************/



		UIIcon::UIIcon()
		{}


		UIIcon::~UIIcon()
		{}


		void UIIcon::Update()
		{
			UpdateAnimation();

			m_spriteRender.SetMulColor(m_color);
			m_transform.UpdateTransform();
			m_spriteRender.SetPosition(m_transform.m_localTransform.m_position);
			m_spriteRender.SetScale(m_transform.m_localTransform.m_scale);
			m_spriteRender.SetRotation(m_transform.m_localTransform.m_rotation);
			m_spriteRender.Update();
		}


		void UIIcon::Render(RenderContext& rc)
		{
			if (m_isDraw)
			{
				m_spriteRender.Draw(rc);
			}
		}


		void UIIcon::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& color)
		{
			m_transform.m_localTransform.m_position = position;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rotation;
			m_color = color;

			m_spriteRender.Init(assetName, width, height);
			m_spriteRender.SetPosition(position);
			m_spriteRender.SetScale(scale);
			m_spriteRender.SetRotation(rotation);
			m_spriteRender.SetMulColor(color);
			m_spriteRender.Update();
		}





		/***************************************/


		UIButton::UIButton()
		{}


		UIButton::~UIButton()
		{}


		void UIButton::Update()
		{
			UpdateAnimation();

			m_spriteRender.SetMulColor(m_color);
			m_transform.UpdateTransform();

			m_spriteRender.SetPosition(m_transform.m_localTransform.m_position);
			m_spriteRender.SetScale(m_transform.m_localTransform.m_scale);
			m_spriteRender.SetRotation(m_transform.m_localTransform.m_rotation);
			m_spriteRender.Update();
		}


		void UIButton::Render(RenderContext& rc)
		{
			if (m_isDraw)
			{
				m_spriteRender.Draw(rc);
			}
		}


		void UIButton::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& color)
		{
			m_transform.m_localTransform.m_position = position;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rotation;
			m_color = color;


			m_spriteRender.Init(assetName, width, height);
			m_spriteRender.SetPosition(position);
			m_spriteRender.SetScale(scale);
			m_spriteRender.SetRotation(rotation);
			m_spriteRender.SetMulColor(color);
			m_spriteRender.Update();
		}





		/***************************************/


		UIGauge::UIGauge()
		{}


		UIGauge::~UIGauge()
		{}


		void UIGauge::Update()
		{
			UpdateAnimation();

			m_spriteRender.SetMulColor(m_color);
			m_transform.UpdateTransform();
			m_spriteRender.SetPosition(m_transform.m_localTransform.m_position);
			m_spriteRender.SetScale(m_transform.m_localTransform.m_scale);
			m_spriteRender.SetRotation(m_transform.m_localTransform.m_rotation);
			m_spriteRender.Update();
		}


		void UIGauge::Render(RenderContext& rc)
		{
			if (m_isDraw) {
				m_spriteRender.Draw(rc);
			}
		}


		void UIGauge::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& color, const Vector2& pivot)
		{
			m_transform.m_localTransform.m_position = position;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rotation;
			m_color = color;
			m_pivot = pivot;

			m_spriteRender.Init(assetName, width, height);
			m_spriteRender.SetPosition(position);
			m_spriteRender.SetScale(scale);
			m_spriteRender.SetRotation(rotation);
			m_spriteRender.SetMulColor(color);
			m_spriteRender.SetPivot(pivot);
			m_spriteRender.Update();
		}





		/********************************/


		UICircleGauge::UICircleGauge()
		{}


		UICircleGauge::~UICircleGauge()
		{}


		void UICircleGauge::Update()
		{
			UpdateAnimation();
			m_gaugeRender.SetGaugeColor(m_color);
			m_transform.UpdateTransform();
			m_gaugeRender.SetPosition(m_transform.m_localTransform.m_position);
			m_gaugeRender.SetScale(m_transform.m_localTransform.m_scale);
			m_gaugeRender.SetRotation(m_transform.m_localTransform.m_rotation);
			m_gaugeRender.SetPivot(m_pivot);
			m_gaugeRender.Update();
		}


		void UICircleGauge::Initialize(const char* assetName ,const char* fxName, const float width, const float height, const Vector3 & position, const Vector3 & scale, const Quaternion & rotation, const Vector2& pivot, const Vector4 & gaugeColor, const Vector4& bgColor,float innerRadius,float outerRadius)
		{
			m_transform.m_localTransform.m_position = position;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rotation;
			m_color = gaugeColor;
			m_pivot = pivot;

			// Init関数にassetNameとfxNameを渡すように変更。
			m_gaugeRender.Init(assetName,fxName,width, height);
			m_gaugeRender.SetPosition(position);
			m_gaugeRender.SetScale(scale);
			m_gaugeRender.SetRotation(rotation);
			m_gaugeRender.SetPivot(pivot);
			m_gaugeRender.SetGaugeColor(gaugeColor);
			m_gaugeRender.SetBgColor(bgColor);
			m_gaugeRender.SetRadius(innerRadius, outerRadius);
			m_gaugeRender.Update();
		}


		void UICircleGauge::Render(RenderContext & rc)
		{
			if (m_isDraw) {
				m_gaugeRender.Draw(rc);
			}
		}






		/********************************/


		UIDigit::UIDigit()
		{}


		UIDigit::~UIDigit()
		{}


		void UIDigit::Update()
		{
			//ComputeFinalColor();
			if (number_ != requestNumber_) {
				number_ = requestNumber_;
				digit_ = ComputeDigit();

				//不要な桁を削除
				while (renderList_.size() > digit_) {
					delete renderList_.back();
					renderList_.pop_back();
				}

				for (int i = 0; i < digit_; ++i) {
					UpdateNumber(i + 1, number_);
				}
			}

			UpdateAnimation();

			m_transform.UpdateTransform();
			for (int i = 0; i < renderList_.size(); ++i)
			{
				auto* spriteRender = renderList_[i];
				UpdatePosition(i);
				spriteRender->SetScale(m_transform.m_localTransform.m_scale);
				spriteRender->SetRotation(m_transform.m_localTransform.m_rotation);
				spriteRender->SetMulColor(m_color);
				spriteRender->Update();
			}

		}


		void UIDigit::Render(RenderContext& rc)
		{
			if (m_isDraw)
			{
				for (SpriteRender* spriteRender : renderList_)
				{
					spriteRender->Draw(rc);
				}
			}
		}


		void UIDigit::Initialize(const char* assetName, const int digit, const int number, const float widht, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation)
		{
			assetPath_ = assetName;
			digit_ = digit;
			number_ = number;
			w_ = widht;
			h_ = height;

			m_transform.m_localTransform.m_position = position;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rotation;

			for (int i = 0; i < digit; i++)
			{
				UpdateNumber(i + 1, number_); // 桁なので＋１する
				auto* spriteRender = renderList_[i];
				spriteRender->SetPosition(position);
				spriteRender->SetScale(scale);
				spriteRender->SetRotation(rotation);
			}
		}


		void UIDigit::UpdateNumber(const int targetDigit, const int number)
		{
			// NOTE: targetDigitは1以上の値になっている
			K2_ASSERT(targetDigit >= 1, "桁指定が間違えています。\n");

			// いらない
			const int targetRenderIndex = targetDigit - 1;
			SpriteRender* nextRender = nullptr;
			// 次のやつをつくる
			if (targetRenderIndex < renderList_.size()) {
				delete renderList_[targetRenderIndex];
				renderList_[targetRenderIndex] = nullptr;
				nextRender = new SpriteRender();
				renderList_[targetRenderIndex] = nextRender;
			}
			else {
				nextRender = new SpriteRender();
				renderList_.push_back(nextRender);
			}

			// 対象の桁の数字
			const int targetDigitNumber = GetDigit(targetDigit);
			std::string assetNname = assetPath_ + "/0.dds";
			assetNname[assetNname.size() - 5] = '0' + targetDigitNumber;
			nextRender->Init(assetNname.c_str(), w_, h_);
		}


		void UIDigit::UpdatePosition(const int index)
		{
			SpriteRender* render = renderList_[index];
			Vector3 position = m_transform.m_localTransform.m_position;
			position.x -= w_ * index;
			render->SetPosition(position);
		}

		int UIDigit::ComputeDigit()
		{
			int n = number_;
			if (n == 0) return 1;
			int count = 0;
			n = std::abs(n);
			while (n > 0) {
				n /= 10;
				count++;
			}
			return count;
		}


		int UIDigit::GetDigit(int digit)
		{
			// NOTE: targetDigitは1以上の値になっている
			K2_ASSERT(digit >= 1, "桁指定が間違えています。\n");
			digit -= 1;
			int divisor = static_cast<int>(pow(10, digit));
			return (number_ / divisor) % 10;
		}




		/*************************************/


		UIText::UIText() : m_scale(1.0f, 1.0f)
		{}


		UIText::~UIText()
		{}


		void UIText::Update()
		{
			UpdateAnimation();
			m_transform.UpdateTransform();

			m_fontRender.SetPosition(m_transform.m_localTransform.m_position.x, m_transform.m_localTransform.m_position.y);
			// クォータニオンのZ・W成分からZ軸回転角(ラジアン)を抽出する
			const auto& q = m_transform.m_localTransform.m_rotation;
			m_fontRender.SetRotation(2.0f * atan2f(q.z, q.w));
			m_fontRender.SetScale(m_scale);
			m_fontRender.SetColor(m_color);
			m_fontRender.SetPivot(m_pivot);
		}


		void UIText::Render(RenderContext& rc)
		{
			if (m_isDraw) {
				m_fontRender.Draw(rc);
			}
		}


		void UIText::SetText(const std::string& text)
		{
			if (text.empty()) {
				m_fontRender.SetText(L"");
				return;
			}

			// Windows APIを使って UTF-8 から wstring(UTF-16) へ正しく変換する
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &text[0], (int)text.size(), NULL, 0);
			std::wstring wstrTo(size_needed, 0);
			MultiByteToWideChar(CP_UTF8, 0, &text[0], (int)text.size(), &wstrTo[0], size_needed);

			m_fontRender.SetText(wstrTo.c_str());
		}




		/*************************************/


		UIVideo::UIVideo()
		{}


		UIVideo::~UIVideo()
		{}


		void UIVideo::Initialize(const UIVideoInitData& data)
		{
			m_videoRender.SetLoop(data.loop);
			m_videoRender.Init(data.clipPath.c_str(), data.width, data.height, data.fps);

			if (data.autoPlay)
				m_videoRender.Play();
		}


		void UIVideo::Update()
		{
			UpdateAnimation();
			m_transform.UpdateTransform();

			m_videoRender.SetMulColor(m_color);
			m_videoRender.SetPosition(m_transform.m_localTransform.m_position);
			m_videoRender.SetScale(m_transform.m_localTransform.m_scale);
			m_videoRender.SetRotation(m_transform.m_localTransform.m_rotation);
			m_videoRender.SetPivot(m_pivot);
			m_videoRender.Update();
		}


		void UIVideo::Render(RenderContext& rc)
		{
			if (m_isDraw)
				m_videoRender.Draw(rc);
		}


		/*************************************/



		UIDummy::UIDummy()
		{}


		UIDummy::~UIDummy()
		{}


		void UIDummy::Update()
		{
			UpdateAnimation();
			m_transform.UpdateTransform();
		}


		void UIDummy::Render(RenderContext& rc)
		{
			// 描画なし
		}




		/*************************************/


		UICanvas::UICanvas()
		{
			uiList_.clear();
		}


		UICanvas::~UICanvas()
		{
			uiList_.clear();
		}


		void UICanvas::Update()
		{
			for (auto& ui : uiList_)
			{
				ui->Update();
			}
		}


		void UICanvas::Render(RenderContext& rc)
		{
			for (auto& ui : uiList_)
			{
				ui->Render(rc);
			}
		}
	}
}