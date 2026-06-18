/// <summary>
/// tkm・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽB
/// </summary>
/// <remarks>
/// tkm・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ3D・ｽ・ｽ・ｽf・ｽ・ｽ・ｽt・ｽH・ｽ[・ｽ}・ｽb・ｽg・ｽﾅゑｿｽ・ｽB
/// ・ｽ・ｽ・ｽﾌク・ｽ・ｽ・ｽX・ｽ・p・ｽ・ｽ・ｽ驍ｱ・ｽﾆゑｿｽtkm・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽﾆゑｿｽ・ｽﾅゑｿｽ・ｽﾜゑｿｽ・ｽB・ｽB
/// </remarks>
#pragma once

#include "geometry/BSP.h"

namespace nsK2EngineLow {

	struct LowTexture {
		std::string filePath;			// ・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽp・ｽX・ｽB
		std::unique_ptr<char[]> data;	// ・ｽ・ｽ・ｽf・ｽ[・ｽ^(dds・ｽt・ｽ@・ｽC・ｽ・ｽ)
		unsigned int dataSize;			// ・ｽf・ｽ[・ｽ^・ｽﾌサ・ｽC・ｽY・ｽB
	};
	/// <summary>
	/// tkm・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽN・ｽ・ｽ・ｽX・ｽB
	/// </summary>
	class  TkmFile : public Noncopyable {
	public:
		/// <summary>
		/// ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ
		/// </summary>
		struct  SMaterial{
			int uniqID;								// ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｬ・ｽ・ｽ・ｽ・ｽ驛・ｿｽj・ｽ[・ｽNID・ｽB
			std::string albedoMapFileName;			// ・ｽA・ｽ・ｽ・ｽx・ｽh・ｽ}・ｽb・ｽv・ｽﾌフ・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ・ｽB
			std::string normalMapFileName;			// ・ｽ@・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽﾌフ・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ・ｽB
			std::string specularMapFileName;		// ・ｽX・ｽy・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽﾌフ・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ・ｽB
			std::string reflectionMapFileName;		// ・ｽ・ｽ・ｽt・ｽ・ｽ・ｽN・ｽV・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽﾌフ・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ・ｽB
			std::string refractionMapFileName;		// ・ｽ・ｽ・ｽﾜマ・ｽb・ｽv・ｽﾌフ・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ・ｽB
			LowTexture*	albedoMap;					// ・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽ黷ｽ・ｽA・ｽ・ｽ・ｽx・ｽh・ｽ}・ｽb・ｽv・ｽﾌ撰ｿｽ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽf・ｽ[・ｽ^・ｽB(dds・ｽt・ｽ@・ｽC・ｽ・ｽ)
			LowTexture*	normalMap;					// ・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽ黷ｽ・ｽ@・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽﾌ撰ｿｽ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽf・ｽ[・ｽ^・ｽB(dds・ｽt・ｽ@・ｽC・ｽ・ｽ9
			LowTexture* specularMap;				// ・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽ黷ｽ・ｽX・ｽy・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽﾌ撰ｿｽ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽf・ｽ[・ｽ^・ｽB(dds・ｽt・ｽ@・ｽC・ｽ・ｽ)
			LowTexture*	reflectionMap;				// ・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽ黷ｽ・ｽ・ｽ・ｽt・ｽ・ｽ・ｽN・ｽV・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽﾌ撰ｿｽ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽf・ｽ[・ｽ^・ｽB(dds・ｽt・ｽ@・ｽC・ｽ・ｽ)
			LowTexture*	refractionMap;				// ・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽ黷ｽ・ｽ・ｽ・ｽﾜマ・ｽb・ｽv・ｽﾌ撰ｿｽ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽf・ｽ[・ｽ^・ｽB(dds・ｽt・ｽ@・ｽC・ｽ・ｽ)
		};
		/// <summary>
		/// ・ｽ・ｽ・ｽ_・ｽB
		/// </summary>
		/// <remarks>
		/// ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽﾒ集・ｽ・ｽ・ｽ・ｽ・ｽ・ｽA・ｽ・ｽ・ｽC・ｽg・ｽ・ｽ・ｽﾌシ・ｽF・ｽ[・ｽ_・ｽ[・ｽﾅ抵ｿｽ`・ｽ・ｽ・ｽ・ｽﾄゑｿｽ・ｽ髓ｸ・ｽ_・ｽ\・ｽ・ｽ・ｽﾌゑｿｽ・ｽﾏ更・ｽ・ｽ・ｽ・ｽK・ｽv・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		/// </remarks>
		struct SVertex {
			Vector3 pos;			//・ｽ・ｽ・ｽW・ｽB
			Vector3 normal;		//・ｽ@・ｽ・ｽ・ｽB
			Vector3 tangent;		//・ｽﾚベ・ｽN・ｽg・ｽ・ｽ・ｽB
			Vector3 binormal;		//・ｽ]・ｽx・ｽN・ｽg・ｽ・ｽ・ｽB
			Vector2 uv;			//UV・ｽ・ｽ・ｽW・ｽB
			int indices[4];			//・ｽX・ｽL・ｽ・ｽ・ｽC・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽB
			Vector4 skinWeights;	//・ｽX・ｽL・ｽ・ｽ・ｽE・ｽF・ｽC・ｽg・ｽB
		};
		/// <summary>
		/// 32・ｽr・ｽb・ｽg・ｽﾌイ・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽo・ｽb・ｽt・ｽ@・ｽB
		/// </summary>
		struct SIndexBuffer32 {
			std::vector< uint32_t > indices;	//・ｽC・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽB
		};
		/// <summary>
		/// 16・ｽr・ｽb・ｽg・ｽﾌイ・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽo・ｽb・ｽt・ｽ@・ｽB
		/// </summary>
		struct SIndexbuffer16 {
			std::vector< uint16_t > indices;	//・ｽC・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽB
		};
		/// <summary>
		/// ・ｽ・ｽ・ｽb・ｽV・ｽ・ｽ・ｽp・ｽ[・ｽc・ｽB
		/// </summary>
		struct SMesh {
			bool isFlatShading;									// ・ｽt・ｽ・ｽ・ｽb・ｽg・ｽV・ｽF・ｽ[・ｽf・ｽB・ｽ・ｽ・ｽO・ｽH
			std::vector< SMaterial > materials;					// ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽﾌ配・ｽ・ｽB
			std::vector< SVertex >	vertexBuffer;				// ・ｽ・ｽ・ｽ_・ｽo・ｽb・ｽt・ｽ@・ｽB
			std::vector<SIndexBuffer32> indexBuffer32Array;		// ・ｽC・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽo・ｽb・ｽt・ｽ@・ｽﾌ配・ｽ・ｽB・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽﾌ撰ｿｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽC・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽo・ｽb・ｽt・ｽ@・ｽﾍゑｿｽ・ｽ・ｽ・ｽB
			std::vector< SIndexbuffer16> indexBuffer16Array;
		};

