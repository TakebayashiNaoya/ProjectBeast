/// <summary>
/// tkm�t�@�C���B
/// </summary>
/// <remarks>
/// tkm�t�@�C����3D���f���t�H�[�}�b�g�ł��B
/// ���̃N���X�𗘗p���邱�Ƃ�tkm�t�@�C�����������Ƃ��ł��܂��B�B
/// </remarks>
#pragma once

#include "geometry/BSP.h"

namespace nsK2EngineLow {

	struct LowTexture {
		std::string filePath;			// �t�@�C���p�X�B
		std::unique_ptr<char[]> data;	// ���f�[�^(dds�t�@�C��)
		unsigned int dataSize;			// �f�[�^�̃T�C�Y�B
	};
	/// <summary>
	/// tkm�t�@�C���N���X�B
	/// </summary>
	class  TkmFile : public Noncopyable {
	public:
		/// <summary>
		/// �}�e���A��
		/// </summary>
		struct  SMaterial{
			int uniqID;								// �e�N�X�`���t�@�C��������쐬����郆�j�[�NID�B
			std::string albedoMapFileName;			// �A���x�h�}�b�v�̃t�@�C�����B
			std::string normalMapFileName;			// �@���}�b�v�̃t�@�C�����B
			std::string specularMapFileName;		// �X�y�L�����}�b�v�̃t�@�C�����B
			std::string reflectionMapFileName;		// ���t���N�V�����}�b�v�̃t�@�C�����B
			std::string refractionMapFileName;		// ���܃}�b�v�̃t�@�C�����B
			LowTexture*	albedoMap;					// ���[�h���ꂽ�A���x�h�}�b�v�̐��e�N�X�`���f�[�^�B(dds�t�@�C��)
			LowTexture*	normalMap;					// ���[�h���ꂽ�@���}�b�v�̐��e�N�X�`���f�[�^�B(dds�t�@�C��9
			LowTexture* specularMap;				// ���[�h���ꂽ�X�y�L�����}�b�v�̐��e�N�X�`���f�[�^�B(dds�t�@�C��)
			LowTexture*	reflectionMap;				// ���[�h���ꂽ���t���N�V�����}�b�v�̐��e�N�X�`���f�[�^�B(dds�t�@�C��)
			LowTexture*	refractionMap;				// ���[�h���ꂽ���܃}�b�v�̐��e�N�X�`���f�[�^�B(dds�t�@�C��)
		};
		/// <summary>
		/// ���_�B
		/// </summary>
		/// <remarks>
		/// ������ҏW������A���C�g���̃V�F�[�_�[�Œ�`����Ă��钸�_�\���̂��ύX����K�v������B
		/// </remarks>
		struct SVertex {
			Vector3 pos;			//���W�B
			Vector3 normal;		//�@���B
			Vector3 tangent;		//�ڃx�N�g���B
			Vector3 binormal;		//�]�x�N�g���B
			Vector2 uv;			//UV���W�B
			int indices[4];			//�X�L���C���f�b�N�X�B
			Vector4 skinWeights;	//�X�L���E�F�C�g�B
		};
		/// <summary>
		/// 32�r�b�g�̃C���f�b�N�X�o�b�t�@�B
		/// </summary>
		struct SIndexBuffer32 {
			std::vector< uint32_t > indices;	//�C���f�b�N�X�B
		};
		/// <summary>
		/// 16�r�b�g�̃C���f�b�N�X�o�b�t�@�B
		/// </summary>
		struct SIndexbuffer16 {
			std::vector< uint16_t > indices;	//�C���f�b�N�X�B
		};
		/// <summary>
		/// ���b�V���p�[�c�B
		/// </summary>
		struct SMesh {
			bool isFlatShading;									// �t���b�g�V�F�[�f�B���O�H
			std::vector< SMaterial > materials;					// �}�e���A���̔z��B
			std::vector< SVertex >	vertexBuffer;				// ���_�o�b�t�@�B
			std::vector<SIndexBuffer32> indexBuffer32Array;		// �C���f�b�N�X�o�b�t�@�̔z��B�}�e���A���̐��������C���f�b�N�X�o�b�t�@�͂����B
			std::vector< SIndexbuffer16> indexBuffer16Array;
		};

