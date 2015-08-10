#pragma once

#include <memory>
#include <d3d11.h>

namespace d3 {
	

	//	���3D�`�惌�C���[��ێ��A����
	class SceneLayer3D {
	private:
		std::shared_ptr<ID3D11DeviceContext>	_context;
		std::shared_ptr<IDXGISwapChain>			_swapChain;

		std::shared_ptr<ID3D11RenderTargetView>	_renderTargetView;
		std::shared_ptr<ID3D11DepthStencilView>	_depthStencilView;

		std::shared_ptr<ID3D11Buffer>		_projectionMtxBuffer;

		//	�A�X�y�N�g��A���e�s��w��̎��Ɏg��
		float _aspectRatio;

	public:
		SceneLayer3D() : _aspectRatio(0){}

		void init(ID3D11Device* device, const DXGI_MODE_DESC& displayMode, HWND hWnd);

		//	�ٕ����t�B���^���g���A�e�N�X�`���͌J��Ԃ��ɂ���
		void setCustomSamplar(ID3D11Device* device);
		//	���u�����h��L����
		void setCustomBlendMode(ID3D11Device* device);

		/**
		 *	�v���W�F�N�V�����s���ݒ�
		 *	@param fovRadian	����p�i���W�A���P�ʁj
		 *	@param startSlot	�V�F�[�_�[�̃o�b�t�@�X���b�gNo
		 */
		void setProjection(ID3D11Device* device, float fovRadian, int startSlot);

		//! �o�b�N�o�b�t�@����ѐ[�x�o�b�t�@���N���A
		void clearViews() {

			static float ClearColor[4] = { 0.3f, 0.3f, 1.0f, 1.0f };

			_context->ClearRenderTargetView(_renderTargetView.get(), ClearColor);
			_context->ClearDepthStencilView(_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		
		HRESULT present() {
			return _swapChain->Present(0, 0);
		}

	};

};