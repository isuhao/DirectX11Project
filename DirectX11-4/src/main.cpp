#include "stdafx.h"

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

#include "Camera.h"
#include "System.h"
#include "SceneLayer3D.h"
#include "effekseer/effekseerWrapper.h"


int APIENTRY _tWinMain(
	HINSTANCE,
	HINSTANCE,
	LPTSTR,
	INT
) {
	::CoInitialize(NULL); // �Ȃ��Ă��������ǏI������O��f��
	
	// �E�B���h�E����
	DXGI_MODE_DESC modeDesc = DX11ThinWrapper::gi::GetOptDisplayMode(800, 600);
	win::Window window(
		TEXT("window"), TEXT("window"), {0, 0, (LONG)modeDesc.Width, (LONG)modeDesc.Height}, WS_OVERLAPPEDWINDOW, win::DefaultProcedure
	);
//	window.setShowState(SW_SHOW);

	sys::SetFPS(60);

	{
		// ������
		dx11::DX11DeviceSharedGuard guard;
		auto device = dx11::AccessDX11Device();

		//	DirectX10.1�f�o�C�X��11�̂��̂Ɠ����ݒ�ō쐬
		dx10::DX10DeviceSharedGuard deviceGuard10(device);

		d3d::SceneLayer3D	scene3D;
		scene3D.init(device, modeDesc, window.getHWnd());

		d2d::CanvasLayer2D	canvas2D;
		canvas2D.init(device, &scene3D, modeDesc.Width, modeDesc.Height);

		d2d::Sprite sprite(dx11::ResourceCache::Texture::Get(L"./assets/circle.png"), &canvas2D);
		d2d::TextSprite textspr(&canvas2D, L"���C���I", 50.0f);

		scene3D.setCustomSamplar(device);
		scene3D.setProjection(DirectX::XM_PI / 6, 0);

		

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

		d3d::Camera camera(device);
		camera.setEyePosition(DirectX::XMFLOAT4(1,3,-10,1));
		
		effekseer11::EffectLayer effectLayer;
		effectLayer.init(device,scene3D.getContext());
		effectLayer.setProjectionMatrix(scene3D.getProjectionMatrix());
		effekseer11::Effect effect(&effectLayer);
		if(effect.reset(L"./assets/tes.efk") == false)
			assert(false);
		effect.play(0,2,10);
		DirectX::XMFLOAT4X4 handPosR;


		window.setShowState(SW_SHOW);
		

		int timer = 0;
		// ���C�����[�v
		while (sys::Update()) {
			timer++;

			scene3D.clearViews();
			scene3D.setCustomBlendState(device);
			scene3D.setProjection(DirectX::XM_PI / 6, 0);

			camera.rotateY(0.01);
			camera.update();
			camera.setBuffer(&scene3D, 1);

			//	�X�L�����b�V�����f���̍s��X�V�A�`��
			skinnedMesh.setAnimationFrame(timer % 60);
			skinnedMesh.updateMatrix(
				DirectX::XMMatrixRotationY(DirectX::XM_PI / 3.5)*DirectX::XMMatrixTranslation(0, -1, 0.5)
				);
			skinnedMesh.render(&scene3D, 2);
			skinnedMesh.getBoneWorldTransform(&handPosR, L"hand_R");


			effectLayer.setProjectionMatrix(scene3D.getProjectionMatrix());
			effectLayer.setCameraMatrix(camera.getMatrix());
			effect.setTransform(handPosR);
			effectLayer.drawAll();
			if (timer == 20)
				effect.setPauseState(true);


			canvas2D.beginDraw();

			if (timer > 12) {
				//sprite.setTransform();
				sprite.setTransform(D2D1::Matrix3x2F::Rotation(timer * 3, D2D1::Point2F(225, 225))*D2D1::Matrix3x2F::Translation(-50, -150));
				sprite.render(&canvas2D, d2d::CanvasLayer2D::BlendMode::Add);
			}
			
			wchar_t text[100];
			wsprintf(text, L"timer = %d", timer);
			textspr.drawText(text, lstrlenW(text), D2D1::RectF(0, 400, 600, 800));
			textspr.render(&canvas2D);

			canvas2D.endDraw();
			
			//	�����I���͂��܂�悭�Ȃ�����ǂ�
			if (FAILED(scene3D.present()))
				SendMessage(window.getHWnd(),WM_CLOSE,0,0);

		}

	}

	//	CoInitialize�ɌĂяo���񐔂Ɠ����񐔌ĂԁB�}���`�X���b�h�̏ꍇ�͊e��񂾂��B
	//	�e�탁�b�Z�[�W���[�v���I��������ɌĂԂ��ƁB
	//	https://msdn.microsoft.com/ja-jp/library/windows/desktop/ms688715%28v=vs.85%29.aspx
	::CoUninitialize();

	return(sys::GetLastMessage().wParam);
}