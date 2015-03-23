#include "cmoMesh.h"


#include <algorithm>

#include "cmoLoader.h"

#include "../dx11//DX11ThinWrapper.h"
#include "../dx11/DX11GlobalDevice.h"



using namespace DirectX;
using namespace DX11ThinWrapper;


namespace {
		
	//	���f�����������ăf�B���N�g�������������o��
	std::wstring extractDirectoryPath(const wchar_t* modelPath) {
		std::wstring path = modelPath;


		auto find = path.find_last_of(L"\\");
		if (find == std::wstring::npos) {
			//	��؂蕶���͓��ގ���
			find = path.find_last_of(L"/");

			//	������Ȃ�������J�����g�f�B���N�g��
			if (find == std::wstring::npos)
				return std::wstring(L".\\");
		}

		path = std::move(path.substr(0,find + 1));

		return path;
	}


	//	�Ƃ肠�������`�⊮
	void interpolateAnimationKeyframe(
		XMFLOAT4X4* outMtx, float time,
		const std::map<float, cmo::element::Keyframe>& keyframeArray) {
		
		using namespace DirectX;

		//	�N�H�[�^�j�I���͐��K���������̂�n�����Ɓi�ǂݍ��ݎ��ɐ��K���ς݂̂͂��j

		if (keyframeArray.count(time) != 0) {
			//	���̃t���[���̃L�[�����݂��鎞
			auto kf = keyframeArray.at(time);
			XMMATRIX scaleMtx = XMMatrixScaling(kf.Scaling.x, kf.Scaling.y, kf.Scaling.z);
			XMMATRIX rotMtx   = XMMatrixRotationQuaternion(XMLoadFloat4(&kf.RotationQt));

			XMStoreFloat4x4(outMtx,scaleMtx*rotMtx);
			outMtx->_41 = kf.Translation.x;
			outMtx->_42 = kf.Translation.y;
			outMtx->_43 = kf.Translation.z;

		} else {
			//	���̃t���[���̃L�[���Ȃ����A�⊮�ōs��Z�o
			auto first  = keyframeArray.upper_bound(time);
			auto second = first--;

			//	first*(1-ratio) + second*ratio	�ŃL�[�t���[��������
			float ratio = (time - first->first) / (second->first - first->first);

			XMMATRIX scaleMtx = XMMatrixScaling(
				first->second.Scaling.x*(1 - ratio) + second->second.Scaling.x*ratio,
				first->second.Scaling.y*(1 - ratio) + second->second.Scaling.y*ratio,
				first->second.Scaling.z*(1 - ratio) + second->second.Scaling.z*ratio
				);
			XMMATRIX rotMtx = XMMatrixRotationQuaternion(
					XMQuaternionSlerp(
						XMLoadFloat4(&first->second.RotationQt),
						XMLoadFloat4(&second->second.RotationQt),
						ratio
					)
				);
			XMStoreFloat4x4(outMtx, scaleMtx*rotMtx);
			outMtx->_41 = first->second.Translation.x*(1 - ratio) + second->second.Translation.x*ratio;
			outMtx->_42 = first->second.Translation.y*(1 - ratio) + second->second.Translation.y*ratio;
			outMtx->_43 = first->second.Translation.z*(1 - ratio) + second->second.Translation.z*ratio;
		}
	}
}

//	Rigid Model
namespace cmo {
	//	���̒�`
	std::weak_ptr<ID3D11Buffer>		Mesh::s_mtxConstBufferShared;

	//------------------------
	//	protected functions
	//------------------------
	void Mesh::_InitMatrixBuffer(ID3D11Device *device) {

		//	�쐬�ς݂̋��p�o�b�t�@������Ȃ炻���p����
		_mtxConstBuffer = Mesh::s_mtxConstBufferShared.lock();

		if (_mtxConstBuffer == nullptr) {
			//	���쐬�̏ꍇ�͍��
			XMStoreFloat4x4(&_mtxWorld, XMMatrixIdentity());
			_mtxConstBuffer = d3::CreateConstantBuffer(device, &_mtxWorld, sizeof(XMFLOAT4X4), D3D11_CPU_ACCESS_WRITE);
			Mesh::s_mtxConstBufferShared = _mtxConstBuffer;
		} else {
			//	�o�b�t�@�쐬�ς݂̏ꍇ�P�ʍs��ŏ�����
			this->updateMatrix(XMMatrixIdentity());
		}
	}

