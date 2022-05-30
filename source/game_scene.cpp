#include "game_scene.h"

#include "imgui.h"
#include "camera.h"
#include "geometric_primitive.h"
#include "collision.h"
#include "sprite_data.h"
#include "bloom.h"
#include "shadertoy_test.h"
#include "myutil.h"
#include "judge_manager.h"




bool ishitrect = false;
bool post_effect = true;
bool tonemap = true;
bool toon = false;

bool freecam = false;

bool demo = false;

float gposx = 0, gposy = 0;
float gscalex = 0, gscaley = 0;
float gx = 1600;
extern float _Outline;

DirectX::XMFLOAT4 brightColor = { 1, 1, 1, 1 };
DirectX::XMFLOAT4 darkColor = { 0, 0, 0, 1 };


float toonthreshod = 0.25, edgethershod = 0.25, shadowbias = 0.01f;

 bool game_scene::initialize(ID3D11Device* device, unsigned int screen_width, unsigned int screen_height, const char* nextscene)
 {


	 character1 = std::make_unique<Character>(pad1p);
	 character2 = std::make_unique<Character>(pad2p);
	 shadertoy = std::make_unique<Sadertoy_test>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT);
	 burubon = std::make_unique<Test>();
	 framebuffers[3] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
	 Rasterizer = std::make_unique<rasterizer>(device);

	 auto function=[&](ID3D11Device* device)
	 {

		 texture::load(loadTexture);

		 camera = std::make_shared<Camera>();

		 mapcamera = std::make_shared<MapCamera>();

		 projection = camera->GetProjectionMatrix();
		 mini_map_projection = mapcamera->GetProjectionMatrix();

		 light_direction = DirectX::XMFLOAT4(0, -1, -0.60f, 0);

		 MAP_light_direction = DirectX::XMFLOAT4(0, -1, 0.0f, 0);

		 eyes = DirectX::XMFLOAT3(0.0f, 0.0f, -730.0f);
		 forcuss = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		 ups = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

		 //gamePad = std::make_unique<GamePad>(0);

		 framebuffers[0] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		 framebuffers[1] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		 framebuffers[2] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);

		 Fullscreen_quad = std::make_unique<fullscreen_quad>(device);

		 bloom_effect = std::make_unique<Bloom>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT);
		 pixel_motionbulur = std::make_unique<PixelMotionBulur>(device);
		 tonemap_effect = std::make_unique<ToneMap>(device);


		 player1p = std::make_unique<player1>();
		 player2p = std::make_unique<player2>();


		 if (ToonShader == nullptr)
		 {
			 ToonShader = std::make_unique<Shader>(ShaderType::TOON);
			 ToonShader->Create("./Shader/toon_liting_vs.cso", "./Shader/toon_liting_ps.cso");
			 //ToonShader->Create("./Shader/model_toon_vs.cso", "./Shader/model_toon_ps.cso", "./Shader/model_toon_gs.cso");
	     }



#ifdef BURUBON
		 //player1p->Add(device, burubon.get(), VECTOR3(-10, 0, 0), "./Data/FBX/mihono_kesuke/mihono_burumon2.fbx", "Testburubon",true);

		 player1p->Add(device, burubon.get(), VECTOR3(-10, 0, 0), "./Data/FBX/Anber/test_manequin.fbx", "Testburubon", true,false);
		 // !BURUBON
#else
//Rnr_fighter
		 player1p->Add(device, character1.get(), VECTOR3(-init_pos, -1.0f, 0), "./Data/FBX/Player/BoxUnityChan.fbx", "PLAYER1", false,false);
		 player2p->Add(device, character2.get(), VECTOR3(init_pos, -1.0f, 0), "./Data/FBX/Player/BoxUnityChan.fbx", "PLAYER2", false,false);

		 //player1p->Add(device, burubon.get(), VECTOR3(-10, 0, 0), "./Data/FBX/Player/mare_1.fbx", "Testburubon");

		 Light::SetPointLight(0, DirectX::XMFLOAT3{0,0,0}, DirectX::XMFLOAT3{ 1,0,0 },10.0f);
		 Light::SetPointLight(1, DirectX::XMFLOAT3{ 0,0,0 }, DirectX::XMFLOAT3{ 1,1,1 }, 10.0f);

