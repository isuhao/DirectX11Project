#pragma once

#include "../comUtil.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <functional>
#include <vector>

namespace DX11ThinWrapper {

	namespace gi {
		// i�Ԗڂ̃f�B�X�v���C���擾
		std::shared_ptr<IDXGIOutput> AccessDisplay(UINT i);

		// ���p�\�ȕ\�����[�h�̎擾
		void GetDisplayModes(DXGI_MODE_DESC* pModeDesc, UINT display_i, DXGI_FORMAT format, UINT * pNum = nullptr);

		// ���p�\�ȕ\�����[�h�̐����擾
		UINT GetNumOfDisplayModes(UINT display_i, DXGI_FORMAT format);

		// �K�؂ȉ𑜓x�̃f�B�X�v���C���[�h���擾
		DXGI_MODE_DESC GetOptDisplayMode(int width, int height, UINT display_i = 0, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

		// IDXGIAdapter -> IDXGIFactory
		std::shared_ptr<IDXGIFactory> AccessGIFactory(IDXGIAdapter * adapter);
		// ID3D11Device -> IDXGIAdapter -> IDXGIFactory
		std::shared_ptr<IDXGIFactory> AccessGIFactory(ID3D11Device * device);

		// ID3D11Device -> IDXGIDevice1
		std::shared_ptr<IDXGIDevice1> AccessInterface(ID3D11Device * device);
		// ID3D11Device -> IDXGIDevice1 -> IDXGIAdapter
		std::shared_ptr<IDXGIAdapter> AccessAdapter(ID3D11Device * device);

		// ID3D11Texture2D -> IDXGISurface
		std::shared_ptr<IDXGISurface> AccessSurface(ID3D11Texture2D * tex);
	};

	namespace d3 {

		// Direct3D�̏�����
		std::shared_ptr<ID3D11Device> InitDirect3D();

		// �V�F�[�_�t�@�C���̃R���p�C��
		std::shared_ptr<ID3DBlob> CompileShader(TCHAR source[], CHAR entryPoint[], CHAR target[]);

		// �s�N�Z���V�F�[�_�̐���
		std::shared_ptr<ID3D11PixelShader> CreatePixelShader(ID3D11Device* device, const byte* shaderBytes, size_t byteSize);
		inline std::shared_ptr<ID3D11PixelShader> CreatePixelShader(ID3D11Device* device, ID3DBlob * blob) {
			return CreatePixelShader(device, static_cast<const byte*>(blob->GetBufferPointer()), blob->GetBufferSize());
		}
		// �o�[�e�b�N�X�V�F�[�_�̐���
		std::shared_ptr<ID3D11VertexShader> CreateVertexShader(ID3D11Device* device, const byte* shaderBytes, size_t byteSize);
		inline std::shared_ptr<ID3D11VertexShader> CreateVertexShader(ID3D11Device* device, ID3DBlob * blob) {
			return CreateVertexShader(device, static_cast<const byte*>(blob->GetBufferPointer()), blob->GetBufferSize());
		}
		// �W�I���g���V�F�[�_�̐���
		std::shared_ptr<ID3D11GeometryShader> CreateGeometryShader(ID3D11Device* device, const byte* shaderBytes, size_t byteSize);
		inline std::shared_ptr<ID3D11GeometryShader> CreateGeometryShader(ID3D11Device* device, ID3DBlob * blob) {
			return CreateGeometryShader(device, static_cast<const byte*>(blob->GetBufferPointer()), blob->GetBufferSize());
		}

		// ���̓��C�A�E�g�̐���
		std::shared_ptr<ID3D11InputLayout> CreateInputLayout(
			ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC layoutDesc[], UINT numOfLayout, const byte* shaderBytes, size_t byteSize
			);
		inline std::shared_ptr<ID3D11InputLayout> CreateInputLayout(
			ID3D11Device* device, D3D11_INPUT_ELEMENT_DESC layoutDesc[], UINT numOfLayout, ID3DBlob * blob
			) {
			return CreateInputLayout(device,layoutDesc, numOfLayout, static_cast<const byte*>(blob->GetBufferPointer()), blob->GetBufferSize());
		}
		

