// UNIT.04
#include "texture.h"
#include "misc.h"
#include "framework.h"
#include <DirectXTex.h>
#include <WICTextureLoader.h>
#include <wrl.h>
#include <map>
#include <sstream>

static std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> cached_textures;

HRESULT load_texture_from_file(ID3D11Device* device, const wchar_t* file_name, ID3D11ShaderResourceView** shader_resource_view, D3D11_TEXTURE2D_DESC* texture2d_desc)
{
	HRESULT hr = S_OK;
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	static std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> cache;
	auto it = cache.find(file_name);
	if (it != cache.end())
	{
		//it->second.Attach(*shader_resource_view);
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
		(*shader_resource_view)->GetResource(resource.GetAddressOf());
	}

	else
	{
		hr = DirectX::CreateWICTextureFromFile(device, file_name, resource.GetAddressOf(), shader_resource_view);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		cache.insert(std::make_pair(file_name, *shader_resource_view));
	}


	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	hr = resource.Get()->QueryInterface<ID3D11Texture2D>(texture2d.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	texture2d->GetDesc(texture2d_desc);

	return hr;
}

void combine_resource_path(wchar_t(&combined_resource_path)[256], const wchar_t* referrer_filename, const wchar_t* referent_filename)
{
	const wchar_t delimiters[] = { L'\\', L'/' };
	// extract directory from obj_filename
	wchar_t directory[256] = {};
	for (wchar_t delimiter : delimiters)
	{
		wchar_t* p = wcsrchr(const_cast<wchar_t*>(referrer_filename), delimiter);
		if (p)
		{
			memcpy_s(directory, 256, referrer_filename, (p - referrer_filename + 1) * sizeof(wchar_t));
			break;
		}
	}
	// extract filename from resource_filename
	wchar_t filename[256];
	wcscpy_s(filename, referent_filename);
	for (wchar_t delimiter : delimiters)
	{
		wchar_t* p = wcsrchr(filename, delimiter);
		if (p)
		{
			wcscpy_s(filename, p + 1);
			break;
		}
	}
	// combine directory and filename
	wcscpy_s(combined_resource_path, directory);
	wcscat_s(combined_resource_path, filename);
};


// UNIT.16
HRESULT make_dummy_texture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view)
{
	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC texture2d_desc = {};
	texture2d_desc.Width = 1;
	texture2d_desc.Height = 1;
	texture2d_desc.MipLevels = 1;
	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subresource_data = {};
	u_int color = 0xFFFFFFFF;
	subresource_data.pSysMem = &color;
	subresource_data.SysMemPitch = 4;
	subresource_data.SysMemSlicePitch = 4;

	ID3D11Texture2D* texture2d;
	hr = device->CreateTexture2D(&texture2d_desc, &subresource_data, &texture2d);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = {};
	shader_resource_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MipLevels = 1;

	hr = device->CreateShaderResourceView(texture2d, &shader_resource_view_desc, shader_resource_view);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	texture2d->Release();

	return hr;
}

