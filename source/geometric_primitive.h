#pragma once

#include	<d3d11.h>
#include	<DirectXMath.h>

//描画エンジンを初めて触った時に作成したプリミティブクラス

class geometric_primitive
{
public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3	scale;
	};
	struct cbuffer
	{
		DirectX::XMFLOAT4X4 world_view_projection;
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 material_color;
		DirectX::XMFLOAT4 light_direction;
	};

private:
	ID3D11Buffer*            vertex_buffer;
	ID3D11Buffer*            index_buffer;
	ID3D11Buffer*            constant_buffer;
						     
	ID3D11VertexShader*      vertex_shader;
	ID3D11PixelShader*       pixel_shader;
	ID3D11InputLayout*       input_layout;

	ID3D11RasterizerState*   rasterizer_states[2];
	ID3D11DepthStencilState* depth_stencil_state;

public:
	geometric_primitive(ID3D11Device* device);
	 ~geometric_primitive() 
	{
		 if(vertex_buffer)vertex_buffer->Release();
		 if (index_buffer)index_buffer->Release();
	     if (constant_buffer)constant_buffer->Release();

		 if (vertex_shader)vertex_shader->Release();
		 if (pixel_shader)pixel_shader->Release();
		 if (input_layout)input_layout->Release();
		 if (rasterizer_states[0]) {

			 rasterizer_states[0]->Release();
		 }

		 if (rasterizer_states[1]) {
		
			 rasterizer_states[1]->Release();
		 }
		if (depth_stencil_state)depth_stencil_state->Release();
	
	}

	void render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world_view_projection, const DirectX::XMFLOAT4X4& world, const DirectX::XMFLOAT4& light_direction, const DirectX::XMFLOAT4& material_color, bool wireframe = false);

protected:
	void create_buffers(ID3D11Device* device, vertex* vertices, int num_vertices, u_int* indices, int num_indices);
};

class geometric_cube : public geometric_primitive
{
public:
	geometric_cube(ID3D11Device* device);
};

class geometric_cylinder : public geometric_primitive
{
public:
	geometric_cylinder(ID3D11Device* device, u_int slices);
};

class geometric_sphere : public geometric_primitive
{
public:

	geometric_sphere(ID3D11Device* device, u_int slices, u_int stacks, float r = 0.5f);
};

class geometric_capsule : public geometric_primitive
{
public:
	geometric_capsule(ID3D11Device* device);
};

class geometric_board : public geometric_primitive
{
public:
	geometric_board(ID3D11Device* device);
	
};


class GeometricRect : public geometric_primitive
{
public:
	GeometricRect(ID3D11Device* device);
	~GeometricRect() {};
};

