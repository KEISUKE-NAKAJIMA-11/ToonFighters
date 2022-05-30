#pragma once
#include <memory>

#include <iostream>
#include <fstream>
#include <shlwapi.h>
#undef max
#undef min
#include "rasterizer.h"
#include "framebuffer.h"
#include "blender.h"
#include "Shadermanager.h"
#include <wrl.h>
#include <cstdint>
#include <d3d11.h>


//独学でやってみたブルーム関連


template <class T>
struct Constant_buffer
{
	T data{};
	Constant_buffer(ID3D11Device* device)
	{
		_ASSERT_EXPR(sizeof(T) % 16 == 0, L"constant buffer's need to be 16 byte aligned");
		HRESULT hr = S_OK;
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(T);
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA subresource_data;
		subresource_data.pSysMem = &data;
		subresource_data.SysMemPitch = 0;
		subresource_data.SysMemSlicePitch = 0;
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, buffer_object.GetAddressOf());
		

	}
	virtual ~Constant_buffer() = default;
	Constant_buffer(Constant_buffer&) = delete;
	Constant_buffer& operator =(Constant_buffer&) = delete;

	void activate(ID3D11DeviceContext* immediate_context, UINT slot, bool vs_on = true, bool ps_on = true)
	{
		
		immediate_context->UpdateSubresource(buffer_object.Get(), 0, 0, &data, 0, 0);

		if (vs_on)
		{
			immediate_context->VSSetConstantBuffers(slot, 1u, buffer_object.GetAddressOf());
		}
		if (ps_on)
		{
			immediate_context->PSSetConstantBuffers(slot, 1u, buffer_object.GetAddressOf());
		}
	}
	void deactivate(ID3D11DeviceContext* immediate_context)
	{
		// NOP
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_object;

};


class bloom_sampler_state
{
#ifdef _DEBUG
#define ACTIVATE(x) ++(x)
#define DEACTIVATE(x) --(x)
	int watchdog = 0;
#else
#define ACTIVATE(x) ((void)0)
#define DEACTIVATE(x) ((void)0)
#endif
public:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> state_object;
	bloom_sampler_state(ID3D11Device* device, D3D11_FILTER filter = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE address_mode = D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_FUNC comparison_func = D3D11_COMPARISON_NEVER, float r = 1, float g = 1, float b = 1, float a = 1)
	{
		//4096 unique sampler state objects can be created on a device at a time.
		//If an application attempts to create a sampler - state interface with the same state as an existing interface,
		//the same interface will be returned and the total number of unique sampler state objects will stay the same.
		D3D11_SAMPLER_DESC sampler_desc = {};
		sampler_desc.Filter = filter;
		sampler_desc.AddressU = address_mode;
		sampler_desc.AddressV = address_mode;
		sampler_desc.AddressW = address_mode;
		sampler_desc.MipLODBias = 0;
		sampler_desc.MaxAnisotropy = 16;
		sampler_desc.ComparisonFunc = comparison_func;
		float border_colour[4] = { r, g, b, a };
		memcpy(sampler_desc.BorderColor, border_colour, sizeof(float) * 4);
		sampler_desc.MinLOD = 0;
		sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
		device->CreateSamplerState(&sampler_desc, state_object.GetAddressOf());
		
	}
	bloom_sampler_state(ID3D11Device* device, const D3D11_SAMPLER_DESC* sampler_desc)
	{
		device->CreateSamplerState(sampler_desc, state_object.GetAddressOf());
	
	}
	~bloom_sampler_state() = default;
	bloom_sampler_state(bloom_sampler_state&) = delete;
	bloom_sampler_state& operator =(bloom_sampler_state&) = delete;

