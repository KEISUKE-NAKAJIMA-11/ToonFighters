#pragma once
#include <iostream>
#include <fstream>
#include <shlwapi.h>
#undef max
#undef min


#include "framebuffer.h"

#include "rasterizer.h"
#include "bloom.h"
#include "Shadermanager.h"

//モーションブラークラス(いったん実装ストップ)

class PixelMotionBulur : fullscreen_quad
{
public:
	struct shader_constants
	{


		template<class T>
		void serialize(T& archive)
		{
			//TODO:Add 'number_of_ghosts', "lens_flare_intensity"
			//archive(CEREAL_NVP(glow_extraction_threshold), CEREAL_NVP(blur_convolution_intensity), CEREAL_NVP(lens_flare_threshold), CEREAL_NVP(lens_flare_ghost_dispersal));
		}
	};
	//std::unique_ptr<<shader_constants>> constant_buffer;
	ID3D11InputLayout* g_pPolygonVertexLayout = nullptr;
     PixelMotionBulur(ID3D11Device* device) : fullscreen_quad(device)
     {
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

	
     	//constant_buffer = std::make_unique<Constant_buffer<shader_constants>>(device);
		speed_map_texture = std::make_unique<framebuffer>(device, 512, 512, false, 1, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_UNKNOWN, true, false, true/*generate_mips*/);
	    motion_bulur = std::make_unique<framebuffer>(device, 512, 512, false, 1, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_UNKNOWN, true, false, true/*generate_mips*/);
		motion_bulur_vs = std::make_unique<vertex_shader>(device, "./Shader/motion_bulur_vs.cso", input_element_desc, ARRAYSIZE(input_element_desc));
		motion_bulur_ps = std::make_unique<pixel_shader>(device, "./Shader/motion_bulur_ps.cso");
		speed_map_texture_vs = std::make_unique<vertex_shader>(device, "./Shader/speed_map_vs.cso", input_element_desc, ARRAYSIZE(input_element_desc));
		speed_map_texture_ps = std::make_unique<pixel_shader>(device, "./Shader/speed_map_ps.cso");
     	sampler_states[LINEAR_BORDER] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
     	sampler_states[POINT] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
     	sampler_states[LINEAR] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
     	sampler_states[ANISOTROPIC] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
     
     }

	~PixelMotionBulur()
	{
		/*	if (glow_extraction_ps)glow_extraction_ps->Release();
			if (gaussian_blur_horizontal_ps)gaussian_blur_horizontal_ps->Release();
			if (gaussian_blur_vertical_ps)gaussian_blur_vertical_ps->Release();
			if (gaussian_blur_convolution_ps)gaussian_blur_convolution_ps->Release();
			if (gaussian_blur_downsampling_ps)gaussian_blur_downsampling_ps->Release();*/

	}
	PixelMotionBulur(PixelMotionBulur&) = delete;
	PixelMotionBulur& operator =(PixelMotionBulur&) = delete;

	enum { LINEAR_BORDER, POINT, LINEAR, ANISOTROPIC };
	std::unique_ptr<bloom_sampler_state> sampler_states[4];
	std::unique_ptr<framebuffer>speed_map_texture;
	std::unique_ptr<framebuffer>motion_bulur;
	std::unique_ptr<vertex_shader>motion_bulur_vs;
	std::unique_ptr<pixel_shader>motion_bulur_ps;
	std::unique_ptr<vertex_shader>speed_map_texture_vs;
	std::unique_ptr<pixel_shader>speed_map_texture_ps;
	std::unique_ptr<Constant_buffer<shader_constants>> constant_buffer;



	void blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* hdr_texture)
	{
		float ClearColor[4] = { 0,0,1,1 }; // クリア色作成　RGBAの順
		//constant_buffer->activate(immediate_context, 0, false, true);
		immediate_context->PSSetShaderResources(1, 0, &hdr_texture);
		
	


		speed_map_texture->activate(immediate_context);
		speed_map_texture->clear(immediate_context, 0.0f, 0.0f, 1.0f, 1.0f);

		//速度マップ用のシェーダーをセット
		speed_map_texture_vs->activate(immediate_context);
		speed_map_texture_ps->activate(immediate_context);

		fullscreen_quad::blit(immediate_context,true,true,false,false);

		speed_map_texture_vs->deactivate(immediate_context);
		speed_map_texture_ps->deactivate(immediate_context);

		speed_map_texture->deactivate(immediate_context);

		
		immediate_context->PSSetShaderResources(1, 1, speed_map_texture->render_target_shader_resource_view.GetAddressOf());
		
		motion_bulur->activate(immediate_context);
		motion_bulur->clear(immediate_context, 0.0f, 0.0f, 1.0f, 1.0f);
		
		motion_bulur_vs->activate(immediate_context);
		motion_bulur_ps->activate(immediate_context);
		
		fullscreen_quad::blit(immediate_context, true, true, false, false);
		
		motion_bulur_vs->deactivate(immediate_context);
		motion_bulur_ps->deactivate(immediate_context);
		
		motion_bulur->deactivate(immediate_context);
		
		ID3D11ShaderResourceView* null_shader_resource_views[2] = {};
		immediate_context->PSSetShaderResources(0, 2, null_shader_resource_views);
	}



	//std::unique_ptr<framebuffer> lens_flare;

private:


};