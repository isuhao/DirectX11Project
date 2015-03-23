/*
*	�萔�o�b�t�@
*/



cbuffer Projection : register(b0) {
	matrix	mtxProj		: packoffset(c0);
};

cbuffer Camera : register(b1) {
	matrix	mtxView		: packoffset(c0);
};

cbuffer World : register(b2) {
	matrix	Bones[32]	: packoffset(c0);
};

//! ���_����
/*!
�V�X�e�����ɓn���Z�}���e�B�N�X��SV_�̐ړ��������Ă���
*/
struct InputVS {
	float4 pos		: POSITION;
	float3 normal	: NORMAL;
	float4 tangent	: TANGENT;
	float4 color	: COLOR;
	float2 tex		: TEXCOORD;
	uint4  boneIndices	: BLENDINDICES;
	float4 blendWeights : BLENDWEIGHT;
};

struct OutputVS {
	float4	pos			: SV_POSITION;
	float2	tex			: TEXTURE;			// �e�N�X�`��UV
};

//! ���_�V�F�[�_
OutputVS RenderVS(InputVS inVert) {
	OutputVS	outVert;

	matrix skinTransform = Bones[inVert.boneIndices.x] * inVert.blendWeights.x;
	skinTransform += Bones[inVert.boneIndices.y] * inVert.blendWeights.y;
	skinTransform += Bones[inVert.boneIndices.z] * inVert.blendWeights.z;
	skinTransform += Bones[inVert.boneIndices.w] * inVert.blendWeights.w;
	
	matrix	mtxVP = mul(mtxView, mtxProj);
	matrix	mtxWVP = mul(skinTransform, mtxVP);
	outVert.pos = mul(inVert.pos, mtxWVP);

	outVert.tex = inVert.tex;

	//	Blender����̏o�͂Ƃ��Č��ߑł��Ay���]
	//	�����Ƃ��ꍇ��Material����UVTransformMtx�����������Ă��Ċ|����K�v����
	outVert.tex.y = 1 - outVert.tex.y;

	return outVert;
}
