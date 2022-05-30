#include "framework.h"
#include <memory>
#include <algorithm>
#include <deque>
#include "Shadermanager.h"
#include "Keyboard.h"

#include "blender.h"
#include "game_scene.h"
//#include "GamePad.h"
#include <Windows.h>
#include "sheder_scene.h"

#include "title.h"
#include "imgui.h"
#include <DirectXMath.h>
#include "scene_modeledit.h"

framework* framework::instance = nullptr;






bool framework::initialize()
{
	HRESULT hr = S_OK;

	Audio::Initialize();

	framework::Instance().system[0] = "select";
	framework::Instance().system[1] = "decide";
	framework::Instance().system[2] = "ko";
	framework::Instance().bgm[0] = "bgm";

	framework::Instance().pl_sound[0] = "hit";
	framework::Instance().pl_sound[1] = "guard";
	framework::Instance().pl_sound[2] = "jump";




	Audio::LoadSound(framework::Instance().system[0], "./Data/Sound/SE/select.wav");
	Audio::LoadSound(framework::Instance().system[1], "./Data/Sound/SE/decide.wav");
	Audio::LoadSound(framework::Instance().system[2], "./Data/Sound/SE/ko2.wav");
	Audio::LoadSound(framework::Instance().pl_sound[0], "./Data/Sound/SE/hit.wav");
	Audio::LoadSound(framework::Instance().pl_sound[1], "./Data/Sound/SE/guard.wav");
	Audio::LoadSound(framework::Instance().pl_sound[2], "./Data/Sound/SE/jump.wav");
	Audio::LoadMusic(framework::Instance().bgm[0], "./Data/Sound/BGM/maousentou.wav");


	RECT rc;
	GetClientRect(hwnd, &rc);



	UINT screen_width = rc.right - rc.left;
	UINT screen_height = rc.bottom - rc.top;
	// Create Device


	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd = {};

	sd.BufferCount = 1;
	sd.BufferDesc.Width = screen_width;
	sd.BufferDesc.Height = screen_height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hwnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = true;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		g_driverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDeviceAndSwapChain(NULL, g_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, swapchain.GetAddressOf(), device.GetAddressOf(), &g_featureLevel, devicecontext.GetAddressOf());

		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return false;
	// Create Render Target View
	D3D11_TEXTURE2D_DESC back_buffer_desc;
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
		hr = swapchain->GetBuffer(0,
			__uuidof(ID3D11Texture2D),
			reinterpret_cast<void**>(back_buffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		hr = device->CreateRenderTargetView(back_buffer.Get(), NULL, rendertargetview.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		back_buffer->GetDesc(&back_buffer_desc);
	}


	// Create Depth Stencil View
	D3D11_TEXTURE2D_DESC depth_stencil_buffer_desc = back_buffer_desc;
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depth_stencil_buffer;
		depth_stencil_buffer_desc.MipLevels = 1;
		depth_stencil_buffer_desc.ArraySize = 1;
		depth_stencil_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		depth_stencil_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depth_stencil_buffer_desc.CPUAccessFlags = 0;
		depth_stencil_buffer_desc.MiscFlags = 0;
		hr = device->CreateTexture2D(&depth_stencil_buffer_desc, NULL, depth_stencil_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc;
		depth_stencil_view_desc.Format = depth_stencil_buffer_desc.Format;
		depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Flags = 0;
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(depth_stencil_buffer.Get(), &depth_stencil_view_desc, depthstencilView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	/*devicecontext->OMSetRenderTargets(1, rendertargetview.GetAddressOf(), depthstencilView.GetAddressOf());*/

	// Setup the viewport

	viewport.Width = (FLOAT)screen_width;
	viewport.Height = (FLOAT)screen_height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	devicecontext->RSSetViewports(1, &viewport);

	UINT m4xMsaaQuality;
	device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality);


#ifdef USE_IMGUI
// setup imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//日本語用フォントの設定
	float size_pixels = 30.0f; // フォントサイズ
	const char* filename = "./Data/Font/mplus-1p-medium.ttf"; // 日本語のフォント
	io.Fonts->AddFontFromFileTTF(filename, size_pixels, nullptr, glyphRangesJapanese);
	// setup platform/renderer
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device.Get(), devicecontext.Get());
	ImGui::StyleColorsDark();

#endif // USE_IMGUI

	textureManager = new TextureManager;
	primitive_2d = new Primitive(device.Get());
	primitive_batch_2d = new PrimitiveBatch(device.Get());
	viewset = new ViewSettings;

	current_scene = "TITLE";

	scenes.insert(std::make_pair("TITLE", std::make_unique<title_scene>()));
	scenes.insert(std::make_pair("MODELEDIT", std::make_unique<Scene_model_editor>()));
	scenes.insert(std::make_pair("GAME", std::make_unique<game_scene>()));
	scenes.insert(std::make_pair("SHEDER", std::make_unique<sheder_scene>()));
	scenes.at(current_scene)->initialize(device.Get(), screen_width, screen_height, 0);

	//particle = std::make_unique<sprite>(device.Get(), L"particle-smoke.png");
	//font = std::make_unique<sprite>(device.Get(), L"./fonts/font0.png");


	game_pad1p = std::make_shared<GamePad>(0);
	game_pad2p = std::make_shared<GamePad>(1);
	Blender = new blender(device.Get());







	//particle_batch = std::make_unique<sprite_batch>(device.Get(), L"particle-smoke.png", 1024);

	return true;
}
void framework::update(float elapsed_time/*Elapsed seconds from last frame*/)
{
#ifdef USE_IMGUI
	// imgui new frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif // USE_IMGUI
	static const char* next_scene = 0;

	if (GetAsyncKeyState(VK_F5) & 0x01)
	{
		useimgui ^= true;
	}

	Audio::Update();
	game_pad1p->Update();
	game_pad2p->Update();

	if (next_scene)
	{
		scenes.at(current_scene)->uninitialize();
		scenes.at(next_scene)->initialize(device.Get(), static_cast<size_t>(viewport.Width), static_cast<size_t>(viewport.Height), next_scene);
		current_scene = next_scene;
		timer.reset();
	}
	next_scene = scenes.at(current_scene)->update(device.Get(), elapsed_time);


}

void framework::render(float elapsed_time/*Elapsed seconds from last frame*/)
{

	HRESULT hr = S_OK;
	//static float angle = 0;
	//angle += 6.0f * elapsed_time;

	static FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0～1.0)

	setBlendMode(devicecontext.Get(), Blender->BS_ALPHA, Blender);

	devicecontext->ClearRenderTargetView(rendertargetview.Get(), color);
	devicecontext->ClearDepthStencilView(depthstencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	devicecontext->OMSetRenderTargets(1, rendertargetview.GetAddressOf(), depthstencilView.Get());

	scenes.at(current_scene)->render(devicecontext.Get(), elapsed_time, Blender);

#ifdef USE_IMGUI
	//imgui render


		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
#endif

	int sync_interval = 1;
#ifdef _DEBUG
	//sync_interval = 0;
#endif // _DEBUG
	swapchain->Present(1, 0);
}




