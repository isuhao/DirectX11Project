#pragma once



#include <memory>
#include <vector>
#include <unordered_map>
#include <map>
#include <string>

#include "../DXGIFormatDetection.h"

#include <DirectXMath.h>
#include <d3d11.h>



namespace cmo {
	
	//	���f���̃{�[���ő吔�A�V�F�[�_�[���ƈ�v�����ăl
	const UINT Max_Bone = 32;


	//	cmo���ł̋K��l�A�ύX�����
	const UINT Max_Texture = 8;
	const UINT Num_Bone_Influences = 4;
	using IndexType = uint16_t;
	//	�܂��A�{�[����e�N�X�`�����ӂ�͂��ׂ�wchar�ŕԂ��Ă���̂Œ���


	namespace element {


		struct Material {
			struct TextureData {
				std::shared_ptr<ID3D11ShaderResourceView>	pTexture;
				std::wstring	textureName;
			};

			//!	���O�̓��b�V������	���O���z��Y����map���̂Ŗ�����

			DirectX::XMFLOAT4   Ambient;
			DirectX::XMFLOAT4   Diffuse;
			DirectX::XMFLOAT4   Specular;		//�F,RGBA��0.0~1.0��4��
			float               SpecularPower;
			DirectX::XMFLOAT4   Emissive;
			DirectX::XMFLOAT4X4	UVTransform;

			TextureData	TextureArray[Max_Texture];
		};

		//!	������Bounding Box�y��Bounding Circle	�̏��
		struct MeshExtents {
			MeshExtents() :
				CenterX(0), CenterY(0), CenterZ(0),
				Radius(-1),
				MinX(0), MinY(0), MinZ(0),
				MaxX(0), MaxY(0), MaxZ(0)
			{}

			float CenterX, CenterY, CenterZ;
			float Radius;

			float MinX, MinY, MinZ;
			float MaxX, MaxY, MaxZ;
		};

		struct SubMesh {
			UINT MaterialIndex;
			UINT IndexBufferIndex;
			UINT VertexBufferIndex;
			UINT StartIndex;
			UINT PrimCount;
		};


		//!	���̃��f���̒��_
		struct Vertex {
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT4 Tangent;
			UINT color;
			DirectX::XMFLOAT2 TextureCoordinates;

		};

		//!	�X�L�����b�V�����郂�f���̒��_�̒ǉ����
		//!	�ǂݍ��݂̎��Ƃ͈Ⴄ�{�[���C���f�b�N�X�̌^��p����B�o�b�t�@�T�C�Y�ߖ�̂��߁B
		struct SkinningVertex {
			using IndexType = uint8_t;

			IndexType boneIndex[Num_Bone_Influences];
			float boneWeight[Num_Bone_Influences];
			
		};


		//!	�{�[�����A�{�[�������z��Y���͕ʂ�map<string,int>������ĕۑ��B
		struct Bone {
			INT ParentIndex;
			//!	�t�p���s��
			DirectX::XMFLOAT4X4 InvBindPos;
			DirectX::XMFLOAT4X4 BindPos;
			DirectX::XMFLOAT4X4 LocalTransform;
		};

		//!	�A�j���[�V�����L�[
		struct Keyframe {
			Keyframe() {};
			//!	�g��k���X�P�[�����O
			DirectX::XMFLOAT3	Scaling;
			//!	���s�ړ�
			DirectX::XMFLOAT3	Translation;
			//!	��]�N�H�[�^�j�I��
			DirectX::XMFLOAT4	RotationQt;
		};

	};	//	end of namespace cmo::element

	//!	�A�j���[�V�������BStarterKit�Q��
	struct AnimClip {
		AnimClip() :StartTime(0), EndTime(0){}
		float StartTime;
		float EndTime;
		//!	[�{�[��Index][�t���[������]	�ł̃L�[�t���[���z��
		std::vector < std::map<float, element::Keyframe>>	BoneKeyframe;
	};



}; // end of namespace cmo

//static_assert(sizeof(cmo::element::Material) == 132, "CMO Mesh structure size incorrect");
static_assert(sizeof(cmo::element::SubMesh) == 20, "CMO Mesh structure size incorrect");
static_assert(sizeof(cmo::element::Vertex) == 52, "CMO Mesh structure size incorrect");
//static_assert(sizeof(cmo::element::SkinningVertex) == 32, "CMO Mesh structure size incorrect");
static_assert(sizeof(cmo::element::MeshExtents) == 40, "CMO Mesh structure size incorrect");
static_assert(sizeof(cmo::element::Bone) == 196, "CMO Mesh structure size incorrect");
//static_assert(sizeof(cmo::element::Keyframe) == 72, "CMO Mesh structure size incorrect");
