#pragma once

#include "tkFile/TkmFile.h"

namespace nsK2EngineLow {
	/// <summary>
	/// ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽB
	/// </summary>
	class Material : public Noncopyable {
	public:
		/// <summary>
		/// tkm・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽﾌマ・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽ・ｽｩら初・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		/// </summary>
		/// <param name="tkmMat">tkm・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ</param>
		void InitFromTkmMaterila(
			const TkmFile::SMaterial& tkmMat,
			const char* fxFilePath,
			const char* vsEntryPointFunc,
			const char* vsSkinEntriyPointFunc,
			const char* psEntryPointFunc,
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			int numSrv,
			int numCbv,
			UINT offsetInDescriptorsFromTableStartCB,
			UINT offsetInDescriptorsFromTableStartSRV,
			AlphaBlendMode alphaBlendMode,
			bool isDepthWrite,
			bool isDepthTest,
			D3D12_CULL_MODE cullMode
		);
		/// <summary>
		/// ・ｽ・ｽ・ｽ・ｽ・ｽ_・ｽ・ｽ・ｽ・ｽ・ｽO・ｽ・ｽ・ｽJ・ｽn・ｽ・ｽ・ｽ・ｽﾆゑｿｽ・ｽﾉ呼び出・ｽ・ｽ・ｽﾖ撰ｿｽ・ｽB
		/// </summary>
		/// <param name="rc">・ｽ・ｽ・ｽ・ｽ・ｽ_・ｽ・ｽ・ｽ・ｽ・ｽO・ｽR・ｽ・ｽ・ｽe・ｽL・ｽX・ｽg</param>
		/// <param name="hasSkin">・ｽX・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ驍ｩ・ｽﾇゑｿｽ・ｽ・ｽ・ｽﾌフ・ｽ・ｽ・ｽO</param>
		void BeginRender(RenderContext& rc, int hasSkin);

		/// <summary>
		/// ・ｽA・ｽ・ｽ・ｽx・ｽh・ｽ}・ｽb・ｽv・ｽ・ｽ・ｽ謫ｾ・ｽB
		/// </summary>
		/// <returns></returns>
		Texture& GetAlbedoMap()
		{
			return *m_albedoMap;
		}
		/// <summary>
		/// ・ｽ@・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽ・ｽ・ｽ謫ｾ・ｽB
		/// </summary>
		/// <returns></returns>
		Texture& GetNormalMap()
		{
			return *m_normalMap;
		}
		/// <summary>
		/// ・ｽX・ｽy・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽ・ｽ・ｽ謫ｾ・ｽB
		/// </summary>
		/// <returns></returns>
		Texture& GetSpecularMap()
		{
			return *m_specularMap;
		}
		/// <summary>
		/// ・ｽ・ｽ・ｽﾋマ・ｽb・ｽv・ｽ・ｽ・ｽ謫ｾ・ｽB
		/// </summary>
		/// <returns></returns>
		Texture& GetReflectionMap()
		{
			return *m_reflectionMap;
		}
		/// <summary>
		/// ・ｽ・ｽ・ｽﾜマ・ｽb・ｽv・ｽ・ｽ・ｽ謫ｾ・ｽB
		/// </summary>
		/// <returns></returns>
		Texture& GetRefractionMap()
		{
			return *m_refractionMap;
		}
		/// <summary>
		/// ・ｽ關費ｿｽo・ｽb・ｽt・ｽ@・ｽ・ｽ・ｽ謫ｾ・ｽB
		/// </summary>
		/// <returns></returns>
		ConstantBuffer& GetConstantBuffer()
		{
			return m_constantBuffer;
		}
		
	private:
		/// <summary>
		/// ・ｽp・ｽC・ｽv・ｽ・ｽ・ｽC・ｽ・ｽ・ｽX・ｽe・ｽ[・ｽg・ｽﾌ擾ｿｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		/// </summary>
		void InitPipelineState(
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			AlphaBlendMode alphaBlendMode,
			bool isDepthWrite,
			bool isDepthTest,
			D3D12_CULL_MODE cullMode
		);
		/// <summary>
		/// ・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾌ擾ｿｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		/// </summary>
		/// <param name="fxFilePath">fx・ｽt・ｽ@・ｽC・ｽ・ｽ・ｽﾌフ・ｽ@・ｽC・ｽ・ｽ・ｽp・ｽX</param>
		/// <param name="vsEntryPointFunc">・ｽ・ｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾌエ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ[・ｽ|・ｽC・ｽ・ｽ・ｽg・ｽﾌ関撰ｿｽ・ｽ・ｽ</param>
		/// <param name="vsEntryPointFunc">・ｽX・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽp・ｽﾌ抵ｿｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾌエ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ[・ｽ|・ｽC・ｽ・ｽ・ｽg・ｽﾌ関撰ｿｽ・ｽ・ｽ</param>
		/// <param name="psEntryPointFunc">・ｽs・ｽN・ｽZ・ｽ・ｽ・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾌエ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ[・ｽ|・ｽC・ｽ・ｽ・ｽg・ｽﾌ関撰ｿｽ・ｽ・ｽ</param>
		void InitShaders(
			const char* fxFilePath,
			const char* vsEntryPointFunc,
			const char* vsSkinEntriyPointFunc,
			const char* psEntryPointFunc);
		/// <summary>
		/// ・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		/// </summary>
		/// <param name="tkmMat"></param>
		void InitTexture(const TkmFile::SMaterial& tkmMat);
	public:
		/// <summary>
		/// ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽﾌ擾ｿｽZ・ｽJ・ｽ・ｽ・ｽ[・ｽ・ｽ・ｽﾝ定す・ｽ・ｽB
		/// </summary>
		/// <param name="mulColor">・ｽZ・ｽJ・ｽ・ｽ・ｽ[(RGBA, 1.0f=・ｽﾏ更・ｽﾈゑｿｽ)</param>
		void SetMulColor(const Vector4& mulColor)
		{
			m_materialParam.mulColor = mulColor;
			m_constantBuffer.CopyToVRAM(m_materialParam);
		}

