#include "Font.h"

#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"dwrite.lib")

#include <d3d11.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include "zumi_font.h"
using namespace Microsoft::WRL;

N_FONT::N_FONT(
    ID3D11Device* d3d11_device,
    ID2D1DeviceContext* d2d1_context,
    IDWriteFactory* dwrite_factory,
    const wchar_t* font_family_name,
    float font_size,
    UINT text_length_max)
    :d2d1_context(d2d1_context),
    text_length_max(text_length_max),
    font_size(font_size)
{
    HRESULT hr = S_OK;

    draw_rect.left = 0.0f;
    draw_rect.top = 0.0f;
    draw_rect.right = font_size * text_length_max;
    draw_rect.bottom = font_size / 2.0f * 3.0f;

    ComPtr<ID3D11Texture2D> buffer_texture = nullptr;
    D3D11_TEXTURE2D_DESC texture_2d_desc = {};
    texture_2d_desc.Width = static_cast<UINT>(draw_rect.right - draw_rect.left);
    texture_2d_desc.Height = static_cast<UINT>(draw_rect.bottom - draw_rect.top);
    texture_2d_desc.MipLevels = 1;
    texture_2d_desc.ArraySize = 1;
    texture_2d_desc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_2d_desc.SampleDesc.Count = 1;
    texture_2d_desc.SampleDesc.Quality = 0;
    texture_2d_desc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
    texture_2d_desc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
    texture_2d_desc.CPUAccessFlags = 0;
    texture_2d_desc.MiscFlags = 0;
    hr = d3d11_device->CreateTexture2D(&texture_2d_desc, nullptr, buffer_texture.GetAddressOf());
    assert(hr == S_OK);
    hr = d3d11_device->CreateRenderTargetView(buffer_texture.Get(), nullptr, buffer_rtv.GetAddressOf());
    assert(hr == S_OK);
    hr = d3d11_device->CreateShaderResourceView(buffer_texture.Get(), nullptr, buffer_srv.GetAddressOf());
    assert(hr == S_OK);

    ComPtr<IDXGISurface> dxgi_surface = nullptr;
    hr = buffer_texture->QueryInterface(dxgi_surface.GetAddressOf());
    assert(hr == S_OK);

    D2D1_BITMAP_PROPERTIES1 bitmapProps = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)
    );
    hr = d2d1_context->CreateBitmapFromDxgiSurface(
        dxgi_surface.Get(),
        bitmapProps,
        buffer_bitmap.GetAddressOf()
    );
    assert(hr == S_OK);

    hr = d2d1_context->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::White),
        brush.GetAddressOf()
    );
    assert(hr == S_OK);

    hr = dwrite_factory->CreateTextFormat(
        font_family_name,
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        font_size,
        L"ja-JP",
        &text_format
    );
    assert(hr == S_OK);
}

void N_FONT::RenderToSRV(
    ID3D11DeviceContext* d3d11_context,
    const wchar_t* text_widget)
{
    UINT text_length = 0;
    while (*(text_widget + text_length)) text_length++;
    text_length = (std::min)(text_length, text_length_max);

    static const float CLEAR_COLOR[4] = { 0.0f,0.0f,0.0f,0.0f };
    d3d11_context->ClearRenderTargetView(buffer_rtv.Get(), CLEAR_COLOR);
    d2d1_context->BeginDraw();
    d2d1_context->SetTarget(buffer_bitmap.Get());
    d2d1_context->DrawTextW(
        text_widget,
        text_length,
        text_format.Get(),
        draw_rect,
        brush.Get()
    );
    d2d1_context->EndDraw();
}

VECTOR2 N_FONT::GetBufferSize()const
{
    return VECTOR2(
        draw_rect.right - draw_rect.left,
        draw_rect.bottom - draw_rect.top
    );
}

