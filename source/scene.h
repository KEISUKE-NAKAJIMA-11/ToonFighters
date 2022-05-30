#pragma once
#include <d3d11.h>
#include "blender.h"
#include "assain.h"

#include "camera.h"

class scene
{
public:
	virtual bool initialize(ID3D11Device* device, unsigned int screen_width, unsigned int screen_height, const char* nextscene) = 0;
	virtual const char* update(ID3D11Device* device,float& elapsed_time/*Elapsed seconds from last frame*/) = 0;
	virtual void render(ID3D11DeviceContext* immediate_context, float elapsed_time/*Elapsed seconds from last frame*/,blender* Blender) = 0;
	virtual void uninitialize() = 0;

	virtual LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) { return 0; }

	
	scene() = default;
	virtual ~scene() = default;

	bool initialized = false;
	scene(scene&) = delete;
	void operator=(scene&) = delete;

	virtual scene* getscene() = 0;

};