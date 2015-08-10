#pragma once

#include <DirectXMath.h>

namespace axis{

	//!	�g�����W�n�̃��X�g�A�b�v
	//!	Blender�ȊO�ɂ��g���ꍇ�͓K�X�ǉ����Ă�������
	enum ModelCoordinate {
		Blender,
	};



	//!	DirectX�̍���n���W�n�֍��킹�ă��f������]������s����擾
	/**
	* @param  modelCoordinate	���f�����̍��W�n
	*
	* @return �ϊ��s��A���[�J���ϊ��~����ꂽ�ϊ��~���[���h�ϊ��̌`�Ŏg��
	 */
	inline DirectX::XMMATRIX getAxisConvertMatrix(ModelCoordinate modelCoordinate){
		
		using namespace DirectX;

		switch (modelCoordinate) {

		case ModelCoordinate::Blender:
			return XMMatrixRotationX(-XM_PI / 2)*XMMatrixRotationY(XM_PI) * XMMatrixScaling(-1, 1, 1);

		default:
			assert(false);
			return XMMatrixIdentity();
		}

	}

}	//	end of namespace axis
