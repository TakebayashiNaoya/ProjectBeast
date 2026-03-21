/**
 * @file EffectManager.h
 * @brief 必要なエフェクトファイルを読み込んだり再生したりなど管理する
 * @author 藤谷
 */
#pragma once
#include "Types.h"


namespace app
{
	/** エフェクト用のハンドル名 */
	using EffectHandle = uint32_t;
	/** ハンドル無効値 */
	static constexpr EffectHandle INVALID_EFFECT_HANDLE = 0xffffffff;


	/**
	 * @brief エフェクト管理クラス
	 */
	class EffectManager
	{
	private:
		/**
		 * エフェクトインスタンスを保持
		 */
		std::map<EffectHandle, EffectEmitter*> m_effectList;
		/**
		 * マップで参照するようにハンドル数を保持
		 */
		EffectHandle m_effectHandleCount = 0;


	private:
		EffectManager();
		~EffectManager();


	public:
		/**
		 * @brief 更新処理
		 */
		void Update();


	public:
		/**
		 * @brief エフェクト再生
		 * @param kind エフェクトの種類
		 * @param position エフェクトの座標
		 * @param rotation エフェクトの回転
		 * @param scale エフェクトのスケール
		 */
		EffectHandle PlayEffect(const EnEffectKind kind, const Vector3& position, const Quaternion& rotation, const Vector3& scale);
		/**
		 * @brief エフェクト停止
		 * @param handle エフェクトのハンドル
		 */
		void StopEffect(const EffectHandle handle);


		EffectEmitter* FindEffect(const EffectHandle handle)
		{
			auto it = m_effectList.find(handle);
			if (it != m_effectList.end()) {
				return it->second;
			}
			K2_ASSERT(false, "削除済みか追加されていないエフェクトにアクセスしようとしています。\n");
			return nullptr;
		}




		/**
		 * シングルトン用
		 */
	public:
		/**
		 * @brief インスタンスを作る
		 */
		static void Initialize()
		{
			if (m_instance == nullptr)
			{
				m_instance = new EffectManager();
			}
		}


		/**
		 * @brief インスタンスを取得
		 */
		static EffectManager& Get()
		{
			return *m_instance;
		}


		/**
		 * @brief インスタンスを破棄
		 */
		static void Finalize()
		{
			if (m_instance != nullptr)
			{
				delete m_instance;
				m_instance = nullptr;
			}
		}

	private:
		/** シングルトンインスタンス */
		static EffectManager* m_instance;
	};
}