#pragma once

#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>
#include <d2d1_1.h>


#include "../SceneLayer3D.h"



namespace d2d {

	class CanvasLayer2D {
	private:
		struct SpriteVertex {
			DirectX::XMFLOAT3 pos;
			DirectX::XMFLOAT2 texture;
			SpriteVertex() {}
			SpriteVertex(float x, float y, float z,
				float texture_x, float texture_y) : pos(x, y, z), texture(texture_x, texture_y) {
			}
		};

	public:
		enum BlendMode { Default, PreMultiPlyedAlpha, Add, BlendMode_Max };

	private:
		std::shared_ptr<ID3D11Buffer>			_vertexBuffer;
		std::shared_ptr<ID3D11InputLayout>		_inputLayout;
		std::shared_ptr<ID3D11VertexShader>		_vertexShader;
		std::shared_ptr<ID3D11PixelShader>		_pixelShader;
		std::shared_ptr<ID3D11Buffer>			_constantBuffer;
		//!	�[�x�e�X�g�̖������p
		std::shared_ptr<ID3D11DepthStencilState>	_depthStencilState;
		//!	�A���t�@�u�����h�̗L��������т��̑��������@�w��p
		std::shared_ptr<ID3D11BlendState>		_blendState[BlendMode_Max];
		
		//!	2D->3D�ϊ����鎞�p
		int	_width, _height;

		d3d::SceneLayer3D*	_renderTarget3DScene;


	public:
		void init(ID3D11Device* device, d3d::SceneLayer3D* renderTarget, int canvasWidth, int canvasHeight);
		int getWidth()const {
			return _width;
		}
		int getHeight()const {
			return _height;
		}

		void renderTexture(
			ID3D11ShaderResourceView *texture, const D2D_MATRIX_3X2_F& transform,
			const D2D_MATRIX_3X2_F& textureTransform, BlendMode blendMode = BlendMode::Default);

		//	�e�탊�\�[�X���Z�b�g���Đ[�x�X�e���V���������A�`��J�n
		void beginDraw() {
			ID3D11DeviceContext* context = _renderTarget3DScene->getContext();

			//	�[�x�X�e���V�����ꎞ�I�ɖ���������B�������Ȃ񂾂낤����
			context->OMSetDepthStencilState(_depthStencilState.get(), 0);

			context->IASetInputLayout(_inputLayout.get());
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);


			context->VSSetShader(_vertexShader.get(), nullptr, 0);
			context->PSSetShader(_pixelShader.get(), nullptr, 0);

			ID3D11Buffer* vbs[] = { _vertexBuffer.get() };
			const UINT stride = sizeof(SpriteVertex);
			const UINT offset = 0;
			context->IASetVertexBuffers(0, 1, vbs, &stride, &offset);

		}
		//	�[�x�X�e���V�������ɖ߂�
		void endDraw() {
			//	�[�x�e�X�g���f�t�H���g�ɖ߂��B�f�t�H���g�ŗǂ��̂��H
			_renderTarget3DScene->getContext()->OMSetDepthStencilState(nullptr, 0);
		}
	};

}