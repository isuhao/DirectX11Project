#pragma once

#include <d2d1.h>
#include <dwrite.h>
#include <wchar.h>
#include <memory>

#include <stdexcept>


namespace d2 {
	//!	DXGI�̃T�[�t�F�C�X�ւ̕`����s��Direct2D�����_�[�^�[�Q�b�g���쐬
	std::shared_ptr<ID2D1RenderTarget> CreateDXGISurfaceRenderTarget(
		IDXGISurface *surface, const D2D1_RENDER_TARGET_PROPERTIES& rtp = D2D1::RenderTargetProperties());



	std::shared_ptr<ID2D1SolidColorBrush> CreateSolidColorBrush(
		ID2D1RenderTarget* renderTarget, const D2D1_COLOR_F &color);
	


	std::shared_ptr<IDWriteTextFormat> CreateTextFormat(
		LPCWSTR					fontName,		//	�t�H���g��
		float					fontSize,		//	�t�H���g�T�C�Y
		DWRITE_FONT_WEIGHT		fontWeight = DWRITE_FONT_WEIGHT_NORMAL,		//	����
		DWRITE_FONT_STYLE		fontStyle  = DWRITE_FONT_STYLE_NORMAL,		//	�X�^�C��(�Ἀ�)
		DWRITE_FONT_STRETCH		fontStretch = DWRITE_FONT_STRETCH_NORMAL,	//	��(����)
		IDWriteFontCollection*	fontCollection = nullptr,	//	�t�H���g�R���N�V����(nullptr�ŃV�X�e���̎��t�H���g���g��)
		LPCWSTR					locale = L"ja-jp"			//	���P�[���A��{�ύX�Ȃ��ł���
		);

} //	end of namespace d2