		/// <summary>
		/// 3D・ｽ・ｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽh・ｽB
		/// </summary>
		/// <param name="filePath">・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽp・ｽX・ｽB</param>
		/// <param name="isOptimize">・ｽﾅ適・ｽ・ｽ・ｽt・ｽ・ｽ・ｽO・ｽB</param>
		/// <param name="isLoadTexture">
		/// ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽ・ｽH
		/// ・ｽR・ｽ・ｽ・ｽW・ｽ・ｽ・ｽ・ｽ・ｽﾌ構・ｽz・ｽﾌゑｿｽ・ｽﾟなどゑｿｽtkm・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽ・ｽﾈどとゑｿｽ・ｽ・ｽ・ｽ・ｽ・ｽA
		/// ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽ・ｽ・ｽs・ｽv・ｽﾌ場合・ｽﾉは、・ｽ・ｽ・ｽﾌ茨ｿｽ・ｽ・ｽ・ｽ・ｽfalse・ｽﾉゑｿｽ・ｽﾄゑｿｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		/// ・ｽ・ｽ・ｽ・ｽﾆ、・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽg・ｽp・ｽﾊ、・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽﾔなどゑｿｽ・ｽ甯ｸ・ｽ・ｽ・ｽ・ｽﾜゑｿｽ・ｽB
		/// </param>
		/// <param name="isOutputMsgTTY">・ｽ・ｽ・ｽb・ｽZ・ｽ[・ｽW・ｽ・ｽW・ｽ・ｽ・ｽ・ｽ・ｽo・ｽﾍデ・ｽo・ｽC・ｽX・ｽﾉ出・ｽﾍゑｿｽ・ｽ・ｽH</param>
		bool Load(const char* filePath, bool isOptimize, bool isLoadTexture = true, bool isOutputErrorCodeTTY = false);
		/// <summary>
		/// tkm・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽ・ｽﾛ托ｿｽ・ｽB
		/// </summary>
		/// <param name="filePath">・ｽﾛ托ｿｽ・ｽ・ｽﾌフ・ｽ@・ｽC・ｽ・ｽ・ｽp・ｽX・ｽB</param>
		bool Save(const char* filePath);
		/// <summary>
		/// ・ｽ・ｽ・ｽb・ｽV・ｽ・ｽ・ｽp・ｽ[・ｽc・ｽﾉ対ゑｿｽ・ｽﾄク・ｽG・ｽ・ｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽB
		/// </summary>
		/// <param name="func">・ｽN・ｽG・ｽ・ｽ・ｽﾖ撰ｿｽ</param>
		void QueryMeshParts(std::function<void(const SMesh& mesh)> func) const
		{
			for (auto& mesh : m_meshParts) {
				func(mesh);
			}
		}
		/// <summary>
		/// ・ｽ・ｽ・ｽb・ｽV・ｽ・ｽ・ｽp・ｽ[・ｽc・ｽ・ｽ・ｽ謫ｾ・ｽB
		/// </summary>
		/// <returns></returns>
		const std::vector< SMesh>& GetMeshParts() const
		{
			return m_meshParts;
		}
		/// <summary>
		/// ・ｽ・ｽ・ｽb・ｽV・ｽ・ｽ・ｽﾌ撰ｿｽ・ｽ・ｽ・ｽ謫ｾ・ｽB
		/// </summary>
		/// <returns></returns>
		int GetNumMesh() const
		{
			return (int)(m_meshParts.size());
		}
		/// <summary>
		/// 繧ｳ繝ｼ繝峨°繧峨Γ繝・す繝･繝・・繧ｿ繧堤峩謗･險ｭ螳壹☆繧具ｼ医ワ繧､繝医・繝・・蝨ｰ蠖｢縺ｪ縺ｩ謇狗ｶ壹″逕滓・逕ｨ・・
		/// </summary>
		void Build(std::vector<SMesh> meshes)
		{
			m_meshParts = std::move(meshes);
		}
	private:
		/// <summary>
		/// ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽh・ｽB
		/// </summary>
		/// <param name="fp"></param>
		/// <returns></returns>
		std::string LoadTextureFileName(FILE* fp);
		/// <summary>
		/// ・ｽC・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽo・ｽb・ｽt・ｽ@・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽh・ｽB
		/// </summary>
		template<class T>
		void LoadIndexBuffer(std::vector<T>& indexBuffer, int numIndex, FILE* fp);
		/// <summary>
		/// ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽ・ｽ・ｽ\・ｽz・ｽB
		/// </summary>
		/// <param name="tkmMat"></param>
		void BuildMaterial(SMaterial& tkmMat, FILE* fp, const char* filePath, bool isLoadTexture, bool isOutputErrorCodeTTY);
		/// <summary>
		/// ・ｽﾚベ・ｽN・ｽg・ｽ・ｽ・ｽﾆ従・ｽx・ｽN・ｽg・ｽ・ｽ・ｽ・ｽ・ｽv・ｽZ・ｽ・ｽ・ｽ・ｽB
		/// </summary>
		/// <remarks>
		/// 3dsMaxScript・ｽﾅゑｿｽ・ｽﾗゑｿｽ・ｽﾈんだろう・ｽ・ｽ・ｽﾇ、・ｽf・ｽo・ｽb・ｽO・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽﾌで搾ｿｽ・ｽﾍゑｿｽ・ｽ・ｽ・ｽ・ｽﾅゑｿｽ・ｽB
		/// </remarks>
		void BuildTangentAndBiNormal();
	private:
		/// <summary>
		/// TKM・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽﾌ最適・ｽ・ｽ・ｽB
		/// </summary>
		void Optimize();
	private:
		BSP m_bpsOnVertexPosition;				// ・ｽ・ｽ・ｽ_・ｽ・ｽ・ｽW・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ・ｽBSP・ｽc・ｽ・ｽ・ｽ[・ｽB
		std::vector< SMesh >	m_meshParts;		// ・ｽ・ｽ・ｽb・ｽV・ｽ・ｽ・ｽp・ｽ[・ｽc・ｽB
	};
}