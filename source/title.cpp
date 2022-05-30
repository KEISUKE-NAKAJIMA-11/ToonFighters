#include "title.h"
#include "texture.h"
#include "sprite_data.h"
#include "framework.h"
#include "camera.h"



bool tonemap_title = false;


bool title_scene::initialize(ID3D11Device* device, unsigned int screen_width, unsigned int screen_height, const char* nextscene)
{
	texture::load(loadTexture);

	camera_title = std::make_shared<Camera>();

//gamePad = std::make_unique<GamePad>(0);
	future = std::async(std::launch::async, [&](ID3D11Device* device) {

		projection = camera_title->GetProjectionMatrix();
		light_direction = DirectX::XMFLOAT4(0.f, -1.f, -1.0f, 0.5f);
		framebuffers[0] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		framebuffers[1] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		framebuffers[2] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		framebuffers[3] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		Rasterizer = std::make_unique<rasterizer>(device);
		Fullscreen_quad = std::make_unique<fullscreen_quad>(device);

		bloom_effect = std::make_unique<Bloom>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT);

		tonemap_effect = std::make_unique<ToneMap>(device);
		shadertoy = std::make_unique<Sadertoy_test>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT);

		timer = 0;

	}, device);
	camera_title->pos = DirectX::XMFLOAT3(.0f, 400.0f, -500.f);
	camera_title->target = DirectX::XMFLOAT3(.0f, 100.0f, 0.f);
	// ÉtÉHÉìÉgÇÃèâä˙âª
	if (!font)
	{
		font = std::make_unique<Font>(device, "./Data/Font/MS Gothic.fnt", 1024);
	}

	
	
	Audio::GetMusic("bgm")->Play(true);


	//Audio::GetMusic("bgm")->SetVolume(0.1f);

	return true;
}

const char* title_scene::update(ID3D11Device* device, float& elapsed_time)
{
	timer++;
	std::future_status status = future.wait_for(0ms);
	if (status == std::future_status::timeout)
	{

		return 0;
	}


	if (pad1p->GetButtonDown() & pad1p->BTN_A || pad2p->GetButtonDown() & pad2p->BTN_A)
	{
		return "GAME";
	}


	if (GetAsyncKeyState(VK_F5) & 0x8000)
	{
		return "MODELEDIT";
	}


	if (GetAsyncKeyState(VK_F6) & 0x8000)
	{
		return "SHEDER";
	}

	tonemap_effect->constant_buffer->data.white_point = 5.0f;
	tonemap_effect->constant_buffer->data.average_gray = 0.340f;
#ifdef USE_IMGUI
	if (framework::Instance().useimgui == false) return 0;
	ImGui::Begin("camera_title");
	ImGui::Checkbox("bloom", &post_effect_title);


	ImGui::SliderFloat("pos_x", &camera_title->pos.x, -5000.0f, 5000.0f);
	ImGui::SliderFloat("pos_y", &camera_title->pos.y, -5000.0f, 5000.0f);
	ImGui::SliderFloat("pos_z", &camera_title->pos.z, -5000.0f, 5000.0f);
	ImGui::SliderFloat("taget_x", &camera_title->target.x, -5000.0f, 5000.0f);
	ImGui::SliderFloat("taget_y", &camera_title->target.y, -5000.0f, 5000.0f);
	ImGui::SliderFloat("taget_z", &camera_title->target.z, -5000.0f, 5000.0f);


	ImGui::SliderFloat("light_x", &light_direction.x, -10.0f, 10.0f);
	ImGui::SliderFloat("light_y", &light_direction.y, -10.0f, 10.0f);
	ImGui::SliderFloat("light_z", &light_direction.z, -10.0f, 10.0f);
	ImGui::InputFloat("glow_extraction_threshold", &bloom_effect->constant_buffer->data.glow_extraction_threshold, 0.01f, 0.1f);
	ImGui::InputFloat("blur_convolution_intensity", &bloom_effect->constant_buffer->data.blur_convolution_intensity, 0.001f, 0.01f);
	ImGui::InputFloat("white_point", &tonemap_effect->constant_buffer->data.white_point, 0.01f, 0.1f);
	ImGui::InputFloat("average_gray", &tonemap_effect->constant_buffer->data.average_gray, 0.01f, 0.1f);

	ImGui::InputFloat("brightness", &tonemap_effect->constant_buffer->data.brightness, 0.01f, 0.1f);
	ImGui::InputFloat("contrast", &tonemap_effect->constant_buffer->data.contrast, 0.01f, 0.1f);
	ImGui::InputFloat("hue", &tonemap_effect->constant_buffer->data.hue, 0.01f, 0.1f);
	ImGui::InputFloat("saturation", &tonemap_effect->constant_buffer->data.saturation, 0.01f, 0.1f);
	ImGui::SliderFloat("amount", &tonemap_effect->constant_buffer->data.amount, 0.0f, 1.0f, "sepia = %.3f");
	ImGui::SliderFloat("offset", &tonemap_effect->constant_buffer->data.offset, 0.0f, 10.0f, "vignette = %.3f");
	ImGui::SliderFloat("darkness", &tonemap_effect->constant_buffer->data.darkness, 0.0f, 1.0f, "vignette = %.3f");
	ImGui::ColorEdit3("colour_filter", reinterpret_cast<float*>(&tonemap_effect->constant_buffer->data.colour_filter));

	ImGui::End();



	

#endif

	//camera_title.pos=  DirectX::XMFLOAT3(.0f, 200.0f, -500.f);


	return 0;
}

