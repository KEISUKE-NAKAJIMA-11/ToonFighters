#ifndef FONTM_MANAGER_H
#define FONTM_MANAGER_H

#include <unordered_map>
#include <string>
#include <memory>
#include <wrl.h>

using Microsoft::WRL::ComPtr;
struct ID3D11Device;
struct ID2D1DeviceContext;
struct IDWriteFactory;
class N_FONT;

class FontManager
{
private:
	ComPtr<ID3D11Device> d3d11_device = nullptr;
	ComPtr<ID2D1DeviceContext> d2d1_context = nullptr;
	ComPtr<IDWriteFactory> dwrite_factory = nullptr;
	std::unordered_map<std::wstring, std::shared_ptr<N_FONT>> fonts;

public:
	FontManager();
	~FontManager();

	void Initialize(ID3D11Device* d3d11_device);
	std::shared_ptr<N_FONT> AddFont(
		const wchar_t* font_family_name,
		float font_size = 256.0f,
		unsigned int text_length_max = 32
	);
	std::shared_ptr<N_FONT> GetFont(const wchar_t* font_family_name);
};

#endif // !FONTM_MANAGER_H