	//	�R�s�[�p������Z�q
	Mesh& Mesh::operator=(const Mesh& source) {

		/******************
		 *	shallow-copy��
		 ******************/
		//	�o�b�t�@����
		this->_submeshArray		 = source._submeshArray;
		this->_vertexBufferArray = source._vertexBufferArray;
		this->_indexBufferArray  = source._indexBufferArray;

		//	�V�F�[�_�[
		this->_vertexShader = source._vertexShader;
		this->_pixelShader	= source._pixelShader;
		this->_inputLayout	= source._inputLayout;

		//	���b�V�����
		this->_extent	= source._extent;
		this->_name		= source._name;


		/******************
		*	deep-copy��
		******************/
		this->_materialArray.reserve(source._materialArray.size());
		for (auto& mat :  source._materialArray) {
			this->_materialArray.emplace_back(std::make_shared<cmo::element::Material>(*mat));
		}
		this->_materialIndexTable= source._materialIndexTable;
		


		//	�s��f�[�^���ڂ��B
		//	�f�o�C�X�̓O���[�o��������������Ă���B������
		this->_InitMatrixBuffer(dx11::AccessDX11Device());
		this->updateMatrix(
			XMLoadFloat4x4(&source._mtxWorld));


		return(*this);	
	}



	//------------------------
	//	public functions
	//------------------------
	HRESULT Mesh::load(ID3D11Device* device, const wchar_t* filePath, const wchar_t* textureDirectory) {
		
		std::wstring textureDir;
		if (textureDirectory == nullptr) {
			textureDir = extractDirectoryPath(filePath);
		} else {
			textureDir = textureDirectory;
		}


		//	���łɎ����Ă��郁�b�V�����폜
		_ClearBuffers();


		//	���b�V�����ǂݍ���ł���
		loader::BinaryReader binaryReader;
		HRESULT hr = binaryReader.importFile(filePath);
		if (FAILED(hr))
			return hr;


		auto numMesh = binaryReader.read<UINT>();

		if (*numMesh == 0) {
			OutputDebugString(TEXT("\n--------------------\nERROR: No meshes found\n--------------------\n"));
			return E_FAIL;
		}

		
//		_meshArray.resize(*numMesh);
//		_pMeshIndexTable = std::make_shared<std::unordered_map<std::wstring, unsigned int> >();
//		_pMeshIndexTable->reserve(*numMesh);

		//	���b�V����������ǂݍ���
		//	�{���̓��b�V���̐��������[�v
		for (size_t meshId = 0; meshId < *numMesh; ++meshId) {
			
//			auto& currentMesh = _meshArray[meshId];
//			auto& currentMesh = _pMesh;
//			currentMesh = std::make_shared<cmo::Mesh>();


			//	���b�V�����擾
			auto _name = loader::getString(&binaryReader);
			if (_name.empty())
				return E_FAIL;



			//	�}�e���A���ǂݍ���
			if ( ! loader::getMaterialArray(&_materialArray, &_materialIndexTable, device, textureDir.c_str(), &binaryReader))
				return E_FAIL;


			// �{�[����񂠂邩�ǂ���
			bool hasSkeleton = false;
			{
				auto bSkeleton = binaryReader.read<BYTE>();
				if (bSkeleton == nullptr)
					return E_FAIL;

			}


			//	�T�u���b�V���ǂݍ���
			_submeshArray = loader::getSubMeshArray(&binaryReader);
			if (_submeshArray.empty())
				return E_FAIL;


			// �C���f�b�N�X�o�b�t�@�ǂݍ��݂���э쐬
			_indexBufferArray = loader::getIndexBufferArray(device, &binaryReader);
			if (_indexBufferArray.empty())
				return E_FAIL;


			// ���_�o�b�t�@�ǂݍ��݂���э쐬
			_vertexBufferArray = loader::getVertexBufferArray(device, &binaryReader);
			if (_vertexBufferArray.empty())
				return E_FAIL;


			// Skinning vertex buffers
			//	�ǂݔ�΂��܂�
			loader::skipSkinningVertexData(&binaryReader);


			// ���b�V�����	Extents
			_extent = loader::getMeshInfo(&binaryReader);
			if (_extent->Radius < 0)
				return E_FAIL;

			//	�A�j���[�V�����A�ǂݔ�΂�
			if (hasSkeleton) {
				if (!loader::skipBoneAndAnimation(&binaryReader))
					return E_FAIL;
			}


			//	�Ƃ肠���������
			break;

		}	//	end of loop (*numMesh)


		//	�V�F�[�_�[�֍s�񑗂邽�߂̃o�b�t�@������
		_InitMatrixBuffer(device);


		return S_OK;
	}

	
	void Mesh::applyVS(std::shared_ptr<ID3D11VertexShader>& vertexShader, std::weak_ptr<ID3DBlob> blob, ID3D11Device *device) {

		auto p = blob.lock();
		if (p == nullptr)
			return;

		static D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"	, 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR"	, 0, DXGI_FORMAT_R8G8B8A8_UNORM		, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT		, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		_inputLayout = d3::CreateInputLayout(device, layout, _countof(layout), p.get());
		_vertexShader = vertexShader;
	}


