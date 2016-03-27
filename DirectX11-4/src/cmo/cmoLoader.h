#pragma once

#include "cmoElement.h"
#include <d3d11.h>



//	�Q�l�\�[�X�R�[�h�F��URL����ԉ���cmodump.cpp
//	https://directxmesh.codeplex.com/wikipage?title=CMO



//	cmo�ւ̕ϊ��Q�l
//	http://masafumi.cocolog-nifty.com/masafumis_diary/2013/02/windows-storefb.html
//	�w�A�Z�b�g�֒ǉ��x�Ƃ��邪�A�K���Ƀv���W�F�N�g���Ƀt�B���^����Ă����ɓ���Ă��΂����B
//	�ǉ�������A�ȉ��ɏ]����fbx�t�@�C�����r���h�Ɋ܂߂Ă��΂���
//	http://msdn.microsoft.com/ja-jp/library/windows/apps/hh972446.aspx


// .CMO files
// UINT - Mesh count
// { [Mesh count]
//      UINT - Length of name
//      wchar_t[] - Name of mesh (if length > 0)
//      UINT - Material count
//      { [Material count]
//          UINT - Length of material name
//          wchar_t[] - Name of material (if length > 0)
//          Material structure
//          UINT - Length of pixel shader name
//          wchar_t[] - Name of pixel shader (if length > 0)
//          { [8]
//              UINT - Length of texture name
//              wchar_t[] - Name of texture (if length > 0)
//          }
//      }
//      BYTE - 1 if there is skeletal animation data present
//      UINT - SubMesh count
//      { [SubMesh count]
//          SubMesh structure
//      }
//      UINT - IB Count
//      { [IB Count]
//          UINT - Number of USHORTs in IB
//          USHORT[] - Array of indices
//      }
//      UINT - VB Count
//      { [VB Count]
//          UINT - Number of verts in VB
//          Vertex[] - Array of vertices
//      }
//      UINT - Skinning VB Count
//      { [Skinning VB Count]
//          UINT - Number of verts in Skinning VB
//          SkinningVertex[] - Array of skinning verts
//      }
//      MeshExtents structure
//      [If skeleton animation data is not present, file ends here]
//      UINT - Bone count
//      { [Bone count]
//          UINT - Length of bone name
//          wchar_t[] - Bone name (if length > 0)
//          Bone structure
//      }
//      UINT - Animation clip count
//      { [Animation clip count]
//          UINT - Length of clip name
//          wchar_t[] - Clip name (if length > 0)
//          float - Start time
//          float - End time
//          UINT - Keyframe count
//          { [Keyframe count]
//              Keyframe structure
//          }
//      }
// }

/*
*	���f�����̉摜��dds�e�N�X�`���֕ϊ����A.png.dds�݂����ɂȂ����Ⴄ���f�B���N�g�������炢���ƂɂȂ�i
*	���O�҂̓\�[�X�R�[�h���Œu��������orDDS�g��Ȃ��A��҂͊֐������ő��΃p�X�n���悤�ɂ��邩�c������
*/




namespace cmo {

	namespace loader {

		class BinaryReader{
		private :
			std::unique_ptr<uint8_t[]> _meshData;
			size_t	_dataSize;
			size_t	_usedSize;

		public :
			BinaryReader(): 
				_meshData(nullptr),
				_dataSize(0),
				_usedSize(0){}

			//!	�t�@�C���������荞��
			HRESULT importFile(const wchar_t* fileName);

			//!	���̃f�[�^��ǂݍ��݉\���ǂ���
			inline bool isValid()const{
				if (_dataSize < _usedSize) {
					//wprintf(L"ERROR: Unexpected \"End of file\" \n");
					assert(false);
					return false;
				}
				else {
					return true;
				}
			}

			//!	�w�肳�ꂽ�^�̃f�[�^����ǂݍ���
			//!	@param count : �ǂݍ��ތ�
			template <typename T>
			inline const T* read(UINT count = 1){
				
				assert(this->isValid());

				auto data = reinterpret_cast<const T*>(_meshData.get() + _usedSize);
				_usedSize += sizeof(T)* count;

#if (DEBUG || _DEBUG)
				if (!this->isValid())
					return nullptr;
#endif
				return data;
			}
		};



		//!	������擾�AusedSize��ǂݍ��񂾕������i�߂�
		inline std::wstring	getString(BinaryReader* meshData) {

			auto length = meshData->read<UINT>();
			if (!meshData->isValid())
				return std::wstring();

			std::wstring name = meshData->read<wchar_t>(*length);
			if (!meshData->isValid())
				return std::wstring();

			return	name;
		}


		//!	�}�e���A���ǂݍ���
		//	textureDirectory�̓e�N�X�`����T���f�B���N�g��
		bool getMaterialArray(std::vector<std::shared_ptr<cmo::element::Material>> *materialArray, std::unordered_map<std::wstring, unsigned int> *materialIndexTable,
			ID3D11Device* device, const wchar_t* textureDirectory, BinaryReader * meshData);


		//!	�T�u���b�V���ǂݍ���
		std::vector<std::shared_ptr<cmo::element::SubMesh>>		getSubMeshArray(BinaryReader * meshData);


		//!	�C���f�b�N�X�o�b�t�@�̓ǂݍ��݋y�э쐬
		std::vector<std::shared_ptr<ID3D11Buffer>>	getIndexBufferArray(ID3D11Device* device, BinaryReader * meshData);

		
		//!	���_�o�b�t�@�̓ǂݍ��݋y�э쐬
		std::vector<std::shared_ptr<ID3D11Buffer>>	getVertexBufferArray(ID3D11Device* device, BinaryReader * meshData);


		//!	���b�V���g�����ǂݍ���
		inline std::shared_ptr<cmo::element::MeshExtents>	getMeshInfo(BinaryReader* meshData) {
			
			auto extents = meshData->read<cmo::element::MeshExtents>();

			if (!meshData->isValid())
				return nullptr;

			//	copy-constructor�Ă�ł��
			return std::make_shared<cmo::element::MeshExtents>(*extents);
		}


		//!	�X�L�����b�V���p���_�o�b�t�@�̓ǂݍ��݂���э쐬
		bool getSkinningVertexBufferArray(std::vector<std::shared_ptr<ID3D11Buffer>>* skinningVertexBuffer, ID3D11Device* device, BinaryReader * meshData);


		//!	�X�L�����b�V���p���_�ǂݔ�΂�
		bool skipSkinningVertexData(BinaryReader * meshData);


		//!	�{�[�����y�уA�j���[�V�����擾
		bool getBoneAndAnimation(
			std::shared_ptr<std::vector<element::Bone>> *ppBoneArray,
			std::shared_ptr<std::unordered_map<std::wstring, unsigned int>> *ppBoneIndexTable,
			std::shared_ptr<std::unordered_map<std::wstring, AnimClip>> *ppAnimationTable,
			BinaryReader *meshData);

		//!	�{�[�����y�уA�j���[�V�����ǂݔ�΂�
		bool skipBoneAndAnimation(BinaryReader * meshData);

	};	//	end of namespace cmo::loader

}; // end of namespace cmo
