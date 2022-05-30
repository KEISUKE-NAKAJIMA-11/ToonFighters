#pragma once
#include "font.h"
#include "assain.h"
#include "scene.h"
#include "framework.h"
#include "actor.h"



class sheder_scene : public scene
{
private:


	struct constants
	{
		template<class T>
		void serialize(T& archive)
		{
			//TODO:Add 'number_of_ghosts', "lens_flare_intensity"
			//archive(CEREAL_NVP(glow_extraction_threshold), CEREAL_NVP(blur_convolution_intensity), CEREAL_NVP(lens_flare_threshold), CEREAL_NVP(lens_flare_ghost_dispersal));
		}
	};



	std::unique_ptr<Font>				font;
	//std::unique_ptr<GamePad>			gamePad;

	std::unique_ptr<Shader> ToonShader = nullptr;


	const char* test;

	DirectX::XMFLOAT3 eyes, forcuss, ups;
	DirectX::XMMATRIX view_MATRIX;
	DirectX::XMMATRIX projection;

	DirectX::XMMATRIX mini_map_view_MATRIX;
	DirectX::XMMATRIX mini_map_projection;

	DirectX::XMFLOAT4 light_direction;
	DirectX::XMFLOAT4 MAP_light_direction;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection4X4;
	std::unique_ptr<geometric_primitive>  ground;

	

	std::unique_ptr<ToneMap> tonemap_effect;
	std::unique_ptr<framebuffer>	framebuffers[5];
	std::unique_ptr<Bloom>			bloom_effect;
	

	std::unique_ptr<ModelRenderer>			modelrender;

	std::unique_ptr<rasterizer>			Rasterizer;
	std::unique_ptr<fullscreen_quad>	Fullscreen_quad;
	std::unique_ptr<Sadertoy_test> shadertoy;


	std::shared_ptr <MapCamera>			camera;

	std::unique_ptr <Stage> stage;
	std::unique_ptr<Constant_buffer<constants>> constant_buffer;




public:
	bool initialize(ID3D11Device* device, unsigned int screen_width, unsigned int screen_height, const char* nextscene);

	const char* update(ID3D11Device* device, float& elapsed_time/*Elapsed seconds from last frame*/);


	std::future<void> future;

	void render(ID3D11DeviceContext* immediate_context, float elapsed_time, blender* Blender);
	void uninitialize()
	{
		
		texture::releaseAll();

		//gamePad.release();
	}

	sheder_scene* getscene() { return this; }
};