	void Mesh::replaceMaterial(const wchar_t* dstMatName, std::shared_ptr<cmo::element::Material> &srcMaterial, const wchar_t* srcMatName) {
#if (DEBUG || _DEBUG)
		bool update = false;
#endif
		auto it = _materialIndexTable.find(dstMatName);
		if (it != _materialIndexTable.end()) {
			//	�}�e���A���X�V
			unsigned int index = _materialIndexTable[dstMatName];
			_materialArray[index] = srcMaterial;

			//	�}�e���A�����C���f�b�N�X�̃e�[�u���X�V
			_materialIndexTable.erase(it);
			_materialIndexTable.emplace(srcMatName, index);

#if (DEBUG || _DEBUG)
			update = true;
#endif	
		}
			
		
		assert(("�X�V�Ώۂ̃}�e���A����������܂���", update));

	}

	//	���[�v�Ԃ�񂷈ȊO�ɕ��@�����Ȃ��ł����R��
	void Mesh::replaceTexture(const wchar_t* dstTextureName, std::shared_ptr<ID3D11ShaderResourceView> &srcTexture, const wchar_t* srcTextureName) {
#if (DEBUG || _DEBUG)
		bool update = false;
#endif

		for (auto& mat : _materialArray) {
			for (auto& texture : mat->TextureArray) {
				if (texture.textureName == dstTextureName) {
					//	�e�N�X�`���X�V
					texture.pTexture	= srcTexture;
					texture.textureName = srcTextureName;
#if (DEBUG || _DEBUG)
					update = true;
#endif
				}
			}
		}
		

		assert(( "�X�V�Ώۂ̃e�N�X�`����������܂���",update));

	}