#endif

		 stage = std::make_unique<Stage>("./Data/FBX/Stage/gridstage.fbx");
		 wall = std::make_unique<Wall>("./Data/FBX/Stage/wall.fbx");

	  };
	  future = std::async(std::launch::async, function, device);
		 //// フォントの初期化
		 if (!font)
		 {
			 font = std::make_unique<Font>(device, "./Data/Font/MS Gothic.fnt", 1024);
		 }


		 framework::Instance().pl_sound[0] = "walk";
		 framework::Instance().heart_beat = "heart";


		 judge_manager::Instance().init();
		 
		 UI_Manager::Instance().init();

		 pause = false;
		 //efcinit
		 effect.EffectInit(device);
		

		 Audio::GetMusic("bgm")->Play(true);

		 //Audio::GetMusic("bgm")->SetVolume(0.1f);



		 return true;
	 }
 

float posx4 = 1679;

const char* game_scene::update(ID3D11Device* device, float& elapsed_time)
{

	view_MATRIX = camera->GetViewMatrix();
	mini_map_view_MATRIX = mapcamera->GetViewMatrix();


	DirectX::XMStoreFloat4x4(&view, view_MATRIX);
	DirectX::XMStoreFloat4x4(&projection4X4, projection);

	DirectX::XMStoreFloat4x4(&free_view, mini_map_view_MATRIX);
	DirectX::XMStoreFloat4x4(&free_projection4X4, mini_map_projection);

	std::future_status status = future.wait_for(0ms);
	if (status == std::future_status::timeout)
	{

		return 0;
	}


	effect.EffectUpdate(elapsed_time);


	
#ifdef USE_IMGUI


#endif // USE_IMGUI






		player1p->update(view, projection4X4);
#ifndef BURUBON
		player2p->update(view, projection4X4);
	

	
	camera->Update(player1p->getList().begin()->get()->position, player2p->getList().begin()->get()->position, player1p->getList().begin()->get()->ishit, player2p->getList().begin()->get()->ishit);

	mapcamera->Update();


    judge_manager::Instance().update(player1p.get(), player2p.get());

	
	
	UI_Manager::Instance().update(player1p.get(), player2p.get(),camera.get());

	
	if (UI_Manager::Instance().Get_1pwinflag()|| UI_Manager::Instance().Get_2pwinflag())
	{
		timer++;

		if (timer >= 60 * 3)
		{

			if (pad1p->GetButtonDown() & pad1p->BTN_A || pad2p->GetButtonDown() & pad2p->BTN_A)
			{

				return "TITLE";
			}
		}

	}

#endif


	//tonemap_effect->constant_buffer->data.saturation = -0.3f;
#ifdef USE_IMGUI
	//sample code

	

	static ImGuiID dockspaceID = 0;
	bool active = true;

	if (framework::Instance().useimgui == false) return 0;

	if (ImGui::Begin("Master Window", &active))
	{


		ImGui::Checkbox(u8"ブルーム処理",&post_effect);
		ImGui::Checkbox(u8"画像フィルタリング処理", &tonemap);

		ImGui::InputFloat(u8"輝度抽出閾値", &bloom_effect->constant_buffer->data.glow_extraction_threshold, 0.01f, 0.1f);
		ImGui::InputFloat(u8"ぼかしの畳み込み強度", &bloom_effect->constant_buffer->data.blur_convolution_intensity, 0.001f, 0.01f);
		ImGui::InputFloat(u8"白とび制御", &tonemap_effect->constant_buffer->data.white_point, 0.01f, 0.1f);
		ImGui::InputFloat(u8"グレースケール", &tonemap_effect->constant_buffer->data.average_gray, 0.01f, 0.1f);

		ImGui::InputFloat(u8"明度", &tonemap_effect->constant_buffer->data.brightness, 0.01f, 0.1f);
		ImGui::InputFloat(u8"コントラスト調整", &tonemap_effect->constant_buffer->data.contrast, 0.01f, 0.1f);
		ImGui::InputFloat(u8"色合い", &tonemap_effect->constant_buffer->data.hue, 0.01f, 0.1f);
		ImGui::InputFloat(u8"彩度", &tonemap_effect->constant_buffer->data.saturation, 0.01f, 0.1f);
		ImGui::SliderFloat(u8"セピア加工", &tonemap_effect->constant_buffer->data.amount, 0.0f, 1.0f, "sepia = %.3f");
		ImGui::SliderFloat(u8"ビネット加工", &tonemap_effect->constant_buffer->data.offset, 0.0f, 10.0f, "vignette = %.3f");
		ImGui::SliderFloat(u8"ビネット加工暗さ", &tonemap_effect->constant_buffer->data.darkness, 0.0f, 1.0f, "vignette = %.3f");
		ImGui::ColorEdit3(u8"カラーフィルター", reinterpret_cast<float*>(&tonemap_effect->constant_buffer->data.colour_filter));

		ImGui::ColorEdit4(u8"アンビエントブライトカラー", reinterpret_cast<float*>(&brightColor));
		ImGui::ColorEdit4(u8"アンビエントダークカラー", reinterpret_cast<float*>(&darkColor));

		ImGui::SliderFloat(u8"アウトラインの太さ", &_Outline, 0.0f, 1.0f);

		ImGui::SliderFloat(u8"エッジの閾値", &edgethershod, 0.0f, 1.0f, "edgethershod = %.3f");
		ImGui::SliderFloat(u8"トゥーンの閾値", &toonthreshod, 0.0f, 1.0f, "toonthreshod = %.3f");
		//ImGui::SliderFloat(u8"シャドウバイアス", &shadowbias, 0.0f, 1.0f, "shadowbias = %.3f");

		ImGui::Checkbox(u8"当たり判定の可視化", &ishitrect);

	/*	ImGui::DragFloat("gaugex", &gposx);
		ImGui::DragFloat("gaugey", &gposy);

		ImGui::DragFloat("gsx", &gx);
		ImGui::DragFloat("gsy", &gscaley);

		ImGui::DragFloat("anglex", &stage->angle.y);
		ImGui::ColorEdit4("GridColor", reinterpret_cast<float*>(&stage->color));




		ImGui::SliderFloat("fade", &UI_Manager::Instance().fade, 0.0f, 1.0f, "fade = %.3f");*/

		//ImGui::InputFloat("lens_flare_threshold", &bloom_effect->constant_buffer->data.lens_flare_threshold, 0.01f, 0.1f);
		//ImGui::InputFloat("lens_flare_ghost_dispersal", &bloom_effect->constant_buffer->data.lens_flare_ghost_dispersal, 0.001f, 0.01f);
		//ImGui::InputInt("number_of_ghosts", &bloom_effect->constant_buffer->data.number_of_ghosts);
		//ImGui::SliderFloat("lens_flare_intensity", &bloom_effect->constant_buffer->data.lens_flare_intensity, 0.0f, 1.0f);


	}
	if (active)
	{
		// Declare Central dockspace
		dockspaceID = ImGui::GetID("HUB_DockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::Image(framebuffers[0]->render_target_shader_resource_view.Get(), ImVec2(256, 256));

	}
	ImGui::End();

	ImGui::SetNextWindowDockID(dockspaceID, ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Dockable Window"))
	{
		ImGui::TextUnformatted("Test");

		ImGui::Checkbox("freecam",&freecam);

		ImGui::SliderFloat("lightx", &light_direction.x, -5.0f, 1.0f);
		ImGui::SliderFloat("lighty", &light_direction.y, -5.0f, 1.0f);
		ImGui::SliderFloat("lightz", &light_direction.z, -5.0f, 1.0f);
		ImGui::SliderFloat("lighta", &light_direction.w, -5.0f, 1.0f);


		ImGui::SliderFloat("Mlightx", &MAP_light_direction.x, -1.0f, 1.0f);
		ImGui::SliderFloat("Mlighty", &MAP_light_direction.y, -1.0f, 1.0f);
		ImGui::SliderFloat("Mlightz", &MAP_light_direction.z, -1.0f, 1.0f);
		ImGui::SliderFloat("Mlighta", &MAP_light_direction.w, -1.0f, 1.0f);

		ImGui::DragFloat("targetx", &camera->target.x);
		ImGui::DragFloat("targety", &camera->target.y);
		ImGui::DragFloat("targetz", &camera->target.z);

		ImGui::DragFloat("posx", &camera->pos.x);
		ImGui::DragFloat("posy", &camera->pos.y);
		ImGui::DragFloat("posz", &camera->pos.z);



	 ImGui::DragFloat4("poinghtlight0", &Light::PointLight[0].pos.x);
	 ImGui::DragFloat4("poinghtlight1", &Light::PointLight[1].pos.x);
		
	
	 ImGui::DragFloat4("ambientcolor", &Light::Ambient.x);
		ImGui::Image(framebuffers[2]->render_target_shader_resource_view.Get(), ImVec2(289, 256));

	}
	ImGui::End();
#endif

	return 0;
}

void game_scene::render(ID3D11DeviceContext* immediate_context, float elapsed_time, blender* Blender)
{



	std::future_status status = future.wait_for(0ms);
	if (status == std::future_status::timeout)
	{


		//font->Begin(immediate_context);

		//font->Draw(540, 300, L"LOAD", DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(2.0f, 2.0f));
		//font->End(immediate_context);

			//test to shadertoy
		framebuffers[3]->clear(immediate_context);

		framebuffers[3]->activate(immediate_context);
		shadertoy->generate(elapsed_time);
		shadertoy->blit(immediate_context, framebuffers[3]->render_target_shader_resource_view.Get(), elapsed_time);
		Rasterizer->blit(immediate_context, shadertoy->toy_buffer->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);
		framebuffers[3]->deactivate(immediate_context);

		Rasterizer->blit(immediate_context, framebuffers[3]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);

	/*	font->Begin(pSystem.devicecontext.Get());

		font->Draw(800, 450, L"LOADING", DirectX::XMFLOAT4(1.0f, 1.0f,1.0f,1.0f), DirectX::XMFLOAT2(4.0f, 4.0f));
		

		font->End(pSystem.devicecontext.Get());*/

		return;
	}

	else

	{

		// ビュー行列、プロジェクション行列を合成し行列データを取り出す。
		DirectX::XMFLOAT4X4 view_projection;
		{
			DirectX::XMMATRIX C = DirectX::XMMatrixSet(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, -1, 0,
				0, 0, 0, 1
			);

			//クラスのステートパターン

			DirectX::XMMATRIX VP;
			VP = C * view_MATRIX * projection;
			DirectX::XMStoreFloat4x4(&view_projection, VP);
		}


		// ビュー行列、プロジェクション行列を合成し行列データを取り出す。
		DirectX::XMFLOAT4X4 mini_map_view_projection;
		{
			DirectX::XMMATRIX C = DirectX::XMMatrixSet(
				1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, -1, 0,
				0, 0, 0, 1
			);

			DirectX::XMMATRIX VP;
			VP = C * mini_map_view_MATRIX * mini_map_projection;
			DirectX::XMStoreFloat4x4(&mini_map_view_projection, VP);
		}



		//tonemap仮生成

		{
			framebuffers[1]->clear(immediate_context);

			framebuffers[1]->activate(immediate_context);

			tonemap_effect->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), elapsed_time);
			framebuffers[1]->deactivate(immediate_context);

		}



		framebuffers[0]->clear(immediate_context);

		framebuffers[0]->activate(immediate_context);

		if (freecam)
		{
			player1p->draw(immediate_context, ToonShader.get(), mini_map_view_projection, free_projection4X4, light_direction, mapcamera, elapsed_time, brightColor, darkColor, edgethershod, toonthreshod, shadowbias);

#ifndef BURUBON
			player2p->draw(immediate_context, ToonShader.get(), mini_map_view_projection, free_projection4X4, light_direction, mapcamera, elapsed_time, brightColor, darkColor, edgethershod, toonthreshod, shadowbias);
#endif
			// Effekseer rendering
			stage->Render(immediate_context, ToonShader.get(), mini_map_view_projection, free_projection4X4, light_direction, mapcamera, elapsed_time, view, brightColor, darkColor);

		}
		else
		{
			player1p->draw(immediate_context, ToonShader.get(), view_projection, projection4X4, light_direction, camera, elapsed_time, brightColor, darkColor, edgethershod, toonthreshod, shadowbias);

#ifndef BURUBON
			player2p->draw(immediate_context, ToonShader.get(), view_projection, projection4X4, light_direction, camera, elapsed_time, brightColor, darkColor, edgethershod, toonthreshod, shadowbias);
#endif
			// Effekseer rendering
			stage->Render(immediate_context, ToonShader.get(), view_projection, projection4X4, light_direction, camera, elapsed_time, view, brightColor, darkColor);

			effect.EffectRender(view, projection);
		}
		

		//wall->Render(immediate_context, view_projection, projection4X4, light_direction, camera, elapsed_time);

		if (post_effect)
		{
			bloom_effect->generate(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), true);

			bloom_effect->blit(immediate_context, Blender);
		}


		//Rasterizer->blit(immediate_context, framebuffers[3]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);

#ifndef BURUBON
		judge_manager::Instance().drawhit(player1p.get(), player2p.get());

#endif	
		framebuffers[0]->deactivate(immediate_context);


		//framebuffers[3]->clear(immediate_context);

		//framebuffers[3]->activate(immediate_context);

	


		Rasterizer->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);


		//framebuffers[3]->deactivate(immediate_context);


		   //tonemap生成

		if (tonemap)
		{
			framebuffers[1]->clear(immediate_context);

			framebuffers[1]->activate(immediate_context);

			tonemap_effect->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), elapsed_time);

			framebuffers[1]->deactivate(immediate_context);

			Rasterizer->blit(immediate_context, framebuffers[1]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);

		}




	

		UI_Manager::Instance().render(font.get());
		
	}

}

void game_scene::uninitialize()
{
	// Effekseer破棄
	effect.EffectUninit();
	texture::releaseAll();
	player2p->uninit();
	player1p->uninit();
	future.wait();

	Audio::GetMusic("bgm")->Stop();


}
