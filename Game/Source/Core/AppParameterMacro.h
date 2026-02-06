/**
 * @file AppParameterMacro.h
 * @brief パラメーターに必要なマクロ定義
 * @author 藤谷
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
 * @note デバッグビルド時のみ有効化
 */
#ifdef K2_DEBUG
#define APP_PARAM_HOT_RELOAD
#endif


 /**
 * @note ホットリロード対応版
 */
#ifdef APP_PARAM_HOT_RELOAD

#define appParameter(name)\
public:\
static constexpr uint32_t ID() {return Hash32(#name);}\
std::function<void(const nlohmann::json& j, name& p)> load;

#else

#define appParameter(name)\
public:\
static constexpr uint32_t ID() {return Hash32(#name);}

#endif

