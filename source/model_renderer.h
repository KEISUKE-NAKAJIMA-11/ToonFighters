#pragma once

#include <memory>
#include <d3d11.h>
#include "Model.h"
#include "camera.h"
#include "cameracontroller.h"
#include "Light.h"
#include "reworkshder.h"


class ModelRenderer
{
public:
	ModelRenderer(ID3D11Device* device);
	void Begin(ID3D11DeviceContext* context, DirectX::XMFLOAT4X4& view_projection, DirectX::XMMATRIX& world_transform, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view);
	void Begin(ID3D11DeviceContext* context, Shader* sheder, DirectX::XMFLOAT4X4& view_projection, DirectX::XMMATRIX& world_transform, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4 _brightColor, DirectX::XMFLOAT4 _darkColor);
	//void Begin(ID3D11DeviceContext* context, Shader* sheder, DirectX::XMFLOAT4X4& view_projection, DirectX::XMMATRIX& world_transform, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view);
	void Begin(ID3D11DeviceContext* context, DirectX::XMFLOAT4X4& view_projection, DirectX::XMMATRIX& world_transform, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, CameraController camera, DirectX::XMFLOAT4X4& view);

	void Draw(ID3D11DeviceContext* context, Model* model, const DirectX::XMFLOAT3& localLightDir, const DirectX::XMFLOAT4& baseColor, const DirectX::XMFLOAT4& emissiveColor, const DirectX::XMFLOAT4& emissiveEdgeColor, float edgeThreshold, float toonThreshold, float shadowBias, BOOL shadowReceived, float zOffset, bool usecolour, DirectX::XMFLOAT4 colours);

	int culling = 0;

	DirectX::XMFLOAT4 brightColor = { 1, 1, 1, 1 };
	DirectX::XMFLOAT4 darkColor = { 0, 0, 0, 1 };

	~ModelRenderer() 
	{
	}

	//void Begin(ID3D11DeviceContext* context,  DirectX::XMFLOAT4X4& view_projection,  DirectX::XMMATRIX& world_transform, const DirectX::XMFLOAT4& light_direction, std::unique_ptr<Camera> camera);

	//void Draw(ID3D11DeviceContext* context, Model* model, bool usecolour = false, DirectX::XMFLOAT4 colours = {1,1,1,1});
	//void Draw(ID3D11DeviceContext* context, Model& model);
	
	void End(ID3D11DeviceContext* context);

	void End(Shader* shader);

private:
	static const int MaxBones = 2000;

	struct CbScene
	{
		DirectX::XMFLOAT4X4	view;
		DirectX::XMFLOAT4X4	view_projection;
		DirectX::XMFLOAT4X4	prev_projection;
		DirectX::XMFLOAT4	light_direction;
		DirectX::XMFLOAT4X4 world_view_projection;
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 eye_position = { 0, 0, -10, 1 };
		DirectX::XMFLOAT4 light_color = { 1, 1, 1, 1 };
		DirectX::XMFLOAT4 outline = { 0.5f,1,1,1 };
		DirectX::XMFLOAT4 camera_front = { 1, 1, 1, 1 };

	};

	struct CbMesh
	{
		DirectX::XMFLOAT4X4	bone_transforms[MaxBones];
		DirectX::XMFLOAT4X4 local_matrix[MaxBones];
		DirectX::XMFLOAT4X4 parent_matrix[MaxBones];
		DirectX::XMFLOAT4X4 inverce_matrix[MaxBones];
	

	};

	struct CbSubset
	{
		DirectX::XMFLOAT4	material_color;
		DirectX::XMFLOAT4X4 world_view_projection;
		DirectX::XMFLOAT4X4 world;

		XMFLOAT4 model_base_color;				//���f���̐F rgb:color a:alpha
		XMFLOAT4 model_emission_color;			//���f���S�̂𔭌������� rgb:color a:strength
		XMFLOAT4 model_emission_edge_color;		//���f���̉��𔭌������� rgb:color a:strength

