#include "FontManager.h"

#include <d2d1_1.h>
#include <dwrite.h>
#include <assert.h>
#include <d3d11.h>
#include "zumi_font.h"

FontManager::FontManager() = default;
FontManager::~FontManager() = default;

void FontManager::Initialize(ID3D11Device * d3d11_device)
{
	HRESULT hr = S_OK;
	this->d3d11_device = d3d11_device;

	ComPtr<IDXGIDevice1> dxgi_device = nullptr;
	hr = d3d11_device->QueryInterface(dxgi_device.GetAddressOf());
	assert(hr == S_OK);

	ComPtr<ID2D1Factory1> d2d_factory = nullptr;
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		d2d_factory.GetAddressOf()
	);
	assert(hr == S_OK);

	ComPtr<ID2D1Device> d2d1_device = nullptr;
	hr = d2d_factory->CreateDevice(dxgi_device.Get(), d2d1_device.GetAddressOf());
	hr = d2d1_device->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		d2d1_context.GetAddressOf()
	);
	assert(hr == S_OK);

	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(dwrite_factory.GetAddressOf())
	);
	assert(hr == S_OK);
}

std::shared_ptr<N_FONT> FontManager::AddFont(
	const wchar_t* font_family_name,
	float font_size,
	unsigned int text_length_max)
{
	auto find = fonts.find(font_family_name);
	assert(find == fonts.end());
	std::shared_ptr<N_FONT> font = std::make_shared<N_FONT>(
		d3d11_device.Get(),
		d2d1_context.Get(),
		dwrite_factory.Get(),
		font_family_name,
		font_size,
		text_length_max
		);
	fonts.emplace(font_family_name, font);
	return font;
}

std::shared_ptr<N_FONT> FontManager::GetFont(const wchar_t* font_family_name)
{
	std::shared_ptr<N_FONT> font = nullptr;
	auto find = fonts.find(font_family_name);
	if (find != fonts.end()) {
		font = find->second;
	}
	return font;
}
