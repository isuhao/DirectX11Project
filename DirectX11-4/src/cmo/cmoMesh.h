#pragma once

#include "cmoElement.h"
#include "AxisConvert.h"
#include <algorithm>


//	�X�L�����b�V���Q�l
//	https://code.msdn.microsoft.com/windowsapps/Visual-Studio-3D-Starter-455a15f1/view/Discussions

namespace cmo {

	//!	���ʂ̃��b�V�����f��
	class Mesh {
	protected:

		//!	�`��p�o�b�t�@��
		std::vector<std::shared_ptr<element::SubMesh>>	_submeshArray;
		std::vector<std::shared_ptr<element::Material>>	_materialArray;
		std::vector<std::shared_ptr<ID3D11Buffer>>		_vertexBufferArray;
		std::vector<std::shared_ptr<ID3D11Buffer>>		_indexBufferArray;


		//!	�}�e���A���ւ̃A�N�Z�X�p
		std::unordered_map<std::wstring, unsigned int>	_materialIndexTable;

		//!	�V�F�[�_�[����
		std::shared_ptr<ID3D11VertexShader>	_vertexShader;
		std::shared_ptr<ID3D11PixelShader>	_pixelShader;
		std::shared_ptr<ID3D11InputLayout>	_inputLayout;

		//!	���b�V����Bounding Box�y��Bounding Circle
		std::shared_ptr<element::MeshExtents>			_extent;
		//!	���b�V�����E�E�E�v��H
		std::wstring				_name;

		//!	���݂̍s��ۑ��p�A�R�s�[���Ɏg�p����
		DirectX::XMFLOAT4X4					_mtxWorld;


		//!	���[���h���W�ϊ��s����V�F�[�_�[�ɑ��邽�߂̒萔�o�b�t�@
		std::shared_ptr<ID3D11Buffer>		_mtxConstBuffer;


		//!	�萔�o�b�t�@�̓��b�V���Ԃŋ��p���邽�߂���̕ۑ��p�B���Ƃł��������l���đg�݂Ȃ���
		static std::weak_ptr<ID3D11Buffer>		s_mtxConstBufferShared;


		//!	�o�b�t�@�ޏ�����
		virtual void _ClearBuffers() {
			_submeshArray.clear();
			_materialArray.clear();
			_vertexBufferArray.clear();
			_indexBufferArray.clear();

			_materialIndexTable.clear();

			_extent.reset();
			_name.clear();

		};

		//!	�s��p�o�b�t�@�ɏ����l�i�P�ʍs��j�ݒ�
		virtual void _InitMatrixBuffer(ID3D11Device *device);

	public:
		//!	�R���X�g���N�^
		Mesh() :
			_mtxWorld(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1){}

		//!	�f�X�g���N�^
		~Mesh() {}

		//!	���b�V������������[�h�A���[�h�ς݂̏ꍇ�͏㏑��
		/**
		 * @param device			�f�o�C�X�ւ̃|�C���^
		 * @param filePath			�ǂݍ��� .cmo���f���t�@�C���̃p�X
		 * @param textureDirecctory	�e�N�X�`����T���f�B���N�g�����A�w�肵�Ȃ��ꍇ�̓��f���Ɠ����K�w����T��
		 */
		virtual HRESULT load(ID3D11Device* device, const wchar_t* filePath, const wchar_t* textureDirectory = nullptr);
		

		//!	�}�e���A���擾
		const std::shared_ptr<cmo::element::Material>	getMaterial(const wchar_t* matName) const {
			if (_materialIndexTable.count(matName) != 0) {
				return(_materialArray[_materialIndexTable(matName)]);
			}
			

			return(nullptr);
		}

		
		//!	�`�掞�Ɏg�����_�V�F�[�_�[�̐ݒ�B
		//!	���̓��C�A�E�g�������ɍX�V����
		virtual void applyVS(std::shared_ptr<ID3D11VertexShader>& vertexShader, std::weak_ptr<ID3DBlob> blob, ID3D11Device *device);

		
		//!	�`�掞�Ɏg���s�N�Z���V�F�[�_�[�̐ݒ�
		void applyPS(std::shared_ptr<ID3D11PixelShader>& pixelShader) {
			_pixelShader = pixelShader;
		}


