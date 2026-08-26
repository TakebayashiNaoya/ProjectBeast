/**
 * @file ChildPenguin.cpp
 * @brief 子ペンギンクラス
 * @author 藤谷
 */
#include "stdafx.h"
#include "ChildPenguin.h"
#include "ChildPenguinAIController.h"
#include "ChildPenguinManager.h"
#include "ChildPenguinParameter.h"
#include "ChildPenguinStateMachine.h"
#include "ChildPenguinStatus.h"
#include "ClumsyChildPenguinStateMachine.h"
#include "NaughtyChildPenguinStateMachine.h"
#include "Physics/Physics.h"
#include "Source/Actor/Character/CharacterStateMachine.h"
#include "Source/Actor/Character/Penguin/PenguinAnimationData.h"
#include "Source/Actor/Character/Penguin/PenguinIState.h"
#include "Source/Core/ParameterManager.h"


namespace app
{
	namespace actor
	{
		namespace
		{
			const ModelData MODEL_DATA =
			{
				"Assets/modelData/penguin/childPenguin/ChildPenguin.tkm",
				ANIMATION_DATA,
				EnModelUpAxis::enModelUpAxisZ,
				std::size(ANIMATION_DATA)
			};

			/** 輪郭線の太さ（スクリーン幅比の法線押し出し量。約2px相当） */
			constexpr float OUTLINE_WIDTH = 0.002f;

			/** 足跡を出す親からの距離。これより遠い（＝画面外の）子は足跡を出さない */
			constexpr float FOOTPRINT_VISIBLE_RANGE = 800.0f;
			/** 輪郭線の色。タイプ色と混ざらないはっきりした黒 */
			const Vector4 OUTLINE_COLOR(0.05f, 0.05f, 0.07f, 1.0f);

			/** クマに狙われている点滅の周波数（Hz） */
			constexpr float BEAR_TARGET_BLINK_FREQUENCY = 3.0f;

			/** 勇敢（再集合に応えて逃げない）中の明滅の周波数（Hz）と明るさの振幅。
			 *  +35%では雪原の明るさに埋もれて視認できなかったため+90%まで上げた。
			 *  タイプ色の明るいチャンネルはピークでブルームしきい値(1.5)をわずかに
			 *  超え、山の瞬間だけ淡く光る */
			constexpr float BRAVE_BLINK_FREQUENCY = 1.2f;
			constexpr float BRAVE_BLINK_GAIN = 0.9f;

			/** ウルト発動中の隊列発光。ゆっくりした呼吸（0.8Hz）で 1.3〜1.7 倍の明るさを
			 *  往復し、山ではブルームしきい値を超えて淡く光る。勇敢明滅（1.2Hz・暗→明）
			 *  とはリズムと下限を変えて見分けが付くようにしている */
			constexpr float ULT_GLOW_FREQUENCY = 0.8f;
			constexpr float ULT_GLOW_GAIN_BASE = 0.2f;
			constexpr float ULT_GLOW_GAIN_WAVE = 0.25f;

			/**
			 * @brief 点滅ピークの乗算カラー
			 * @details 輝度が1を超える赤にしてあるので、点滅の山でブルームが拾って光る。
			 */
			const Vector4 BEAR_TARGET_BLINK_COLOR(2.5f, 0.25f, 0.25f, 1.0f);
		}



		void ChildPenguin::SetChildPenguinType(EnChildPenguinType type)
		{
			m_type = type;
			m_colorApplied = false;

			/** タイプ別乗算カラーをJSONパラメーターから設定 */
			const int typeIndex = static_cast<int>(type);
			const auto* param = core::ParameterManager::Get()->GetParameter<MasterChildPenguinTypeParameter>(typeIndex);
			m_typeColor = Vector4(param->colorR, param->colorG, param->colorB, param->colorA);

			/** タイプ変更に伴いステートマシンを作成 */
			/** おっちょこちょいタイプは固有ステートを持つため専用クラスを使う */
			if (m_type == EnChildPenguinType::Clumsy)
			{
				m_stateMachine = std::make_unique<ClumsyChildPenguinStateMachine>(this);
			}
			else if (m_type == EnChildPenguinType::Naughty)
			{
				m_stateMachine = std::make_unique<NaughtyChildPenguinStateMachine>(this);
			}
			else
			{
				m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			}
			m_characterStateMachine = m_stateMachine.get();

			CreateAIController();
		}


