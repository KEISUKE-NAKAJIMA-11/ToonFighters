#ifndef FONT_H
#define FONT_H

#include "Vector.h"
#include <d3d11.h>
#include <dcommon.h>
#include <wrl.h>

struct ID2D1DeviceContext;
struct IDWriteFactory;
struct IDWriteTextFormat;
struct ID2D1SolidColorBrush;
struct ID2D1Bitmap1;


//友達からもらったフォントクラス

class N_FONT
{
private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> buffer_rtv = nullptr;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> buffer_srv = nullptr;

	Microsoft::WRL::ComPtr<ID2D1DeviceContext> d2d1_context = nullptr;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> text_format = nullptr;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush = nullptr;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> buffer_bitmap = nullptr;
	D2D1_RECT_F draw_rect = {};
	float font_size = 0.0f;
	UINT text_length_max = 0;

public:
	N_FONT(
		ID3D11Device* d3d11_device,
		ID2D1DeviceContext* d2d1_context,
		IDWriteFactory* dwrite_factory,
		const wchar_t* font_family_name,
		float font_size = 256.0f,
		UINT text_length_max = 32
	);

	void RenderToSRV(
		ID3D11DeviceContext* d3d11_context,
		const wchar_t* text_widget
	);
	ID3D11ShaderResourceView* GetBufferSRV()const { return buffer_srv.Get(); }
	float GetFontSize()const { return font_size; }
	VECTOR2 GetBufferSize()const;
};

#endif // !FONT_H

