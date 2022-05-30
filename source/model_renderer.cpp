#include "model_renderer.h"
#include "misc.h"
#include "Shadermanager.h"
#include "camera.h"
#include "game_scene.h"

extern float _Outline;

//3Dモデルを描画するクラス


ModelRenderer::ModelRenderer(ID3D11Device* device)
{
	HRESULT hr = S_OK;
	// 頂点シェーダー
	{
		// ファイルを開く

		// 入力レイアウト
		D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BONES",    0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
#if 0
		create_vs_from_cso(device, "./Shader/model_vs.cso", m_vertex_shader.GetAddressOf(), m_input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
#else 
		
		//create_vs_from_cso(device, "./Shader/model_phong_vs.cso", m_vertex_shader_phong.GetAddressOf(), m_input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
		//create_vs_from_cso(device, "./Shader/model_simple_urp_toon_vs.cso", m_vertex_shader.GetAddressOf(), m_input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
	    create_vs_from_cso(device, "./Shader/model_toon_vs.cso", m_vertex_shader.GetAddressOf(), m_input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
		//create_vs_from_cso(device, "./Shader/model_phong_vs.cso", m_vertex_shader_phong.GetAddressOf(), m_input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));

#endif
	}

	// ピクセルシェーダー
	{
		// ファイルを開く
#if 0
		create_ps_from_cso(device, "./Shader/model_ps.cso", m_pixel_shader.GetAddressOf());
#else 
		//create_ps_from_cso(device, "./Shader/model_phong_ps.cso", m_pixel_shader_phong.GetAddressOf());

		//create_ps_from_cso(device, "./Shader/model_simple_urp_toon_ps.cso", m_pixel_shader.GetAddressOf());
		//create_ps_from_cso(device, "./Shader/model_phong_ps.cso", m_pixel_shader.GetAddressOf());

		create_ps_from_cso(device, "./Shader/model_toon_ps.cso", m_pixel_shader.GetAddressOf());
		/////////////////////Geometry Shader///////////////////
		create_gs_from_cso(device, "./Shader/model_toon_gs.cso", m_geometry_shader.GetAddressOf());


#endif
	}

	// 定数バッファ
	{
		// シーン用バッファ
		D3D11_BUFFER_DESC desc;
		::memset(&desc, 0, sizeof(desc));
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		desc.ByteWidth = sizeof(CbScene);
		desc.StructureByteStride = 0;

		HRESULT hr = device->CreateBuffer(&desc, 0, m_cb_scene.GetAddressOf());

		// ライト用バッファ
		desc.ByteWidth = sizeof(CBforLight);

		hr = device->CreateBuffer(&desc, 0, cb_light.GetAddressOf());

		// サブセット用バッファ
		desc.ByteWidth = sizeof(CbSubset);

		hr = device->CreateBuffer(&desc, 0, m_cb_subset.GetAddressOf());

		// サブセット用バッファ
		desc.ByteWidth = sizeof(CbEnviroment);

		hr = device->CreateBuffer(&desc, 0, m_cb_enviroment.GetAddressOf());

		//トゥーン用バッファ
		desc.ByteWidth = sizeof(CbUrpToon);

		hr = device->CreateBuffer(&desc, 0, m_cb_toon.GetAddressOf());

		// メッシュ用バッファ
		desc.ByteWidth = sizeof(CbMesh);
		desc.Usage = D3D11_USAGE::D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		hr = device->CreateBuffer(&desc, 0, m_cb_mesh.GetAddressOf());
	}

	// ブレンドステート
	{
		D3D11_BLEND_DESC desc;
		::memset(&desc, 0, sizeof(desc));
		desc.AlphaToCoverageEnable = false;
		desc.IndependentBlendEnable = false;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		HRESULT hr = device->CreateBlendState(&desc, m_blend_state.GetAddressOf());

	}

	// 深度ステンシルステート
	{
		D3D11_DEPTH_STENCIL_DESC desc;
		::memset(&desc, 0, sizeof(desc));
		desc.DepthEnable = true;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

		HRESULT hr = device->CreateDepthStencilState(&desc, m_depth_stencil_state.GetAddressOf());

	}

	// ラスタライザーステート
	{
		D3D11_RASTERIZER_DESC desc;
		::memset(&desc, 0, sizeof(desc));
		desc.FrontCounterClockwise = true;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0;
		desc.SlopeScaledDepthBias = 0;
		desc.DepthClipEnable = true;
		desc.ScissorEnable = false;
		desc.MultisampleEnable = true;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_BACK;
		desc.AntialiasedLineEnable = false;

		HRESULT hr = device->CreateRasterizerState(&desc, m_rasterizer_state[0].GetAddressOf());

	}


	{
		D3D11_RASTERIZER_DESC desc;
		::memset(&desc, 0, sizeof(desc));
		desc.FrontCounterClockwise = true;
		desc.DepthBias = 0;
		desc.DepthBiasClamp = 0;
		desc.SlopeScaledDepthBias = 0;
		desc.DepthClipEnable = true;
		desc.ScissorEnable = false;
		desc.MultisampleEnable = true;
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_FRONT;
		desc.AntialiasedLineEnable = false;

		HRESULT hr = device->CreateRasterizerState(&desc, m_rasterizer_state[1].GetAddressOf());

	}


	// サンプラステート
	{
		D3D11_SAMPLER_DESC desc;
		::memset(&desc, 0, sizeof(desc));
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = -FLT_MAX;
		desc.MaxLOD = FLT_MAX;
		desc.BorderColor[0] = 1.0f;
		desc.BorderColor[1] = 1.0f;
		desc.BorderColor[2] = 1.0f;
		desc.BorderColor[3] = 1.0f;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;

		HRESULT hr = device->CreateSamplerState(&desc, m_sampler_state.GetAddressOf());

	}

	// ダミーテクスチャ
	{
		const int width = 8;
		const int height = 8;
		UINT pixels[width * height];
		::memset(pixels, 0xFF, sizeof(pixels));

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA data;
		::memset(&data, 0, sizeof(data));
		data.pSysMem = pixels;
		data.SysMemPitch = width;

		Microsoft::WRL::ComPtr<ID3D11Texture2D>	texture;
		HRESULT hr = device->CreateTexture2D(&desc, &data, texture.GetAddressOf());


		hr = device->CreateShaderResourceView(texture.Get(), nullptr, m_dummy_srv.GetAddressOf());

	}
}

// 描画開始
extern bool toon;//test to cell shade
void ModelRenderer::Begin(ID3D11DeviceContext* context, DirectX::XMFLOAT4X4& view_projection, DirectX::XMMATRIX& world_transform, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view)
{
	//if (toon)
	//{
		context->VSSetShader(m_vertex_shader.Get(), nullptr, 0);
		context->GSSetShader(m_geometry_shader.Get(), nullptr, 0);
		context->PSSetShader(m_pixel_shader.Get(), nullptr, 0);
	//}

	//test
	//else
	{
		//context->VSSetShader(m_vertex_shader_phong.Get(), nullptr, 0);
		//context->PSSetShader(m_pixel_shader_phong.Get(), nullptr, 0);
	}

#if 0
	context->GSSetShader(m_geometry_shader.Get(), nullptr, 0);
#endif // 1

	context->IASetInputLayout(m_input_layout.Get());

	ID3D11Buffer* constant_buffers[] =
	{
		m_cb_scene.Get(),
		m_cb_mesh.Get(),
		m_cb_subset.Get(),
	    m_cb_toon.Get(),
		cb_light.Get(),
		m_cb_enviroment.Get(),
	};
	context->VSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);
	context->PSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);
	context->GSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);

	const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(m_blend_state.Get(), blend_factor, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_depth_stencil_state.Get(), 0);
	context->RSSetState(m_rasterizer_state[culling].Get());
	context->PSSetSamplers(0, 1, m_sampler_state.GetAddressOf());

	// シーン用定数バッファ更新
	DirectX::XMMATRIX wm = world_transform * DirectX::XMLoadFloat4x4(&view_projection);
	DirectX::XMFLOAT4X4 wvp;
	DirectX::XMFLOAT4X4 w;
	DirectX::XMStoreFloat4x4(&w, world_transform);
	DirectX::XMStoreFloat4x4(&wvp, wm);
	CbScene cb_scene;
	cb_scene.view = view;
	cb_scene.view_projection = view_projection;
	cb_scene.prev_projection = projection;
	cb_scene.light_direction = light_direction;
	cb_scene.world_view_projection = wvp;
	cb_scene.world = w;
	cb_scene.outline.x = _Outline ;
	cb_scene.eye_position =
		/*DirectX::XMFLOAT4(light_direction.x,light_direction.y,-light_direction.z,light_direction.w);*/
		DirectX::XMFLOAT4(camera->pos.x, camera->pos.y, camera->pos.z, 1.0f);

	cb_scene.camera_front = DirectX::XMFLOAT4(camera->at.x, camera->at.y, camera->at.z, 1);
	context->UpdateSubresource(m_cb_scene.Get(), 0, 0, &cb_scene, 0, 0);

	DirectX::XMVECTOR At = DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z));
	DirectX::XMVECTOR Eye = DirectX::XMVectorSet(camera->pos.x, camera->pos.y + 25, camera->pos.z, 1.0f);
	DirectX::XMMATRIX V = DirectX::XMMatrixLookToLH(Eye, At, DirectX::XMVectorSet(0, 1, 0, 0));
	DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicLH(128.0f, 128.0f, 0.1f, 480.0f);

	CbEnviroment cb_enb;
	cb_enb.environment_ambientbrightcolor = brightColor;
	cb_enb.environment_ambientdarkcolor = darkColor;
	DirectX::XMStoreFloat3(&cb_enb.environment_global_lightVec, DirectX::XMVector3Normalize(At));
	DirectX::XMStoreFloat4x4(&cb_enb.environment_globallightvp, V * P);
	context->UpdateSubresource(m_cb_enviroment.Get(), 0, 0, &cb_enb, 0, 0);


	CBforLight _cb_light;
	memcpy(_cb_light.PointLight, Light::PointLight, sizeof(POINTLIGHT)* Light::POINTMAX);
	//memcpy(_cb_light.SpotLight, Light::SpotLight, sizeof(SPOTLIGHT)* Light::SPOTMAX);
	_cb_light.ambient = DirectX::XMFLOAT3(Light::Ambient.x, Light::Ambient.y, Light::Ambient.z);
	//コンスタントバッファ更新
	context->UpdateSubresource(cb_light.Get(), 0, 0, &_cb_light, 0, 0);
}



