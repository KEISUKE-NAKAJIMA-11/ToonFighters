#pragma once

#include "texture.h"
#include "util.h"
#include <windows.h>
#include <tchar.h>
#include <sstream>
#include <memory>
#include <map>
#include <d3d11.h>
#include <wrl.h>
#include "misc.h"
#include "high_resolution_timer.h"
#include "sprite.h"
#include "imgui.h"
#include "geometric_primitive.h"
#include "static_mesh.h"
#include "scene.h"

#include "game_scene.h"
#include "model.h"
#include "model_renderer.h"
#include "font.h"
#include "blender.h"
#include "assain.h"
#include "2d_primitive.h"
#include "view_seting_2d.h"
#include "debug_primitive.h"

#include "FontManager.h"
#include "AudioManager.h"
#include "renderpath.h"

class framework
{
private:
	static framework* instance;



public:
	static framework& Instance() { return *instance; }

	CONST HWND hwnd;
	static CONST LONG SCREEN_WIDTH = 1920;
	static CONST LONG SCREEN_HEIGHT = 1080;
	D3D_DRIVER_TYPE   g_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL  g_featureLevel = D3D_FEATURE_LEVEL_11_0;

	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> devicecontext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rendertargetview;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthstencilView;
	blender* Blender;

	std::map<std::string, std::unique_ptr<scene>> scenes;
	std::string current_scene;

	std::shared_ptr<GamePad> game_pad1p;
	std::shared_ptr<GamePad> game_pad2p;

	std::shared_ptr<FontManager> fontmanager;


	std::unique_ptr<geometric_primitive> cube;
	geometric_primitive* cylinder;
	geometric_primitive* sphere;

	static_mesh* mesh;

	TextureManager* textureManager;
	Primitive* primitive_2d;
	PrimitiveBatch* primitive_batch_2d;
	ViewSettings* viewset;
	RenderingPath m_renderingPath;

	//InputManager* inputManager;
	/*FontManager* fontManager;*/

	bool useimgui = false;
	bool dulation = false;


	D3D11_VIEWPORT viewport = {};

	const char* bgm[4];
	const char* pl_sound[5];
	const char* itemSE[2];
	const char* system[2];
	const char* heart_beat;

	float elapased_time;

	framework(HWND hwnd) : hwnd(hwnd)
	{
		instance = this;
	}


	~framework()
	{
		scenes[current_scene]->uninitialize();

		safe_delete(Blender);

		safe_delete(textureManager);

		safe_delete(primitive_2d);

		safe_delete(primitive_batch_2d);


		safe_delete(viewset);

		resourceManager::Release();

		//_CrtDumpMemoryLeaks();

	}

	void setdulation(float time)
	{
		timer.setdulation(time);
	}


	//描画パスの設定
	void RenderPathBegin(RenderingPath::PathType pathType) { m_renderingPath.BeginPath(devicecontext.Get(), pathType); }
	void RenderPathEnd() { m_renderingPath.EndPath(devicecontext.Get()); }

	//描画パスをシェーダー読み取りできるようにする
	ID3D11ShaderResourceView* GetRenderPathSRV(RenderingPath::PathType pathType) { return m_renderingPath.GetShaderResource(pathType); }
	ID3D11ShaderResourceView* GetDepthCopySRV() { return m_renderingPath.GetDepthCopy(); }

	//現在のシーン深度値をバッファにコピー
	void CopyDepthBuffer() { m_renderingPath.CopyDepthBuffer(devicecontext.Get()); }


	ID3D11Device* GetDevice() { return device.Get(); }
	ID3D11DeviceContext* GetDeviceContext()
	{
		auto a = devicecontext.Get();
		return devicecontext.Get();
	}

	int run()
	{
		MSG msg = {};

		if (!initialize()) return 0;
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				timer.tick();
				elapased_time = timer.time_interval();
				dulation = timer.dulation;

				calculate_frame_stats();
				update(timer.time_interval());
				render(timer.time_interval());
			}
		}

#ifdef USE_IMGUI
		// cleanup imgui
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
#endif // USE_IMGUI




		return static_cast<int>(msg.wParam);
			}

	LRESULT CALLBACK handle_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
#ifdef USE_IMGUI
		//imgui event catch
		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) { return true; }
#endif

		switch (msg)
		{
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc;
			hdc = BeginPaint(hwnd, &ps);
			EndPaint(hwnd, &ps);
			break;
		}
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_CREATE:
			break;
		case WM_KEYDOWN:
			if (wparam == VK_ESCAPE) PostMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		case WM_ENTERSIZEMOVE:
			// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
			timer.stop();
			break;
		case WM_EXITSIZEMOVE:
			// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
			// Here we reset everything based on the new window dimensions.
			timer.start();
			break;
		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
		}
		return 0;
		}

	high_resolution_timer gettimer() { return timer; }


private:
	bool initialize();
	void update(float elapsed_time/*Elapsed seconds from last frame*/);
	void render(float elapsed_time/*Elapsed seconds from last frame*/);

private:
	high_resolution_timer timer;
	void calculate_frame_stats()
	{
		// Code computes the average frames per second, and also the 
		// average time it takes to render one frame.  These stats 
		// are appended to the window caption bar.
		static int frames = 0;
		static float time_tlapsed = 0.0f;

		frames++;

		// Compute averages over one second period.
		if ((timer.time_stamp() - time_tlapsed) >= 1.0f)
		{
			float fps = static_cast<float>(frames); // fps = frameCnt / 1
			float mspf = 1000.0f / fps;
			std::ostringstream outs;
			outs.precision(6);
			outs << "FPS : " << fps << " / " << "Frame Time : " << mspf << " (ms)";
			SetWindowTextA(hwnd, outs.str().c_str());

			// Reset for next average.
			frames = 0;
			time_tlapsed += 1.0f;
		}
	}
	};
#define pSystem framework::Instance()



