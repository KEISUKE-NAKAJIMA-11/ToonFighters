#include "scene_modeledit.h"
#include "texture.h"
#include "sprite_data.h"
#include "camera.h"
#include "diarog.h"
#include <ImGuizmo.h>
#include "imgui.h"


bool post_effect_t = true;
bool tonemap_t = true;
float _Outline = 0.241;



bool Scene_model_editor::initialize(ID3D11Device* device, unsigned int screen_width, unsigned int screen_height, const char* nextscene)
{
	

	//future = std::async(std::launch::async, [&](ID3D11Device* device) {



	

		//eyes_t = DirectX::XMFLOAT3(0.0f, 0.0f, -500.0f);
		//forcuss_t = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		//ups_t = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

		framebuffers[0] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		framebuffers[1] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		framebuffers[2] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		framebuffers[3] = std::make_unique<framebuffer>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT, false/*enable_msaa*/, 8, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
		Rasterizer = std::make_unique<rasterizer>(device);
		Fullscreen_quad = std::make_unique<fullscreen_quad>(device);

		bloom_effect = std::make_unique<Bloom>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT);

		tonemap_effect = std::make_unique<ToneMap>(device);
		shadertoy = std::make_unique<Sadertoy_test>(device, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT);

		modelrenderer= std::make_unique<ModelRenderer>(device);

		linerenderer = std::make_unique<LineRenderer>(device,1024);

		camera_t = std::make_shared<Camera>();

		eyes = DirectX::XMFLOAT3(0.0f, 0.0f, -500.0f);
		forcuss = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		ups = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);


		//import_model("./Data/FBX/Player/Sira.fbx");

		//}, device);


	effect.EffectInit(device);

	// フォントの初期化
	if (!font)
	{
		font = std::make_unique<Font>(device, "./Data/Font/MS Gothic.fnt", 1024);
	}

	return true;
}

const char* Scene_model_editor::update(ID3D11Device* device, float& elapsed_time)
{
	

	timer++;
	
	

	effect.EffectUpdate(elapsed_time);



	

	// カメラ更新
	camera.Update();
	
	// モデル更新
	if (model != nullptr)
	{
		model->UpdateAnimation(elapsed_time);

		
			model->CalculateLocalTransform();
			model->CalculateWorldTransform(DirectX::XMMatrixIdentity());
		

	}

	
	static ImGuiID dockspaceID = 0;

	draw_gui();

	ImGui::Begin("Light");
	ImGui::DragFloat3("light",&lightDirection.x,0.1f,-1.0f,1.0f);

	ImGui::Checkbox("post_effect", &post_effect_t);
	ImGui::Checkbox("tone_map", &tonemap_t);

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

	return 0;
}

