/**
 * @file WhirlpoolPowerSystem.h
 * @brief 渦潮の引き寄せ、押し出しを管理するクラス
 * @author 藤谷
 */
#pragma once
#include "IObject.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinManager;
	}

	namespace nature
	{
		/** 前方宣言 */
		class Whirlpool;


		/**
		 * @brief 渦潮の引き寄せ、押し出しを管理するクラス
		 */
		class WhirlpoolPowerSytem : public IObject
		{
		public:
			/**
			 * @brief 渦潮の引き寄せの情報
			 */
			struct WhirlpoolPowerInfo
			{
				/** 渦潮から子ペンギンへのベクトル */
				Vector3 toTargetVector;
				/** 対象の子ペンギン */
				actor::ChildPenguin* target;
				/** 渦潮の影響を受けているか */
				bool isAffected;
				/** 現在の回転角度（ラジアン） */
				float angle;
				/** 現在の半径オフセット（ランダム変動量） */
				float radiusOffset;
				/** 目標の半径オフセット */
				float radiusOffsetTarget;
				/** 個体固有の軌道半径オフセット（巻き込まれた瞬間にランダム決定） */
				float individualOrbitOffset;
				/** 個体固有の回転速度倍率（巻き込まれた瞬間にランダム決定） */
				float individualRotateScale;
			};


		public:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		public:
			WhirlpoolPowerSytem(Whirlpool* ownerWhirlpool);
			~WhirlpoolPowerSytem() = default;


		public:
			/**
			 * @brief 引き寄せの情報のリストを取得
			 * @return 情報リストの参照
			 */
			std::vector<WhirlpoolPowerInfo>& GetWhirlpoolPowerInfos()
			{
				return m_wpPowerInfos;
			}


		private:
			/**
			 * @brief 渦潮の引き寄せの情報を初期化する関数
			 */
			void InitializeWhirlpoolInfo();

			/**
			 * @brief 渦潮、子ペンギンの情報を更新する
			 * @param deltaTime フレーム間の経過時間
			 */
			void UpdateWhirlpoolInfo(const float deltaTime);

			/**
			 * @brief 引き寄せ処理：軌道半径まで近づき、そこで回転しながらランダムにふらつく
			 * @param info      対象の渦潮情報
			 * @param deltaTime フレーム間の経過時間
			 */
			void UpdateAttract(WhirlpoolPowerInfo& info, float deltaTime);

			/**
			 * @brief 共通の渦巻き移動処理：角度を進めて極座標で位置を更新する
			 * @param info      対象の渦潮情報
			 * @param newRadius 今フレームの半径
			 * @param deltaTime フレーム間の経過時間
			 */
			void UpdateSpiral(WhirlpoolPowerInfo& info, float newRadius, float deltaTime);


		private:
			/** 渦潮のポインタ */
			Whirlpool* m_owner;
			/** 子ペンギンマネージャーのポインタ */
			actor::ChildPenguinManager* m_cpManager;
			/** 引き寄せの情報のリスト */
			std::vector<WhirlpoolPowerInfo> m_wpPowerInfos;
		};
	}
}