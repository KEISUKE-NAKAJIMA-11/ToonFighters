#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <memory>


//フレームバッファクラス
//レンダーターゲットビューとデプスステンシルビューが格納されており
//activateメソッドとdeactivateメソッドで挟んでその中で描画することで
//描画パスを管理する

class framebuffer
{
public:
	framebuffer(ID3D11Device* device, std::uint32_t width, std::uint32_t height, bool enable_msaa = false, int subsamples = 1, DXGI_FORMAT render_target_texture2d_format = DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT depth_stencil_texture2d_format = DXGI_FORMAT_R24G8_TYPELESS,
		bool need_render_target_shader_resource_view = true, bool need_depth_stencil_shader_resource_view = true, bool generate_mips = false);
	virtual ~framebuffer() = default;
	framebuffer(framebuffer &) = delete;
	framebuffer &operator=(framebuffer &) = delete;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> render_target_shader_resource_view;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> depth_stencil_shader_resource_view;

	D3D11_VIEWPORT viewport;

	void clear(ID3D11DeviceContext *immediate_context, float r = 0, float g = 0, float b = 0, float a = 0, unsigned int clear_flags = D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, float depth = 1, unsigned char stencil = 0)
	{
		float colour[4] = { r, g, b, a };
		if (render_target_view)
		{
			immediate_context->ClearRenderTargetView(render_target_view.Get(), colour);
		}
		if (depth_stencil_view)
		{
			immediate_context->ClearDepthStencilView(depth_stencil_view.Get(), clear_flags, depth, stencil);
		}
	}
	void activate(ID3D11DeviceContext *immediate_context)
	{
		number_of_stored_viewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
		immediate_context->RSGetViewports(&number_of_stored_viewports, default_viewports);
		immediate_context->RSSetViewports(1, &viewport);

		immediate_context->OMGetRenderTargets(1, default_render_target_view.ReleaseAndGetAddressOf(), default_depth_stencil_view.ReleaseAndGetAddressOf());
		immediate_context->OMSetRenderTargets(1, render_target_view.GetAddressOf(), depth_stencil_view.Get());
	}
	void deactivate(ID3D11DeviceContext *immediate_context)
	{
		immediate_context->RSSetViewports(number_of_stored_viewports, default_viewports);
		immediate_context->OMSetRenderTargets(1, default_render_target_view.GetAddressOf(), default_depth_stencil_view.Get());
	}
private:
	unsigned int number_of_stored_viewports;
	D3D11_VIEWPORT default_viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> default_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> default_depth_stencil_view;
};
