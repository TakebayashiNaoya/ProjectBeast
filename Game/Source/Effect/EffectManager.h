/**
 * @file EffectManager.h
 * @brief 必要なエフェクトファイルを読み込んだり再生したりなど管理する
 * @author 藤谷、竹林
 */
#pragma once
#include "Types.h"


namespace nsBeastEngine
{
	class Frustum;
	class BeastEffectEmitter;
}


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
		 * @brief エフェクトのインスタンスと種別をまとめて保持する構造体
		 */
		struct EffectEntry
		{
			/** エフェクトエミッター */
			nsBeastEngine::BeastEffectEmitter* emitter = nullptr;
			/** エフェクトの種類（バウンディング半径の参照に使用） */
			EnEffectKind kind = EnEffectKind::None;
		};

		/** エフェクトインスタンスを保持 */
		std::map<EffectHandle, EffectEntry> m_effectList;
		/** マップで参照するようにハンドル数を保持 */
		EffectHandle m_effectHandleCount = 0;


	private:
		EffectManager();
		~EffectManager();


	public:
		/**
		 * @brief 更新処理
		 * @details
		 *   再生が終了したエフェクトを m_effectList から除外し、
		 *   フラスタム判定によりエフェクトの描画可否を切り替える。
		 * @param frustum カリングに使用する視錐台
		 */
		void Update(const nsBeastEngine::Frustum& frustum);


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

		/**
		 * @brief ハンドルからエフェクトを取得する
		 * @param handle エフェクトのハンドル
		 * @return エフェクトのポインタ（存在しない場合はnullptr）
		 */
		nsBeastEngine::BeastEffectEmitter* FindEffect(const EffectHandle handle)
		{
			auto it = m_effectList.find(handle);
			if (it != m_effectList.end()) {
				return it->second.emitter;
			}
			return nullptr;
		}

		/**
		 * @brief ハンドルをm_effectListから除外する
		 * @details BeastEffectEmitterが自己削除される際に呼び出す
		 * @param handle 除外するエフェクトのハンドル
		 */
		void UnregisterEffect(const EffectHandle handle);


		/**
		 * シングルトン用
		 */
	public:
		/**
		 * @brief インスタンスを作る
		 */
		static void CreateInstance()
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
		static void DestroyInstance()
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