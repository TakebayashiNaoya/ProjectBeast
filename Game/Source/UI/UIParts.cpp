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
		{
		}


		UIImage::~UIImage()
		{
		}


		void UIImage::Update()
		{
		}


		void UIImage::Render(RenderContext& rc)
		{
		}





		/***************************************/



		UIIcon::UIIcon()
		{
		}


		UIIcon::~UIIcon()
		{
		}


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


		void UIIcon::Initialize(const char* assetName, const float width, const float height,const Vector3& position,const Vector3& scale,const Quaternion& rotation, const Vector4& color)
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
		{
		}


		UIButton::~UIButton()
		{
		}


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
		{
		}


		UIGauge::~UIGauge()
		{
		}


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
			m_spriteRender.Draw(rc);
		}


		void UIGauge::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation, const Vector4& color,const Vector2& pivot)
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
			for (SpriteRender* spriteRender : renderList_)
			{
				spriteRender->Draw(rc);
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