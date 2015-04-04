#include "stdafx.h"

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

#include "./d2d/Sprite.h"

int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE,
	LPTSTR,
	INT
) {
	::CoInitialize(NULL); // �Ȃ��Ă��������ǏI������O��f��
	
	// �E�B���h�E����
	DXGI_MODE_DESC modeDesc = DX11ThinWrapper::gi::GetOptDisplayMode(800, 600);
	win::Window window(
		TEXT("window"), TEXT("window"), {0, 0, modeDesc.Width, modeDesc.Height}, WS_OVERLAPPEDWINDOW, win::DefaultProcedure
	);
	
	MSG msg = {};

	{
		// ������
		dx11::DX11DeviceSharedGuard guard;
		auto device = dx11::AccessDX11Device();

		auto context = DX11ThinWrapper::d3::AccessD3Context(device);
		auto swapChain = dx11::CreateDefaultSwapChain(&modeDesc, window.getHWnd(), device, true);
		
		dx11::SetDefaultRenderTarget(swapChain.get());
		dx11::SetDefaultViewport(swapChain.get());


		//	�V�F�[�_�[�쐬
		auto vertex_blob = DX11ThinWrapper::d3::CompileShader(_T("./assets/shader/sample.hlsl"), "RenderVS", VERTEX_SHADER_PROFILE);
		auto vertexShader = DX11ThinWrapper::d3::CreateVertexShader(device, vertex_blob.get());
		
		//	�X�L�j���O���_�V�F�[�_�\
		auto skinningVS_blob = DX11ThinWrapper::d3::CompileShader(_T("./assets/shader/skinning.hlsl"), "RenderVS", VERTEX_SHADER_PROFILE);
		auto skinningVS = DX11ThinWrapper::d3::CreateVertexShader(device, skinningVS_blob.get());

		//	�s�N�Z���V�F�[�_�͋��ʂ̂��̂��g����
		auto pixelShader = DX11ThinWrapper::d3::CreatePixelShader(
			device, DX11ThinWrapper::d3::CompileShader(_T("./assets/shader/sample.hlsl"), "RenderPS", PIXEL_SHADER_PROFILE).get()
		);

		cmo::SkinnedMesh skinnedMesh;
		skinnedMesh.load(device, L"./assets/model/skinning.cmo", nullptr);
		skinnedMesh.applyPS(pixelShader);
		skinnedMesh.applyVS(skinningVS, skinningVS_blob,device);
		skinnedMesh.setAnimation(L"walk");


		DirectX::XMFLOAT4X4 param;

		//	�v���W�F�N�V�����s��ݒ�
		//	�萔�o�b�t�@�� float*4�̔{���̃T�C�Y�ł���K�v������
		//	���̑��͂܂肻���ȂƂ��� �F https://twitter.com/43x2/status/144821841977028608

		float aspectRatio = modeDesc.Width / static_cast<float>(modeDesc.Height);
		DirectX::XMMATRIX mtxProj = DirectX::XMMatrixPerspectiveFovLH(3.1415926f / 6.0f, aspectRatio, 1.0f, 100.0f);
		XMStoreFloat4x4(&param, DirectX::XMMatrixTranspose(mtxProj));
		auto projBuffer = DX11ThinWrapper::d3::CreateConstantBuffer(
			device, &param, sizeof(DirectX::XMFLOAT4X4), D3D11_CPU_ACCESS_WRITE
			);
		ID3D11Buffer * projectionBuffers[] = { projBuffer.get() };
		//	�X���b�g0�Ƀv���W�F�N�V�����s��ݒ�
		context->VSSetConstantBuffers(0, 1, projectionBuffers);
		


		// �J�����s��ݒ�
		//	�o�b�t�@�T�C�Y���Ɋւ��Ă̓v���W�F�N�V�����̏ꍇ�Ɠ��l	
		DirectX::XMMATRIX mtxView = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixLookAtLH(
			DirectX::XMVectorSet(0.0f, 0.0f, -5.0f, 1),
			DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1),
			DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1)
			)
		);
		XMStoreFloat4x4(&param, DirectX::XMMatrixTranspose(mtxView));
		auto cameraBuffer = DX11ThinWrapper::d3::CreateConstantBuffer(
			device, &param, sizeof(DirectX::XMFLOAT4X4), D3D11_CPU_ACCESS_WRITE
			);




		// �^�[�Q�b�g�r���[
		auto rv = DX11ThinWrapper::d3::AccessRenderTargetViews(context.get(), 1)[0];
		auto dv = DX11ThinWrapper::d3::AccessDepthStencilView(context.get());

		// �E�B���h�E�\��
		::ShowWindow(window.getHWnd(), SW_SHOW);
		::UpdateWindow(window.getHWnd());


		// �T���v���[�ݒ�Y��Ă��̂�
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;		// �T���v�����O���Ɏg�p����t�B���^�B�����ł͈ٕ����t�B���^�[���g�p����B
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;	// 0 �` 1 �͈̔͊O�ɂ��� u �e�N�X�`���[���W�̕`����@
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;	// 0 �` 1 �͈̔͊O�ɂ��� v �e�N�X�`���[���W
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;	// 0 �` 1 �͈̔͊O�ɂ��� w �e�N�X�`���[���W
		samplerDesc.MipLODBias = 0;			// �v�Z���ꂽ�~�b�v�}�b�v ���x������̃o�C�A�X
		samplerDesc.MaxAnisotropy = 16;		// �T���v�����O�Ɉٕ�����Ԃ��g�p���Ă���ꍇ�̌��E�l�B�L���Ȓl�� 1 �` 16
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;  // ��r�I�v�V����
		samplerDesc.BorderColor[0] = 0.0f;
		samplerDesc.BorderColor[1] = 0.0f;
		samplerDesc.BorderColor[2] = 0.0f;
		samplerDesc.BorderColor[3] = 0.0f;
		samplerDesc.MinLOD = 0;						// �A�N�Z�X�\�ȃ~�b�v�}�b�v�̉����l
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;		// �A�N�Z�X�\�ȃ~�b�v�}�b�v�̏���l
		auto sampler = DX11ThinWrapper::d3::CreateSampler(device, samplerDesc);
		ID3D11SamplerState* states[] = { sampler.get() };
		context->PSSetSamplers(0, 1, states);


		//--------------------------
		//	DirectX10.1�f�o�C�X��11�̂��̂Ɠ����ݒ�ō쐬
		//--------------------------
		dx10::DX10DeviceSharedGuard deviceGuard10(device);
		d2::Canvas2D canvas2D;

		canvas2D.init(device, modeDesc.Width, modeDesc.Height);
		auto txtes = DX11ThinWrapper::d3::CreateWICTextureFromFile(device,L"./assets/model/body.png");
		d2::Sprite sprite(txtes, &canvas2D);
		d2::TextSprite textspr(&canvas2D,L"���C���I",50.0f);

		int curTime = 0, prevTime = timeGetTime();
		const int Frame_Per_Second = 60;
		int timer = 0;
		
		// ���C�����[�v
		do {
			if (::PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			} else {

				curTime = timeGetTime();
				if (curTime - prevTime >= 1000 / Frame_Per_Second) {
					prevTime = curTime;
					timer++;

					// �o�b�N�o�b�t�@����ѐ[�x�o�b�t�@���N���A
					static float ClearColor[4] = { 0.3f, 0.3f, 1.0f, 1.0f };
					context->ClearRenderTargetView(rv.get(), ClearColor);
					context->ClearDepthStencilView(dv.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

					// �J�����̐ݒ�
					DX11ThinWrapper::d3::mapping(cameraBuffer.get(), context.get(), [&](D3D11_MAPPED_SUBRESOURCE resource) {
						auto param = static_cast<DirectX::XMFLOAT4X4 *>(resource.pData);
						auto mtxView = DirectX::XMMatrixLookAtLH(
							DirectX::XMVectorSet(0.0f, 2.0f, -10.0f, 1),
							DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1),
							DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1)
							);
						XMStoreFloat4x4(param, DirectX::XMMatrixTranspose(mtxView));
					});
					ID3D11Buffer * cameraBuffers[] = { cameraBuffer.get() };
					//	�X���b�g1�ɃJ�����s���ݒ�
					context->VSSetConstantBuffers(1, 1, cameraBuffers);


					//	�X���b�g0�Ƀv���W�F�N�V�����s��ݒ�
					context->VSSetConstantBuffers(0, 1, projectionBuffers);


					//	�X�L�����b�V�����f���̍s��X�V�A�`��
					skinnedMesh.setAnimationFrame(timer % 60);
					skinnedMesh.updateMatrix(
						DirectX::XMMatrixRotationY(DirectX::XM_PI/3.5)*DirectX::XMMatrixTranslation(0, -1, 0.5)
						);
					skinnedMesh.render(context.get(), 2);

					
					canvas2D.beginDraw(context.get());
					
					if (timer > 120) {
						sprite.setTransform(D2D1::Matrix3x2F::Translation(0,0));
						sprite.render(context.get());
					}
					wchar_t text[100];
					wsprintf(text, L"timer = %d", timer);
					textspr.drawText(text,lstrlenW(text),D2D1::RectF(0,400,600,800));
					textspr.render(context.get());

					canvas2D.endDraw(context.get());
					/**/
					if (FAILED(swapChain->Present(0, 0))) break;
				}
			}

		} while (msg.message != WM_QUIT);
	}

	::CoUninitialize(); // �Ȃ��Ă��������ǏI������O��f��
	return msg.wParam;
}