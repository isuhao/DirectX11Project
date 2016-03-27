#include "D2DWrapper.h"


#pragma comment (lib, "Dwrite.lib")

#include "../comUtil.h"


namespace {

	std::shared_ptr<IDWriteFactory> CreateDWriteFactory() {

		IDWriteFactory* pWriteFactory = nullptr;
		HRESULT hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&pWriteFactory));

		if (FAILED(hr))
			throw std::runtime_error("DWriteFactory�̍쐬�Ɏ��s���܂���");

		return std::shared_ptr<IDWriteFactory>(pWriteFactory, comUtil::ReleaseIUnknown);
	}
}


namespace d2d {


	std::shared_ptr<IDWriteTextFormat> CreateTextFormat(
		LPCWSTR					fontName,		//	�t�H���g��
		float					fontSize,		//	�t�H���g�T�C�Y
		DWRITE_FONT_WEIGHT		fontWeight,		//	����
		DWRITE_FONT_STYLE		fontStyle,		//	�X�^�C��(�Ἀ�)
		DWRITE_FONT_STRETCH		fontStretch,	//	��(����)
		IDWriteFontCollection*	fontCollection,	//	�t�H���g�R���N�V����(nullptr�ŃV�X�e���̎��t�H���g���g��)
		LPCWSTR					locale	//	���P�[���A��{�ύX�Ȃ��ł���
		) {

		IDWriteTextFormat* pFormat = nullptr;
		HRESULT hr = CreateDWriteFactory()->CreateTextFormat(
			fontName,fontCollection,fontWeight,fontStyle,fontStretch,fontSize,locale,&pFormat);

		if (FAILED(hr))
			throw std::runtime_error("IDWriteTextFormat�̍쐬�Ɏ��s���܂���");

		return std::shared_ptr<IDWriteTextFormat>(pFormat, comUtil::ReleaseIUnknown);

	}
}