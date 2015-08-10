#pragma once 

#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <d3d11.h>

//	todo:���O�t�����S�̓I�Ɉ�a������̂ŏC��


/*
			static std::unordered_map<std::wstring, std::shared_ptr<ID3D11ShaderResourceView>>	_texContainer;
			static std::mutex	_mutex;
			*/
namespace dx11 {

	namespace ResourceCache{

	namespace Texture {
		void PreLoadAsync(std::vector<std::wstring> texPathList);
		std::shared_ptr<ID3D11ShaderResourceView> Get(const std::wstring& texturePath, bool useCache = true);
		void Release(const std::wstring& textureName);
		void ReleaseAll();
	};

	namespace Shader {
		//!	������
		std::shared_ptr<ID3D11VertexShader> GetVS(TCHAR filePath[], CHAR entryPoint[]);
		//!	������
		std::shared_ptr<ID3DBlob>			GetBlob(TCHAR filePath[], CHAR entryPoint[]);
		//!	������
		std::shared_ptr<ID3D11PixelShader>	GetPS(TCHAR filePath[], CHAR entryPoint[]);
	}

	}
}

