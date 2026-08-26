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

			/** @brief 子ペンギンの足跡サイズ（小さめ） */
			virtual float GetFootprintSize() const override { return 6.0f; }

			/** @brief 子ペンギンの足跡優先度（最優先で消される） */
			virtual int GetFootprintPriority() const override { return 0; }

			/**
			 * @brief 足跡を出さない状態かどうか
			 * @details 共通条件（ジャンプ・泳ぎ・スライド中）に加えて、
			 *          親から遠い（＝画面外の）子は出さない。見えない足跡で
			 *          プールの枠を消費すると、画面内の子の足跡が薄くなるため
			 */
			virtual bool ShouldSuppressFootprint() const override;


		public:
			void SetIglooFixedPos(const Vector3& pos) { m_iglooFixedPos = pos; }

			void SetInsideIgloo(bool isInside) { m_isInsideIgloo = isInside; }
			bool IsInsideIgloo() const { return m_isInsideIgloo; }


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

			/**
			 * @brief 体色ハイライト（点滅・発光）の更新
			 * @details 優先度の高い順に、①クマに狙われている赤点滅（輝度1超で
			 *          ブルームが拾って光る）②ウルト発動中の隊列発光（ゆっくりした
			 *          呼吸のような明滅）③勇敢時間の明滅、を適用する。
			 *          どれでもなければタイプ別カラーへ戻す。
			 *          毎フレーム ChildPenguinManager::Update() から呼ばれる。
			 * @param isTargeted   このフレームにクマに狙われているかどうか
			 * @param isUltGlowing ウルト発動中の隊列メンバーとして発光させるかどうか
			 */
			void UpdateBearTargetHighlight(const bool isTargeted, const bool isUltGlowing);


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
			/** クマに狙われている点滅の経過時間（秒） */
			float m_highlightTimer = 0.0f;
			/** クマに狙われている点滅中かどうか */
			bool m_isHighlighted = false;

			bool m_isInsideIgloo = false;
			Vector3 m_iglooFixedPos;  // 固定座標
		};
	}
}