	void Mesh::render(ID3D11DeviceContext* context, UINT startSlot)const {

		assert(_vertexShader != nullptr);
		assert(_pixelShader != nullptr);

		//	�s��X�V
		d3::mapping(_mtxConstBuffer.get(), context, [&](D3D11_MAPPED_SUBRESOURCE resource) {
			auto param = (XMFLOAT4X4*)(resource.pData);
			XMStoreFloat4x4(param,
				XMMatrixTranspose(axis::getAxisConvertMatrix(axis::ModelCoordinate::Blender)*XMLoadFloat4x4(&_mtxWorld))
				);
		});


		//	���̓��C�A�E�g�A�`������̐ݒ�
		context->IASetInputLayout(_inputLayout.get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		//	�V�F�[�_�[�ݒ�
		context->VSSetShader(_vertexShader.get(), nullptr, 0);
		context->PSSetShader(_pixelShader.get() , nullptr, 0);

		//	���[���h�ϊ��s��ݒ�
		ID3D11Buffer * constBuffers[] = { _mtxConstBuffer.get() };
		//	�������̑������i�ݒ�Ώۂ̃o�b�t�@�̏ꏊ�j�̓V�F�[�_�[�ɉ����ĕύX����K�v����
		context->VSSetConstantBuffers(startSlot, 1, constBuffers);



		//	�`��

		// Loop over each submesh.
		for (auto& submesh : _submeshArray) {

			if (submesh->IndexBufferIndex < _indexBufferArray.size() &&
				submesh->VertexBufferIndex < _vertexBufferArray.size()) {

				//	���_�o�b�t�@�A�C���f�b�N�X�o�b�t�@�̐ݒ�
				UINT stride = sizeof(cmo::element::Vertex);
				UINT offset = 0;
				ID3D11Buffer* vb[] = { _vertexBufferArray[submesh->VertexBufferIndex].get() };
				context->IASetVertexBuffers(0, 1, vb, &stride, &offset);
				context->IASetIndexBuffer(_indexBufferArray[submesh->IndexBufferIndex].get(), IndexFormatof<cmo::IndexType>::Vertex, 0);
			}

			if (submesh->MaterialIndex < _materialArray.size()) {
				cmo::element::Material* pMaterial = _materialArray[submesh->MaterialIndex].get();

				/*********************************************************
				*	�K�v�ɉ����ă}�e���A�������V�F�[�_�[�ɑ����Ă�������
				*	material.Ambient�ȂǂȂǁB
				**********************************************************/

				for (UINT texIndex = 0; texIndex < cmo::Max_Texture; ++texIndex) {
					ID3D11ShaderResourceView* texture = pMaterial->TextureArray[texIndex].pTexture.get();

					context->PSSetShaderResources(0 + texIndex, 1, &texture);
				}

				// Draw the Mesh
				context->DrawIndexed(submesh->PrimCount * 3, submesh->StartIndex, 0);
			}
		}


	}

}	// end of namespace cmo


//	Skinning Model
namespace cmo {
	//	����
	std::weak_ptr<ID3D11Buffer>		SkinnedMesh::s_boneMtxConstBufferShared;

	//------------------------
	//	private functions
	//------------------------

	void SkinnedMesh::_InitMatrixBuffer(ID3D11Device *device) {

		//	�쐬�ς݂̋��p�o�b�t�@������Ȃ炻���p����
		_mtxConstBuffer = SkinnedMesh::s_boneMtxConstBufferShared.lock();

		if (_mtxConstBuffer == nullptr) {
			//	���쐬�̏ꍇ�͍��
			XMStoreFloat4x4(&_mtxWorld, XMMatrixIdentity());
			_mtxConstBuffer = d3::CreateConstantBuffer(device, nullptr, sizeof(XMFLOAT4X4)*cmo::Max_Bone, D3D11_CPU_ACCESS_WRITE);
			SkinnedMesh::s_boneMtxConstBufferShared = _mtxConstBuffer;
		} else {
			//	�o�b�t�@�쐬�ς݂̏ꍇ�P�ʍs��ŏ�����
			this->updateMatrix(XMMatrixIdentity());
		}
	}

