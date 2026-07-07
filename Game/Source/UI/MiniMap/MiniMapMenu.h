/**
 * @file MiniMapMenu.h
 * @brief ミニマップの動的処理クラス
 * @author 忽那
 */
#pragma once
#include "Source/UI/Menu.h"

#include "Source/UI/Modules/InGameStartingAnimLogic/InGameStartingAnimLogic.h"


namespace app
{
	namespace ui
	{
		/** 前方宣言 */
		class MiniMapStatus;


		class MiniMapMenu : public MenuBase
		{
		public:
			MiniMapMenu();
			~MiniMapMenu();
			void Update() override final;
			void InitializeLogic() override final;


			//======================================//
			// 外部からのアクセス関数
			//======================================//
		public:
			/**
			 * @brief アイコンの数を設定する
			 */
			void InitializeMapIcon();

			/**
			 * @brief アイコンの数を設定する
			 * @param type アイコンの種類
			 * @param num アイコンの数
			 */
			void SetIconNum(const EnMiniMapIconType type, const uint8_t num);


			void SetActorPositions(
				const Vector3& centerActorPosition,
				const ActorPositions& actorPositions
			);


			//======================================//
			// カプセル化関数
			//======================================//
		private:
			/**
			 * @brief UIIconの描画フラグを設定する
			 * @param icon UIIconのポインタ。
			 * @param isDraw 描画するかどうかのフラグ。
			 */
			void SetDrawFlag(UIIcon* icon, const bool isDraw);


			/**
			 * @brief UIIconのポインタを取得、初期化する
			 * @param icon UIIconのポインタ。
			 * @param name UIIconの名前。
			 */
			UIIcon* GetAndInitIcon(const uint32_t key);


			/**
			 * @brief ワールド座標系をマップ座標系に変換する。
			 * @param worldCenterPos マップの中心とするオブジェクトのワールド座標。
			 * @param worldPos マップに表示したいオブジェクトのワールド座標。
			 * @param mapPos 変換した後のマップ座標。
			 * @return マップの範囲内なら true、範囲外なら false。
			 */
			bool WorldPosConverterToMapPos(
				Vector3 worldCenterPos
				, Vector3 worldPos
				, Vector3& mapPos
			);


			/**
			 * @brief マップのフレームアイコンをカメラの向きに合わせて回転させる。
			 */
			void MapFrameRotation();


		private:
			/** ミニマップのステータス */
			std::unique_ptr<MiniMapStatus> m_miniMapStatus;

			/** 親ペンギンのアイコン */
			UIIcon* m_daddy;
			/** マップのアイコン */
			UIIcon* m_map;
			/** マップのフレームアイコン */
			UIIcon* m_frame;


			struct MapIconInfo
			{
				/** アイコンの配列 */
				std::vector<UIIcon*> icons;
				/** アイコンの数 */
				uint8_t num;
				/** 呼ばれたのが一度目かどうか */
				bool isFirstCall;


				MapIconInfo();
				~MapIconInfo() = default;
			};



			/** アイコンの種類ごとの情報を保持する配列 */
			std::array<MapIconInfo, static_cast<uint8_t>(EnMiniMapIconType::Num)> m_iconVectors;

			/** ゲーム開始時のアニメーションロジック */
			InGameStartingAnimLogic m_startingAnimLogic;
		};
	}
}
