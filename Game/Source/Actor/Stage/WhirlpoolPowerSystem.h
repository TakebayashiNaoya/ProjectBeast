/**
 * @file WhirlpoolPowerSystem.h
 * @brief 渦潮の引き寄せ、押し出しを管理するクラス
 * @author 藤谷
 */
#pragma once
#include "IStage.h"


namespace app
{
	namespace actor
	{
		/** 前方宣言 */
		class ChildPenguin;
		class ChildPenguinManager;
		class Whirlpool;


		/**
		 * @brief 渦潮の引き寄せ、押し出しを管理するクラス
		 */
		class WhirlpoolPowerSytem : public Actor
		{
		public:
			/**
			 * @brief 渦潮の引き寄せ、押し出しの情報
			 */
			struct WhirlpoolPowerInfo
			{
				/** 渦潮から子ペンギンへのベクトル */
				Vector3 toTargetVector;
				/** 対象の子ペンギン */
				ChildPenguin* target;
				/** 渦潮の影響を受けているか */
				bool isAffected;
				/** 引き寄せ完了（押し出しフェーズ中）か */
				bool isPushing;
				/** 現在の回転角度（ラジアン） */
				float angle;
			};


		public:
			void Start() override final;
			void Update() override final;
			void Render(RenderContext& rc) override final;


		public:
			WhirlpoolPowerSytem(Whirlpool* ownerWhirlpool);
			~WhirlpoolPowerSytem() override = default;


		private:
			/** 渦潮の引き寄せ、押し出しの情報を初期化する関数 */
			void InitializeWhirlpoolInfo();


			/** 渦潮、子ペンギンの情報を更新する */
			void UpdateWhirlpoolInfo(const float deltaTime);

			/**
			 * @brief 引き寄せ処理：円を描きながら中心へ近づける
			 * @param info      対象の渦潮情報
			 * @param deltaTime フレーム間の経過時間
			 */
			void UpdateAttract(WhirlpoolPowerInfo& info, float deltaTime);

			/**
			 * @brief 押し出し処理：円を描きながら外へ押し出す
			 * @param info      対象の渦潮情報
			 * @param deltaTime フレーム間の経過時間
			 */
			void UpdatePush(WhirlpoolPowerInfo& info, float deltaTime);

			/**
			 * @brief 共通の渦巻き移動処理：角度を進めて極座標で位置を更新する
			 * @param info        対象の渦潮情報
			 * @param newRadius   今フレームの半径
			 * @param deltaTime   フレーム間の経過時間
			 */
			void UpdateSpiral(WhirlpoolPowerInfo& info, float newRadius, float deltaTime);


		private:
			/** 渦潮のポインタ */
			Whirlpool* m_ownerWhirlpool;
			/** 子ペンギンマネージャーのポインタ */
			ChildPenguinManager* m_childPenguinManager;
			/** 引き寄せ、押し出しの情報のリスト */
			std::vector<WhirlpoolPowerInfo> m_whirlpoolPowerInfos;
		};
	}
}
