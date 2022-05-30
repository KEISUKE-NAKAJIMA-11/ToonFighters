
#include "caluculation_function.hlsli"

//-----------------------------------------
//	�X�|�b�g���C�g�\����
//-----------------------------------------
struct SPOTLIGHT
{
    float index;
    float range; //���̓͂��͈�
    float type; //�L����������
    float inner_corn;
    float outer_corn;
    float dummy0;
    float dummy1;
    float dummy2;
    float4 pos;
    float4 color;
    float4 dir;
};



//--------------------------------------------
//	�|�C���g���C�g�\����
//--------------------------------------------

struct POINTLIGHT
{
    float index;
    float range; //���̓͂��͈�
    float type; //�L����������
    float dummy;
    float4 pos;
    float4 color;
};

//--------------------------------------------
//	�O���[�o���ϐ�
//--------------------------------------------


static const int SPOTMAX = 32;


static const int POINTMAX = 32;
cbuffer CBPerFrame2 : register(b4)
{
    POINTLIGHT PointLight[POINTMAX];
    SPOTLIGHT SpotLight[SPOTMAX];
    float3 A;
    float dummy;
};

//--------------------------------------------
//	�X�|�b�g���C�g�֐�
//--------------------------------------------

// P:
//�s�N�Z�����[���h�ʒu
// N : ���[���h�@��(���K���ς�)
// E: ���������x�N�g��(���K���ς�)
// Kd:
//�g�U���˗�
// Ks : ���ʔ��˗�

float3 AddSpotLight(float3 P, float3 N, float3 E, float3 Kd, float3 Ks)
{
    float3 SL;
    float3 SC;
    float influence = 1.0;
    float3 SD = (float3) 0;
    float3 SS = (float3) 0;
    for (int j = 0; j < SPOTMAX; j++)
    {
		//�_�����Ă��Ȃ����C�g���O
        if (SpotLight[j].type == 0)
            continue;
        SL = P - SpotLight[j].pos.xyz;
        float d = length(SL);
        float r = SpotLight[j].range;
		//�͂��Ȃ����C�g���O
        if (d > r)
            continue;
        float3 SFront = normalize(SpotLight[j].dir.xyz);
        SL = normalize(SL);
        float angle = dot(SL, SFront);
        float area = SpotLight[j].inner_corn - SpotLight[j].outer_corn;
        float influence2 = SpotLight[j].inner_corn - angle;
        influence2 = saturate(1.0 - influence2 / area);
        SC = SpotLight[j].color.rgb;
        SD += ToonDiffuse(N, SL, SC, Kd) * influence * influence2;
        SS += ToonSpecular(N, SL, SC, E, Ks, 20) * influence * influence2;
    }
    return (SD + SS);
}


//--------------------------------------------
//	�|�C���g���C�g�֐�
//--------------------------------------------

// P: �s�N�Z�����[���h�ʒu
// N: ���[���h�@��(���K���ς�)
// E: ���������x�N�g��(���K���ς�)
// Kd: �g�U���˗�
// Ks: ���ʔ��˗� 

float3 AddPointLight(float3 P, float3 N, float3 E, float3 Kd, float3 Ks)
{
    float3 PL; //�|�C���g���C�g�x�N�g��
    float3 PC; //�|�C���g���C�g�F
    float influence; //�|�C���g���C�g�̉e��
    float3 PD = (float3) 0; //�g�U����
    float3 PS = (float3) 0; //���ʔ���
    for (int i = 0; i < POINTMAX; i++)
    {
		//�_�����Ă��Ȃ����C�g���O
        if (PointLight[i].type == 0)
            continue;
        PL = P - PointLight[i].pos.xyz;
        float d = length(PL);
        float r = PointLight[i].range;
		//�͂��Ȃ����C�g���O
        if (d > r)
            continue;
        influence = saturate(10.0f - d / r);
        PL = normalize(PL);
        PC = PointLight[i].color.rgb;
		//�g�U���̉��Z		
        PD += ToonDiffuse(N, PL, PC, Kd) * influence;
		//���ʌ��̉��Z
        PS += ToonSpecular(N, PL, PC, E, Ks, 0.1f) * influence;
    }
    return (PD + PS);
}