	void activate(ID3D11DeviceContext* immediate_context, UINT slot, bool set_in_vs = false)
	{
		ACTIVATE(watchdog);
		using_slot = slot;
		immediate_context->PSGetSamplers(using_slot, 1, cached_state_objects[0].ReleaseAndGetAddressOf());
		immediate_context->VSSetSamplers(using_slot, 1, state_object.GetAddressOf());

		immediate_context->PSSetSamplers(using_slot, 1, state_object.GetAddressOf());
		if (set_in_vs)
		{
			immediate_context->VSGetSamplers(using_slot, 1, cached_state_objects[1].ReleaseAndGetAddressOf());
		}
	}
	void deactivate(ID3D11DeviceContext* immediate_context)
	{
		DEACTIVATE(watchdog);
		immediate_context->PSSetSamplers(using_slot, 1, cached_state_objects[0].GetAddressOf());
		immediate_context->VSSetSamplers(using_slot, 1, cached_state_objects[1].GetAddressOf());
	}
private:
	UINT using_slot = 0;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> cached_state_objects[2];
};


class Bloom : fullscreen_quad
{
public:
	struct shader_constants
	{
		float glow_extraction_threshold = 0.580f;
		float blur_convolution_intensity = 0.5f;
		float lens_flare_threshold = 1.000f;
		float lens_flare_ghost_dispersal = 0.300f; //dispersion factor
		int number_of_ghosts = 6;
		float lens_flare_intensity = 0.35f;
		int options[2];


		template<class T>
		void serialize(T& archive)
		{
			//TODO:Add 'number_of_ghosts', "lens_flare_intensity"
			//archive(CEREAL_NVP(glow_extraction_threshold), CEREAL_NVP(blur_convolution_intensity), CEREAL_NVP(lens_flare_threshold), CEREAL_NVP(lens_flare_ghost_dispersal));
		}
	};
	//std::unique_ptr<<shader_constants>> constant_buffer;

	Bloom(ID3D11Device* device, uint32_t width, uint32_t height);
	~Bloom() 
	{
	/*	if (glow_extraction_ps)glow_extraction_ps->Release();
		if (gaussian_blur_horizontal_ps)gaussian_blur_horizontal_ps->Release();
		if (gaussian_blur_vertical_ps)gaussian_blur_vertical_ps->Release();
		if (gaussian_blur_convolution_ps)gaussian_blur_convolution_ps->Release();
		if (gaussian_blur_downsampling_ps)gaussian_blur_downsampling_ps->Release();*/
	
	}
	Bloom(Bloom&) = delete;
	Bloom& operator =(Bloom&) = delete;

	void generate(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* hdr_texture, bool enable_lens_flare);
	void blit(ID3D11DeviceContext* immediate_context,blender* Brender);
	void CreatePixelShader(ID3D11Device* device, const char* filename, ID3D11PixelShader* pixel_shader);


	enum { LINEAR_BORDER, POINT, LINEAR, ANISOTROPIC };
	std::unique_ptr<bloom_sampler_state> sampler_states[4];

	std::unique_ptr<framebuffer> glow_extraction;



	// if you change value of 'number_of_downsampled', you must change 'number_of_downsampled' in bloom.hlsli to this same value.
	const static size_t number_of_downsampled = 6;
	std::unique_ptr<framebuffer> gaussian_blur[number_of_downsampled][2];
	//ID3D11PixelShader* glow_extraction_ps = nullptr;
	//ID3D11PixelShader* gaussian_blur_horizontal_ps = nullptr;
	//ID3D11PixelShader* gaussian_blur_vertical_ps = nullptr;
	//ID3D11PixelShader* gaussian_blur_convolution_ps = nullptr;
	//ID3D11PixelShader* gaussian_blur_downsampling_ps = nullptr;


	std::unique_ptr<pixel_shader> glow_extraction_ps;
	std::unique_ptr<pixel_shader> gaussian_blur_horizontal_ps;
	std::unique_ptr<pixel_shader> gaussian_blur_vertical_ps;
	std::unique_ptr<pixel_shader> gaussian_blur_convolution_ps;
	std::unique_ptr<pixel_shader> gaussian_blur_downsampling_ps;
	std::unique_ptr<Constant_buffer<shader_constants>> constant_buffer;

	//std::unique_ptr<framebuffer> lens_flare;

private:


	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> gradient_map;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noise_map;


};


