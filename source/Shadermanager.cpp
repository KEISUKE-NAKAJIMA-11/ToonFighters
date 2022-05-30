
#include <d3dcompiler.h>

#include <memory>
#include <map>
#include <sstream>
#include "Shadermanager.h"
#include "misc.h"


struct vs_combo
{
	vs_combo(ID3D11VertexShader* vertex_shader, ID3D11InputLayout* input_layout) : vertex_shader(vertex_shader), input_layout(input_layout) {}
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
};
static std::map<std::string, vs_combo> cached_vertex_shaders;

//you can set null for 'input_layout' if it is not necessary 
HRESULT create_vs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11VertexShader** vertex_shader, ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, size_t num_elements, bool enable_caching)
{
	std::map<std::string, vs_combo>::iterator it = cached_vertex_shaders.find(cso_name);
	if (it != cached_vertex_shaders.end())
	{
		*vertex_shader = it->second.vertex_shader.Get();
		(*vertex_shader)->AddRef();
		if (input_layout)
		{
			*input_layout = it->second.input_layout.Get();
			_ASSERT_EXPR_A(*input_layout, "cached input_layout must be not nil");
			(*input_layout)->AddRef();
		}
		return S_OK;
	}

	FILE* fp = 0;
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, (std::string("cso File not found : ") + cso_name).c_str());

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<uint8_t[]> cso_data = std::make_unique<uint8_t[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = S_OK;
	hr = device->CreateVertexShader(cso_data.get(), cso_sz, 0, vertex_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	if (input_layout)
	{
		hr = device->CreateInputLayout(input_element_desc, static_cast<UINT>(num_elements), cso_data.get(), cso_sz, input_layout);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	if (enable_caching)
	{
		cached_vertex_shaders.insert(std::make_pair(cso_name, vs_combo(*vertex_shader, input_layout ? *input_layout : 0)));
	}

	return hr;
}
HRESULT create_vs_from_source(ID3D11Device* device, const char* key_name, const std::string& source,
	ID3D11VertexShader** vertex_shader, const char* entry_point, const char* target,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, size_t num_elements, bool enable_caching)
{
	std::map<std::string, vs_combo>::iterator it = cached_vertex_shaders.find(key_name);
	if (it != cached_vertex_shaders.end())
	{
		*vertex_shader = it->second.vertex_shader.Get();
		(*vertex_shader)->AddRef();
		if (input_layout)
		{
			*input_layout = it->second.input_layout.Get();
			_ASSERT_EXPR_A(*input_layout, "cached input_layout must be not nil");
			(*input_layout)->AddRef();
		}
		return S_OK;
	}

	HRESULT hr = S_OK;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> compiled_shader_blob;
	Microsoft::WRL::ComPtr<ID3DBlob> error_message_blob;
	hr = D3DCompile(source.c_str(), source.length(), 0, 0, 0, entry_point, "vs_5_0", flags, 0, compiled_shader_blob.GetAddressOf(), error_message_blob.GetAddressOf());
	_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));
	hr = device->CreateVertexShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize(), 0, vertex_shader);
	_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));

	if (input_layout)
	{
		hr = device->CreateInputLayout(input_element_desc, static_cast<UINT>(num_elements), compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize(), input_layout);
		_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));
	}

	if (enable_caching)
	{
		cached_vertex_shaders.insert(std::make_pair(key_name, vs_combo(*vertex_shader, input_layout ? *input_layout : 0)));
	}

	return hr;
}
void release_all_cached_vertex_shaders()
{
	cached_vertex_shaders.clear();
}

