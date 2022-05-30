#include "bloom.h"
#include "framework.h"
#include "blender.h"

#include <vector>



Bloom::Bloom(ID3D11Device* device, uint32_t width, uint32_t height) : fullscreen_quad(device)
{
	
	constant_buffer = std::make_unique<Constant_buffer<shader_constants>>(device);
	


	glow_extraction = std::make_unique<framebuffer>(device, width, height, false/*enable_msaa*/, 1/*subsamples*/,
		DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN,
		true/*need_render_target_shader_resource_view*/, false/*need_depth_stencil_shader_resource_view*/,
		false/*generate_mips*/);
	for (size_t index_of_downsampled = 0; index_of_downsampled < number_of_downsampled; ++index_of_downsampled)
	{
		gaussian_blur[index_of_downsampled][0] = std::make_unique<framebuffer>(device, width >> index_of_downsampled, height >> index_of_downsampled, false/*enable_msaa*/, 1/*subsamples*/,
			DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN,
			true/*need_render_target_shader_resource_view*/, false/*need_depth_stencil_shader_resource_view*/,
			false/*generate_mips*/);
		gaussian_blur[index_of_downsampled][1] = std::make_unique<framebuffer>(device, width >> index_of_downsampled, height >> index_of_downsampled, false/*enable_msaa*/, 1/*subsamples*/,
			DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN,
			true/*need_render_target_shader_resource_view*/, false/*need_depth_stencil_shader_resource_view*/,
			false/*generate_mips*/);
	}

	//CreatePixelShader(device, "./Shader/glow_extraction_ps.cso", glow_extraction_ps);
	//CreatePixelShader(device, "./Shader/gaussian_blur_convolution_ps.cso",gaussian_blur_convolution_ps);
	//CreatePixelShader(device, "./Shader/gaussian_blur_downsampling_ps.cso", gaussian_blur_downsampling_ps);
	//CreatePixelShader(device, "./Shader/gaussian_blur_horizontal_ps.cso", gaussian_blur_horizontal_ps);
	//CreatePixelShader(device, "./Shader/gaussian_blur_vertical_ps.cso", gaussian_blur_vertical_ps);

	glow_extraction_ps = std::make_unique<pixel_shader>(device, "./Shader/glow_extraction_ps.cso");
	gaussian_blur_horizontal_ps = std::make_unique<pixel_shader>(device, "./Shader/gaussian_blur_horizontal_ps.cso");
	gaussian_blur_vertical_ps = std::make_unique<pixel_shader>(device, "./Shader/gaussian_blur_vertical_ps.cso");
	gaussian_blur_convolution_ps = std::make_unique<pixel_shader>(device, "./Shader/gaussian_blur_convolution_ps.cso");
	gaussian_blur_downsampling_ps = std::make_unique<pixel_shader>(device, "./Shader/gaussian_blur_downsampling_ps.cso");



	sampler_states[LINEAR_BORDER] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
	sampler_states[POINT] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
	sampler_states[LINEAR] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
	sampler_states[ANISOTROPIC] = std::make_unique<bloom_sampler_state>(device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);


}

