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

class ToneMap:fullscreen_quad
{
public:
	ToneMap(ID3D11Device* device) : fullscreen_quad(device)
	{
		average_luminance = std::make_unique<framebuffer>(device, 512, 512, false, 1, DXGI_FORMAT_R16_FLOAT, DXGI_FORMAT_UNKNOWN, true, false, true/*generate_mips*/);
		average_luminance_ps = std::make_unique<pixel_shader>(device, "./Shader/average_luminance_ps.cso");
		tonemap_ps = std::make_unique<pixel_shader>(device, "./Shader/tonemap_ps.cso");
		constant_buffer = std::make_unique<Constant_buffer<shader_constants>>(device);
		sampler_states[LINEAR_BORDER] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[POINT] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[LINEAR] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
		sampler_states[ANISOTROPIC] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);

	}
	virtual ~ToneMap() = default;

	enum { LINEAR_BORDER, POINT, LINEAR, ANISOTROPIC };
	std::unique_ptr<bloom_sampler_state> sampler_states[4];
	std::unique_ptr<framebuffer> average_luminance;
	struct shader_constants
	{
		float white_point = 0.380f;
		float average_gray = 0.100f;

		float brightness = 0.0f;
		float contrast = 0.0f;

		float hue = 0.0f;
		float saturation = 0.0f;

		float offset = 0.0f;
		float darkness = 1.0f;

		// Screen Filter
		DirectX::XMFLOAT3 colour_filter = { 1, 1, 1 };

	
		float amount = 0.0f;

		template<class T>
		void serialize(T& archive)
		{
		
		}


	};

	std::unique_ptr<Constant_buffer<shader_constants>> constant_buffer;

	std::unique_ptr<pixel_shader> average_luminance_ps;
	std::unique_ptr<pixel_shader> tonemap_ps;

	float cumulative_time = 0;
public:

	void blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* hdr_texture, float elapsed_time)
	{
		sampler_states[LINEAR_BORDER]->activate(immediate_context, 2, false);
		{
			immediate_context->PSSetShaderResources(0, 1, &hdr_texture);

			//create average luminance
			const float interval_time = 1.0f / 30.0f;
			cumulative_time += elapsed_time;
			if (cumulative_time > interval_time)
			{

				average_luminance->clear(immediate_context);
				average_luminance->activate(immediate_context);
				average_luminance_ps->activate(immediate_context);
				fullscreen_quad::blit(immediate_context);
				average_luminance_ps->deactivate(immediate_context);
				average_luminance->deactivate(immediate_context);
				immediate_context->GenerateMips(average_luminance->render_target_shader_resource_view.Get());
				cumulative_time -= interval_time;
			}


			constant_buffer->activate(immediate_context, 0, false, true);

			immediate_context->PSSetShaderResources(1, 1, average_luminance->render_target_shader_resource_view.GetAddressOf());

			tonemap_ps->activate(immediate_context);
			fullscreen_quad::blit(immediate_context);
			tonemap_ps->deactivate(immediate_context);

			ID3D11ShaderResourceView* null_shader_resource_views[2] = {};
			immediate_context->PSSetShaderResources(0, 2, null_shader_resource_views);
		}
		sampler_states[LINEAR_BORDER]->deactivate(immediate_context);

	}


};

