#pragma once

#include <WinUser.h>

namespace sys {

	const int Default_FPS = 30;

	//!	�X�V�������s������true���Ԃ�B�A�v���P�[�V�����I���̏ꍇ��false���Ԃ�B
	bool Update();

	void SetFPS(int fps);

	MSG GetLastMessage();

}