		/// <summary>
		/// 3D���f�������[�h�B
		/// </summary>
		/// <param name="filePath">�t�@�C���p�X�B</param>
		/// <param name="isOptimize">�œK���t���O�B</param>
		/// <param name="isLoadTexture">
		/// �e�N�X�`�������[�h����H
		/// �R���W�����̍\�z�̂��߂Ȃǂ�tkm�t�@�C�������[�h����ȂǂƂ������A
		/// �e�N�X�`�����s�v�̏ꍇ�ɂ́A���̈�����false�ɂ��Ă��������B
		/// ����ƁA�������g�p�ʁA���[�h���ԂȂǂ��팸����܂��B
		/// </param>
		/// <param name="isOutputMsgTTY">���b�Z�[�W��W�����o�̓f�o�C�X�ɏo�͂���H</param>
		bool Load(const char* filePath, bool isOptimize, bool isLoadTexture = true, bool isOutputErrorCodeTTY = false);
		/// <summary>
		/// tkm�t�@�C����ۑ��B
		/// </summary>
		/// <param name="filePath">�ۑ���̃t�@�C���p�X�B</param>
		bool Save(const char* filePath);
		/// <summary>
		/// ���b�V���p�[�c�ɑ΂��ăN�G�����s���B
		/// </summary>
		/// <param name="func">�N�G���֐�</param>
		void QueryMeshParts(std::function<void(const SMesh& mesh)> func) const
		{
			for (auto& mesh : m_meshParts) {
				func(mesh);
			}
		}
		/// <summary>
		/// ���b�V���p�[�c���擾�B
		/// </summary>
		/// <returns></returns>
		const std::vector< SMesh>& GetMeshParts() const
		{
			return m_meshParts;
		}
		/// <summary>
		/// ���b�V���̐����擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumMesh() const
		{
			return (int)(m_meshParts.size());
		}
		/// <summary>
		/// コードからメッシュデータを直接設定する（ハイトマップ地形など手続き生成用）
		/// </summary>
		void Build(std::vector<SMesh> meshes)
		{
			m_meshParts = std::move(meshes);
		}
	private:
		/// <summary>
		/// �e�N�X�`���������[�h�B
		/// </summary>
		/// <param name="fp"></param>
		/// <returns></returns>
		std::string LoadTextureFileName(FILE* fp);
		/// <summary>
		/// �C���f�b�N�X�o�b�t�@�����[�h�B
		/// </summary>
		template<class T>
		void LoadIndexBuffer(std::vector<T>& indexBuffer, int numIndex, FILE* fp);
		/// <summary>
		/// �}�e���A�����\�z�B
		/// </summary>
		/// <param name="tkmMat"></param>
		void BuildMaterial(SMaterial& tkmMat, FILE* fp, const char* filePath, bool isLoadTexture, bool isOutputErrorCodeTTY);
		/// <summary>
		/// �ڃx�N�g���Ə]�x�N�g�����v�Z����B
		/// </summary>
		/// <remarks>
		/// 3dsMaxScript�ł��ׂ��Ȃ񂾂낤���ǁA�f�o�b�O�������̂ō��͂�����ł��B
		/// </remarks>
		void BuildTangentAndBiNormal();
	private:
		/// <summary>
		/// TKM�t�@�C���̍œK���B
		/// </summary>
		void Optimize();
	private:
		BSP m_bpsOnVertexPosition;				// ���_���W���g����BSP�c���[�B
		std::vector< SMesh >	m_meshParts;		// ���b�V���p�[�c�B
	};
}