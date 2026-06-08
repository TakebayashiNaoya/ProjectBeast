/**
 * @file IStatus.cpp
 * @brief ステータス基底クラス実装
 * @author 藤谷
 */
#include "stdafx.h"
#include "IStatus.h"


namespace app
{
	namespace core
	{
		IStatus::IStatus()
			: m_isSetUp(false)
		{}
	}
}