		//!	�}�e���A����S�Ēu��������
		/**
		 * @param dstMatName	�u�������Ώۂ̃}�e���A����
		 * @param srcMaterial	�V���ɐݒ肷��}�e���A��
		 * @param srcMatName	�V���ɐݒ肷��}�e���A���̖��O
		 */
		void replaceMaterial(const wchar_t* dstMatName, std::shared_ptr<cmo::element::Material> &srcMaterial, const wchar_t* srcMatName);
		

		//!	�e�N�X�`����S�Ēu��������
		/**
		 * @param dstTextureName	�u�������Ώۂ̃e�N�X�`����
		 * @param srcTexture		�V���ɐݒ肷��e�N�X�`��
		 * @param srcTextureName	�V���ɐݒ肷��e�N�X�`���̖��O
		 */
		void replaceTexture(const wchar_t* dstTextureName, std::shared_ptr<ID3D11ShaderResourceView> &srcTexture, const wchar_t* srcTextureName);
		

		//! ���[���h�ϊ��s��̐ݒ�
		virtual void updateMatrix(const DirectX::XMMATRIX& mtxWorld) {
			//	���݂̏�ԕۑ��i�R�s�[���鎞�p�j
			XMStoreFloat4x4(&_mtxWorld,mtxWorld);
		}


		//! �`��
		/**
		 *	@param context		�R���e�L�X�g�ւ̃|�C���^
		 *	@param startSlot	���[���h�ϊ��s���ݒ肷��A�V�F�[�_�[���̃o�b�t�@�X���b�gNo
		 */
		virtual void render(ID3D11DeviceContext* context, UINT startSlot)const;



	public:
		/**
		 *	���f���R�s�[�p�̃R�s�[�R���X�g���N�^�A������Z�q
		 */
		Mesh(const Mesh& mesh) {
			//	������Z�q�̎g���܂킵
			*this = mesh;
		}
		Mesh& operator=(const Mesh& mesh);

		//!	�S�Ẵ��b�V����z��Ƃ��ēǂݍ��� static�֐��B
		/**
		* @param device				�f�o�C�X�ւ̃|�C���^
		* @param filePath			�ǂݍ��� .cmo���f���t�@�C���̃p�X
		* @param textureDirecctory	�e�N�X�`����T���f�B���N�g�����A�w�肵�Ȃ��ꍇ�̓��f���Ɠ����K�w����T��
		*/
		static std::vector<cmo::Mesh> LoadMeshArray(ID3D11Device* device, const wchar_t* filePath, const wchar_t* textureDirectory = nullptr);

	};	// end of class Mesh { ...



	//!	�X�L�����b�V�����f��
	class SkinnedMesh : public Mesh {
	protected:
		//!	�A�j���[�V�������݃t���[��(0.0~60.0)
		float _animFrame;

		//!	�X�L�����b�V���p
		std::vector<std::shared_ptr<ID3D11Buffer>>	_skinningVertexBufferArray;
		//!	�{�[���̃��[���h�ϊ��s��
		std::vector<DirectX::XMFLOAT4X4>	_boneMtxArray;
		//!	�ϊ��s�񂪕ύX�ς݂��ǂ���
		std::vector<bool>					_isTransformCombined;
		//!	�{�[�����B���p�\���\�[�X�̂��߃|�C���^�ŊǗ�
		std::shared_ptr<std::vector<element::Bone>>			_pBoneArray;
		//!	�{�[�������z��Y���e�[�u���B���������p�ł�����̂Ȃ̂Ń|�C���^
		std::shared_ptr<std::unordered_map<std::wstring, unsigned int>>	_pBoneIndexTable;
		//!	�A�j���[�V������->�L�[�t���[�����B��������������p�ł�����̂Ȃ̂Ń|�C���^
		std::shared_ptr<std::unordered_map<std::wstring, AnimClip>>		_pAnimationTable;
		//!	���݃A�j���[�V����
		std::unordered_map<std::wstring, AnimClip>::iterator	_currentAnim;
		

		//!	�{�[���s��p�萔�o�b�t�@�B�X�L�j���O���b�V���Ԃŋ��p�B
		static std::weak_ptr<ID3D11Buffer>		s_boneMtxConstBufferShared;



		//!	�s��p�o�b�t�@�ɏ����l�i�P�ʍs��j�ݒ�
		void _InitMatrixBuffer(ID3D11Device *device) override;

		//!	�{�[���s������ׂčX�V
		void _UpdateBoneTransform();

		//! �ċA�I�Ƀ{�[���ϊ���K�p
		void _CombineBoneTransforms(INT currentBoneIndex);

