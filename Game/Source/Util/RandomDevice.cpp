/**
 * @file RandomDevice.cpp
 * @brief ランダムデバイスの実装
 */
#include "stdafx.h"
#include "RandomDevice.h"


namespace app
{
	namespace util
	{
		std::mt19937 RandomDevice::m_randomEngine{ std::random_device{}() };
	}
}