void ModelRenderer::Begin(ID3D11DeviceContext* context, Shader* sheder, DirectX::XMFLOAT4X4& view_projection, DirectX::XMMATRIX& world_transform, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4 _brightColor, DirectX::XMFLOAT4 _darkColor)
{

	   sheder->Acivate();

	   brightColor = _brightColor;
	   darkColor = _darkColor;

#if 0
	context->GSSetShader(m_geometry_shader.Get(), nullptr, 0);
#endif // 1

	//context->IASetInputLayout(m_input_layout.Get());

	ID3D11Buffer* constant_buffers[] =
	{
		m_cb_scene.Get(),
		m_cb_mesh.Get(),
		m_cb_subset.Get(),
		m_cb_toon.Get(),
		cb_light.Get(),
	    m_cb_enviroment.Get(),
	};
	context->VSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);
	context->PSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);
	context->GSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);

	const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(m_blend_state.Get(), blend_factor, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_depth_stencil_state.Get(), 0);
	context->RSSetState(m_rasterizer_state[culling].Get());
	context->PSSetSamplers(0, 1, m_sampler_state.GetAddressOf());

	// シーン用定数バッファ更新
	DirectX::XMMATRIX wm = world_transform * DirectX::XMLoadFloat4x4(&view_projection);
	DirectX::XMFLOAT4X4 wvp;
	DirectX::XMFLOAT4X4 w;
	DirectX::XMStoreFloat4x4(&w, world_transform);
	DirectX::XMStoreFloat4x4(&wvp, wm);
	CbScene cb_scene;
	cb_scene.view = view;
	cb_scene.view_projection = view_projection;
	cb_scene.prev_projection = projection;
	cb_scene.light_direction = light_direction;
	cb_scene.world_view_projection = wvp;
	cb_scene.world = w;
	cb_scene.outline.x = _Outline;
	cb_scene.eye_position =
		/*DirectX::XMFLOAT4(light_direction.x,light_direction.y,-light_direction.z,light_direction.w);*/
		DirectX::XMFLOAT4(camera->pos.x, camera->pos.y, camera->pos.z, 1.0f);
	cb_scene.camera_front= DirectX::XMFLOAT4(camera->at.x, camera->at.y, camera->at.z, 1);
	context->UpdateSubresource(m_cb_scene.Get(), 0, 0, &cb_scene, 0, 0);

	DirectX::XMVECTOR At = DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z));
	DirectX::XMVECTOR Eye = DirectX::XMVectorSet(camera->pos.x, camera->pos.y + 25, camera->pos.z, 1.0f);
	DirectX::XMMATRIX V = DirectX::XMMatrixLookToLH(Eye, At, DirectX::XMVectorSet(0, 1, 0, 0));
	DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicLH(128.0f, 128.0f, 0.1f, 480.0f);

	CbEnviroment cb_enb;
	cb_enb.environment_ambientbrightcolor = brightColor;
	cb_enb.environment_ambientdarkcolor = darkColor;
	DirectX::XMStoreFloat3(&cb_enb.environment_global_lightVec, DirectX::XMVector3Normalize(At));
	DirectX::XMStoreFloat4x4(&cb_enb.environment_globallightvp, V * P);
	context->UpdateSubresource(m_cb_enviroment.Get(), 0, 0, &cb_enb, 0, 0);


	CBforLight _cb_light;
	memcpy(_cb_light.PointLight, Light::PointLight, sizeof(POINTLIGHT) * Light::POINTMAX);
	//memcpy(_cb_light.SpotLight, Light::SpotLight, sizeof(SPOTLIGHT)* Light::SPOTMAX);
	_cb_light.ambient = DirectX::XMFLOAT3(Light::Ambient.x, Light::Ambient.y, Light::Ambient.z);
	//コンスタントバッファ更新
	context->UpdateSubresource(cb_light.Get(), 0, 0, &_cb_light, 0, 0);
}





