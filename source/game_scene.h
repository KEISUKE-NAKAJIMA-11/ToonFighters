#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <future>
#include <chrono>
#include <iostream>
#include <fstream>
#include <shlwapi.h>
#include <random>

#include "scene.h"
#include "tonemap.h"
#include "MotionBlur.h"

#include "font.h"
#include "blender.h"
#include "framework.h"
#include <Windows.h>

#include "util.h"
#include "shadertoy_test.h"
#include "Stage.h"
#include "Effect.h"

#include "character.h"
#include "ui_hpgauge.h"
#include "reworkshder.h"


using namespace std::chrono_literals;





class game_scene : public scene
{

private:
	//static_mesh* mesh;

	//static_mesh* mesh;
	int cursor = 0;
	enum MENU
	{
		Back,
		Title,
	};

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
	DirectX::XMFLOAT4X4 free_view;
	DirectX::XMFLOAT4X4 free_projection4X4;
	std::unique_ptr<geometric_primitive>  ground;

	std::unique_ptr<Font>				font;

	std::unique_ptr<ToneMap> tonemap_effect;
	std::unique_ptr<framebuffer>	framebuffers[5];
	std::unique_ptr<Bloom>			bloom_effect;
	std::unique_ptr<PixelMotionBulur>			pixel_motionbulur;

	std::unique_ptr<rasterizer>			Rasterizer;
	std::unique_ptr<fullscreen_quad>	Fullscreen_quad;
	std::unique_ptr<Sadertoy_test> shadertoy;

	std::shared_ptr <Camera>			camera;
	std::shared_ptr <MapCamera>			mapcamera;
	std::unique_ptr<player1> player1p;
	std::unique_ptr<player2> player2p;

	std::unique_ptr <Stage> stage;
	std::unique_ptr <Wall> wall;



	std::shared_ptr<Character> character1;
	std::shared_ptr<Character> character2;
	std::shared_ptr<Test>   burubon;

	std::unique_ptr<Shader> ToonShader = nullptr;


	//std::unique_ptr<judge_manager> judge;

	int round = 1;

	int timer = 0;

	bool pause = false;

public:
	// std::unique_ptr<GamePad>			gamePad;

	bool initialize(ID3D11Device* device, unsigned int screen_width, unsigned int screen_height, const char* nextscene);
	
	const char* update(ID3D11Device* device,float& elapsed_time/*Elapsed seconds from last frame*/);
	std::future<void> future;

	void render(ID3D11DeviceContext* immediate_context, float elapsed_time, blender* Blender);
	
	void uninitialize();

	game_scene* getscene() { return this; }

	player1* getplayer1() { return player1p.get(); }
	player2* getplayer2() { return player2p.get(); }
};