		XMFLOAT3 model_local_light;			//���f���̃��[�J�����C�g�̌���
		float model_edge_threshold;				//���f���̃������C�g��臒l

		XMFLOAT2 dammy;				
		float model_shadowbias;					//�V���h�E�}�b�v��臒l
		float model_toon_threshold;				//�g�D�[���̖��Â�臒l

		int model_receivedShadow;				//�V���h�E���󂯂邩�ǂ��� 1:�󂯂� 0:�󂯂Ȃ�
		float model_zoffset;					//Z�����ւ������炷���ǂ���
		float modelp_adding1;
		float model_padding2;


	};


	struct CbEnviroment
	{
		XMFLOAT4 environment_ambientbrightcolor;	//rgb:�g�D�[���̖��邢�F�Ƀu�����h�������F a:�u�����h��
		XMFLOAT4 environment_ambientdarkcolor;		//rgb:�g�D�[���̈Â��F�i�e���܂ށj�Ƀu�����h�������F a:�u�����h��
		XMFLOAT4X4 environment_globallightvp;		//���C�g�̃r���[�v���W�F�N�V�����s��

		XMFLOAT3 environment_global_lightVec;		//�e�����p�̃��C�g�x�N�g��
		float dammy;
	};


	struct CbUrpToon
	{
	

		// base color
		DirectX::XMFLOAT4 BaseMap_ST;
		DirectX::XMFLOAT4 BaseColor;

		// alpha
		float Cutoff;
		// high level settings
		float IsFace;
		// emission
		float UseEmission;
		float EmissionMulByBaseColor;

		DirectX::XMFLOAT3 EmissionColor;
		float dummy;

		DirectX::XMFLOAT3 EmissionMapChannelMask;
		float dummy2;
		// occlusion

		float UseOcclusion;
		float OcclusionStrength;
		float OcclusionRemapStart;
		float OcclusionRemapEnd;

		DirectX::XMFLOAT4 OcclusionMapChannelMask;


		// lighting
		DirectX::XMFLOAT3 IndirectLightMinColor;
		float dummy3;

		float CelShadeMidPoint;
		float CelShadeSoftness;
		// shadow mapping
		float ReceiveShadowMappingAmount;
		float ReceiveShadowMappingPosOffset;


		DirectX::XMFLOAT3 ShadowMapColor;
		float dummy4;
		// outline
	
		DirectX::XMFLOAT3 OutlineColor;
		float dummy5;

		float OutlineWidth;
		float OutlineZOffset;
		float OutlineZOffsetMaskRemapStart;
		float OutlineZOffsetMaskRemapEnd;

	};



	//�R���X�^���g�o�b�t�@�\����
	struct CBforLight
	{
		POINTLIGHT  PointLight[Light::POINTMAX];
		SPOTLIGHT  SpotLight[Light::SPOTMAX];
		DirectX::XMFLOAT3 ambient;
		float dummy;
	};



	Microsoft::WRL::ComPtr<ID3D11Buffer>			m_cb_scene;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			m_cb_mesh;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			m_cb_subset;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			m_cb_enviroment;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			m_cb_toon;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			cb_light;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_vertex_shader_phong;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		m_vertex_shader_gtoon;
	//Shadow map
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		s_vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		s_pixel_shader;

	Microsoft::WRL::ComPtr<ID3D11GeometryShader>		m_geometry_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pixel_shader_phong;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		m_pixel_shader_gtoon;

	Microsoft::WRL::ComPtr<ID3D11InputLayout>		m_input_layout;

	Microsoft::WRL::ComPtr<ID3D11BlendState>		m_blend_state;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>	m_rasterizer_state[2];
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>	m_depth_stencil_state;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>		m_sampler_state;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	m_dummy_srv;


	CbMesh cbMesh = CbMesh();//�R���X�^���g�o�b�t�@�p�̕ϐ�
};