		void ChildPenguin::CreateAIController()
		{
			/** 親ペンギンが設定されたら、タイプに応じたAIコントローラーを作成 */
			switch (m_type)
			{
			case EnChildPenguinType::Serious:
				m_aiController = std::make_unique<SeriousChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Clingy:
				m_aiController = std::make_unique<ClingyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Naughty:
				m_aiController = std::make_unique<NaughtyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Clumsy:
				m_aiController = std::make_unique<ClumsyChildPenguinAI>(this);
				break;
			case EnChildPenguinType::Caring:
				m_aiController = std::make_unique<CaringChildPenguinAI>(this);
				break;
			}
		}


		ChildPenguin::ChildPenguin()
		{
			Init(MODEL_DATA);

			m_stateMachine = std::make_unique<ChildPenguinStateMachine>(this, m_type);
			m_characterStateMachine = m_stateMachine.get();

			m_status = std::make_unique<ChildPenguinStatus>();
			m_status->Setup();

			CreateAIController();
		}


		void ChildPenguin::Start()
		{
			/** スケールを初期化 */
			m_transform.m_scale = CHILD_PENGUIN_SCALE;
			PenguinBase::Start();
		}


		void ChildPenguin::Update()
		{
			/** モデルロード完了後、一度だけカラーを適用 */
			if (m_modelReady && !m_colorApplied)
			{
				m_modelRender.SetMulColor(m_typeColor);

				/** 雪原の白背景に埋もれないよう、黒の太めの輪郭線で縁取る */
				m_modelRender.SetOutlineParam(OUTLINE_WIDTH, OUTLINE_COLOR);
				m_modelRender.EnableOutline();

				m_modelRender.Update();
				m_colorApplied = true;
			}

			/** AIコントローラーがあれば更新 */
			if (m_aiController)
			{
				m_aiController->Update();
			}

			/** ステートマシン更新 */
			m_stateMachine->Update();

			PenguinBase::Update();

			/** 泳ぎ中はモデルの描画位置のみY座標をオフセットする */
			/** 物理・ステート判定には影響を与えない */
			if (m_stateMachine->IsSwimming() && m_modelReady)
			{
				Vector3 renderPos = m_transform.m_position;
				renderPos.y += SWIM_Y_OFFSET;
				m_modelRender.SetTRS(renderPos, m_transform.m_rotation, m_transform.m_scale);
				m_modelRender.Update();
				return;
			}

			/** スライド中は地形の法線に沿ってモデルを傾ける */
			/** 物理・ステート判定には影響を与えない */
			UpdateSlideTilt();
		}


		void ChildPenguin::Render(RenderContext& rc)
		{
			PenguinBase::Render(rc);
		}