struct ps_combo
{
	ps_combo(ID3D11PixelShader* pixel_shader, ID3D11ClassLinkage* class_linkage, ID3D11ShaderReflection* reflector) : pixel_shader(pixel_shader), class_linkage(class_linkage), reflector(reflector) {}
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11ClassLinkage> class_linkage;
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
};
static std::map<std::string, ps_combo> cached_pixel_shaders;
HRESULT create_ps_from_cso(ID3D11Device* device, const char* cso_name,
	ID3D11PixelShader** pixel_shader, ID3D11ClassLinkage** class_linkage, ID3D11ShaderReflection** reflector,
	bool enable_caching)
{
	std::map<std::string, ps_combo>::iterator it = cached_pixel_shaders.find(cso_name);
	if (it != cached_pixel_shaders.end())
	{
		*pixel_shader = it->second.pixel_shader.Get();
		(*pixel_shader)->AddRef();
		if (class_linkage)
		{
			*class_linkage = it->second.class_linkage.Get();
			(*class_linkage)->AddRef();
		}
		if (reflector)
		{
			*reflector = it->second.reflector.Get();
			(*reflector)->AddRef();
		}
		return S_OK;
	}
	FILE* fp = 0;
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, (std::string("cso File not found : ") + cso_name).c_str());

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<uint8_t[]> cso_data = std::make_unique<uint8_t[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = S_OK;
	if (class_linkage)
	{
		hr = device->CreateClassLinkage(class_linkage);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	if (reflector)
	{
		hr = D3DReflect(cso_data.get(), cso_sz, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(reflector));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	hr = device->CreatePixelShader(cso_data.get(), cso_sz, class_linkage ? *class_linkage : 0, pixel_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	if (enable_caching)
	{
		cached_pixel_shaders.insert(std::make_pair(cso_name, ps_combo(*pixel_shader, class_linkage ? *class_linkage : 0, reflector ? *reflector : 0)));
	}

	return hr;
}
HRESULT create_ps_from_source(ID3D11Device* device, const char* key_name, const std::string& source, ID3D11PixelShader** pixel_shader, const char* entry_point, const char* target, bool enable_caching)
{
	std::map<std::string, ps_combo>::iterator it = cached_pixel_shaders.find(key_name);
	if (it != cached_pixel_shaders.end())
	{
		*pixel_shader = it->second.pixel_shader.Get();
		(*pixel_shader)->AddRef();
		return S_OK;
	}

	HRESULT hr = S_OK;
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	Microsoft::WRL::ComPtr<ID3DBlob> compiled_shader_blob;
	Microsoft::WRL::ComPtr<ID3DBlob> error_message_blob;
	hr = D3DCompile(source.c_str(), source.length(), 0, 0, 0, entry_point, target, flags, 0, compiled_shader_blob.GetAddressOf(), error_message_blob.GetAddressOf());
	_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));
	hr = device->CreatePixelShader(compiled_shader_blob->GetBufferPointer(), compiled_shader_blob->GetBufferSize(), 0, pixel_shader);
	_ASSERT_EXPR_A(SUCCEEDED(hr), reinterpret_cast<LPCSTR>(error_message_blob->GetBufferPointer()));

	if (enable_caching)
	{
		cached_pixel_shaders.insert(std::make_pair(key_name, ps_combo(*pixel_shader, 0, 0)));
	}

	return hr;
}
void release_all_cached_pixel_shaders()
{
	cached_pixel_shaders.clear();
}

struct cs_combo
{
	cs_combo(ID3D11ComputeShader* pixel_shader, ID3D11ClassLinkage* class_linkage, ID3D11ShaderReflection* reflector) : pixel_shader(pixel_shader), class_linkage(class_linkage), reflector(reflector) {}
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11ClassLinkage> class_linkage;
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
};
static std::map<std::string, cs_combo> cached_compute_shaders;

HRESULT create_cs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11ComputeShader** pixel_shader, ID3D11ClassLinkage** class_linkage, ID3D11ShaderReflection** reflector, bool enable_caching)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT create_cs_from_source(ID3D11Device* device, const char* key_name, const std::string& source, ID3D11ComputeShader** pixel_shader, const char* entry_point, const char* target, bool enable_caching)
{
	HRESULT hr = S_OK;

	return hr;
}



void release_all_cached_compute_shaders()
{
	cached_compute_shaders.clear();
}


struct gs_combo
{
	gs_combo(ID3D11GeometryShader* geometry_shader, ID3D11ClassLinkage* class_linkage, ID3D11ShaderReflection* reflector) : geometry_shader(geometry_shader), class_linkage(class_linkage), reflector(reflector) {}
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometry_shader;
	Microsoft::WRL::ComPtr<ID3D11ClassLinkage> class_linkage;
	Microsoft::WRL::ComPtr<ID3D11ShaderReflection> reflector;
};
static std::map<std::string, gs_combo> cached_geometry_shaders;


HRESULT create_gs_from_cso(ID3D11Device* device, const char* cso_name, ID3D11GeometryShader** geometry_shader, ID3D11ClassLinkage** class_linkage, ID3D11ShaderReflection** reflector, bool enable_caching)
{
	std::map<std::string, gs_combo>::iterator it = cached_geometry_shaders.find(cso_name);
	if (it != cached_geometry_shaders.end())
	{
		*geometry_shader = it->second.geometry_shader.Get();
		(*geometry_shader)->AddRef();
		if (class_linkage)
		{
			*class_linkage = it->second.class_linkage.Get();
			(*class_linkage)->AddRef();
		}
		if (reflector)
		{
			*reflector = it->second.reflector.Get();
			(*reflector)->AddRef();
		}
		return S_OK;
	}
	FILE* fp = 0;
	fopen_s(&fp, cso_name, "rb");
	_ASSERT_EXPR_A(fp, (std::string("cso File not found : ") + cso_name).c_str());

	fseek(fp, 0, SEEK_END);
	long cso_sz = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::unique_ptr<uint8_t[]> cso_data = std::make_unique<uint8_t[]>(cso_sz);
	fread(cso_data.get(), cso_sz, 1, fp);
	fclose(fp);

	HRESULT hr = S_OK;
	if (class_linkage)
	{
		hr = device->CreateClassLinkage(class_linkage);
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}
	if (reflector)
	{
		hr = D3DReflect(cso_data.get(), cso_sz, IID_ID3D11ShaderReflection, reinterpret_cast<void**>(reflector));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	hr = device->CreateGeometryShader(cso_data.get(), cso_sz, class_linkage ? *class_linkage : 0, geometry_shader);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	if (enable_caching)
	{
		cached_geometry_shaders.insert(std::make_pair(cso_name, gs_combo(*geometry_shader, class_linkage ? *class_linkage : 0, reflector ? *reflector : 0)));
	}

	return hr;
}

HRESULT create_gs_from_source(ID3D11Device* device, const char* key_name, const std::string& source, ID3D11GeometryShader** compute_shader, const char* entry_point, const char* target, bool enable_caching)
{
	HRESULT hr = S_OK;

	return hr;
}

void release_all_cached_geometry_shaders()
{
	cached_geometry_shaders.clear();
}