#pragma once
#include "font.h"
#include "assain.h"
#include "scene.h"
#include "scene.h"
#include "tonemap.h"
#include "blender.h"
#include "framework.h"
#include <Windows.h>
#include "Effect.h"
#include <future>
#include "game_scene.h"
#include "shadertoy_test.h"

class title_scene : public scene
{
private:
	int timer;
	DirectX::XMMATRIX view_MATRIX;
	DirectX::XMMATRIX projection;

	DirectX::XMFLOAT4 light_direction;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection4X4;

	std::unique_ptr<ToneMap> tonemap_effect;
	std::unique_ptr<framebuffer>	framebuffers[4];
	std::unique_ptr<Bloom>			bloom_effect;

	std::unique_ptr<rasterizer>			Rasterizer;
	std::unique_ptr<fullscreen_quad>	Fullscreen_quad;
	std::unique_ptr<Sadertoy_test> shadertoy;
	std::unique_ptr<Font>				font;
	std::shared_ptr <Camera>			camera_title;
	//std::unique_ptr<GamePad>			gamePad;


public:
	bool post_effect_title = true;

public:

	bool initialize(ID3D11Device* device, unsigned int screen_width, unsigned int screen_height, const char* nextscene);

	const char* update(ID3D11Device* device, float& elapsed_time/*Elapsed seconds from last frame*/);
	std::future<void> future;


	void render(ID3D11DeviceContext* immediate_context, float elapsed_time, blender* Blender);
	void uninitialize();

	title_scene* getscene() { return this; }
};