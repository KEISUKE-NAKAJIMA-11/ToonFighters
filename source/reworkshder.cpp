#include "reworkshder.h"
#include "framework.h"

HRESULT Shader::create_vertex(const char* cso_file, ID3D11VertexShader** vert, D3D11_INPUT_ELEMENT_DESC* layout, UINT numElements, ID3D11InputLayout** input)
{
	struct Vertex_and_Layout
	{
		Vertex_and_Layout(ID3D11VertexShader* vert, ID3D11InputLayout* input) : pVertex(vert), pInput(input) {}
		Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertex;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> pInput;
	};
	static std::map<std::string, Vertex_and_Layout> cache;

	auto it = cache.find(cso_file);
	if (it != cache.end())
	{
		*vert = it->second.pVertex.Get();
		(*vert)->AddRef();
		*input = it->second.pInput.Get();
		(*input)->AddRef();
		return S_OK;
	}

	HRESULT hr = S_OK;
	FILE* fp = 0;

	fopen_s(&fp, cso_file, "rb");
	_ASSERT_EXPR_A(fp, "CSO File not found");
	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	std::unique_ptr< unsigned char[] >cso_data = std::make_unique< unsigned char[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	hr = pSystem.device->CreateVertexShader(cso_data.get(), cso_sz, NULL, vert);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	hr = pSystem.device->CreateInputLayout(layout, numElements, cso_data.get(),
		cso_sz, input);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	cache.insert(std::make_pair(cso_file, Vertex_and_Layout(*vert, *input)));
	return hr;
}

HRESULT Shader::CreatePixel(const char* ps_file, ID3D11PixelShader** pixel)
{
	static std::map < std::string, Microsoft::WRL::ComPtr<ID3D11PixelShader>> pixelcache;

	auto it = pixelcache.find(ps_file);
	if (it != pixelcache.end())
	{
		*pixel = it->second.Get();
		(*pixel)->AddRef();
		return S_OK;
	}

	HRESULT hr = S_OK;

	FILE* fpp = 0;

	fopen_s(&fpp, ps_file, "rb");
	_ASSERT_EXPR_A(fpp, "CSO File not found");
	fseek(fpp, 0, SEEK_END);
	long cso_szp = ftell(fpp);
	fseek(fpp, 0, SEEK_SET);
	std::unique_ptr<unsigned char[]> cso_datap = std::make_unique< unsigned char[]>(cso_szp);
	fread(cso_datap.get(), cso_szp, 1, fpp);
	fclose(fpp);

	hr = pSystem.device->CreatePixelShader(cso_datap.get(), cso_szp, NULL, pixel);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//delete[]cso_datap;

	pixelcache.insert(std::make_pair(ps_file, *pixel));

	return hr;
}

HRESULT Shader::create_gs_from_cso(const char* cso_name, ID3D11GeometryShader** geometry_shader)
{
	static std::map<std::string, Microsoft::WRL::ComPtr<ID3D11GeometryShader>> cache;
	auto it = cache.find(cso_name);
	if (it != cache.end())
	{
		*geometry_shader = it->second.Get();
		(*geometry_shader)->AddRef();
		return S_OK;
	}

	FILE* fp = nullptr;
	errno_t error;

	error = fopen_s(&fp, cso_name, "rb");
	assert("CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = pSystem.device->CreateGeometryShader(cso_data.get(), cso_sz, nullptr, geometry_shader);
	assert(SUCCEEDED(hr));

	cache.insert(std::make_pair(cso_name, *geometry_shader));

	return hr;
}

HRESULT Shader::create_ds_from_cso(const char* cso_name, ID3D11DomainShader** domain_shader)
{
	static std::map<std::string, Microsoft::WRL::ComPtr<ID3D11DomainShader>> cache;
	auto it = cache.find(cso_name);
	if (it != cache.end())
	{
		*domain_shader = it->second.Get();
		(*domain_shader)->AddRef();
		return S_OK;
	}

	FILE* fp = nullptr;
	errno_t error;

	error = fopen_s(&fp, cso_name, "rb");
	assert("CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = pSystem.device->CreateDomainShader(cso_data.get(), cso_sz, nullptr, domain_shader);
	assert(SUCCEEDED(hr));

	cache.insert(std::make_pair(cso_name, *domain_shader));

	return hr;
}

HRESULT Shader::create_hs_from_cso(const char* cso_name, ID3D11HullShader** hull_shader)
{
	static std::map<std::string, Microsoft::WRL::ComPtr<ID3D11HullShader>> cache;
	auto it = cache.find(cso_name);
	if (it != cache.end())
	{
		*hull_shader = it->second.Get();
		(*hull_shader)->AddRef();
		return S_OK;
	}

	FILE* fp = nullptr;
	errno_t error;

	error = fopen_s(&fp, cso_name, "rb");
	assert("CSO File not found");

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<unsigned char[]> cso_data = std::make_unique<unsigned char[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = pSystem.device->CreateHullShader(cso_data.get(), cso_sz, nullptr, hull_shader);
	assert(SUCCEEDED(hr));

	cache.insert(std::make_pair(cso_name, *hull_shader));

	return hr;
}



bool Shader::Create_element(const char* VS_cso_file)
{
	switch (type)
	{
	case ShaderType::SKIN:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.skin_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.skin_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::STATIC:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.static_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.static_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::SPRITE:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.sprite_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.sprite_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::GEO:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.geometric_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.geometric_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::BOARD:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.board_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.board_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::ANIM:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.anim_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.anim_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::TOON:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.toon_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.toon_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::TOGBUF:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.toGbuf_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.toGbuf_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::SPRITE_EX:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.spriteEx_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.spriteEx_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::FLAT:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.flat_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.flat_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::GAUSS:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.gauss_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.gauss_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::MULTI_GAUSS:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.multi_gauss_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.multi_gauss_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::FUR:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.fur_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.fur_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::SKY:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.sky_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.sky_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::TITLE:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.title_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.title_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::TRAJECTORY:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.trajectory_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.trajectory_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::TESSELLATION:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.tessellation_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.tessellation_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	case ShaderType::SCREENQUAD:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.tessellation_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.tessellation_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
		VertextLayout.Reset();
	}
	break;
	case ShaderType::SKIN_EX:
	{
		UINT numElements = YRINPUT_ELEMENT_DESC.skinEx_element_desc.size();
		create_vertex(
			VS_cso_file,
			VSShader.GetAddressOf(),
			YRINPUT_ELEMENT_DESC.skinEx_element_desc.data(),
			numElements,
			VertextLayout.GetAddressOf());
	}
	break;
	default:
		assert(0);
		break;
	}
	return true;
}



//Create

bool Shader::Create(const char* VS_cso_file, const char* PS_cso_file)
{
	Create_element(VS_cso_file);
	CreatePixel(PS_cso_file, PSShader.GetAddressOf());
	return true;
}

bool Shader::Create(const char* VS_cso_file, const char* PS_cso_file, const char* GS_cso_file)
{
	Create_element(VS_cso_file);
	CreatePixel(PS_cso_file, PSShader.GetAddressOf());
	create_gs_from_cso(GS_cso_file, GSShader.GetAddressOf());
	return true;
}

bool Shader::Create(const char* VS_cso_file, const char* PS_cso_file, const char* DS_cso_file, const char* HS_cso_file)
{
	Create_element(VS_cso_file);
	CreatePixel(PS_cso_file, PSShader.GetAddressOf());
	create_ds_from_cso(DS_cso_file, DSShader.GetAddressOf());
	create_hs_from_cso(HS_cso_file, HSShader.GetAddressOf());
	return true;
}

bool Shader::Create(const char* VS_cso_file, const char* PS_cso_file, const char* GS_cso_file, const char* DS_cso_file, const char* HS_cso_file)
{
	Create_element(VS_cso_file);
	CreatePixel(PS_cso_file, PSShader.GetAddressOf());
	create_gs_from_cso(GS_cso_file, GSShader.GetAddressOf());
	create_ds_from_cso(DS_cso_file, DSShader.GetAddressOf());
	create_hs_from_cso(HS_cso_file, HSShader.GetAddressOf());
	return true;
}

void Shader::Acivate()
{
	 pSystem.devicecontext->IASetInputLayout(VertextLayout.Get());
	 pSystem.devicecontext->VSSetShader(VSShader.Get(), NULL, 0);
	 pSystem.devicecontext->HSSetShader(HSShader.Get(), NULL, 0);
	 pSystem.devicecontext->DSSetShader(DSShader.Get(), NULL, 0);
	 pSystem.devicecontext->GSSetShader(GSShader.Get(), NULL, 0);
	 pSystem.devicecontext->PSSetShader(PSShader.Get(), NULL, 0);
}

void Shader::Inactivate()
{
	 pSystem.devicecontext->IASetInputLayout(NULL);
	 pSystem.devicecontext->VSSetShader(NULL, NULL, 0);
	 pSystem.devicecontext->HSSetShader(NULL, NULL, 0);
	 pSystem.devicecontext->DSSetShader(NULL, NULL, 0);
	 pSystem.devicecontext->GSSetShader(NULL, NULL, 0);
	 pSystem.devicecontext->PSSetShader(NULL, NULL, 0);
}