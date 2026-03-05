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


		void UIIcon::Initialize(const char* assetName, const float width, const float height)
		{
			m_spriteRender.Init(assetName, width, height);
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
		}


		void UIButton::Render(RenderContext& rc)
		{

		}





		/****************************************/


		UIGauge::UIGauge()
		{
		}


		UIGauge::~UIGauge()
		{
		}


		void UIGauge::Update()
		{
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


		void UIGauge::Initialize(const char* assetName, const float width, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation)
		{
			m_transform.m_localTransform.m_position = position;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rotation;

			m_spriteRender.Init(assetName, width, height);
			m_spriteRender.SetPosition(position);
			m_spriteRender.SetScale(scale);
			m_spriteRender.SetRotation(rotation);
			m_spriteRender.Update();
		}





		/******************************************/


		UIDigit::UIDigit()
			: m_number(0)
			, m_requestNumber(0)
			, m_digit(0)
			,m_wide(0.0f)
			,m_height(0.0f)
		{
		}


		UIDigit::~UIDigit()
		{
		}


		void UIDigit::Update()
		{
			/** 桁更新 */
			if (m_number != m_requestNumber)
			{
				m_number = m_requestNumber;
				for (int i = 0; i < m_digit; i++)
				{
					UpdateNumber(i + 1, m_number);
				}
			}

			UpdateAnimation();

			for (int i = 0; i < m_renderList.size(); i++)
			{
				auto* spriteRender = m_renderList[i];
				UpdatePosition(i);

				spriteRender->Update();
			}
		}


		void UIDigit::Render(RenderContext& rc)
		{
			for (SpriteRender* spriteRender : m_renderList)
			{
				spriteRender->Draw(rc);
			}
		}


		void UIDigit::Initialize(const char* assetName, const int digit, const int number, const float wide, const float height, Vector3& position, Vector3& scale, Quaternion& rotation)
		{
			m_assetsPath = assetName;
			m_digit = digit;
			m_number = number;
			m_wide = wide;
			m_height = height;

			m_transform.m_localTransform.m_position = position;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rotation;


			for (int i = 0; i < m_digit; i++)
			{
				SpriteRender* spriteRender = new SpriteRender;
				spriteRender->Init(assetName,wide,height);
				spriteRender->SetPosition(m_transform.m_localTransform.m_position);
				spriteRender->SetScale(m_transform.m_localTransform.m_scale);
				spriteRender->SetRotation(m_transform.m_localTransform.m_rotation);
				m_renderList.push_back(spriteRender);
				/** 桁なので +1する */
				UpdateNumber(i + 1, m_number);
			}
		}


		void UIDigit::UpdateNumber(const int targetDigit, const int number)
		{
			/** 1以上なら警告 */
			K2_ASSERT(targetDigit >= 1, "桁指定間違ってるよ!\n");

			const int targetRenderIndex = targetDigit - 1;
			SpriteRender* nextRender = nullptr;

			/** 次のものを作る */
			if (targetRenderIndex < m_renderList.size())
			{
				nextRender = m_renderList[targetRenderIndex];
			}
			else
			{
				nextRender = new SpriteRender();
				m_renderList.push_back(nextRender);
			}

			/** 対象の桁の数字 */
			const int targetDigitNumber = GetDigit(targetDigit);
			std::string assetName = m_assetsPath + "/.dds";
			assetName[assetName.size() - 5] = '0' + targetDigitNumber;
			nextRender->Init(assetName.c_str(), m_wide, m_height);
		}


		void UIDigit::UpdatePosition(const int index)
		{
			SpriteRender* render = m_renderList[index];
			Vector3 position = m_transform.m_localTransform.m_position;
			position.x -= m_wide * index;
			render->SetPosition(position);
		}


		int UIDigit::GetDigit(int digit)
		{
			/** 1以上なら警告 */
			K2_ASSERT(digit >= 1, "桁指定を間違えています\n");
			digit -= 1;
			int diver = static_cast<int>(pow(10, digit));
			return (m_number / diver) % 10;
		}





		/*************************************/


		UICanvas::UICanvas()
		{
			m_uiMap.clear();
		}


		UICanvas::~UICanvas()
		{
			m_uiMap.clear();
		}


		void UICanvas::Update()
		{
			for (auto& ui : m_uiMap)
			{
				ui.second->Update();
			}
		}


		void UICanvas::Render(RenderContext& rc)
		{
			for (auto& ui : m_uiMap)
			{
				ui.second->Render(rc);
			}
		}
	}
}