	void SkinnedMesh::_UpdateBoneTransform() {
		
		float currentFrame = (_animFrame / 60.0f)*(_currentAnim->second.EndTime - _currentAnim->second.StartTime) + _currentAnim->second.StartTime;

		//	�e�{�[���̃��[�J���ϊ��s����o��
		const auto& KeyFrames = _currentAnim->second.BoneKeyframe;
		
		for (size_t boneId = 0; boneId < KeyFrames.size(); ++boneId)
			interpolateAnimationKeyframe(&_boneMtxArray[boneId],currentFrame,KeyFrames[boneId]);


		_isTransformCombined.assign((*_pBoneArray).size(), false);

		//	�e�{�[���̃��[�J���ϊ����v�Z
		for (UINT b = 0; b < (*_pBoneArray).size(); ++b) {
			_CombineBoneTransforms(b);
		}
		
		XMMATRIX worldMtx = XMLoadFloat4x4(&_mtxWorld);

		//	�t�p���~�ϊ��s��	�Ŋe�{�[���̃��[���h�ϊ��s������߂�
		for (UINT b = 0; b < (*_pBoneArray).size(); ++b) {
			XMMATRIX invPose = XMLoadFloat4x4(&(*_pBoneArray)[b].InvBindPos);
			XMMATRIX transformMtx = XMLoadFloat4x4(&_boneMtxArray[b]);


			XMStoreFloat4x4(&_boneMtxArray[b],
				XMMatrixTranspose(
					(invPose * transformMtx)*axis::getAxisConvertMatrix(axis::ModelCoordinate::Blender) *worldMtx
				)
			);
		}

	}

	void SkinnedMesh::_CombineBoneTransforms(INT currentBoneIndex) {
		cmo::element::Bone& bone = (*_pBoneArray)[currentBoneIndex];

		//	���[�g�̃{�[��
		if (bone.ParentIndex < 0) {
			//XMMATRIX currentMtx = XMLoadFloat4x4(&_boneMtxArray[currentBoneIndex]);
			//XMMATRIX parentMtx = XMLoadFloat4x4(&_mtxWorld);

			//XMMATRIX ret = currentMtx * parentMtx;

			XMStoreFloat4x4(&_boneMtxArray[currentBoneIndex], XMLoadFloat4x4(&_boneMtxArray[currentBoneIndex]));


			_isTransformCombined[currentBoneIndex] = true;
			return;
		}

		if (_isTransformCombined[currentBoneIndex] || bone.ParentIndex == (currentBoneIndex)) {
			_isTransformCombined[currentBoneIndex] = true;
			return;
		}

		//	�e�{�[�������ǂ��čċA�I�Ɍ���
		_CombineBoneTransforms(bone.ParentIndex);


		//	���[���h�ւ̕ϊ��s����Z�o
		//	cmo�̌���fbx�����s�D��̂��ߓ]�u���Ă���v�Z
		XMMATRIX currentMtx = XMMatrixTranspose(XMLoadFloat4x4(&_boneMtxArray[currentBoneIndex]));
		XMMATRIX parentMtx = XMMatrixTranspose(XMLoadFloat4x4(&_boneMtxArray[bone.ParentIndex]));
				

		XMStoreFloat4x4(&_boneMtxArray[currentBoneIndex], XMMatrixTranspose(parentMtx * currentMtx));

		_isTransformCombined[currentBoneIndex] = true;
	}

	SkinnedMesh& SkinnedMesh::operator=(const SkinnedMesh& mesh) {
		 
		/******************
		*	shallow-copy��
		******************/
		this->_skinningVertexBufferArray = mesh._skinningVertexBufferArray;
		this->_pBoneArray		= mesh._pBoneArray;
		this->_pBoneIndexTable	= mesh._pBoneIndexTable;
		this->_pAnimationTable	= mesh._pAnimationTable;


		/******************
		*	deep-copy��
		******************/
		this->_animFrame	= mesh._animFrame;
		this->_boneMtxArray = mesh._boneMtxArray;
		this->_isTransformCombined = mesh._isTransformCombined;
		this->_currentAnim = mesh._currentAnim;

		//	���N���X�̕��͂�����ɃR�s�[������C����
		Mesh::operator=(mesh);
		
		return(*this);
		
	}


