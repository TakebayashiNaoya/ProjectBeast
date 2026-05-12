/**
 * @file MiniMapMenu.h
 * @brief ミニマップの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"
#include "Source/Actor/Character/penguin/daddyPenguin/DaddyPenguin.h"
#include "Source/UI/Model/MiniMapStatus.h"


namespace app
{
	namespace ui
	{
		class DaddyPenguin;


		class MiniMapMenu : public MenuBase
		{
		public:
			MiniMapMenu();
			~MiniMapMenu();
			void Update() override;
			void InitializeLogic() override;

			/**
			 * @brief 描画の設定。
			 * @param isDraw 描画するかどうかのフラグ
			 */
			void SetDraw(const bool isDraw) { m_isDraw = isDraw; }

			/**
			 * @brief ワールド座標系をマップ座標系に変換する
			 * @param worldCenterPos マップの中心とするオブジェクトのワールド座標。
			 * @param worldPos マップに表示したいオブジェクトのワールド座標。
			 * @param mapPos 変換した後のマップ座標。
			 */
			bool WorldPosConverterToMapPos(
					Vector3 worldCenterPos
				,	Vector3 worldPos
				,	Vector3& mapPos
			);

			/**
			 * @brief 子ペンギンのアイコンをマップに表示する用
			 */
			void MapChildPen();

			/**
			 * @brief 親ペンギンのアイコンをマップに表示する用
			 */
			void MapDaddyPen();

			/**
			 * @brief シロクマのアイコンをマップに表示する用
			 */
			void MapPolarBear();

			/**
			 * @brief 渦潮のアイコンをマップに表示する用
			 */
			void MapWhirlpool();

			/**
			 * @brief イグルーのアイコンをマップに表示する用
			 */
			void MapIgloo();

			/**
			 * @brief シロクマの巣のアイコンをマップに表示する用
			 */
			void MapPolarBearNest();

			/**
			 * @brief マップのフレームアイコンをカメラの向きに合わせて回転させる用
			 */
			void MapFrameRotation();

			/**
			 * @brief 親ペンギンのポインタを設定。
			 * @param daddy 親ペンギンのポインタ。
			 */
			inline void SetDaddyPenguin(actor::DaddyPenguin* daddy) { m_daddyPenguin = daddy; }


		private:
			/** ミニマップのステータスをunique_ptrで所有 */
			std::unique_ptr<MiniMapStatus> m_miniMapStatus;
			/** UIを表示するかのフラグ */
			bool m_isDraw;
			/** 親ペンギンのポインタ */
			actor::DaddyPenguin* m_daddyPenguin;
		};
	}
}