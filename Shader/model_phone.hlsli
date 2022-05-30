

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
	float4 color    : COLOR;

	float4 world_position : WORLD_POSITION;
	float4 world_normal : WORLD_NORMAL;
	float2 Speed : TEXCOORD1;
    float3 VX : TEXCOORD2; //�ڋ�ԗpX��
    float3 VY : TEXCOORD3; //�ڋ�ԗpY��
    float3 VZ : TEXCOORD4; //�ڋ�ԗpZ��
};

cbuffer CbScene : register(b0)
{
    row_major float4x4 view;
    row_major float4x4 view_projection;
    row_major float4x4 prevview_projection;
    float4 light_direction;
    row_major float4x4 da;
    row_major float4x4 d2;
    float4 eye_position;
    float4 light_color;
    float4 outoline;
    float4 camera_front;

};

#define MAX_BONES 32
cbuffer CbMesh : register(b1)
{
	row_major float4x4	bone_transforms[MAX_BONES];

};

cbuffer CbSubset : register(b2)
{
    float4 material_color;
    row_major float4x4 world_view_projection;
    row_major float4x4 world;
    
    float4 xModelBaseColor; //���f���̐F rgb:color a:alpha
    float4 xModelEmissionColor; //���f���S�̂𔭌������� rgb:color a:strength
    float4 xModelEmissionEdgeColor; //���f���̉��𔭌������� rgb:color a:strength

    float3 xModelLocalLightVec; //���f���̃��[�J�����C�g�̌���
    float xModelEdgeThreshold; //���f���̃������C�g��臒l

    float2 dammy; //�\��p�̃e�N�X�`���I�t�Z�b�g
    float xModelShadowBias; //�V���h�E�}�b�v��臒l
    float xModelToonThreshold; //�g�D�[���̖��Â�臒l

    int xModelReceivedShadow; //�V���h�E���󂯂邩�ǂ��� 1:�󂯂� 0:�󂯂Ȃ�
    float xModelZoffset; //���f����z�����ւ������炷���ǂ���
    float xModelpadding1;
    float xModelpadding2;
    
};

cbuffer CBEnvironment : register(b4)
{
    float4 xEnvironmentAmbientBrightColor; //rgb:�g�D�[���̖��邢�F�Ƀu�����h�������F a:�u�����h��
    float4 xEnvironmentAmbientDarkColor; //rgb:�g�D�[���̈Â��F�i�e���܂ށj�Ƀu�����h�������F a:�u�����h��
    row_major float4x4 xEnvironmentGlobalLightVP; //���C�g�̃r���[�v���W�F�N�V�����s��
	
    float3 xEnvironmentGlobalLightVec; //�e�����p�̃��C�g�x�N�g��
    float xEnvironmentPadding1;
};

