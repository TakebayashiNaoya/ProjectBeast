/**
 * ParameterManager.cpp
 *
 * パラメーター管理
 * ステータスなどの数値を外部ファイルから読み込んで使用する
 */
#include "stdafx.h"
#include "ParameterManager.h"


ParameterManager* ParameterManager::m_instance = nullptr; //初期化


ParameterManager::ParameterManager()
{
	m_parameterMap.clear();
}

ParameterManager::~ParameterManager()
{
	for (auto& it : m_parameterMap) {
		auto& parameters = it.second;
		for (auto* parameter : parameters) {
			delete parameter;
		}
		parameters.clear();
	}
	m_parameterMap.clear();
}