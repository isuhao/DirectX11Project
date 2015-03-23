#pragma once
#include <memory>

#include <d2d1.h>
#include <d3d10_1.h>

#include "../dx11/DX11ThinWrapper.h"

//!	Direct3D���C���[��Direct2D���C���[�����������邽�߂�DirectX10.1��p���܂�
//!	�Q�l
//!	http://mitsunagistudio.net/tips/d2d-d3d11-sharing/

//	DX11�̃f�o�C�X�ɏ����ݒ肾�����ˑ����������c�ǂ�����āH
namespace dx10 {
	class DX10DeviceSharedGuard {
	private:
		std::shared_ptr<ID3D10Device1> _device;
	public:
		DX10DeviceSharedGuard(ID3D11Device *pDX11Device);
	};

	::ID3D10Device1 * AccessDX10Device();



	namespace thin_template{
		LPVOID OpenSharedResource_base(ID3D11Texture2D* texture, const IID& uuid);
	} //	end of namespace thin_template 

	template <typename T>
	//!	DirectX11�ō쐬�������\�[�X��DX10�ŋ��L
	//!	�n�����\�[�X�͍쐬����D3D11_RESOURCE_MISC_SHARED�̋��L�t���O��ݒ肵��2D�e�N�X�`���B
	/**
	* @param  texture11	���L������2D�e�N�X�`�����\�[�X
	* @return DX10.1�̋��L�I�u�W�F�N�g
	*
	* @see https://msdn.microsoft.com/ja-jp/library/ee419814(v=vs.85).aspx
	*/
	std::shared_ptr<T> OpenSharedResource(ID3D11Texture2D* texture11) {
		T* sharedResource = reinterpret_cast<T*>(thin_template::OpenSharedResource_base(texture11, __uuidof(T)));
		
		return(std::shared_ptr<T>(sharedResource, DX11ThinWrapper::ReleaseIUnknown));
	}

} //	end of namespace dx10