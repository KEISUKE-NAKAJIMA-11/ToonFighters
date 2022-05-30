#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <d3d11shader.h>

#include <string>


//you can set null for 'input_layout' if 'input_layout' is not necessary 
HRESULT create_vs_from_cso(ID3D11Device* device, const char* cso_name,
	ID3D11VertexShader** vertex_shader,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, size_t num_elements, bool enable_caching = true);
HRESULT create_vs_from_source(ID3D11Device* device, const char* key_name, const std::string& source,
	ID3D11VertexShader** vertex_shader, const char* entry_point, const char* target,
	ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, size_t num_elements, bool enable_caching = true);
void release_all_cached_vertex_shaders();

HRESULT create_ps_from_cso(ID3D11Device* device, const char* cso_name,
	ID3D11PixelShader** pixel_shader, ID3D11ClassLinkage** class_linkage = 0, ID3D11ShaderReflection** reflector = 0, bool enable_caching = true);
HRESULT create_ps_from_source(ID3D11Device* device, const char* key_name, const std::string& source,
	ID3D11PixelShader** pixel_shader, const char* entry_point, const char* target = "ps_5_0", bool enable_caching = true);
void release_all_cached_pixel_shaders();

HRESULT create_cs_from_cso(ID3D11Device* device, const char* cso_name,
	ID3D11ComputeShader** compute_shader, ID3D11ClassLinkage** class_linkage = 0, ID3D11ShaderReflection** reflector = 0, bool enable_caching = true);
HRESULT create_cs_from_source(ID3D11Device* device, const char* key_name, const std::string& source,
	ID3D11ComputeShader** compute_shader, const char* entry_point, const char* target = "ps_5_0", bool enable_caching = true);
void release_all_cached_compute_shaders();



HRESULT create_gs_from_cso(ID3D11Device* device, const char* cso_name,
	ID3D11GeometryShader** compute_shader, ID3D11ClassLinkage** class_linkage = 0, ID3D11ShaderReflection** reflector = 0, bool enable_caching = true);
HRESULT create_gs_from_source(ID3D11Device* device, const char* key_name, const std::string& source,
	ID3D11GeometryShader** compute_shader, const char* entry_point, const char* target = "ps_5_0", bool enable_caching = true);
void release_all_cached_geometry_shaders();




class vertex_shader
{
public:
	vertex_shader() = default;
	
	vertex_shader(ID3D11Device* device, const char* cso, D3D11_INPUT_ELEMENT_DESC* input_element_desc , size_t num_elements, bool enable_caching = true)
	{
		create_vs_from_cso(device, cso, shader_object.GetAddressOf(), input_layout.GetAddressOf(), const_cast<D3D11_INPUT_ELEMENT_DESC*>(input_element_desc), num_elements, enable_caching);
	}

	virtual ~vertex_shader() = default;
	vertex_shader(vertex_shader&) = delete;
	vertex_shader& operator=(vertex_shader&) = delete;



	Microsoft::WRL::ComPtr<ID3D11VertexShader> shader_object;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;

	void activate(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->IAGetInputLayout(cached_input_layout.ReleaseAndGetAddressOf());
		immediate_context->VSGetShader(cached_shader_object.ReleaseAndGetAddressOf(), 0, 0);

		immediate_context->IASetInputLayout(input_layout.Get());
		immediate_context->VSSetShader(shader_object.Get(), 0, 0);
	}
	void deactivate(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->IASetInputLayout(cached_input_layout.Get());
		immediate_context->VSSetShader(cached_shader_object.Get(), 0, 0);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> cached_shader_object;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> cached_input_layout;
};

class pixel_shader
{
public:
	pixel_shader() = default;
	pixel_shader(ID3D11Device* device, const char* cso)
	{
		create_ps_from_cso(device, cso, shader_object.GetAddressOf());
	}
	virtual ~pixel_shader() = default;
	pixel_shader(pixel_shader&) = delete;
	pixel_shader& operator=(pixel_shader&) = delete;

	Microsoft::WRL::ComPtr<ID3D11PixelShader> shader_object;

	void activate(ID3D11DeviceContext* immediate_context)
	{
		
		immediate_context->PSSetShader(shader_object.Get(), 0, 0);
	}
	void deactivate(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->PSSetShader(cached_shader_object.Get(), 0, 0);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11PixelShader> cached_shader_object;
};


class compute_shader
{
public:
	compute_shader() = default;
	compute_shader(ID3D11Device* device, const char* cso)
	{
		create_cs_from_cso(device, cso, shader_object.GetAddressOf());
	}
	virtual ~compute_shader() = default;
	compute_shader(compute_shader&) = delete;
	compute_shader& operator=(compute_shader&) = delete;

	Microsoft::WRL::ComPtr<ID3D11ComputeShader> shader_object;

	void activate(ID3D11DeviceContext* immediate_context)
	{

		immediate_context->CSSetShader(shader_object.Get(), 0, 0);
	}
	void deactivate(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->CSSetShader(cached_shader_object.Get(), 0, 0);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> cached_shader_object;
};



class geometry_shader
{
public:
	geometry_shader() = default;
	geometry_shader(ID3D11Device* device, const char* cso)
	{
		create_gs_from_cso(device, cso, shader_object.GetAddressOf());
     }
	virtual ~geometry_shader() = default;
	geometry_shader(geometry_shader&) = delete;
	geometry_shader& operator=(geometry_shader&) = delete;

	Microsoft::WRL::ComPtr<ID3D11GeometryShader> shader_object;

	void activate(ID3D11DeviceContext* immediate_context)
	{

		immediate_context->GSSetShader(shader_object.Get(), 0, 0);
	}
	void deactivate(ID3D11DeviceContext* immediate_context)
	{
		immediate_context->GSSetShader(cached_shader_object.Get(), 0, 0);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> cached_shader_object;
};



#if 0
HRESULT create_input_layout(ID3D11Device* device, ID3D11InputLayout** input_layout, D3D11_INPUT_ELEMENT_DESC* input_element_desc, size_t num_elements, bool enable_caching = true);
void release_all_cached_input_layouts();
#endif