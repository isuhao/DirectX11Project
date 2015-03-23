#pragma once

#include "./DX10Wrapper1.h"
#include "./D2DWrapper.h"

//	�ŏI�I��d2���O��ԂɊ܂߂邩�͖���

//	DX10.1�f�o�C�X�̍쐬��ɂ��̃L�����o�X���C���[���쐬������悤�Ȑ݌v��
namespace d2 {


	class D2DCanvasLayer {
	protected:
		//!	3D�ł̃L�����o�X�f�[�^
		//!	�\���̖������ȋC������
		class RenderingLayer {
		private:
			struct CanvasVertex {
				DirectX::XMFLOAT3 pos;
				DirectX::XMFLOAT2 texture;
				CanvasVertex() {}
				CanvasVertex(float x, float y, float z,
					float texture_x, float texture_y) : pos(x, y, z), texture(texture_x, texture_y) {
				}
			};
			std::shared_ptr<ID3D11Buffer>		_vertexBuffer;
			std::shared_ptr<ID3D11InputLayout>	_inputLayout;
			std::shared_ptr<ID3D11VertexShader>	_vertexShader;
			std::shared_ptr<ID3D11PixelShader>	_pixelShader;
			//!	�[�x�e�X�g���������邽�߂�
			std::shared_ptr<ID3D11DepthStencilState>	_depthStencilState;
			//!	�A���t�@�u�����h�L����
			std::shared_ptr<ID3D11BlendState>	_blendState;

		public:
			void draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView *texture);
			void init(ID3D11Device *device);
		};

	protected:
		//!	DX11��DX10.1�Ԃ̋��L�e�N�X�`���B������o�R����D3���C���[�ւ̕`����s��
		std::shared_ptr<ID3D11ShaderResourceView>	_deviceSharedTexture;

		//!	DX11��DX10.1�̔r������pMutex
		std::shared_ptr<IDXGIKeyedMutex>	_keyedMutex10, _keyedMutex11;

		//!	Direct2D���̃����_�[�^�[�Q�b�g
		std::shared_ptr<ID2D1RenderTarget>	_renderTarget2D;

		//!	�`��ɕK�v�Ȃ��̂܂Ƃ�
		std::shared_ptr<RenderingLayer>	_paintLayer;

		//!	�I�u�W�F�N�g�ԋ��L�\���\�[�X
		static std::weak_ptr<RenderingLayer>		s_sharedPaintLayer;

	public:
		D2DCanvasLayer() {}
		~D2DCanvasLayer() {}
		
		//!	������
		void init(IDXGISwapChain* swapchain);
		//!	Direct2D�ł̕`����J�n�A�Ԃ�l��ID2D1RenderTarget��p���ĕ`�悷��
		std::weak_ptr<ID2D1RenderTarget> beginDraw() {
			
			// D3D 11 ������̃e�N�X�`���̎g�p�𒆒f����
			_keyedMutex11->ReleaseSync(0);
			// D3D 10.1 (D2D) ������̃e�N�X�`���̎g�p���J�n
			// �ҋ@���Ԃ͖����Ƃ��Ă��܂��B
			_keyedMutex10->AcquireSync(0, INFINITE);

			// �`��J�n
			_renderTarget2D->BeginDraw();
			_renderTarget2D->Clear(D2D1::ColorF(0.0f,0.0f,0.0f,0.0f));


			return _renderTarget2D;
		}
		//!	Direct2D�ł̕`����I���A���ʂ�3D���C���[�ɕ`��
		void endDraw(ID3D11DeviceContext* context) {

			// �`��I��
			_renderTarget2D->EndDraw();
			
			// D3D 10.1 (D2D) ������̃e�N�X�`���̎g�p���I������
			_keyedMutex10->ReleaseSync(1);
			// D3D 11 ������̃e�N�X�`���̎g�p���J�n����
			_keyedMutex11->AcquireSync(1, INFINITE);

			// D2D�ŕ`�悵���e�N�X�`����D3D���̃o�b�N�o�b�t�@�֓]��
			_paintLayer->draw(context,_deviceSharedTexture.get());
		}
		
		//!	���̃L�����o�X�ւ̕`��Ɏg���u���V���쐬����
		std::shared_ptr<ID2D1SolidColorBrush> createBrush(const D2D1::ColorF& color = D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f)) {
			
			return d2::CreateSolidColorBrush(_renderTarget2D.get(),color);
		}
	};

}