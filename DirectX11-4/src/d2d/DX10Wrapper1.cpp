#include "DX10Wrapper1.h"

#pragma comment (lib, "d3d10_1.lib")
#pragma comment (lib, "d2d1.lib")


#include "../dx11/DX11ThinWrapper.h"

namespace {

	std::weak_ptr<ID3D10Device1> g_device10;


	std::shared_ptr<ID3D10Device1> initDirect3DDevice10_1(ID3D11Device *pDX11Device) {
		
		UINT createDeviceFlag = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlag |= D3D10_CREATE_DEVICE_DEBUG;
#endif

		ID3D10Device1* device10 = nullptr;

		//	DX11�Ƌ��p�̂��߁A�A�_�v�^�[�͕K���������̂�p����B
		//	D3D10_DRIVER_TYPE_HARDWARE �� D3D10_CREATE_DEVICE_BGRA_SUPPORT ���w��B
		HRESULT hr = D3D10CreateDevice1(
			DX11ThinWrapper::gi::AccessAdapter(pDX11Device).get(),
			D3D10_DRIVER_TYPE_HARDWARE,
			nullptr,
			createDeviceFlag,
			D3D10_FEATURE_LEVEL_9_3,
			D3D10_1_SDK_VERSION,
			&device10
			);
		if (SUCCEEDED(hr)) {
			return std::shared_ptr<ID3D10Device1>(device10, DX11ThinWrapper::ReleaseIUnknown);
		}
		
		throw std::runtime_error("ID3D10Device1�̐����Ɏ��s���܂���.");
		return std::shared_ptr<ID3D10Device1>();

	}

}

namespace dx10 {
	DX10DeviceSharedGuard::DX10DeviceSharedGuard(ID3D11Device *pDX11Device) {
		if (auto device = g_device10.lock()) {
			_device = device;
		} else {
			g_device10 = _device = initDirect3DDevice10_1(pDX11Device);
		}
	}

	::ID3D10Device1 * AccessDX10Device() {
		return g_device10.lock().get();
	}


	namespace thin_template {
		LPVOID OpenSharedResource_base(ID3D11Texture2D* texture, const IID& uuid){


			HANDLE sharedHandle;
			// ���L�̂��߂̃n���h�����擾
			if (FAILED(DX11ThinWrapper::QueryInterface<IDXGIResource>(texture)->GetSharedHandle(&sharedHandle)))
				throw std::runtime_error("���L�n���h���̍쐬�Ɏ��s");


			LPVOID sharedObject = nullptr;
			// DX10.1 �ŋ��L�I�u�W�F�N�g�𐶐�
			if (FAILED(AccessDX10Device()->OpenSharedResource(sharedHandle, uuid, &sharedObject)))
				throw std::runtime_error("���L���\�[�X�̍쐬�Ɏ��s");	
			
			return sharedObject;
		}

	} //	end of namespace thin_template

} //	end of namespace dx10 