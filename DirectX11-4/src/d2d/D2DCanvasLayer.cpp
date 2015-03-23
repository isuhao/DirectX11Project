#include "D2DCanvasLayer.h"

#include "./ShaderBytes.h"



namespace {

	//	�e�N�X�`������V�F�[�_�[�E���\�[�X�E�r���[���쐬
	std::shared_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(ID3D11Device* device, ID3D11Texture2D * resourceTexture) {

		ID3D11ShaderResourceView * srv;
		auto hr = device->CreateShaderResourceView(resourceTexture, nullptr, &srv);
		if (FAILED(hr)) throw std::runtime_error("ID3D11ShaderResourceView�̍쐬�Ɏ��s���܂���.");
		return std::shared_ptr<ID3D11ShaderResourceView>(srv, DX11ThinWrapper::ReleaseIUnknown);
	};

	//	�[�x�X�e���V���r���[�ݒ�I�u�W�F�N�g�̍쐬
	std::shared_ptr<ID3D11DepthStencilState> CreateDepthStencilState(ID3D11Device* device, const D3D11_DEPTH_STENCIL_DESC& desc) {
		ID3D11DepthStencilState* dss;
		HRESULT hr = device->CreateDepthStencilState(&desc, &dss);
		if (FAILED(hr))	throw std::runtime_error("ID3D11DepthStencilState�̍쐬�Ɏ��s");
		return std::shared_ptr<ID3D11DepthStencilState>(dss, DX11ThinWrapper::ReleaseIUnknown);

	};
}


namespace d2 {

	std::weak_ptr<D2DCanvasLayer::RenderingLayer>	D2DCanvasLayer::s_sharedPaintLayer;

	void D2DCanvasLayer::RenderingLayer::init(ID3D11Device* device) {

		static D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		_vertexShader = DX11ThinWrapper::d3::CreateVertexShader(device, cso::compiledShader::simpleVS, sizeof(cso::compiledShader::simpleVS));
		_inputLayout = DX11ThinWrapper::d3::CreateInputLayout(device, layout, _countof(layout), cso::compiledShader::simpleVS, sizeof(cso::compiledShader::simpleVS));

		_pixelShader = DX11ThinWrapper::d3::CreatePixelShader(
			device, cso::compiledShader::simplePS, sizeof(cso::compiledShader::simplePS)
			);

		CanvasVertex verts[] = {
			CanvasVertex(-1.0f, -1.0f, 0.0f, 0.0f, 1.0f),
			CanvasVertex(-1.0f, 1.0f, 0.0f, 0.0f, 0.0f),
			CanvasVertex(1.0f, -1.0f, 0.0f, 1.0f, 1.0f),
			CanvasVertex(1.0f, 1.0f, 0.0f, 1.0f, 0.0f),
		};

		_vertexBuffer = DX11ThinWrapper::d3::CreateVertexBuffer(device, verts, sizeof(CanvasVertex)* 4);


		//	�[�x�e�X�g�𖳌��ɂ��邽�߂̐[�x�X�e���V���X�e�[�g�̍쐬
		D3D11_DEPTH_STENCIL_DESC dsd = {};
		dsd.DepthEnable = FALSE;
		dsd.StencilEnable = FALSE;
		_depthStencilState = CreateDepthStencilState(device, dsd);

		//	���u�����h�ݒ�
		D3D11_BLEND_DESC blendDesc[] = { {
				FALSE, FALSE, {
					{
						TRUE,
						D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
						D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD,
						D3D11_COLOR_WRITE_ENABLE_ALL
					}
				}
			} };

		_blendState = DX11ThinWrapper::d3::CreateBlendState(device, blendDesc);
	}

