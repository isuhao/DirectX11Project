/*
*	�萔�o�b�t�@
*/

//	�f�t�H���g�͗�D��(column_major)
cbuffer Transform : register(b3) {
	column_major float3x2	mtxTransform	: packoffset(c0);
	column_major float3x2	mtxTextureTransform : packoffset(c2);
	//	�F
};


//! ���_����
/*!
�V�X�e�����ɓn���Z�}���e�B�N�X��SV_�̐ړ��������Ă���
*/
struct InputVS {
	float4 pos		: POSITION;
	float2 tex		: TEXCOORD;	//	x,y�̏�
};

struct OutputVS {
	float4	pos			: SV_POSITION;
	float2	tex			: TEXTURE;			// �e�N�X�`��UV
};

//! ���_�V�F�[�_
OutputVS RenderVS(InputVS inVert){
	OutputVS	outVert;

	outVert.pos = float4(mul(inVert.pos.xyz,mtxTransform).xy, 0, 1);

	outVert.tex = mul(float4(inVert.tex.xy, 1,0), mtxTextureTransform).xy;

	return outVert;
}