		//!	�o�b�t�@�N���A
		void _ClearBuffers()override {
			Mesh::_ClearBuffers();

			_skinningVertexBufferArray.clear();
			_boneMtxArray.clear();
			_isTransformCombined.clear();
			_pBoneArray.reset();
			_pBoneIndexTable.reset();
			_pAnimationTable.reset();
			_currentAnim = decltype(_currentAnim)();
		}


	public:
		//!	�R���X�g���N�^
		SkinnedMesh() :_animFrame(0.0f){
		}
		//!	�f�X�g���N�^
		~SkinnedMesh() {}
		
		//!	�{�[�������烏�[���h�ϊ��s��擾
		//!	�s�D��s���Ԃ�
		void getBoneMatrix( DirectX::XMFLOAT4X4* pMatrix, const wchar_t* boneName)const {
			using namespace DirectX;
			if (_pBoneIndexTable->count(boneName) != 0) {
				XMStoreFloat4x4(pMatrix,XMMatrixTranspose(XMLoadFloat4x4(&_boneMtxArray[_pBoneIndexTable->at(boneName)])));
			}
		}


		//!	�g�p����A�j���[�V������ݒ�
		void setAnimation(const wchar_t* animName) {
			if (_pAnimationTable->count(animName) != 0) {
				_currentAnim = _pAnimationTable->find(animName);
			}
		}

		
		//!	�A�j���[�V�������ԍX�V
		/**
		* @param frame	�A�j���[�V�����̃t���[���ݒ�A0~60�͈̔͂Őݒ�
		*/
		void setAnimationFrame(float frame) {
			_animFrame = std::min(std::max(frame, 0.0f), 60.0f);
		}
		
		
	public:
		/*******************
		 * override methods
		 *******************/

		//!	���b�V������������[�h�A���[�h�ς݂̏ꍇ�͏㏑���B
		/**
		* @param device				�f�o�C�X�ւ̃|�C���^
		* @param filePath			�ǂݍ��� .cmo���f���t�@�C���̃p�X
		* @param textureDirecctory	�e�N�X�`����T���f�B���N�g�����A�w�肵�Ȃ��ꍇ�̓��f���Ɠ����K�w����T��
		*/
		HRESULT load(ID3D11Device* device, const wchar_t* filePath, const wchar_t* textureDirectory = nullptr) override;
		

		//!	�`�掞�Ɏg�����_�V�F�[�_�[�̐ݒ�B
		//!	���̓��C�A�E�g�������ɍX�V����
		virtual void applyVS(std::shared_ptr<ID3D11VertexShader>& vertexShader, std::weak_ptr<ID3DBlob> blob, ID3D11Device *device) override;


		//! ���[���h�ϊ��s��̐ݒ�
		virtual void updateMatrix(const DirectX::XMMATRIX& mtxWorld)override{
			//	���[�g�̏�ԕۑ�
			Mesh::updateMatrix(mtxWorld);
			//	�{�[���ϊ�
			_UpdateBoneTransform();
		}


		//! �`��
		/**
		*	@param context		�R���e�L�X�g�ւ̃|�C���^
		*	@param startSlot	���[���h�ϊ��s���ݒ肷��A�V�F�[�_�[���̃o�b�t�@�X���b�gNo
		*/
		virtual void render(ID3D11DeviceContext* context, UINT startSlot)const override;



	public:
		/*******************
		* �R�s�[�R���X�g���N�^�A������Z�q
		*******************/
		SkinnedMesh(const SkinnedMesh& mesh) {
			//	������Z�q�g���܂킵
			*this = mesh;
		}
		SkinnedMesh& operator=(const SkinnedMesh& mesh);


		//!	�S�Ẵ��b�V����z��Ƃ��ēǂݍ���
		/**
		* @param device				�f�o�C�X�ւ̃|�C���^
		* @param filePath			�ǂݍ��� .cmo���f���t�@�C���̃p�X
		* @param textureDirecctory	�e�N�X�`����T���f�B���N�g�����A�w�肵�Ȃ��ꍇ�̓��f���Ɠ����K�w����T��
		*/
		static std::vector<cmo::SkinnedMesh> LoadMeshArray(ID3D11Device* device, const wchar_t* filePath, const wchar_t* textureDirectory = nullptr);


	};	// end of class SkinnedMesh { ...


};// end of namespace cmo