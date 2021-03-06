#pragma once

#include <memory>
#include <d3d11.h>
#include <functional>

namespace d3d {
	

	//	主に3D描画レイヤーを保持、操作
	class SceneLayer3D {
	public:
		static const float ClipNearZ;
		static const float ClipFarZ;

	private:
		//	共用定数バッファのサイズ、現状使い得る最大サイズで決め打ち
		const size_t ConstantBufferSize = sizeof(DirectX::XMFLOAT4X4)*32;
		
	private:
		std::shared_ptr<ID3D11DeviceContext>	_context;
		std::shared_ptr<IDXGISwapChain>			_swapChain;

		std::shared_ptr<ID3D11RenderTargetView>	_renderTargetView;
		std::shared_ptr<ID3D11DepthStencilView>	_depthStencilView;

		std::shared_ptr<ID3D11Buffer>		_constantBuffer;
		std::shared_ptr<ID3D11Buffer>		_projectionMtxBuffer;

		std::shared_ptr<ID3D11BlendState>	_blendState;
		std::shared_ptr<ID3D11SamplerState>	_samplerState;

		//	投影行列指定の時に使うパラメータ類
		float _aspectRatio;
		float _fovRadian;

	public:
		SceneLayer3D() :
			_aspectRatio(0),
			_fovRadian(0){}

		void init(ID3D11Device* device, const DXGI_MODE_DESC& displayMode, HWND hWnd);

		//	異方性フィルタを使う、テクスチャは繰り返しにする
		void setCustomSamplar(ID3D11Device* device);
		//	αブレンドを有効に
		void setCustomBlendState(ID3D11Device* device);

		/**
		 *	プロジェクション行列を設定
		 *	@param fovRadian	視野角（ラジアン単位）
		 *	@param startSlot	シェーダーのバッファスロットNo
		 */
		void setProjection(float fovRadian, UINT startSlot);

		DirectX::XMFLOAT4X4 getProjectionMatrix()const {

			DirectX::XMFLOAT4X4 retMtx;
			DirectX::XMMATRIX mtxProj = DirectX::XMMatrixPerspectiveFovLH(
				_fovRadian, _aspectRatio, ClipNearZ, ClipFarZ);
			XMStoreFloat4x4(&retMtx, DirectX::XMMatrixTranspose(mtxProj));
			
			return retMtx;
		}

		/**
		 *	定数バッファに数値書き込み、設定
		 *	@param mapFunc		マッピング用関数
		 *	@param startSlot	シェーダーのバッファスロットNo
		 */
		void setConstants(std::function<void(D3D11_MAPPED_SUBRESOURCE)> mapFunc, UINT startSlot);

		ID3D11DeviceContext* getContext()const{
			return _context.get();
		}

		//! バックバッファおよび深度バッファをクリア
		void clearViews() {

			static float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

			_context->ClearRenderTargetView(_renderTargetView.get(), ClearColor);
			_context->ClearDepthStencilView(_depthStencilView.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
		}
		
		HRESULT present() {
			return _swapChain->Present(0, 0);
		}

	};

};