		void ChildPenguin::UpdateBearTargetHighlight(const bool isTargeted, const bool isUltGlowing)
		{
			// タイプ別カラーの適用前に触ると、適用処理に上書きされたり戻し先が狂ったりする
			if (!m_modelReady || !m_colorApplied) return;

			if (isTargeted)
			{
				m_isHighlighted = true;
				m_highlightTimer += g_gameTime->GetFrameDeltaTime();

				// タイプ別カラーと高輝度の赤を正弦波でブレンドする。
				// cosを使って0から立ち上げることで、狙われた瞬間に色が飛ばない
				const float wave =
					0.5f - 0.5f * cosf(2.0f * Math::PI * BEAR_TARGET_BLINK_FREQUENCY * m_highlightTimer);

				Vector4 color;
				color.x = m_typeColor.x + (BEAR_TARGET_BLINK_COLOR.x - m_typeColor.x) * wave;
				color.y = m_typeColor.y + (BEAR_TARGET_BLINK_COLOR.y - m_typeColor.y) * wave;
				color.z = m_typeColor.z + (BEAR_TARGET_BLINK_COLOR.z - m_typeColor.z) * wave;
				color.w = m_typeColor.w;
				m_modelRender.SetMulColor(color);
				return;
			}

			// ウルト発動中の隊列メンバーは、ゆっくりした呼吸のような明滅で発光させる。
			// 効果の担い手は「ついてきている子ペンギン全員」なので、範囲表示ではなく
			// 本人たちを光らせて「今強化されている」ことを見せる
			if (isUltGlowing)
			{
				m_isHighlighted = true;
				m_highlightTimer += g_gameTime->GetFrameDeltaTime();

				const float wave =
					0.5f - 0.5f * cosf(2.0f * Math::PI * ULT_GLOW_FREQUENCY * m_highlightTimer);
				const float gain = 1.0f + ULT_GLOW_GAIN_BASE + ULT_GLOW_GAIN_WAVE * wave;

				Vector4 color = m_typeColor;
				color.x *= gain;
				color.y *= gain;
				color.z *= gain;
				m_modelRender.SetMulColor(color);
				return;
			}

			// 再集合の呼びかけに応えた「勇敢」時間中は、うっすら明滅させて
			// 「いまはシロクマから逃げない子」であることを見せる（狙われ点滅より弱い演出）
			if (m_aiController != nullptr && m_aiController->IsBraveFromRegroup())
			{
				m_isHighlighted = true;
				m_highlightTimer += g_gameTime->GetFrameDeltaTime();

				const float wave =
					0.5f - 0.5f * cosf(2.0f * Math::PI * BRAVE_BLINK_FREQUENCY * m_highlightTimer);
				const float gain = 1.0f + BRAVE_BLINK_GAIN * wave;

				Vector4 color = m_typeColor;
				color.x *= gain;
				color.y *= gain;
				color.z *= gain;
				m_modelRender.SetMulColor(color);
				return;
			}

			// 点滅終了：タイプ別カラーへ戻す（毎フレーム設定し直さないよう一度だけ）
			if (m_isHighlighted)
			{
				m_isHighlighted = false;
				m_highlightTimer = 0.0f;
				m_modelRender.SetMulColor(m_typeColor);
			}
		}


		void ChildPenguin::UpdateAtCountDownTime()
		{
			// ロード完了待ち（Update()と同じ処理）
			ModelLoadUpdate();



			if (m_modelReady)
			{
				if (!m_colorApplied)
				{
					m_modelRender.SetMulColor(m_typeColor);

					/** 通常のUpdate()と同じ縁取りをここでも適用する。
					 *  カウントダウン中にロード完了した子はこちらの経路しか通らず、
					 *  ここで m_colorApplied が立つと Update() 側の適用ブロックが
					 *  一度も走らないため、輪郭線もここで有効化しておく必要がある */
					m_modelRender.SetOutlineParam(OUTLINE_WIDTH, OUTLINE_COLOR);
					m_modelRender.EnableOutline();

					m_colorApplied = true;
				}


				// 海にいる子ペンギンはアニメーションを再生させる
				// ステートマシンのUpdate()は呼ばない：カウントダウン中はキャラクターコントローラーの
				// 接地判定がまだ安定しておらず、誤ってジャンプステートに遷移してSEが鳴ってしまうため
				if (m_stateMachine && m_stateMachine->IsInWater() && m_modelRender.IsPlayingAnimation())
				{
					m_modelRender.PlayAnimation(
						static_cast<int>(EnPenguinAnimationID::MoveSwim),
						actor::ActorStateMachine::ANIMATION_INTERPOLATE_TIME);
				}

				// ロード完了済み → 行列更新
				m_modelRender.SetTRS(m_transform.m_position, m_transform.m_rotation, m_transform.m_scale);
				m_modelRender.Update();
			}
		}


		bool ChildPenguin::ShouldSuppressFootprint() const
		{
			// 共通条件（ジャンプ・泳ぎ・スライド中）
			if (PenguinBase::ShouldSuppressFootprint()) return true;

			// 親から遠い（＝画面外の）子は出さない。見えない足跡でプールの枠を
			// 消費すると、画面内の子に行き渡る足跡が減ってしまう
			auto* manager = ChildPenguinManager::GetInstance();
			if (manager == nullptr) return false;

			Vector3 toDaddy = manager->GetDaddyPosition() - m_transform.m_position;
			toDaddy.y = 0.0f;
			return toDaddy.LengthSq() > FOOTPRINT_VISIBLE_RANGE * FOOTPRINT_VISIBLE_RANGE;
		}
	}
}