/**
 * @file ChildPenguin.h
 * @brief 子ペンギンクラス
 * @author 藤谷
 */
#pragma once
#include "ChildPenguinTypes.h"
#include "Source/Actor/Character/Penguin/PenguinBase.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguinStateMachine;
		class ChildPenguinAIController;
		class DaddyPenguin;


		namespace
		{
			/** 子ペンギンのスケール */
			const Vector3 CHILD_PENGUIN_SCALE = Vector3(0.6f, 0.4f, 0.6f);
		}


		/**
		 * @brief 子ペンギンクラス
		 */
		class ChildPenguin : public PenguinBase
		{
		public:
			/**
			 * @brief ステートマシンを取得
			 * @return ステートマシンのポインタ
			 */
			inline ChildPenguinStateMachine* GetStateMachine() { return m_stateMachine.get(); }
			/**
			 * @brief AIコントローラーを取得
			 * @return AIコントローラーのポインタ
			 */
			inline ChildPenguinAIController* GetAIController()const { return m_aiController.get(); }
			/**
			 * @brief 子ペンギンのタイプを取得
			 * @return 子ペンギンのタイプ
			 */
			inline EnChildPenguinType GetChildPenguinType() const { return m_type; }

			inline int  GetLogId() const { return m_logId; }
			inline void SetLogId(int id) { m_logId = id; }

			/** ログ用：タイプ名を文字列で返す */
			const char* GetChildPenguinTypeStr() const
			{
				switch (m_type)
				{
				case EnChildPenguinType::Serious: return "Serious";
				case EnChildPenguinType::Clingy:  return "Clingy";
				case EnChildPenguinType::Naughty: return "Naughty";
				case EnChildPenguinType::Clumsy:  return "Clumsy";
				case EnChildPenguinType::Caring:  return "Caring";
				default:                          return "Unknown";
				}
			}
			/**
			 * @brief 目標座標を取得する（AIコントローラーが移動処理に使う）
			 * @return 目標座標
			 */
			inline const Vector3& GetFormationTargetPosition() const { return m_formationTarget; }
			/**
			 * @brief 陣形における自身の目標座標を設定する
			 * @param targetPos 目標座標
			 */
			inline void SetFormationTargetPosition(const Vector3& targetPos) { m_formationTarget = targetPos; }
			/**
			 * @brief 子ペンギンのタイプを設定
			 * @param type 子ペンギンのタイプ
			 */
			void SetChildPenguinType(EnChildPenguinType type);
			/**
			 * @brief AIコントローラーの作成
			 * @note 親ペンギンが設定された後に呼び出す必要がある
			 */
			void CreateAIController();

			virtual float GetFootprintSize() const override { return 6.0f; }

		public:
			void SetIglooFixedPos(const Vector3& pos) { m_iglooFixedPos = pos; }

			void SetInsideIgloo(bool isInside) { m_isInsideIgloo = isInside; }
			bool IsInsideIgloo() const { return m_isInsideIgloo; }


		public:
			ChildPenguin();
			virtual ~ChildPenguin() override = default;


		private:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		public:
			/**
			 * @brief ゲーム開始前のカウントダウン中の更新処理
			 */
			void UpdateAtCountDownTime();


		private:
			/** ステートマシン */
			std::unique_ptr<ChildPenguinStateMachine> m_stateMachine;
			/** AIコントローラー */
			std::unique_ptr<ChildPenguinAIController> m_aiController;
			/** 子ペンギンのタイプ */
			EnChildPenguinType m_type = EnChildPenguinType::Serious;
			/** ログ用の連番ID（ChildPenguinManager が生成順に割り当てる） */
			int m_logId = -1;
			/** 陣形における自身の目標座標 */
			Vector3 m_formationTarget;
			/** タイプ別乗算カラー */
			Vector4 m_typeColor = Vector4::One;
			/** カラー適用済みフラグ（モデルロード完了後に一度だけ適用） */
			bool m_colorApplied = false;

			bool m_isInsideIgloo = false;
			Vector3 m_iglooFixedPos;  // 固定座標
		};
	}
}