HRESULT load_texture_from_file_model(ID3D11Device* device, const wchar_t* file_name, ID3D11ShaderResourceView** shader_resource_view)
{
	//std::mutex load_texture_mutex;
	//std::lock_guard<std::mutex> lock(load_texture_mutex);



	HRESULT hr = S_OK;

	std::map<std::wstring, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>::iterator it = cached_textures.find(file_name);
	if (it != cached_textures.end())
	{
		*shader_resource_view = it->second.Get();
		(*shader_resource_view)->AddRef();
	}
	else
	{
		DirectX::TexMetadata metadata;
		DirectX::ScratchImage image;

		std::wstring extension = PathFindExtensionW(file_name);
		std::transform(extension.begin(), extension.end(), extension.begin(), ::towlower);
		//WIC includes several built - in codecs.The following standard codecs are provided with the platform.
		//	Codec																	Mime Types								Decoders	Encoders
		//	BMP(Windows Bitmap Format), BMP Specification v5.						image / bmp								Yes			Yes
		//	GIF(Graphics Interchange Format 89a), GIF Specification 89a / 89m		image / gif								Yes			Yes
		//	ICO(Icon Format)														image / ico								Yes			No
		//	JPEG(Joint Photographic Experts Group), JFIF Specification 1.02			image / jpeg, image / jpe, image / jpg	Yes			Yes
		//	PNG(Portable Network Graphics), PNG Specification 1.2					image / png								Yes			Yes
		//	TIFF(Tagged Image File Format), TIFF Specification 6.0					image / tiff, image / tif				Yes			Yes
		//	Windows Media Photo, HD Photo Specification 1.0							image / vnd.ms - phot					Yes			Yes
		//	DDS(DirectDraw Surface)													image / vnd.ms - dds					Yes			Yes
		if (L".png" == extension || L".jpeg" == extension || L".jpg" == extension || L".jpe" == extension || L".gif" == extension || L".tiff" == extension || L".tif" == extension || L".bmp" == extension)
		{
			hr = DirectX::LoadFromWICFile(file_name, 0, &metadata, image);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else if (L".dds" == extension)
		{
			hr = DirectX::LoadFromDDSFile(file_name, 0, &metadata, image);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else if (L".tga" == extension || L".vda" == extension || L".icb" == extension || L".vst" == extension)
		{
			hr = DirectX::LoadFromTGAFile(file_name, &metadata, image);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
		else
		{
			_ASSERT_EXPR(false, L"File format not supported.");
		}

		if (hr == S_OK)
		{
			//HRESULT DirectX::CreateShaderResourceViewEx(
			//	ID3D11Device* pDevice,
			//	const Image* srcImages,
			//	size_t nimages,
			//	const TexMetadata& metadata,
			//	D3D11_USAGE usage,
			//	unsigned int bindFlags,
			//	unsigned int cpuAccessFlags,
			//	unsigned int miscFlags,
			//	bool forceSRGB,
			//	ID3D11ShaderResourceView** ppSRV);
			hr = DirectX::CreateShaderResourceViewEx(
				device,
				image.GetImages(),
				image.GetImageCount(),
				metadata,
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_SHADER_RESOURCE,
				0,
				D3D11_RESOURCE_MISC_TEXTURECUBE,
				true,
				shader_resource_view);
			_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
		}
	}
	return hr;
}


TextureManager::TextureManager() : maxNum(0), texture(nullptr), current(0)
	{
		init(256);
	}

	//--------------------------------------------------------------
	//  デストラクタ
	//--------------------------------------------------------------
	TextureManager::~TextureManager()
	{
		uninit();
	}

	//--------------------------------------------------------------
	//  初期化
	//--------------------------------------------------------------
	void TextureManager::init(size_t max)
	{
		uninit();	// 念のため

		texture = new Texture[max];
		maxNum = max;
	}

	//--------------------------------------------------------------
	//  終了処理
	//--------------------------------------------------------------
	void TextureManager::uninit()
	{
		if (texture)
		{
			releaseAll();
			safe_delete_array(texture);
			maxNum = 0;
		}
	}

	//--------------------------------------------------------------
	//  テクスチャロード（番号指定）
	//--------------------------------------------------------------
	void TextureManager::load(ID3D11Device* device, int i, const wchar_t* fileName, size_t maxInstance)
	{
		if ((size_t)i < maxNum) texture[i].load(device, fileName, maxInstance);
	}

	//--------------------------------------------------------------
	//  テクスチャのロード（空き番号検索）
	//--------------------------------------------------------------
	int TextureManager::load(ID3D11Device* device, const wchar_t* fileName, size_t maxInstance)
	{
		for (size_t i = 0; i < maxNum; i++)
		{
			if (texture[i].isEmpty())
			{
				load(device, i, fileName, maxInstance);
				return (int)i;
			}
		}
		return -1;
	}

	//--------------------------------------------------------------
	//  テクスチャの一括ロード
	//--------------------------------------------------------------
	void TextureManager::load(ID3D11Device* device, const LoadTexture* data)
	{
		while ((size_t)data->texNum < maxNum)
		{
			release(data->texNum);
			load(device, data->texNum, data->fileName, data->maxInstance);
			data++;
		}
	}

	//--------------------------------------------------------------
	//  テクスチャ解放（１つ）
	//--------------------------------------------------------------
	void TextureManager::release(int i)
	{
		if ((size_t)i < maxNum)
		{
			texture[i].release();
		}
	}

	//--------------------------------------------------------------
	//  テクスチャの一括解放
	//--------------------------------------------------------------
	void TextureManager::release(const LoadTexture* data)
	{
		while ((size_t)data->texNum < maxNum)
		{
			release(data->texNum);
			data++;
		}
	}

	//--------------------------------------------------------------
	//  テクスチャ解放（すべて）
	//---------------------- ----------------------------------------
	void TextureManager::releaseAll()
	{
		for (size_t i = 0; i < maxNum; i++) release((int)i);
	}

	//--------------------------------------------------------------
	//  描画の前処理
	//--------------------------------------------------------------
	void TextureManager::begin(ID3D11DeviceContext* context, int texNo)
	{
		texture[texNo].begin(context);
	}

	//--------------------------------------------------------------
	//  描画の後処理
	//--------------------------------------------------------------
	void TextureManager::end(ID3D11DeviceContext* context, int texNo)
	{
		texture[texNo].end(context);
	}

	//--------------------------------------------------------------
	//  テクスチャ描画
	//--------------------------------------------------------------
	void TextureManager::draw(int i,
		const VECTOR2& position, const VECTOR2& scale,
		const VECTOR2& texPos, const VECTOR2& texSize,
		const VECTOR2& center, float angle, const VECTOR4& color,
		bool world)
	{
		if ((size_t)i < maxNum)
		{
			texture[i].draw(
				position, scale,
				texPos, texSize,
				center, angle,
				color,
				world);
		}
	}

	//--------------------------------------------------------------
	//  テクスチャ描画
	//--------------------------------------------------------------
	void TextureManager::draw(int i,
		float x, float y,
		float scaleX, float scaleY,
		float left, float top,
		float width, float height,
		float centerX, float centerY, float angle,
		float r, float g, float b, float a,
		bool world)
	{
		if ((size_t)i < maxNum)
		{
			draw(i,
				VECTOR2(x, y), VECTOR2(scaleX, scaleY),
				VECTOR2(left, top), VECTOR2(width, height),
				VECTOR2(centerX, centerY), angle,
				VECTOR4(r, g, b, a),
				world);
		}
	}

	//==========================================================================
	//
	//      テクスチャクラス
	//
	//==========================================================================

	//--------------------------------------------------------------
	//  コンストラクタ
	//--------------------------------------------------------------
	Texture::Texture() : sprBat(nullptr)
	{
	}

	//--------------------------------------------------------------
	//  デストラクタ
	//--------------------------------------------------------------
	Texture::~Texture()
	{
		release();
	}

	//--------------------------------------------------------------
	//  テクスチャのロード
	//--------------------------------------------------------------
	bool Texture::load(ID3D11Device* device, const wchar_t* fileName, size_t maxInstance)
	{
		sprBat = new sprite_batch(device, fileName, maxInstance);
		return (sprBat != nullptr);
	}

	//--------------------------------------------------------------
	//  描画の前処理
	//--------------------------------------------------------------
	void Texture::begin(ID3D11DeviceContext* context)
	{
		sprBat->begin(context);
	}

	//--------------------------------------------------------------
	//  描画の後処理
	//--------------------------------------------------------------
	void Texture::end(ID3D11DeviceContext* context)
	{
		sprBat->end(context);
	}

	//--------------------------------------------------------------
	//  テクスチャの描画
	//--------------------------------------------------------------
	void Texture::draw(
		float x, float y, float scaleX, float scaleY,
		float left, float top, float width, float height,
		float centerX, float centerY, float angle,
		float r, float g, float b, float a,
		bool world)
	{
		draw(VECTOR2(x, y), VECTOR2(scaleX, scaleY),
			VECTOR2(left, top), VECTOR2(width, height),
			VECTOR2(centerX, centerY), angle,
			VECTOR4(r, g, b, a),
			world);
	}

	//--------------------------------------------------------------
	//  テクスチャの描画
	//--------------------------------------------------------------
	void Texture::draw(
		const VECTOR2& pos, const VECTOR2& scale,
		const VECTOR2& texPos, const VECTOR2& texSize,
		const VECTOR2& center, float angle,
		const VECTOR4& color,
		bool world)
	{
		if (!sprBat) return;

		
		
		sprBat->render(
				pos, scale, texPos, texSize,
				center, angle, color);
	}

	//--------------------------------------------------------------
	//  テクスチャの解放
	//--------------------------------------------------------------
	void Texture::release()
	{
		safe_delete(sprBat);
	}

	//--------------------------------------------------------------
	//  テクスチャの有無
	//--------------------------------------------------------------
	bool Texture::isEmpty()
	{
		return (sprBat == nullptr);
	}






	namespace texture
	{

		//--------------------------------------------------------------
		//  テクスチャマネージャ初期化
		//--------------------------------------------------------------
		void init(size_t max)
		{
			framework::Instance().textureManager->init(max);
		}

		//--------------------------------------------------------------
		//  テクスチャマネージャ終了処理
		//--------------------------------------------------------------
		void uninit()
		{
			framework::Instance().textureManager->uninit();
		}

		//--------------------------------------------------------------
		//  テクスチャのロード（番号を指定してロード）
		//--------------------------------------------------------------
		void load(int texNo, const wchar_t* fileName, size_t maxInstance)
		{
			framework::Instance().textureManager->load(framework::Instance().GetDevice(), texNo, fileName, maxInstance);
		}

		//--------------------------------------------------------------
		//  テクスチャのロード（空き番号を探してロード）
		//--------------------------------------------------------------
		int load(const wchar_t* fileName, size_t maxInstance)
		{
			return framework::Instance().textureManager->load(framework::Instance().GetDevice(), fileName, maxInstance);
		}

		//--------------------------------------------------------------
		//  テクスチャのロード（ロードテクスチャのデータを使ってロード）
		//--------------------------------------------------------------
		void load(const LoadTexture* data)
		{
			framework::Instance().textureManager->load(framework::Instance().GetDevice(), data);
		}

		//--------------------------------------------------------------
		//  テクスチャの解放（番号を指定して解放）
		//--------------------------------------------------------------
		void release(int texNo)
		{
			framework::Instance().textureManager->release(texNo);
		}

		//--------------------------------------------------------------
		//  テクスチャの解放（ロードテクスチャのデータを使って解放）
		//--------------------------------------------------------------
		void release(const LoadTexture* data)
		{
			framework::Instance().textureManager->release(data);
		}

		//--------------------------------------------------------------
		//  テクスチャの解放（全て）
		//--------------------------------------------------------------
		void releaseAll()
		{
			framework::Instance().textureManager->releaseAll();
		}

		//--------------------------------------------------------------
		//  描画の前処理
		//--------------------------------------------------------------
		void begin(int texNo)
		{
			framework::Instance().textureManager->begin(framework::Instance().GetDeviceContext(), texNo);
		}

		//--------------------------------------------------------------
		//  描画の後処理
		//--------------------------------------------------------------
		void end(int texNo)
		{
			framework::Instance().textureManager->end(framework::Instance().GetDeviceContext(), texNo);
		}

		//--------------------------------------------------------------
		//  テクスチャの描画
		//--------------------------------------------------------------
		void draw(int texNo,
			const VECTOR2& position, const VECTOR2& scale,
			const VECTOR2& texPos, const VECTOR2& texSize,
			const VECTOR2& center, float angle,
			const VECTOR4& color,
			bool world)
		{
			framework::Instance().textureManager->draw(texNo,
				position, scale,
				texPos, texSize,
				center, angle,
				color,
				world);
		}

		//--------------------------------------------------------------
		//  テクスチャの描画
		//--------------------------------------------------------------
		void draw(int texNo,
			float x, float y, float scaleX, float scaleY,
			float left, float top, float width, float height,
			float centerX, float centerY, float angle,
			float r, float g, float b, float a,
			bool world)
		{
			draw(texNo,
				VECTOR2(x, y), VECTOR2(scaleX, scaleY),
				VECTOR2(left, top), VECTOR2(width, height),
				VECTOR2(centerX, centerY), angle,
				VECTOR4(r, g, b, a),
				world);
		}

	}






	//TextureShader::TextureShader() :ShaderResourceView(nullptr)
	//{
	//}



	//TextureShader::~TextureShader()
	//{
	//}


	//bool TextureShader::Load(const wchar_t* filename)
	//{
	//	ID3D11Device* device = pSystem.GetDevice();


	//	// 画像ファイル読み込み DirectXTex
	//	DirectX::TexMetadata metadata;
	//	DirectX::ScratchImage image;
	//	HRESULT hr = LoadFromWICFile(filename,
	//		0, &metadata, image);
	//	if (FAILED(hr)) {
	//		// 読み込めなかったらDDSファイルを読み込む
	//		hr = LoadFromDDSFile(filename, 0, &metadata, image);
	//	}
	//	assert(SUCCEEDED(hr));


	//	// 画像からシェーダリソースView
	//	hr = DirectX::CreateShaderResourceView(
	//		device,
	//		image.GetImages(),
	//		image.GetImageCount(),
	//		image.GetMetadata(),
	//		ShaderResourceView.GetAddressOf());

	//	assert(SUCCEEDED(hr));

	//	//テクスチャデータ取得
	//	texture2d_desc.Width = metadata.width;
	//	texture2d_desc.Height = metadata.height;


	//	return true;
	//}


	//bool TextureShader::LoadMipMap(const wchar_t* filename)
	//{
	//	ID3D11Device* device = pSystem.GetDevice();

	//	// 画像ファイル読み込み DirectXTex
	//	DirectX::TexMetadata metadata;
	//	DirectX::ScratchImage image;
	//	HRESULT hr = LoadFromWICFile(filename,
	//		0, &metadata, image);
	//	if (FAILED(hr)) {
	//		// 読み込めなかったらDDSファイルを読み込む
	//		hr = LoadFromDDSFile(filename, 0, &metadata, image);
	//	}
	//	assert(SUCCEEDED(hr));



	//	//ミップマップ生成
	//	DirectX::ScratchImage mipChain;
	//	DirectX::GenerateMipMaps(
	//		image.GetImages(),		//元画像
	//		image.GetImageCount(),
	//		image.GetMetadata(),
	//		DirectX::TEX_FILTER_DEFAULT,
	//		0,						//ミップマップをすべて作る
	//		mipChain);

	//	// 画像からシェーダリソースView
	//	hr = CreateShaderResourceView(
	//		device,
	//		mipChain.GetImages(),
	//		mipChain.GetImageCount(),
	//		mipChain.GetMetadata(),
	//		ShaderResourceView.GetAddressOf());

	//	assert(SUCCEEDED(hr));
	//	//テクスチャデータ取得
	//	texture2d_desc.Width = metadata.width;
	//	texture2d_desc.Height = metadata.height;

	//	return true;
	//}
	//void TextureShader::HDR_CheckHeader(FILE* fp)
	//{
	//	char  buf[256];

	//	// ヘッダチェック
	//	for (;;) {
	//		if (fgets(buf, 256, fp) == NULL) return;
	//		if (buf[0] == '\n' || buf[0] == '\r') break;

	//		if (strncmp(buf, "#?", 2) == 0)
	//		{
	//			//if (strncmp(buf, "#?RADIANCE", 10) != 0) return -1;
	//		}
	//		//	フォーマットチェック
	//		if (strncmp(buf, "FORMAT=", 7) == 0)
	//		{
	//			//if( strncmp( buf, "FORMAT=32-bit_rle_rgbe", 22) != 0 ) return -1;
	//		}
	//	}

	//	// 画像サイズ取得
	//	if (fgets(buf, 256, fp) == NULL) return;
	//	char X[5], Y[5];
	//	sscanf(buf, "%s %d %s %d", Y, &texture2d_desc.Height, X, &texture2d_desc.Width);
	//}

	//bool TextureShader::HDR_ReadLine(BYTE* scanline, FILE* fp)
	//{
	//	//	ライン情報読み込み
	//	int val0 = getc(fp);	// 0x02
	//	int val1 = getc(fp);	// 0x02
	//	int val2 = getc(fp);	// size
	//	int val3 = getc(fp);	// size
	//							// 幅チェック
	//	if ((val2 << 8 | val3) != texture2d_desc.Width) return false;

	//	for (int ch = 0; ch < 4; ch++)
	//	{
	//		for (int x = 0; x < texture2d_desc.Width; )
	//		{
	//			int length = getc(fp);
	//			if (length == EOF) return false;

	//			bool Runlength = false;
	//			int val = 0;
	//			if (length > 128) {
	//				// ランレングス設定
	//				Runlength = true;
	//				length -= 128;
	//				val = getc(fp);
	//			}
	//			//	データ読み込み
	//			while (length > 0) {
	//				if (Runlength) scanline[x * 4 + ch] = val;
	//				else scanline[x * 4 + ch] = getc(fp);
	//				x++;
	//				length--;
	//			}
	//		}
	//	}
	//	return true;
	//}

	//bool TextureShader::HDR_ReadPixels(FILE* fp, float* buf)
	//{
	//	int ret = 0;
	//	BYTE scanin[8192 * 4];
	//	float* work = buf;

	//	for (int y = texture2d_desc.Height - 1; y >= 0; y--) {
	//		if (!HDR_ReadLine(scanin, fp)) return false;
	//		for (int x = 0; x < texture2d_desc.Width; x++) {
	//			float exp = powf(2, scanin[x * 4 + 3] - 128.0f);
	//			work[0] = scanin[x * 4 + 0] * exp;
	//			work[1] = scanin[x * 4 + 1] * exp;
	//			work[2] = scanin[x * 4 + 2] * exp;
	//			work[3] = 1;
	//			work += 4;
	//		}
	//	}
	//	return true;
	//}

	//bool TextureShader::LoadHDR(const wchar_t* filename)
	//{
	//	setlocale(LC_CTYPE, "");
	//	char name[128] = {};
	//	wcstombs(name, filename, wcslen(filename));


	//	HRESULT hr = S_OK;

	//	FILE* fd;

	//	if ((fd = fopen(name, "rb")) == NULL)
	//	{
	//		return false;
	//	}

	//	// ヘッダーを調べる
	//	HDR_CheckHeader(fd);

	//	// コンバートする
	//	float* buf = new float[texture2d_desc.Width * texture2d_desc.Height * 4];
	//	HDR_ReadPixels(fd, buf);
	//	fclose(fd);

	//	DirectX::Image img;
	//	img.format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//	img.width = texture2d_desc.Width;
	//	img.height = texture2d_desc.Height;
	//	img.rowPitch = sizeof(float) * 4 * img.width;
	//	img.slicePitch = img.rowPitch * img.height;
	//	img.pixels = (uint8_t*)buf;

	//	DirectX::ScratchImage image;
	//	image.InitializeFromImage(img);

	//	DirectX::ScratchImage mipChain;
	//	DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_DEFAULT, 0, mipChain);

	//	// 画像からシェーダリソースView
	//	ID3D11Device* device = pSystem.GetDevice();
	//	CreateShaderResourceView(device, mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), &ShaderResourceView);


	//	return true;
	//}


	//void TextureShader::Set(UINT Slot, BOOL flg)
	//{
	//	ID3D11DeviceContext* device_context = pSystem.GetDeviceContext();

	//	if (flg == FALSE) {

	//		ID3D11ShaderResourceView* rtv[1] = { NULL };
	//		ID3D11SamplerState* ss[1] = { NULL };
	//		device_context->PSSetShaderResources(Slot, 1, rtv);
	//		device_context->PSSetSamplers(Slot, 1, ss);
	//		device_context->HSSetShaderResources(Slot, 1, rtv);
	//		device_context->HSSetSamplers(Slot, 1, ss);
	//		device_context->DSSetShaderResources(Slot, 1, rtv);
	//		device_context->DSSetSamplers(Slot, 1, ss);
	//		device_context->GSSetShaderResources(Slot, 1, rtv);
	//		device_context->GSSetSamplers(Slot, 1, ss);


	//		return;
	//	}
	//	if (ShaderResourceView) {
	//		device_context->PSSetShaderResources(Slot, 1, ShaderResourceView.GetAddressOf());
	//		device_context->HSSetShaderResources(Slot, 1, ShaderResourceView.GetAddressOf());
	//		device_context->DSSetShaderResources(Slot, 1, ShaderResourceView.GetAddressOf());
	//		device_context->GSSetShaderResources(Slot, 1, ShaderResourceView.GetAddressOf());

	//	}
	//}

	//bool TextureShader::Create(u_int width, u_int height, DXGI_FORMAT format)
	//{
	//	ID3D11Device* device = pSystem.GetDevice();

	//	ComPtr<ID3D11Texture2D> Texture2D;
	//	HRESULT hr = S_OK;
	//	//	テクスチャ作成
	//	ZeroMemory(&texture2d_desc, sizeof(texture2d_desc));
	//	texture2d_desc.Width = width;
	//	texture2d_desc.Height = height;
	//	texture2d_desc.MipLevels = 1;
	//	texture2d_desc.ArraySize = 1;
	//	texture2d_desc.Format = format;
	//	texture2d_desc.SampleDesc.Count = 1;
	//	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	//	texture2d_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//	texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	//	hr = device->CreateTexture2D(&texture2d_desc, NULL, Texture2D.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	//	レンダーターゲットビュー作成
	//	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	//	ZeroMemory(&rtvd, sizeof(rtvd));
	//	rtvd.Format = format;
	//	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//	rtvd.Texture2D.MipSlice = 0;
	//	hr = device->CreateRenderTargetView(Texture2D.Get(), &rtvd, RenderTargetView.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	//	シェーダーリソースビュー作成
	//	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	//	ZeroMemory(&srvd, sizeof(srvd));
	//	srvd.Format = format;
	//	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//	srvd.Texture2D.MostDetailedMip = 0;
	//	srvd.Texture2D.MipLevels = 1;
	//	hr = device->CreateShaderResourceView(Texture2D.Get(), &srvd, ShaderResourceView.GetAddressOf());

	//	assert(SUCCEEDED(hr));

	//	return true;
	//}

	//bool TextureShader::CreateDepth(u_int width, u_int height, DXGI_FORMAT format)
	//{
	//	ID3D11Device* device = pSystem.GetDevice();

	//	ComPtr<ID3D11Texture2D> Texture2D;
	//	HRESULT hr = S_OK;
	//	//	テクスチャ作成
	//	ZeroMemory(&texture2d_desc, sizeof(texture2d_desc));
	//	texture2d_desc.Width = width;
	//	texture2d_desc.Height = height;
	//	texture2d_desc.MipLevels = 1;
	//	texture2d_desc.ArraySize = 1;
	//	texture2d_desc.Format = format;
	//	texture2d_desc.SampleDesc.Count = 1;
	//	texture2d_desc.SampleDesc.Quality = 0;
	//	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	//	texture2d_desc.CPUAccessFlags = 0;
	//	texture2d_desc.MiscFlags = 0;
	//	texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	//	hr = device->CreateTexture2D(&texture2d_desc, NULL, Texture2D.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	// 深度ステンシルビュー設定
	//	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	//	ZeroMemory(&dsvd, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	//	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	//	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//	dsvd.Texture2D.MipSlice = 0;
	//	hr = device->CreateDepthStencilView(Texture2D.Get(), &dsvd, DepthStencilView.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	//	シェーダーリソースビュー作成
	//	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	//	ZeroMemory(&srvd, sizeof(srvd));
	//	srvd.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	//	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//	srvd.Texture2D.MostDetailedMip = 0;
	//	srvd.Texture2D.MipLevels = 1;
	//	hr = device->CreateShaderResourceView(Texture2D.Get(), &srvd, ShaderResourceView.GetAddressOf());

	//	assert(SUCCEEDED(hr));

	//	return true;
	//}

	//bool TextureShader::CreateCube(u_int width, u_int height, DXGI_FORMAT format, int miplevel)
	//{
	//	ID3D11Device* device = pSystem.GetDevice();

	//	ComPtr<ID3D11Texture2D> Texture2D;
	//	HRESULT hr = S_OK;
	//	//	テクスチャ作成
	//	ZeroMemory(&texture2d_desc, sizeof(texture2d_desc));
	//	texture2d_desc.Width = width;
	//	texture2d_desc.Height = height;
	//	texture2d_desc.MipLevels = miplevel;//★
	//	texture2d_desc.ArraySize = 6;
	//	texture2d_desc.Format = format;
	//	texture2d_desc.SampleDesc.Count = 1;
	//	texture2d_desc.SampleDesc.Quality = 0;
	//	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	//	texture2d_desc.CPUAccessFlags = 0;
	//	texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	//	texture2d_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS |
	//		D3D11_RESOURCE_MISC_TEXTURECUBE;

	//	hr = device->CreateTexture2D(&texture2d_desc, NULL, Texture2D.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	//	シェーダーリソースビュー作成
	//	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	//	ZeroMemory(&srvd, sizeof(srvd));
	//	srvd.Format = format;
	//	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	//	srvd.TextureCube.MostDetailedMip = 0;
	//	srvd.TextureCube.MipLevels = miplevel;
	//	hr = device->CreateShaderResourceView(Texture2D.Get(), &srvd, ShaderResourceView.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	//	レンダーターゲットビュー作成
	//	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	//	ZeroMemory(&rtvd, sizeof(rtvd));
	//	rtvd.Format = format;
	//	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;//2Dテクスチャ配列
	//	rtvd.Texture2DArray.FirstArraySlice = 0;
	//	rtvd.Texture2DArray.MipSlice = 0;
	//	rtvd.Texture2DArray.ArraySize = 6;
	//	hr = device->CreateRenderTargetView(Texture2D.Get(), &rtvd, RenderTargetView.GetAddressOf());
	//	assert(SUCCEEDED(hr));



	//	return true;

	//}
	//bool TextureShader::CreateCubeDepthStencil(u_int width, u_int height)
	//{
	//	ID3D11Device* device = pSystem.GetDevice();

	//	ComPtr<ID3D11Texture2D> Texture2D;
	//	HRESULT hr = S_OK;
	//	//	テクスチャ作成
	//	ZeroMemory(&texture2d_desc, sizeof(texture2d_desc));
	//	texture2d_desc.Width = width;
	//	texture2d_desc.Height = height;
	//	texture2d_desc.MipLevels = 1;
	//	texture2d_desc.ArraySize = 6;
	//	texture2d_desc.Format = DXGI_FORMAT_D32_FLOAT;
	//	texture2d_desc.SampleDesc.Count = 1;
	//	texture2d_desc.SampleDesc.Quality = 0;
	//	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	//	texture2d_desc.CPUAccessFlags = 0;

	//	texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	//	texture2d_desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	//	hr = device->CreateTexture2D(&texture2d_desc, NULL, Texture2D.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	//	デプスステンシルビュー作成
	//	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	//	ZeroMemory(&dsvd, sizeof(dsvd));
	//	dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	//	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;//2Dテクスチャ配列
	//	dsvd.Texture2DArray.FirstArraySlice = 0;
	//	dsvd.Texture2DArray.ArraySize = 6;
	//	dsvd.Texture2DArray.MipSlice = 0;
	//	dsvd.Flags = D3D11_RESOURCE_MISC_TEXTURECUBE;
	//	hr = device->CreateDepthStencilView(Texture2D.Get(), NULL, DepthStencilView.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	return true;


	//}




	//bool TextureShader::CreateMipMap(u_int width, u_int height, DXGI_FORMAT format)
	//{
	//	ID3D11Device* device = pSystem.GetDevice();

	//	ComPtr<ID3D11Texture2D> Texture2D;
	//	HRESULT hr = S_OK;
	//	//	テクスチャ作成
	//	ZeroMemory(&texture2d_desc, sizeof(texture2d_desc));
	//	texture2d_desc.Width = width;
	//	texture2d_desc.Height = height;
	//	texture2d_desc.MipLevels = 9;
	//	texture2d_desc.ArraySize = 1;
	//	texture2d_desc.Format = format;
	//	texture2d_desc.SampleDesc.Count = 1;
	//	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	//	texture2d_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	//	texture2d_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	//	hr = device->CreateTexture2D(&texture2d_desc, NULL, Texture2D.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	//	レンダーターゲットビュー作成
	//	D3D11_RENDER_TARGET_VIEW_DESC rtvd;
	//	ZeroMemory(&rtvd, sizeof(rtvd));
	//	rtvd.Format = format;
	//	rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	//	rtvd.Texture2D.MipSlice = 0;
	//	hr = device->CreateRenderTargetView(Texture2D.Get(), &rtvd, RenderTargetView.GetAddressOf());
	//	assert(SUCCEEDED(hr));

	//	//	シェーダーリソースビュー作成
	//	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	//	ZeroMemory(&srvd, sizeof(srvd));
	//	srvd.Format = format;
	//	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//	srvd.Texture2D.MostDetailedMip = 0;
	//	srvd.Texture2D.MipLevels = 9;
	//	hr = device->CreateShaderResourceView(Texture2D.Get(), &srvd, ShaderResourceView.GetAddressOf());

	//	assert(SUCCEEDED(hr));

	//	return true;
	//}

