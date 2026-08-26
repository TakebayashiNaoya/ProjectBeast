/**
 * @file AppParameterMacro.h
 * @brief パラメーターに必要なマクロ定義
 */
#pragma once
#include "Source/Util/CRC32.h"


namespace app
{
	namespace core
	{

	}
}


/**
* @brief ホットリロード有効化マクロ
 * @note デバッグビルド時のみ有効化
 */
#ifdef APP_DEBUG
#define APP_PARAM_HOT_RELOAD
#endif


 /**
 * @brief パラメーター用マクロ定義
 * @note ホットリロード対応版
 */
#ifdef APP_PARAM_HOT_RELOAD

#define appParameter(name)\
public:\
static constexpr uint32_t ID() {return Hash32(#name);}\
static inline std::function<void(const nlohmann::json& j, name& p)> load;

#else

#define appParameter(name)\
public:\
static constexpr uint32_t ID() {return Hash32(#name);}

#endif