	private:
		/// <summary>
		/// ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽp・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽ^・ｽB
		/// </summary>
		struct SMaterialParam {
			int hasNormalMap;	//・ｽ@・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽ・ｽﾛ趣ｿｽ・ｽ・ｽ・ｽﾄゑｿｽ・ｽ驍ｩ・ｽﾇゑｿｽ・ｽ・ｽ・ｽﾌフ・ｽ・ｽ・ｽO・ｽB
			int hasSpecMap;		//・ｽX・ｽy・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽ・ｽﾛ趣ｿｽ・ｽ・ｽ・ｽﾄゑｿｽ・ｽ驍ｩ・ｽﾇゑｿｽ・ｽ・ｽ・ｽﾌフ・ｽ・ｽ・ｽO・ｽB
			int pad0;			// ・ｽA・ｽ・ｽ・ｽC・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽg・ｽp・ｽB
			int pad1;			// ・ｽA・ｽ・ｽ・ｽC・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽg・ｽp・ｽB
			Vector4 mulColor;	// ・ｽ・ｽZ・ｽJ・ｽ・ｽ・ｽ[(RGBA)・ｽB
		};
		SMaterialParam m_materialParam;					//・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ・ｽp・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽ^・ｽiCopyToVRAM・ｽp・ｽﾉ保托ｿｽj・ｽB
		Texture* m_albedoMap;						//・ｽA・ｽ・ｽ・ｽx・ｽh・ｽ}・ｽb・ｽv・ｽB
		Texture* m_normalMap;						//・ｽ@・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽB
		Texture* m_specularMap;						//・ｽX・ｽy・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽB
		Texture* m_reflectionMap;					//・ｽ・ｽ・ｽt・ｽ・ｽ・ｽN・ｽV・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽb・ｽv・ｽB
		Texture* m_refractionMap;					//・ｽ・ｽ・ｽﾜマ・ｽb・ｽv・ｽB

		ConstantBuffer m_constantBuffer;				//・ｽ關費ｿｽo・ｽb・ｽt・ｽ@・ｽB
		RootSignature m_rootSignature;					//・ｽ・ｽ・ｽ[・ｽg・ｽV・ｽO・ｽl・ｽ`・ｽ・ｽ・ｽB
		PipelineState m_nonSkinModelPipelineState;		//・ｽX・ｽL・ｽ・ｽ・ｽﾈゑｿｽ・ｽ・ｽ・ｽf・ｽ・ｽ・ｽp・ｽﾌパ・ｽC・ｽv・ｽ・ｽ・ｽC・ｽ・ｽ・ｽX・ｽe・ｽ[・ｽg・ｽB
		PipelineState m_skinModelPipelineState;			//・ｽX・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ閭ゑｿｽf・ｽ・ｽ・ｽp・ｽﾌパ・ｽC・ｽv・ｽ・ｽ・ｽC・ｽ・ｽ・ｽX・ｽe・ｽ[・ｽg・ｽB
		PipelineState m_transSkinModelPipelineState;	//・ｽX・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ閭ゑｿｽf・ｽ・ｽ・ｽp・ｽﾌパ・ｽC・ｽv・ｽ・ｽ・ｽC・ｽ・ｽ・ｽX・ｽe・ｽ[・ｽg(・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ)・ｽB
		PipelineState m_transNonSkinModelPipelineState;	//・ｽX・ｽL・ｽ・ｽ・ｽﾈゑｿｽ・ｽ・ｽ・ｽf・ｽ・ｽ・ｽp・ｽﾌパ・ｽC・ｽv・ｽ・ｽ・ｽC・ｽ・ｽ・ｽX・ｽe・ｽ[・ｽg(・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ}・ｽe・ｽ・ｽ・ｽA・ｽ・ｽ)・ｽB
		Shader* m_vsNonSkinModel = nullptr;				//・ｽX・ｽL・ｽ・ｽ・ｽﾈゑｿｽ・ｽ・ｽ・ｽf・ｽ・ｽ・ｽp・ｽﾌ抵ｿｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽB
		Shader* m_vsSkinModel = nullptr;				//・ｽX・ｽL・ｽ・ｽ・ｽ・ｽ・ｽ閭ゑｿｽf・ｽ・ｽ・ｽp・ｽﾌ抵ｿｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽB
		Shader* m_psModel = nullptr;					//・ｽ・ｽ・ｽf・ｽ・ｽ・ｽp・ｽﾌピ・ｽN・ｽZ・ｽ・ｽ・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽB
	};
}