		// ���_�o�b�t�@�̐���
		std::shared_ptr<ID3D11Buffer> CreateVertexBuffer(
			ID3D11Device* device, void* data, size_t size, UINT cpuAccessFlag = 0
		);
		// �C���f�b�N�X�o�b�t�@�̐���
		std::shared_ptr<ID3D11Buffer> CreateIndexBuffer(
			ID3D11Device* device, void* data, size_t size, UINT cpuAccessFlag = 0
		);
		// �萔�o�b�t�@�̐���
		std::shared_ptr<ID3D11Buffer> CreateConstantBuffer(
			ID3D11Device* device, void* data, size_t size, UINT cpuAccessFlag = 0
		);

		// ��̃e�N�X�`�����\�[�X�̐���
		std::shared_ptr<ID3D11Texture2D> CreateTexture2D(ID3D11Device * device, const D3D11_TEXTURE2D_DESC & descDS);

		// �����_�[�^�[�Q�b�g�r���[�̐���
		// ��⌈�ߑł��̕�������
		std::shared_ptr<ID3D11RenderTargetView> CreateRenderTargetView(IDXGISwapChain * swapChain);
		// �[�x�o�b�t�@�r���[�̐���
		// ��⌈�ߑł��̕�������
		std::shared_ptr<ID3D11DepthStencilView> CreateDepthStencilView(IDXGISwapChain * swapChain);


		// �X���b�v�`�F�C���̐���
		std::shared_ptr<IDXGISwapChain> CreateSwapChain(ID3D11Device * device, DXGI_SWAP_CHAIN_DESC sd);

		// �}���`�T���v�����O�ŗ��p�\�ȕi�����x���̐����擾
		UINT CheckMultisampleQualityLevels(ID3D11Device * device, DXGI_FORMAT format, UINT sampleCount);

		// �X���b�v�`�F�C���̐ݒ���擾
		DXGI_SWAP_CHAIN_DESC GetSwapChainDescription(IDXGISwapChain * swapChain);
		// �X���b�v�`�F�C���̃o�b�N�o�b�t�@�փA�N�Z�X
		std::shared_ptr<ID3D11Texture2D> AccessBackBuffer(IDXGISwapChain * swapChain);
		// �X���b�v�`�F�C���𐶐������f�o�C�X�փA�N�Z�X
		std::shared_ptr<ID3D11Device> AccessD3Device(IDXGISwapChain * swapChain);

		// ID3D11Device -> ID3D11DeviceContext
		std::shared_ptr<ID3D11DeviceContext> AccessD3Context(ID3D11Device * device);
		// IDXGISwapChain -> ID3D11Device -> ID3D11DeviceContext
		std::shared_ptr<ID3D11DeviceContext> AccessD3Context(IDXGISwapChain * swapChain);

		// ID3D11DeviceContext -> ID3D11DepthStencilView
		std::shared_ptr<ID3D11DepthStencilView> AccessDepthStencilView(ID3D11DeviceContext * context);
		// ID3D11DeviceContext -> ID3D11RenderTargetView
		std::vector<std::shared_ptr<ID3D11RenderTargetView>> AccessRenderTargetViews(
			ID3D11DeviceContext * context, UINT numOfViews
		);

		// ID3D11View -> ID3D11Resource
		std::shared_ptr<ID3D11Resource> AccessResource(ID3D11View * view);


		// WICTextureLoader.h ��p�����e�N�X�`���t�@�C���ǂݍ���
		std::shared_ptr<ID3D11ShaderResourceView> CreateWICTextureFromFile(ID3D11Device * device, const wchar_t * path);

		// �T���v���[�̐���
		std::shared_ptr<ID3D11SamplerState> CreateSampler(ID3D11Device * device, const D3D11_SAMPLER_DESC & desc);

		//	�u�����f�B���O�X�e�[�g�̍쐬
		std::shared_ptr<ID3D11BlendState> CreateBlendState(ID3D11Device * device, const D3D11_BLEND_DESC *desc);

		//	�[�x�X�e���V���r���[�ݒ�I�u�W�F�N�g�̍쐬
		std::shared_ptr<ID3D11DepthStencilState> CreateDepthStencilState(ID3D11Device* device, const D3D11_DEPTH_STENCIL_DESC& desc);

		//	�e�N�X�`������V�F�[�_�[���\�[�X�r���[�̍쐬
		std::shared_ptr<ID3D11ShaderResourceView> CreateShaderResourceView(ID3D11Device* device, ID3D11Texture2D * resourceTexture);


		void mapping(ID3D11Resource * buffer, ID3D11DeviceContext * context, std::function<void(D3D11_MAPPED_SUBRESOURCE)> function);
	};


};