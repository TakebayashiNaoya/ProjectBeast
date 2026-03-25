/**
 * @file ChildPenguinStateMachine.h
 * @brief 子ペンギンのステートマシン
 * @author 藤谷
 */
#pragma once
#include "Source/Actor/Character/CharacterStateMachine.h"


namespace app
{
	namespace actor
	{

		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinStatus;


		/**
		 * @brief 子ペンギンのステートマシンクラス
		 */
		class ChildPenguinStateMachine : public CharacterStateMachine
		{
			// ここに子ペンギン固有のセッター関数を追加していく
		public:
			/**
			 * @brief 追従命令を受けたかどうかを設定
			 * @param isFollowCommanded 追従命令を受けたかどうか
			 */
			inline void SetIsFollowCommanded(const bool isFollowCommanded)
			{
				m_isFollowCommanded = isFollowCommanded;
			}
			/**
			 * @brief 待機命令を受けたかどうかを設定
			 * @param isWaitCommanded 待機命令を受けたかどうか
			 */
			inline void SetIsWaitCommanded(const bool isWaitCommanded)
			{
				m_isWaitCommanded = isWaitCommanded;
			}


			// ここに子ペンギン固有のゲッター関数を追加していく
		public:
			/**
			 * @brief 子ペンギンのステータスを取得
			 * @return 子ペンギンのステータスポインタ
			 */
			const ChildPenguinStatus* GetChildPenuinStatus() const;


			/** ステートの変更先を取得する */
			core::IState* GetChangeState();


		public:
			ChildPenguinStateMachine(ChildPenguin* ownerChildPenguin);
			~ChildPenguinStateMachine() = default;


		private:
			/** 子ペンギンのポインタ */
			ChildPenguin* m_ownerChildPenguin;
			/** 追従命令を受けたかどうか */
			bool m_isFollowCommanded;
			/** 待機命令を受けたかどうか */
			bool m_isWaitCommanded;
		};
	}
}