	void D2DCanvasLayer::RenderingLayer::draw(ID3D11DeviceContext* context, ID3D11ShaderResourceView *texture){

		//	�[�x�X�e���V�����ꎞ�I�ɖ���������B�������Ȃ񂾂���������Ă��킩���
		context->OMSetDepthStencilState(_depthStencilState.get(), 0);

		//	���u�����h�L����
		FLOAT blendFactor[] = { 1, 1, 1, 1 };
		context->OMSetBlendState(_blendState.get(), blendFactor, 0xffffffff);

		context->IASetInputLayout(_inputLayout.get());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		

		context->VSSetShader(_vertexShader.get(), nullptr, 0);
		context->PSSetShader(_pixelShader.get(), nullptr, 0);

		ID3D11Buffer* vbs[] = { _vertexBuffer.get() };
		const UINT stride = sizeof(CanvasVertex);
		const UINT offset = 0;
		context->IASetVertexBuffers(0, 1, vbs, &stride, &offset);

		context->PSSetShaderResources(0, 1, &texture);

		context->Draw(4, 0);

		//	�[�x�e�X�g���f�t�H���g�ɖ߂��B�f�t�H���g�ŗǂ��̂��H
		context->OMSetDepthStencilState(nullptr, 0);
	}

	void D2DCanvasLayer::init(IDXGISwapChain* swapchain) {

		using namespace DX11ThinWrapper;

		// ���L����e�N�X�`���� D3D 11 ����p�ӁB
		// �����ł̓o�b�N�o�b�t�@�Ɠ����T�C�Y�̃e�N�X�`�����쐬�B
		D3D11_TEXTURE2D_DESC backBufferDesc, canvasDesc;

		auto backBuffer3D = d3::AccessBackBuffer(swapchain);

		backBuffer3D->GetDesc(&backBufferDesc);

		// �쐬����e�N�X�`�����̐ݒ�B
		// �EDXGI_FORMAT_B8G8R8A8_UNORM �͌Œ�B
		// �ED3D11_BIND_RENDER_TARGET �� D2D �ł̕`��ΏۂƂ��邽�߂ɕK�{�B
		// �ED3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX �̓e�N�X�`�������L����̂ɕK�{�B
		ZeroMemory(&canvasDesc, sizeof(canvasDesc));
		canvasDesc.Width = backBufferDesc.Width;
		canvasDesc.Height = backBufferDesc.Height;
		canvasDesc.MipLevels = 1;		//��~�b�v�}�b�v�e�N�X�`���̂݋��L�\
		canvasDesc.ArraySize = 1;
		canvasDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		canvasDesc.SampleDesc.Count = 1;
		canvasDesc.Usage = D3D11_USAGE_DEFAULT;
		canvasDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		canvasDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX;

		
		ID3D11Device* device = d3::AccessD3Device(swapchain).get();
		auto tex2D = d3::CreateTexture2D(device, canvasDesc);
		
		//	���L�̂��߂�D3D11�̃L�[�~���[�e�b�N�X���擾
		_keyedMutex11 = QueryInterface<IDXGIKeyedMutex>(tex2D.get());


		//	DX10.1���Ŏg�����L�T�[�t�F�C�X���쐬
		auto surface10 = dx10::OpenSharedResource<IDXGISurface1>(tex2D.get());
		

		//	���L�̂��߂�D3D10.1�̃L�[�~���[�e�b�N�X���擾
		_keyedMutex10 = QueryInterface<IDXGIKeyedMutex>(surface10.get());

		// D2D �̃����_�[�^�[�Q�b�g�� D3D 10.1 �̋��L�T�[�t�F�C�X���琶��
		_renderTarget2D = d2::CreateDXGISurfaceRenderTarget(
			surface10.get(), D2D1::RenderTargetProperties(
			D2D1_RENDER_TARGET_TYPE_HARDWARE, D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED))
			);


		//	���L�e�N�X�`�������ɃV�F�[�_�[���\�[�X�r���[�̍쐬
		_deviceSharedTexture = CreateShaderResourceView(device, tex2D.get());


		_paintLayer = D2DCanvasLayer::s_sharedPaintLayer.lock();
		if (_paintLayer == nullptr) {
			D2DCanvasLayer::s_sharedPaintLayer = _paintLayer = std::make_shared<RenderingLayer>();
			_paintLayer->init(device);
		}
	}
	
}