	//------------------------
	//	public override functions
	//------------------------
	HRESULT SkinnedMesh::load(ID3D11Device* device, const wchar_t* filePath, const wchar_t* textureDirectory) {

		std::wstring textureDir;
		if (textureDirectory == nullptr) {
			textureDir = extractDirectoryPath(filePath);
		} else {
			textureDir = textureDirectory;
		}


		//	���łɎ����Ă��郁�b�V�����폜
		_ClearBuffers();


		//	���b�V�����ǂݍ���ł���
		loader::BinaryReader binaryReader;
		HRESULT hr = binaryReader.importFile(filePath);
		if (FAILED(hr))
			return hr;


		auto numMesh = binaryReader.read<UINT>();

		if (*numMesh == 0) {
			OutputDebugString(TEXT("\n--------------------\nERROR: No meshes found\n--------------------\n"));
			return E_FAIL;
		}


		//		_meshArray.resize(*numMesh);
		//		_pMeshIndexTable = std::make_shared<std::unordered_map<std::wstring, unsigned int> >();
		//		_pMeshIndexTable->reserve(*numMesh);

		//	���b�V����������ǂݍ���
		//	�{���̓��b�V���̐��������[�v
		for (size_t meshId = 0; meshId < *numMesh; ++meshId) {

			//			auto& currentMesh = _meshArray[meshId];
			//			auto& currentMesh = _pMesh;
			//			currentMesh = std::make_shared<cmo::Mesh>();


			//	���b�V�����擾
			auto _name = loader::getString(&binaryReader);
			if (_name.empty())
				return E_FAIL;



			//	�}�e���A���ǂݍ���
			if (!loader::getMaterialArray(&_materialArray, &_materialIndexTable, device, textureDir.c_str(), &binaryReader))
				return E_FAIL;


			// �{�[����񂠂邩�ǂ���
			bool hasSkeleton = false;
			{
				auto bSkeleton = binaryReader.read<BYTE>();
				if (bSkeleton == nullptr)
					return E_FAIL;

				hasSkeleton = ((*bSkeleton) != 0);
			}


			//	�T�u���b�V���ǂݍ���
			_submeshArray = loader::getSubMeshArray(&binaryReader);
			if (_submeshArray.empty())
				return E_FAIL;


			// �C���f�b�N�X�o�b�t�@�ǂݍ��݂���э쐬
			_indexBufferArray = loader::getIndexBufferArray(device, &binaryReader);
			if (_indexBufferArray.empty())
				return E_FAIL;


			// ���_�o�b�t�@�ǂݍ��݂���э쐬
			_vertexBufferArray = loader::getVertexBufferArray(device, &binaryReader);
			if (_vertexBufferArray.empty())
				return E_FAIL;


			// Skinning vertex buffers
			if(!loader::getSkinningVertexBufferArray(&_skinningVertexBufferArray,device,&binaryReader))
				return E_FAIL;


			// ���b�V�����	Extents
			_extent = loader::getMeshInfo(&binaryReader);
			if (_extent->Radius < 0)
				return E_FAIL;

			//	�A�j���[�V�����ƃ{�[��
			if (hasSkeleton) {
				if (!loader::getBoneAndAnimation(&_pBoneArray,&_pBoneIndexTable,&_pAnimationTable,&binaryReader))
					return E_FAIL;

				//	�f�t�H���g�̃A�j���[�V�����ݒ�
				_currentAnim = (*_pAnimationTable).begin();
				//	�{�[���p���s�񏉊���
				_boneMtxArray.resize((*_pBoneArray).size());
				_isTransformCombined.assign((*_pBoneArray).size(), false);
			} else {
				OutputDebugStringW(L"No skeltal data found in (\n");
				OutputDebugStringW(filePath);
				OutputDebugStringW(L")\n");
				assert(false);
			}


			//	�Ƃ肠���������
			break;

		}	//	end of loop (*numMesh)


		//	�V�F�[�_�[�֍s�񑗂邽�߂̃o�b�t�@������
		_InitMatrixBuffer(device);


		return S_OK;
	}


