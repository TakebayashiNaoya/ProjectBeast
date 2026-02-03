#include "stdafx.h"
#include "UIBase.h"

namespace app {
	namespace ui {
		namespace {
			const int ONE = 1;
			const int TEN = 10;
		}

		UIBase::UIBase()
		{
		}


		UIBase::~UIBase()
		{
		}



		/*************************************************/


		UIImage::UIImage()
		{
		}


		UIImage::~UIImage()
		{
		}


		bool UIImage::Start()
		{
			return true;
		}


		void UIImage::Update()
		{
		}


		void UIImage::Render(RenderContext& rc)
		{
		}

		void UIImage::Initialize(const char* assetName, const float wide, const float height, const Vector3& position, const Vector3& scale, const Quaternion& rotation)
		{
			m_transform.m_localTransform.m_position = position;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rotation;
			
			m_spriteRender.Init(assetName, wide, height);
			m_spriteRender.SetPosition(position);
			m_spriteRender.SetScale(scale);
			m_spriteRender.SetRotation(rotation);

			m_spriteRender.Update();
		}




		/*****************************************/


		UIDigit::UIDigit()
		{
		}

		UIDigit::~UIDigit()
		{
		}

		void UIDigit::Initialize(const char* assetName, const int digit, const int number, const float wide, const float height, Vector3& pos, Vector3& scale, Quaternion& rot)
		{
			m_assetsPath = assetName;
			m_digit = digit;
			m_number = number;
			m_wide = wide;
			m_height = height;
			m_transform.m_localTransform.m_position = pos;
			m_transform.m_localTransform.m_scale = scale;
			m_transform.m_localTransform.m_rotation = rot;


			/** 必要な数のスプライトを生成 */
			m_renderList.clear();
			for (int i = 0; i < digit; i++) {
				/** スプライトの配列の中に入れる */
				SpriteRender* sprite = new SpriteRender();
				m_renderList.push_back(sprite);
			}


			/** 各桁の数字を更新 */
			for (int i = 0; i < digit; i++) {
				UpdateNumber(i + 1, m_number);
			}
		}


		bool UIDigit::Start()
		{
			return true;
		}


		void UIDigit::Update()
		{
			//もし数字が変わっていたら更新
			if (m_number != m_requestNumber) {
				m_number = m_requestNumber;
				for (int i = 0; i < m_digit; i++) {
					UpdateNumber(i + ONE, m_number);
				}
			}


			/** トランスフォーム更新とスプライトレンダーの更新 */
			m_transform.UpdateTransform();
			for (int i = 0; i < m_renderList.size(); i++) {
				auto* spriteRender = m_renderList[i];
				UpdatePosition(i);
				spriteRender->SetScale(m_transform.m_worldTransform.m_scale);
				spriteRender->SetRotation(m_transform.m_worldTransform.m_rotation);
				spriteRender->Update();
			}
		}


		void UIDigit::Render(RenderContext& rc)
		{
			for (SpriteRender* spriteRender : m_renderList) {
				spriteRender->Draw(rc);
			}
		}


		void UIDigit::UpdateNumber(const int targetDigit, const int number)
		{
			/** 警告メッセージ */
			K2_ASSERT(targetDigit >= ONE, "桁指定を間違えています。\n");
			const int targetRenderIndex = targetDigit - ONE;
			SpriteRender* nextRender = nullptr;

			if (targetRenderIndex >= m_renderList.size()) {
				nextRender = m_renderList[targetRenderIndex];
			}
			else {
				nextRender = new SpriteRender();
				m_renderList.push_back(nextRender);
			}
		}


		void UIDigit::UpdatePosition(const int index)
		{
			SpriteRender* sprite = m_renderList[index];
			Vector3 position = m_transform.m_worldTransform.m_position;
			position.x -= m_wide * index;
			sprite->SetPosition(position);
		}


		int UIDigit::GetDigit(int digit)
		{
			K2_ASSERT(digit >= ONE, "桁指定が間違えています。\n");
			digit -= ONE;
			int divisor = static_cast<int>(pow(TEN, digit));
			return (m_number / divisor) % TEN;
		}



		/*****************************************/


		UICanvas::UICanvas()
		{
			m_uiList.clear();
		}


		UICanvas::~UICanvas()
		{
			for (auto* ui : m_uiList) {
				/** トランスフォームの親子関係を解除 */
				m_transform.RemoveChild(&ui->m_transform);
				/** キャンバス上にあるuiを削除 */
				delete ui;
				ui = nullptr;
			}
			m_uiList.clear();
		}


		bool UICanvas::Start()
		{
			return true;
		}


		void UICanvas::Update()
		{
			m_transform.UpdateTransform();

			for (auto* ui : m_uiList) {
				ui->Update();
			}
		}


		void UICanvas::Render(RenderContext& rc)
		{
			for (auto* ui : m_uiList) {
				ui->Render(rc);
			}
		}
	}
}