void Scene_model_editor::render(ID3D11DeviceContext* immediate_context, float elapsed_time, blender* Blender)
{



	

	float fovY = DirectX::XMConvertToRadians(30.0f);
	float aspect = framework::Instance().SCREEN_WIDTH / framework::Instance().SCREEN_HEIGHT;
	float nearZ = 1.0f;
	float farZ = 10000.0f;
	DirectX::XMVECTOR Eye = DirectX::XMLoadFloat3(&camera.GetEye());
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&camera.GetFocus());
	DirectX::XMVECTOR Up = DirectX::XMLoadFloat3(&camera.GetUp());
	DirectX::XMMATRIX View = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	DirectX::XMFLOAT4X4 view, projection;
	DirectX::XMStoreFloat4x4(&view, View);
	DirectX::XMStoreFloat4x4(&projection, Projection);


	//tonemap仮生成
	{
		framebuffers[1]->clear(immediate_context);

		framebuffers[1]->activate(immediate_context);
		tonemap_effect->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), elapsed_time);
		framebuffers[1]->deactivate(immediate_context);

	}



	framebuffers[0]->clear(immediate_context);

	framebuffers[0]->activate(immediate_context);







	if (post_effect_t)
	{


		bloom_effect->generate(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), true);

		bloom_effect->blit(immediate_context, Blender);
	}



	// モデル描画
	if (model != nullptr)
	{
		// ライトの方向

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
			VP = C * View * Projection;
			DirectX::XMStoreFloat4x4(&view_projection, VP);
		}

		DirectX::XMFLOAT4 emissionEdgeColor = { 0, 0, 0, 1 };
		DirectX::XMFLOAT4 emissionColor = { 0, 0, 0, 1 };

		modelrenderer->Begin(immediate_context, view_projection, DirectX::XMMatrixIdentity(), projection, lightDirection, camera,view);

		modelrenderer->Draw(immediate_context, model.get(),DirectX::XMFLOAT3(lightDirection.x, lightDirection.y, lightDirection.z), DirectX::XMFLOAT4(1.0f,1.0f,1.0f,1.0f), emissionColor, emissionEdgeColor, 0.75f, 0.25f, 0.01f, FALSE, 0.0f, false, DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

		modelrenderer->End(immediate_context);
	}


	static float posy = 920;


	framebuffers[0]->deactivate(immediate_context);

	Rasterizer->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);
	//tonemap生成

	if (tonemap_t)
	{
		framebuffers[1]->clear(immediate_context);

		framebuffers[1]->activate(immediate_context);

		tonemap_effect->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), elapsed_time);

		framebuffers[1]->deactivate(immediate_context);

		Rasterizer->blit(immediate_context, framebuffers[1]->render_target_shader_resource_view.Get(), 0, 0, (float)framework::Instance().SCREEN_WIDTH, (float)framework::Instance().SCREEN_HEIGHT);

	}

	//effect.EffectRender(view, projection);
	effect.EffectRender(view, Projection);


	// ギズモ描画
	ImGuizmo::BeginFrame();
	ImGuizmo::SetOrthographic(false);
	ImGuizmo::SetRect(0, 0, framework::Instance().SCREEN_WIDTH, framework::Instance().SCREEN_HEIGHT);

	if (selectionNode != nullptr)
	{
		draw_guizmo(view, projection, selectionNode);
	}

	// グリッド描画
	drawgrid(immediate_context, 20, 10.0f);

	// ライン描画
	linerenderer->Render(immediate_context, view, projection);


#ifdef USE_IMGUI
#endif
	// チュートリアルの文字




}
void Scene_model_editor::uninitialize()
{
		
		
	    effect.EffectUninit();
		texture::releaseAll();
}

void Scene_model_editor::draw_guizmo(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection, Model::s_node* node)
{
	// 選択ノードの行列を操作する
	ImGuizmo::Manipulate(
		&view._11, &projection._11,	// ビュー＆プロジェクション行列
		ImGuizmo::TRANSLATE,		// 移動操作
		ImGuizmo::LOCAL,			// ローカル空間での操作
		&node->worldTransform._11,	// 操作するワールド行列
		nullptr);

	// 操作したワールド行列をローカル行列に反映
	if (node->parent != nullptr)
	{
		// 親のワールド逆行列と自身のワールド行列を乗算することで自身のローカル行列を算出できる
		DirectX::XMMATRIX ParentWorldTransform = DirectX::XMLoadFloat4x4(&node->parent->worldTransform);
		DirectX::XMMATRIX ParentInverseTransform = DirectX::XMMatrixInverse(nullptr, ParentWorldTransform);
		DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&node->worldTransform);
		DirectX::XMMATRIX LocalTransform = DirectX::XMMatrixMultiply(WorldTransform, ParentInverseTransform);
		DirectX::XMStoreFloat4x4(&node->localTransform, LocalTransform);
	}
	else
	{
		node->localTransform = node->worldTransform;
	}

	// ローカル行列からSRT値に反映する
	node->translate.x = node->localTransform._41;
	node->translate.y = node->localTransform._42;
	node->translate.z = node->localTransform._43;
	// 回転やスケールは自分で考えてみよう


}