void Bloom::generate(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* hdr_texture, bool enable_lens_flare)
{


	constant_buffer->activate(immediate_context, 0, false, true);

	ID3D11ShaderResourceView* null_shader_resource_view = 0;
	//Extracting bright color
	glow_extraction->clear(immediate_context);
	glow_extraction->activate(immediate_context);
	//ps_activate

	/*immediate_context->PSSetShader(glow_extraction_ps, 0, 0);*/
	glow_extraction_ps->activate(immediate_context);
	//
	immediate_context->PSSetShaderResources(0, 1, &hdr_texture);
	fullscreen_quad::blit(immediate_context);
	immediate_context->PSSetShaderResources(0, 1, &null_shader_resource_view);
	//ps_deactivate
	/*immediate_context->PSSetShader(glow_extraction_ps, 0, 0);*/
	glow_extraction_ps->deactivate(immediate_context);
	//
	glow_extraction->deactivate(immediate_context);


	sampler_states[LINEAR_BORDER]->activate(immediate_context, 0, false);
	{
		// downsampling
		immediate_context->PSSetShaderResources(0, 1, glow_extraction->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[0][0]->clear(immediate_context);
		gaussian_blur[0][0]->activate(immediate_context);
		gaussian_blur_downsampling_ps->activate(immediate_context);
		
		//immediate_context->PSSetShader(gaussian_blur_downsampling_ps, 0, 0);
		fullscreen_quad::blit(immediate_context);
		gaussian_blur_downsampling_ps->deactivate(immediate_context);
		//immediate_context->PSSetShader(glow_extraction_ps, 0, 0);
		gaussian_blur[0][0]->deactivate(immediate_context);

		// ping-pong gaussian blur
		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[0][0]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[0][1]->clear(immediate_context);
		gaussian_blur[0][1]->activate(immediate_context);
		gaussian_blur_horizontal_ps->activate(immediate_context);
	
		//immediate_context->PSSetShader(gaussian_blur_horizontal_ps, 0, 0);
		fullscreen_quad::blit(immediate_context);
	    gaussian_blur_horizontal_ps->deactivate(immediate_context);
		//immediate_context->PSSetShader(gaussian_blur_horizontal_ps, 0, 0);
		gaussian_blur[0][1]->deactivate(immediate_context);

		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[0][1]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[0][0]->clear(immediate_context);
		gaussian_blur[0][0]->activate(immediate_context);
		gaussian_blur_vertical_ps->activate(immediate_context);
	
		//immediate_context->PSSetShader(gaussian_blur_vertical_ps, 0, 0);
		fullscreen_quad::blit(immediate_context);
		gaussian_blur_vertical_ps->deactivate(immediate_context);
		//immediate_context->PSSetShader(gaussian_blur_vertical_ps, 0, 0);
		gaussian_blur[0][0]->deactivate(immediate_context);
	}
	for (size_t index_of_downsampled = 1; index_of_downsampled < number_of_downsampled; ++index_of_downsampled)
	{
		// downsampling
		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[index_of_downsampled - 1][0]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[index_of_downsampled][0]->clear(immediate_context);
		gaussian_blur[index_of_downsampled][0]->activate(immediate_context);
		gaussian_blur_downsampling_ps->activate(immediate_context);
		
		//immediate_context->PSSetShader(gaussian_blur_downsampling_ps, 0, 0);
		fullscreen_quad::blit(immediate_context);
		gaussian_blur_downsampling_ps->deactivate(immediate_context);
		//immediate_context->PSSetShader(glow_extraction_ps, 0, 0);
		gaussian_blur[index_of_downsampled][0]->deactivate(immediate_context);

		// ping-pong gaussian blur
		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[index_of_downsampled][0]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[index_of_downsampled][1]->clear(immediate_context);
		gaussian_blur[index_of_downsampled][1]->activate(immediate_context);
		gaussian_blur_horizontal_ps->activate(immediate_context);
	
		//immediate_context->PSSetShader(gaussian_blur_horizontal_ps, 0, 0);
		fullscreen_quad::blit(immediate_context);
		gaussian_blur_horizontal_ps->deactivate(immediate_context);
		//immediate_context->PSSetShader(gaussian_blur_horizontal_ps, 0, 0);
		gaussian_blur[index_of_downsampled][1]->deactivate(immediate_context);

		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[index_of_downsampled][1]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[index_of_downsampled][0]->clear(immediate_context);
		gaussian_blur[index_of_downsampled][0]->activate(immediate_context);
		gaussian_blur_vertical_ps->activate(immediate_context);
		
		//immediate_context->PSSetShader(gaussian_blur_vertical_ps, 0, 0);
		fullscreen_quad::blit(immediate_context);
		gaussian_blur_vertical_ps->deactivate(immediate_context);
		//immediate_context->PSSetShader(gaussian_blur_vertical_ps, 0, 0);
		gaussian_blur[index_of_downsampled][0]->deactivate(immediate_context);
	}
	sampler_states[LINEAR_BORDER]->deactivate(immediate_context);


}

void Bloom::blit(ID3D11DeviceContext* immediate_context, blender* Brender)
{
	sampler_states[POINT]->activate(immediate_context, 0, false);
	sampler_states[LINEAR]->activate(immediate_context, 1, false);
	sampler_states[ANISOTROPIC]->activate(immediate_context, 2, false);
	//blend_state->activate(immediate_context);
	setBlendMode(immediate_context, blender::BLEND_STATE::BS_ADD,Brender);

	constant_buffer->activate(immediate_context, 0, false, true);

	std::vector< ID3D11ShaderResourceView*> shader_resource_views;
	for (size_t index_of_downsampled = 0; index_of_downsampled < number_of_downsampled; ++index_of_downsampled)
	{
		shader_resource_views.push_back(gaussian_blur[index_of_downsampled][0]->render_target_shader_resource_view.Get());
	}
	immediate_context->PSSetShaderResources(0, number_of_downsampled, &shader_resource_views.at(0));
	gaussian_blur_convolution_ps->activate(immediate_context);

	//immediate_context->PSSetShader(gaussian_blur_convolution_ps, 0, 0);
	fullscreen_quad::blit(immediate_context);
	gaussian_blur_convolution_ps->deactivate(immediate_context);
	//immediate_context->PSSetShader(gaussian_blur_convolution_ps, 0, 0);
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { 0 };
	immediate_context->PSSetShaderResources(0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, null_shader_resource_views);

	constant_buffer->deactivate(immediate_context);
	//blend_state->deactivate(immediate_context);
	setBlendMode(immediate_context, blender::BLEND_STATE::BS_ALPHA, Brender);
	sampler_states[POINT]->deactivate(immediate_context);
	sampler_states[LINEAR]->deactivate(immediate_context);
	sampler_states[ANISOTROPIC]->deactivate(immediate_context);
}

void Bloom::CreatePixelShader(ID3D11Device* device, const char* filename,  ID3D11PixelShader* pixel_shader)
{
	HRESULT hr = S_OK;

	std::vector<char> psdata;
	{
		FILE* fp = nullptr;
		fopen_s(&fp, filename, "rb");
		assert(fp && "file open failed");
		fseek(fp, 0, SEEK_END);
		psdata.resize(ftell(fp));
		fseek(fp, 0, SEEK_SET);
		fread(psdata.data(), sizeof(psdata[0]), psdata.size(), fp);
		fclose(fp);
	}

	hr = device->CreatePixelShader(psdata.data(), psdata.size(), nullptr, &pixel_shader);
	assert(hr == S_OK && "create pixel shader failed");
}