float posx = 0;
float posy = 0;
float scale = 1;
void title_scene::render(ID3D11DeviceContext* immediate_context, float elapsed_time, blender* Blender)
{
	
	tonemap_title = true;

	view_MATRIX = camera_title->GetViewMatrix();

	std::future_status status = future.wait_for(0ms);
	if (status == std::future_status::timeout)
	{

		framebuffers[3]->clear(immediate_context);

		framebuffers[3]->activate(immediate_context);
		shadertoy->generate(elapsed_time);
		shadertoy->blit(immediate_context, framebuffers[3]->render_target_shader_resource_view.Get(), elapsed_time);
		Rasterizer->blit(immediate_context, shadertoy->toy_buffer->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);
		framebuffers[3]->deactivate(immediate_context);

		Rasterizer->blit(immediate_context, framebuffers[3]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);


	

		return ;
	}


	DirectX::XMFLOAT4X4 view_projection;
	{
		DirectX::XMMATRIX C = DirectX::XMMatrixSet(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, -1, 0,
			0, 0, 0, 1
		);

		DirectX::XMMATRIX VP;
		VP = C * view_MATRIX * projection;
		DirectX::XMStoreFloat4x4(&view_projection, VP);
	}
	DirectX::XMStoreFloat4x4(&view, view_MATRIX);
	DirectX::XMStoreFloat4x4(&projection4X4, projection);


	//tonemapâºê∂ê¨
	{
		framebuffers[1]->clear(immediate_context);

		framebuffers[1]->activate(immediate_context);
		tonemap_effect->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), elapsed_time);
		framebuffers[1]->deactivate(immediate_context);

	}

	framebuffers[3]->clear(immediate_context);

	framebuffers[3]->activate(immediate_context);
	shadertoy->generate(elapsed_time);
	shadertoy->blit(immediate_context, framebuffers[3]->render_target_shader_resource_view.Get(), elapsed_time);
	Rasterizer->blit(immediate_context, shadertoy->toy_buffer->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);
	framebuffers[3]->deactivate(immediate_context);

	




	framebuffers[0]->clear(immediate_context);

	framebuffers[0]->activate(immediate_context);


	if (post_effect_title)
	{
		Rasterizer->blit(immediate_context, framebuffers[3]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);


		font->Begin(immediate_context);

		font->Draw(framework::Instance().SCREEN_WIDTH / 2.5, framework::Instance().SCREEN_HEIGHT / 3, L"Toon Fighters", DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(5.0f,5.0f));

		font->End(immediate_context);


		bloom_effect->generate(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), true);

		bloom_effect->blit(immediate_context, Blender);
	}


	framebuffers[0]->deactivate(immediate_context);

	Rasterizer->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);
	//tonemapê∂ê¨



	if (tonemap_title)
	{
		framebuffers[1]->clear(immediate_context);

		framebuffers[1]->activate(immediate_context);

		tonemap_effect->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), elapsed_time);

		framebuffers[1]->deactivate(immediate_context);

		Rasterizer->blit(immediate_context, framebuffers[1]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);

	}
	//framebuffers[2]->clear(immediate_context);

	//framebuffers[2]->activate(immediate_context);

	//framebuffers[2]->deactivate(immediate_context);


	//Rasterizer->blit(immediate_context, framebuffers[2]->render_target_shader_resource_view.Get(), 1272, 0, 256, 256);

}

void title_scene::uninitialize()
{

	
	timer = 0;
	Audio::GetMusic("bgm")->Stop();

	texture::releaseAll();
}
