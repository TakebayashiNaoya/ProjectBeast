/**
 * @file UIAnimationFactory.h
 * @brief UIAnimationParameterの定義からUIAnimationを生成するラッパー
 * @detail UIAnimation.h,UIAnimationParameter.h 側はこのファイルを知らないため循環しない
 * author 忽那
 */
#pragma once
#include "UIAnimation.h"
#include "UIParts.h"
#include "UIAnimationParameter.h"


namespace app
{
	namespace ui
	{
		class UIAniamtionFactory
		{
		public:
			/**
			 * @brief 具象クラスを指定してアニメーションを生成する
			 * @tparam T 生成するアニメーションの具象クラス
			 * @param key ハッシュキー
			 * @return 生成されたアニメーション(定義が無ければ　nullptr)
			 */
			template<typename T>
			static std::unique_ptr<T>Create(uint32_t key)
			{
				const UIAnimationDef* def = UIAnimationParameter::Get().Find(key);
				if (!def)return nullptr;

				auto anim = std::make_unique<T>();
				ApplyParameter(anim.get(), *def);
				return anim;
			}


			/**
			 * @brief アニメーションを生成し、UIに直接登録する
			 * @tparam T 具象アニメーション型
			 * @param target 登録先のUI
			 * @param key ハッシュキー
			 * @return 成功したか
			 * @detail
			 * UIAnimationFactory::AttachParameter<UIColorAnimation>(gauge,Hash32("FadeIn"));
			 */
			template<typename T>
			static bool AttachParameter(UIBase* target, uint32_t key)
			{
				if (!target)return false;

				auto anim = Create<T>(key);
				if (!anim)return false;

				target->AddAnimation(key, std::move(anim));
				return true;
			}


		private:
			/**
			 * @brief Floatアニメーションのパラメーターを適用
			 * @param anim 適用先のアニメーション
			 * @param def アニメーションの定義
			 */
			static void ApplyParameter(UIFloatAnimation* anim, const UIAnimationDef& def)
			{
				anim->SetParameter(def.startFloat, def.endFloat, def.duration, def.easingType, def.loopMode);
			}

			
			/**
			 * @brief Vector2アニメーションのパラメーターを適用
			 * @param anim 適用先のアニメーション
			 * @param def アニメーションの定義
			 */
			static void ApplyParameter(UIVector2Animation* anim, const UIAnimationDef& def)
			{
				anim->SetParameter(def.startV2, def.endV2, def.duration, def.easingType, def.loopMode);
			}


			/**
			 * @brief Vector3アニメーションのパラメーターを適用
			 * @pram anim 適用先のアニメーション
			 * @param def アニメーションの定義
			 */
			static void ApplyParameter(UIVector3Animation* anim, const UIAnimationDef& def)
			{
				anim->SetParameter(def.startV3, def.endV3, def.duration, def.easingType, def.loopMode);
			}


			/**
			 * @brief Vector4アニメーションのパラメーターを適用
			 * @param anim 適用先のアニメーション
			 * @param def アニメーションの定義
			 */
			static void ApplyParameter(UIVector4Animation* anim, const UIAnimationDef& def)
			{
				anim->SetParameter(def.startV4, def.endV4, def.duration, def.easingType, def.loopMode);
			}
		};
	}
}
