/**
 * @file NoiseManager.h
 * @brief 音（ノイズ）イベントを管理し、エネミーの検知処理を補助するマネージャー
 * @author 竹林
 */
#pragma once


namespace app
{
	/** 音の種類 */
	enum class EnNoiseType
	{
		Sneak,
		Dash,
		Slide,
		Fall,
		ClumsyCRY,
		Custom
	};


	/** 基準となるパラメータのセット */
	struct NoiseParameter
	{
		float volume;			/** 音の大きさ */
		float falloffRatio;		/** 音の減衰率 */
		float range;			/* 音が届く最大距離 */
	};


	/** 発生した音のイベントデータ */
	struct NoiseEvent
	{
		Vector3 position;		/* 音源の位置 */
		float	volume;			/* 音の大きさ */
		float	falloffRatio;	/* 音の減衰率 */
		float	range;			/* 音が届く最大距離 */
	};


	/**
	 * 音（ノイズ）イベントを管理し、エネミーの検知処理を補助するマネージャー
	 */
	class NoiseManager
	{
	public:
		/**
		 * @brief 音イベントを登録
		 * @param position	音源の位置
		 * @param type		音の種類
		 *
		 */
		void AddNoise(const Vector3& position, EnNoiseType type);


		/**
		 * @brief 詳細な数値を直接指定して登録
		 * @param position		音源の位置
		 * @param volume		音の大きさ
		 * @param falloffRatio	音の減衰率
		 * @param range			音が届く最大距離
		 */
		void AddNoise(const Vector3& position, float volume, float falloffRatio, float range);


		/**
		 * @brief 指定座標で聞こえる音量の合計と、最も大きい音源の座標を計算
		 * @param listenerPos			リスナーの位置
		 * @param outLoudestPosition	最も大きい音源の座標の出力先
		 */
		float CalculateTotalNoiseAt(const Vector3& listenerPos, Vector3& outLoudestPosition) const;


		/**
		 * @brief 登録されている音イベントを全てクリア
		 */
		void ClearNoises();


	private:
		NoiseManager() = default;
		~NoiseManager() = default;

		/* 音の種類に対応する基準となるパラメータを取得 */
		NoiseParameter GetDefaultParameter(EnNoiseType type) const;

		/** 発生した音のイベントのリスト */
		std::vector<NoiseEvent> m_noises;




		//============================================//
		// シングルトン関連
		//============================================//

	public:
		/**
		 * インスタンスを作成
		 */
		static void CreateInstance()
		{
			if (m_instance == nullptr)
			{
				m_instance = new NoiseManager();
			}
		}
		/**
		 * インスタンスを取得
		 */
		static NoiseManager& GetInstance()
		{
			return *m_instance;
		}
		/**
		 * インスタンスを削除
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
		static NoiseManager* m_instance;
	};
}