void Scene_model_editor::drawgrid(ID3D11DeviceContext* context, int subdivisions, float scale)
{
	int numLines = (subdivisions + 1) * 2;
	int vertexCount = numLines * 2;

	float corner = 0.5f;
	float step = 1.0f / static_cast<float>(subdivisions);

	int index = 0;
	float s = -corner;

	const DirectX::XMFLOAT4 white = DirectX::XMFLOAT4(1, 1, 1, 1);


	// Create vertical lines
	float scaling = static_cast<float>(subdivisions) * scale;
	DirectX::XMMATRIX M = DirectX::XMMatrixScaling(scaling, scaling, scaling);
	DirectX::XMVECTOR V, P;
	DirectX::XMFLOAT3 position;
	for (int i = 0; i <= subdivisions; i++)
	{
		V = DirectX::XMVectorSet(s, 0, corner, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, white);

		V = DirectX::XMVectorSet(s, 0, -corner, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, white);

		s += step;
	}

	// Create horizontal lines
	s = -corner;
	for (int i = 0; i <= subdivisions; i++)
	{
		V = DirectX::XMVectorSet(corner, 0, s, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, white);

		V = DirectX::XMVectorSet(-corner, 0, s, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, white);

		s += step;
	}

	// X軸
	{
		const DirectX::XMFLOAT4 red = DirectX::XMFLOAT4(1, 0, 0, 1);
		V = DirectX::XMVectorSet(0, 0, 0, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, red);

		V = DirectX::XMVectorSet(corner, 0, 0, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, red);
	}

	// Y軸
	{
		const DirectX::XMFLOAT4 green = DirectX::XMFLOAT4(0, 1, 0, 1);
		V = DirectX::XMVectorSet(0, 0, 0, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, green);

		V = DirectX::XMVectorSet(0, corner, 0, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, green);
	}

	// Z軸
	{
		const DirectX::XMFLOAT4 blue = DirectX::XMFLOAT4(0, 0, 1, 1);
		V = DirectX::XMVectorSet(0, 0, 0, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, blue);

		V = DirectX::XMVectorSet(0, 0, corner, 0);
		P = DirectX::XMVector3TransformCoord(V, M);
		DirectX::XMStoreFloat3(&position, P);
		linerenderer->AddVertex(position, blue);
	}
}

void Scene_model_editor::draw_gui()
{
	draw_menuGUI();
	draw_hierarchy_gui();
	draw_propertygui();
	draw_animation_gui();
	draw_timeline_gui();
}

void Scene_model_editor::draw_menuGUI()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			bool check = false;
			if (ImGui::MenuItem("Open Model", "", &check))
			{
				open_model_file();
			}

			if (ImGui::MenuItem("Open Animation", "", &check))
			{
				open_animation_file();
			}

			if (ImGui::MenuItem("Save Model", "", &check))
			{
				save_model_file();
			}

			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void Scene_model_editor::draw_node_gui(Model::s_node* node)
{
	// 矢印をクリック、またはノードをダブルクリックで階層を開く
	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	// 選択フラグ
	if (selectionNode == node)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Selected;
	}
	// 子がいない場合は矢印をつけない
	size_t child_count = node->children.size();
	if (child_count <= 0)
	{
		nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	// ツリーノードを表示
	bool opened = ImGui::TreeNodeEx(node, nodeFlags, node->name);

	// アクティブ化されたノードは選択する
	if (ImGui::IsItemClicked())
	{
		selectionNode = node;
	}

	// 開かれている場合、子階層も同じ処理を行う
	if (opened && child_count > 0)
	{
		for (Model::s_node* child : node->children)
		{
			draw_node_gui(child);
		}
		ImGui::TreePop();
	}
}