	void SkinnedMesh::applyVS(std::shared_ptr<ID3D11VertexShader>& vertexShader, std::weak_ptr<ID3DBlob> blob, ID3D11Device *device) {
		using cmo::element::SkinningVertex;

		auto p = blob.lock();
		if (p == nullptr)
			return;

		static D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL"	, 0, DXGI_FORMAT_R32G32B32_FLOAT	, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT	, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR"	, 0, DXGI_FORMAT_R8G8B8A8_UNORM		, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT		, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, IndexFormatof<SkinningVertex::IndexType>::Bone	, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT"	, 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		
		_inputLayout = d3::CreateInputLayout(device, layout, _countof(layout), p.get());
		_vertexShader = vertexShader;
	}


	void SkinnedMesh::render(ID3D11DeviceContext* context, UINT startSlot)const {

		assert(_vertexShader != nullptr);
		assert(_pixelShader != nullptr);

		//	�s��X�V
		//	�����_�L���v�`���p�̈ꎞ�ϐ�
		auto& boneMtxArray = this->_boneMtxArray;
		d3::mapping(_mtxConstBuffer.get(), context, [&](D3D11_MAPPED_SUBRESOURCE resource) {
			auto param = (XMFLOAT4X4*)(resource.pData);
			std::copy(_boneMtxArray.begin(), _boneMtxArray.end(), param);
		});


		//	���̓��C�A�E�g�A�`������̐ݒ�
		context->IASetInputLayout(_inputLayout.get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//	�V�F�[�_�[�ݒ�
		context->VSSetShader(_vertexShader.get(), nullptr, 0);
		context->PSSetShader(_pixelShader.get(), nullptr, 0);

		//	���[���h�ϊ��s��ݒ�
		ID3D11Buffer * boneBuffers[] = { _mtxConstBuffer.get() };
		//	�������̑������i�ݒ�Ώۂ̃o�b�t�@�̏ꏊ�j�̓V�F�[�_�[�ɉ����ĕύX����K�v����
		context->VSSetConstantBuffers(startSlot, 1, boneBuffers);


		//	�`��

		// Loop over each submesh.
		for (auto& submesh : _submeshArray) {

			if (submesh->IndexBufferIndex < _indexBufferArray.size() &&
				submesh->VertexBufferIndex < _vertexBufferArray.size()) {

				//	�X�L�j���O�p���_�o�b�t�@�A�C���f�b�N�X�o�b�t�@�̐ݒ�
				ID3D11Buffer* vbs[] =
				{
					_vertexBufferArray[submesh->VertexBufferIndex].get(),
					_skinningVertexBufferArray[submesh->VertexBufferIndex].get()
				};

				UINT stride[] = { sizeof(cmo::element::Vertex), sizeof(cmo::element::SkinningVertex) };
				UINT offset[] = { 0, 0 };
				context->IASetVertexBuffers(0, 2, vbs, stride, offset);
				context->IASetIndexBuffer(_indexBufferArray[submesh->IndexBufferIndex].get(), IndexFormatof<cmo::IndexType>::Vertex, 0);

			}

			if (submesh->MaterialIndex < _materialArray.size()) {
				cmo::element::Material* pMaterial = _materialArray[submesh->MaterialIndex].get();

				/*********************************************************
				*	�K�v�ɉ����ă}�e���A�������V�F�[�_�[�ɑ����Ă�������
				*	material.Ambient�ȂǂȂǁB
				**********************************************************/

				for (UINT texIndex = 0; texIndex < cmo::Max_Texture; ++texIndex) {
					ID3D11ShaderResourceView* texture = pMaterial->TextureArray[texIndex].pTexture.get();

					context->PSSetShaderResources(0 + texIndex, 1, &texture); break;
				}

				// Draw the Mesh
				context->DrawIndexed(submesh->PrimCount * 3, submesh->StartIndex, 0);
			}
		}

		// Clear the extra vertex buffer.
		ID3D11Buffer* vbs[1] = { nullptr };
		UINT stride = 0;
		UINT offset = 0;
		context->IASetVertexBuffers(1, 1, vbs, &stride, &offset);

	}

}// end of namespace cmo