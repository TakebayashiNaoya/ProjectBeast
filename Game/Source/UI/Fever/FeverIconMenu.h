/**
 * @file FeverIconMenu.h
 * @brief フィーバータイム開始時にアイコンを画面上から下へ落下させる演出
 * @author 竹林
 */
#pragma once
#include "Source/UI/Menu.h"


namespace app
{
	namespace ui
	{
		/**
		 * @brief フィーバータイム開始時、アイコンを画面上端から下端へ落下させる演出用メニュー
		 */
		class FeverIconMenu : public MenuBase
		{
		public:
			FeverIconMenu();
			~FeverIconMenu() override = default;

			/** 更新処理 */
			void Update() override;

			/** UIのロジック初期化処理 */
			void InitializeLogic() override;


		private:
			/** フィーバー中かどうか（前フレームの状態。false→trueのエッジ検出用） */
			bool m_wasFeverActive = false;
			/** 現在、落下演出を再生中かどうか */
			bool m_isFalling = false;
			/** アイコンの本来の座標（JSON上の初期値をX座標などに再利用するためキャッシュ） */
			Vector3 m_defaultPos = Vector3::Zero;
		};
	}
}