void ModelRenderer::Begin(ID3D11DeviceContext* context, DirectX::XMFLOAT4X4& view_projection, DirectX::XMMATRIX& world_transform, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, CameraController camera, DirectX::XMFLOAT4X4& view)
{
	//if (toon)
	//{
	context->VSSetShader(m_vertex_shader.Get(), nullptr, 0);
	context->GSSetShader(m_geometry_shader.Get(), nullptr, 0);
	context->PSSetShader(m_pixel_shader.Get(), nullptr, 0);
	//}

	//test
	//else
	//{
	//	context->VSSetShader(m_vertex_shader_phong.Get(), nullptr, 0);
	//	context->PSSetShader(m_pixel_shader_phong.Get(), nullptr, 0);
	//}

#if 0
	context->GSSetShader(m_geometry_shader.Get(), nullptr, 0);
#endif // 1

	context->IASetInputLayout(m_input_layout.Get());

	ID3D11Buffer* constant_buffers[] =
	{
		m_cb_scene.Get(),
		m_cb_mesh.Get(),
		m_cb_subset.Get(),
		m_cb_toon.Get(),
		cb_light.Get(),
		m_cb_enviroment.Get(),
	};
	context->VSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);
	context->PSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);
	//context->GSSetConstantBuffers(0, ARRAYSIZE(constant_buffers), constant_buffers);

	const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	context->OMSetBlendState(m_blend_state.Get(), blend_factor, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_depth_stencil_state.Get(), 0);
	context->RSSetState(m_rasterizer_state[culling].Get());
	context->PSSetSamplers(0, 1, m_sampler_state.GetAddressOf());

	// シーン用定数バッファ更新
	DirectX::XMMATRIX wm = world_transform * DirectX::XMLoadFloat4x4(&view_projection);
	DirectX::XMFLOAT4X4 wvp;
	DirectX::XMFLOAT4X4 w;
	DirectX::XMStoreFloat4x4(&w, world_transform);
	DirectX::XMStoreFloat4x4(&wvp, wm);




	CbScene cb_scene;
	cb_scene.view = view;
	cb_scene.view_projection = view_projection;
	cb_scene.prev_projection = projection;
	cb_scene.light_direction = light_direction;
	cb_scene.world_view_projection = wvp;
	cb_scene.world = w;
	cb_scene.outline.x = 0.241f;
	cb_scene.eye_position =
		/*DirectX::XMFLOAT4(light_direction.x,light_direction.y,-light_direction.z,light_direction.w);*/
		DirectX::XMFLOAT4(camera.GetEye().x, camera.GetEye().y, camera.GetEye().z, 1.0f);
	context->UpdateSubresource(m_cb_scene.Get(), 0, 0, &cb_scene, 0, 0);


	DirectX::XMVECTOR At = DirectX::XMLoadFloat3(&DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z));
	DirectX::XMVECTOR Eye = DirectX::XMVectorSet(camera.GetEye().x, camera.GetEye().y + 25, camera.GetEye().z, 1.0f);
	DirectX::XMMATRIX V = DirectX::XMMatrixLookToLH(Eye, At, DirectX::XMVectorSet(0, 1, 0, 0));
	DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicLH(128.0f, 128.0f, 0.1f, 480.0f);

	CbEnviroment cb_enb;
	cb_enb.environment_ambientbrightcolor = brightColor;
	cb_enb.environment_ambientdarkcolor = darkColor;
	DirectX::XMStoreFloat3(&cb_enb.environment_global_lightVec, DirectX::XMVector3Normalize(At));
	DirectX::XMStoreFloat4x4(&cb_enb.environment_globallightvp, V * P);
	context->UpdateSubresource(m_cb_enviroment.Get(), 0, 0, &cb_enb, 0, 0);



	CBforLight _cb_light;
	memcpy(_cb_light.PointLight, Light::PointLight, sizeof(POINTLIGHT) * Light::POINTMAX);
	//memcpy(_cb_light.SpotLight, Light::SpotLight, sizeof(SPOTLIGHT)* Light::SPOTMAX);
	_cb_light.ambient = DirectX::XMFLOAT3(Light::Ambient.x, Light::Ambient.y, Light::Ambient.z);
	//コンスタントバッファ更新
	context->UpdateSubresource(cb_light.Get(), 0, 0, &_cb_light, 0, 0);
}