void Scene_model_editor::draw_hierarchy_gui()
{
	ImGui::SetNextWindowPos(ImVec2(10, 30), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	hiddenHierarchy = !ImGui::Begin("Hierarchy", &active, ImGuiWindowFlags_None);
	if (!hiddenHierarchy)
	{
		if (model != nullptr)
		{
			for (Model::s_node& node : model->GetNodes())
			{
				if (node.parent == nullptr)
				{
					draw_node_gui(&node);
				}
			}
		}
	}
	ImGui::End();
}

void Scene_model_editor::draw_propertygui()
{
	ImGui::SetNextWindowPos(ImVec2(970, 30), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	hiddenProperty = !ImGui::Begin("Property", &active, ImGuiWindowFlags_None);
	if (!hiddenProperty)
	{
		if (selectionNode != nullptr)
		{
			DirectX::XMFLOAT3 position = selectionNode->translate;
			//DirectX::XMFLOAT3 rotation = selectionNode->angle;
			DirectX::XMFLOAT3 scale = selectionNode->scale;
			ImGui::InputFloat3("Position", &position.x);
			//ImGui::InputFloat3("Rotation", &rotation.x);
			ImGui::InputFloat3("Scale", &scale.x);
		}
	}
	ImGui::End();
}

void Scene_model_editor::draw_animation_gui()
{
	ImGui::SetNextWindowPos(ImVec2(10, 350), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	hiddenAnimation = !ImGui::Begin("Animation", &active, ImGuiWindowFlags_None);
	if (!hiddenAnimation)
	{
		if (modelResource != nullptr)
		{
			ModelResource::s_animation* selectionAnimation = GetSelectionAnimation();

			int animationIndex = 0;
			for (const ModelResource::s_animation& animation : modelResource->GetAnimations())
			{
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;

				if (selectionAnimation == &animation)
				{
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				ImGui::TreeNodeEx(&animation, nodeFlags, animation.name.c_str());

				if (ImGui::IsItemClicked())
				{
					selectionAnimationIndex = animationIndex;
					selectionNode = nullptr;
				}

				ImGui::TreePop();

				++animationIndex;
			}
		}
	}
	ImGui::End();
}

void Scene_model_editor::draw_timeline_gui()
{
	ImGui::SetNextWindowPos(ImVec2(350, 600), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(900, 100), ImGuiCond_FirstUseEver);

	hiddenTimeline = !ImGui::Begin("Timeline", &active, ImGuiWindowFlags_None);
	if (!hiddenTimeline)
	{
		ModelResource::s_animation* selectionAnimation = GetSelectionAnimation();
		if (model != nullptr && selectionAnimation != nullptr)
		{
			ImGui::Checkbox("Loop", &animationLoop);
			ImGui::SameLine();

			char name[256];
			::strcpy_s(name, sizeof(name), selectionAnimation->name.c_str());
			if (ImGui::InputText("Name", name, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				selectionAnimation->name = name;
			}

			if (ImGui::Button("Play"))
			{
				model->PlayAnimation(selectionAnimationIndex, animationLoop);
				animationPause = false;
			}
			ImGui::SameLine();

			float animationCurrentSeconds = model->GetCurrentSeconds();
			if (ImGui::SliderFloat("Timeline", &animationCurrentSeconds, 0, selectionAnimation->seconds_length, "current frame = %.3f"))
			{
				model->SetAnimationSeconds(animationCurrentSeconds);
				animationPause = true;
			}
		}

	}
	ImGui::End();
}

void Scene_model_editor::open_model_file()
{

	static const char* filter = "Model Files(*.fbx;*.bin;*.json)\0*.fbx;*.bin;*.json\0All Files(*.*)\0*.*;\0\0";

	char filename[256] = { 0 };
	DialogResult result = Dialog::OpenFileName(filename, sizeof(filename), filter, nullptr, framework::Instance().hwnd);
	if (result == DialogResult::OK)
	{
		import_model(filename);
	}

}

void Scene_model_editor::save_model_file()
{
	static const char* filter = "Model Files(*.bin)\0*.bin;\0All Files(*.*)\0*.*;\0\0";

	char filename[256] = { 0 };
    //DialogResult result = Dialog::SaveFileName(filename, sizeof(filename), filter, nullptr, "bin", framework::Instance().hwnd);
	//if (result == DialogResult::OK)
	//{
		export_moodel(filename);
	//}
		
	
}

void Scene_model_editor::open_animation_file()
{
	static const char* filter = "Animation Files(*.fbx)\0*.fbx;\0All Files(*.*)\0*.*;\0\0";

	char filename[256] = { 0 };
	DialogResult result = Dialog::OpenFileName(filename, sizeof(filename), filter, nullptr, framework::Instance().hwnd);
	if (result == DialogResult::OK)
	{
		import_animation(filename);
	}
}

void Scene_model_editor::import_model(const char* filename)
{
	ID3D11Device* device = framework::Instance().GetDevice();
	modelResource = std::make_shared<ModelResource>();
	modelResource->Init(device, filename,true);
	model = std::make_unique<Model>();
	model->Init(modelResource);

	selectionNode = nullptr;
	selectionAnimationIndex = -1;
}

void Scene_model_editor::import_animation(const char* filename)
{
	if (modelResource != nullptr)
	{
		modelResource->AddAnimation(filename);
	}
}

void Scene_model_editor::export_moodel(std::string filename)
{
	if (modelResource != nullptr)
	{
		modelResource->SaveBinary();
	}
}

ModelResource::s_animation* Scene_model_editor::GetSelectionAnimation()
{
	if (modelResource != nullptr)
	{
		const std::vector<ModelResource::s_animation>& animations = modelResource->GetAnimations();
		if (selectionAnimationIndex >= 0 && selectionAnimationIndex < static_cast<int>(animations.size()))
		{
			return const_cast<ModelResource::s_animation*>(&animations.at(selectionAnimationIndex));
		}
	}
	return nullptr;
}


