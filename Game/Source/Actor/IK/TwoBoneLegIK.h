/**
 * @file TwoBoneLegIK.h
 * @brief 2ボーン解析式IK（脚1本分）
 * @author 立山
 */
#pragma once

namespace nsK2EngineLow
{
	class Skeleton;
}

namespace app
{
	namespace actor
	{
		namespace ik
		{
			/**
			 * @brief 脚1本分のIK設定
			 */
			struct LegIKChain
			{
				/** 股関節のボーン番号 */
				int hipBoneNo = -1;
				/** 膝（前足なら肘に相当）のボーン番号 */
				int kneeBoneNo = -1;
				/** 足首・足先のボーン番号 */
				int footBoneNo = -1;

				/**
				 * @brief 膝を曲げたい方向のヒント（股関節のローカル空間、単位ベクトルでなくてOK）
				 * @details 後ろ足（膝が前に曲がる）なら (0,-1, 1) 寄り、
				 *          前足＝肘（後ろに曲がる）なら (0,-1,-1) 寄り、など
				 *          実際にゲーム内で見ながら調整してください。
				 *          曲がる方向が逆だった場合はまず bendSign を反転してみてください。
				 */
				Vector3 poleHintLocal = Vector3(0.0f, -1.0f, 1.0f);

				/** 曲げ方向が逆になってしまう場合は -1.0f にする */
				float bendSign = 1.0f;

				/** 足の中心から接地面までのオフセット（足裏・肉球の厚み分など） */
				float footGroundOffset = 0.0f;

				/** L1+L2に対する最大到達距離の割合（伸びきり防止。1.0未満推奨） */
				float maxReachRatio = 0.98f;

				/**
				 * @brief 足首（foot）より先の追従ボーン番号（つま先など）
				 * @details IKでは回転させず、足首が動いた分だけ同じ平行移動を適用する。
				 */
				std::vector<int> footChildBoneNos;

				/** 股関節～膝の間のツイスト補助ボーン番号 */
				std::vector<int> hipToKneeMidBoneNos;
				/** 膝～足首の間のツイスト補助ボーン番号 */
				std::vector<int> kneeToFootMidBoneNos;
			};

			/**
			 * @brief 脚1本分の2ボーンIKを解いて、Skeletonのボーン行列に直接反映する
			 * @details
			 *  Skeleton::Update() が計算し終えたあとに呼び出してください。
			 * @param skeleton         対象のスケルトン（Skeleton::Update()実行済みのもの）
			 * @param chain            脚の設定
			 * @param targetWorldPos   足先を置きたいワールド座標
			 * @return 解決できればtrue（ボーン番号が不正な場合などはfalse）
			 */
			bool SolveTwoBoneLegIK(nsK2EngineLow::Skeleton* skeleton, const LegIKChain& chain, const Vector3& targetWorldPos);
		}
	}
}