// 描画
void ModelRenderer::Draw(ID3D11DeviceContext* context, Model* model, const DirectX::XMFLOAT3& localLightDir,
	const DirectX::XMFLOAT4& baseColor,
	const DirectX::XMFLOAT4& emissiveColor,
	const DirectX::XMFLOAT4& emissiveEdgeColor,
	float edgeThreshold, float toonThreshold, float shadowBias,
	BOOL shadowReceived, float zOffset, bool usecolour, DirectX::XMFLOAT4 colours)
{

	const std::vector<Model::s_node>& nodes = model->GetNodes();

	for (auto& mesh : model->GetResource()->GetMeshes())
	{


			D3D11_MAPPED_SUBRESOURCE mappedCBMESH;

			HRESULT hr = context->Map(m_cb_mesh.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCBMESH);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
			DirectX::XMFLOAT4X4* matrixe_cursor = reinterpret_cast<DirectX::XMFLOAT4X4*>(mappedCBMESH.pData);
			if (mesh.node_indices.size() > 0)
			{
				for (size_t i = 0; i < mesh.node_indices.size(); ++i)
				{
					DirectX::XMMATRIX inverse_transform = DirectX::XMLoadFloat4x4(&mesh.inverseTransforms[i].ConvertXMFLOAT4x4());
					DirectX::XMMATRIX world_transform = DirectX::XMLoadFloat4x4(&nodes.at(mesh.node_indices[i]).worldTransform);
					DirectX::XMMATRIX bone_transform = inverse_transform * world_transform;
					DirectX::XMStoreFloat4x4(matrixe_cursor + i, bone_transform);
				}
			}
			else
			{
				matrixe_cursor[0] = nodes.at(mesh.node_index).worldTransform;
			}
			context->Unmap(m_cb_mesh.Get(), 0);

			UINT stride = sizeof(ModelResource::s_vertex);
			UINT offset = 0;
			context->IASetVertexBuffers(0, 1, mesh.vertex_buffer.GetAddressOf(), &stride, &offset);
			context->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			for (const ModelResource::s_subset& subset : mesh.subsets)
			{
				CbSubset cb_subset;
				CbScene cb_scene;
				usecolour ? cb_subset.material_color = colours : cb_subset.material_color = subset.material->diffuse.color;


				cb_subset.world_view_projection = cb_scene.world_view_projection;
				cb_subset.world = cb_scene.world;
			
				usecolour ? cb_subset.model_base_color = colours : cb_subset.model_base_color = subset.material->diffuse.color;
				cb_subset.model_emission_color = emissiveColor;
				cb_subset.model_emission_edge_color = emissiveEdgeColor;
				cb_subset.model_local_light = localLightDir;
			
			

				
				cb_subset.model_edge_threshold = edgeThreshold;	
				cb_subset.model_shadowbias = shadowBias;	    
				cb_subset.model_toon_threshold = toonThreshold;	

	
				cb_subset.model_receivedShadow = shadowReceived;
				cb_subset.model_zoffset = zOffset;

				context->PSSetShaderResources(0, 1, subset.material->diffuse.shader_resource_view ? &subset.material->diffuse.shader_resource_view : m_dummy_srv.GetAddressOf());
				context->PSSetShaderResources(1, 1, subset.material->bump.shader_resource_view ? &subset.material->bump.shader_resource_view : m_dummy_srv.GetAddressOf());
				context->PSSetShaderResources(2, 1, subset.material->speculer.shader_resource_view ? &subset.material->speculer.shader_resource_view : m_dummy_srv.GetAddressOf());

				context->PSSetShaderResources(3, 1, subset.material->diffuse.shader_resource_view ? &subset.material->diffuse.shader_resource_view : m_dummy_srv.GetAddressOf());
				context->PSSetSamplers(0, 1, m_sampler_state.GetAddressOf());
				context->PSSetSamplers(1, 1, m_sampler_state.GetAddressOf());
				context->PSSetSamplers(2, 1, m_sampler_state.GetAddressOf());


			/*	cb_subset.diffuse_color = subset.material->diffuse.color;
				cb_subset.ambient_color = subset.material->ambient.color;
				cb_subset.speculer_color = subset.material->speculer.color;
				cb_subset.ambient_color.w = (float)_is_pbr;
				cb_subset.has_emissive_map = subset.material->emissive.shader_resource_view != nullptr;
				if (subset.material->metalrough.shader_resource_view == nullptr)
				{
					cb_subset.has_parm_map = 0;
					cb_subset.speculer_color.w = _metallic;
					cb_subset.diffuse_color.w = _roughness;
				}
				else
				{
					cb_subset.has_parm_map = 1;
					cb_subset.speculer_color.w = subset.material->metalrough.color.x;
					cb_subset.diffuse_color.w = subset.material->metalrough.color.y;
				}
				cb_subset.has_normalMap = subset.material->bump.shader_resource_view != nullptr;*/

				context->UpdateSubresource(m_cb_subset.Get(), 0, 0, &cb_subset, 0, 0);
				context->DrawIndexed(subset.index_count, subset.index_start, 0);
			}
		}

	}






// 描画終了
void ModelRenderer::End(ID3D11DeviceContext* context)
{
	


	context->VSSetShader(nullptr, nullptr, 0);

	context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(nullptr, nullptr, 0);
	context->IASetInputLayout(nullptr);
}


void ModelRenderer::End(Shader* shader)
{

	shader->Inactivate();
}