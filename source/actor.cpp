#include "imgui.h"
#include "framework.h"
#include "component.h"
#include "Actor.h"

// �J�n����
void Actor::Start()
{
	for (std::shared_ptr<Component>& component : components)
	{
		component->Start();
	}
}

// �X�V
void Actor::Update(float elapsedTime)
{
	// �A�j���[�V�����̍X�V
	if (model != nullptr)
	{
		model->UpdateAnimation(elapsedTime);
	}

	for (std::shared_ptr<Component>& component : components)
	{
		component->Update(elapsedTime);
	}
}

// �s��̍X�V
void Actor::UpdateTransform()
{
	// ���[���h�s��̍X�V
	DirectX::XMVECTOR Q = DirectX::XMLoadFloat4(&rotation);
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(Q);
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	DirectX::XMMATRIX W = S * R * T;
	DirectX::XMStoreFloat4x4(&transform, W);

	// ���f���̍s��X�V
	if (model != nullptr)
	{
		model->CalculateLocalTransform();
		model->CalculateWorldTransform(W);
	}
}

// GUI�\��
void Actor::OnGUI()
{
	// ���O
	{
		char buffer[1024];
		::strncpy_s(buffer, sizeof(buffer), GetName(), sizeof(buffer));
		if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
		{
			SetName(buffer);
		}
	}

	// �g�����X�t�H�[��
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputFloat3("Position", &position.x);
	}

	if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputFloat3("Scale", &scale.x);
	}


	if (ImGui::CollapsingHeader("Angle", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::InputFloat3("Angle", &rotation.x);
	}


	// �R���|�[�l���g
	for (std::shared_ptr<Component>& component : components)
	{
		ImGui::Spacing();
		ImGui::Separator();

		if (ImGui::CollapsingHeader(component->GetName(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			//ImGui::PushID(StringToHash(component->GetName()));
			component->OnGUI();
			//ImGui::PopID();
		}
	}
}

// ���f���̃Z�b�g�A�b�v
void Actor::SetupModel(std::shared_ptr<ModelResource>& resource)
{
	model = std::make_unique<Model>();
	model->Init(resource);
}

// �쐬
std::shared_ptr<Actor> ActorManager::Create()
{
	std::shared_ptr<Actor> actor = std::make_shared<Actor>();
	{
		static int id = 0;
		char name[256];
		::sprintf_s(name, sizeof(name), "Actor%d", id++);
		actor->SetName(name);
	}
	startActors.emplace_back(actor);
	return actor;
}

// �폜
void ActorManager::Destroy(std::shared_ptr<Actor> actor)
{
	std::vector<std::shared_ptr<Actor>>::iterator itStart = std::find(startActors.begin(), startActors.end(), actor);
	if (itStart != startActors.end())
	{
		startActors.erase(itStart);
	}

	std::vector<std::shared_ptr<Actor>>::iterator itUpdate = std::find(updateActors.begin(), updateActors.end(), actor);
	if (itUpdate != updateActors.end())
	{
		updateActors.erase(itUpdate);
	}

	std::set<std::shared_ptr<Actor>>::iterator itSelection = selectionActors.find(actor);
	if (itSelection != selectionActors.end())
	{
		selectionActors.erase(itSelection);
	}
}

// �X�V
void ActorManager::Update(float elapsedTime)
{
	for (std::shared_ptr<Actor>& actor : startActors)
	{
		actor->Start();
		updateActors.emplace_back(actor);
	}
	startActors.clear();

	for (std::shared_ptr<Actor>& actor : updateActors)
	{
		actor->Update(elapsedTime);
	}
}

// �s��X�V
void ActorManager::UpdateTransform()
{
	for (std::shared_ptr<Actor>& actor : updateActors)
	{
		actor->UpdateTransform();
	}
}


void ActorManager::Render(ModelRenderer* renderer, Shader* shader,DirectX::XMFLOAT4X4& view_projection, 
	DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor, float edgeThershood, float toonThereshood, float shadowbias)
{

	framework& graphics = framework::Instance();

	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	DirectX::XMFLOAT4 emissionEdgeColor = { 0, 0, 0, 1 };
	DirectX::XMFLOAT4 emissionColor = { 0, 0, 0, 1 };
	//�G�b�W�̃J���[��ύX����
   // EdgeColorSet(invincible, emissionEdgeColor);



	for (std::shared_ptr<Actor>& actor : updateActors)
	{
		// ���f��������Ε`��
		Model* model = actor->GetModel();
		if (model != nullptr)
		{
			// �`��

			DirectX::XMMATRIX W;
			W = DirectX::XMLoadFloat4x4(&actor->GetTransform());

			renderer->Begin(dc,shader, view_projection, W , projection, light_direction, camera,view, brightColor, darkColor);
			renderer->Draw(dc, actor->GetModel(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), actor->color, emissionColor, emissionEdgeColor, edgeThershood, toonThereshood, shadowbias, FALSE, 0.0f, true, actor->color);
			renderer->End(dc);
		}
	}



	// ���X�^�[�`��
	DrawLister();

	// �ڍו`��
	DrawDetail();

}

void ActorManager::Render(ModelRenderer* renderer, DirectX::XMFLOAT4X4& view_projection,
	DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view)
{

	framework& graphics = framework::Instance();

	ID3D11DeviceContext* dc = graphics.GetDeviceContext();

	DirectX::XMFLOAT4 emissionEdgeColor = { 0, 0, 0, 1 };
	DirectX::XMFLOAT4 emissionColor = { 0, 0, 0, 1 };
	//�G�b�W�̃J���[��ύX����
   // EdgeColorSet(invincible, emissionEdgeColor);



	for (std::shared_ptr<Actor>& actor : updateActors)
	{
		// ���f��������Ε`��
		Model* model = actor->GetModel();
		if (model != nullptr)
		{
			// �`��

			DirectX::XMMATRIX W;
			W = DirectX::XMLoadFloat4x4(&actor->GetTransform());

			renderer->Begin(dc, view_projection, W, projection, light_direction, camera, view);
			renderer->Draw(dc, actor->GetModel(), DirectX::XMFLOAT3(light_direction.x, light_direction.y, light_direction.z), actor->color, emissionColor, emissionEdgeColor, 0.75f, 0.25f, 0.01f, FALSE, 0.0f, true, actor->color);
			renderer->End(dc);
		}
	}



	// ���X�^�[�`��
	DrawLister();

	// �ڍו`��
	DrawDetail();

}



// ���X�^�[�`��
void ActorManager::DrawLister()
{
	ImGui::SetNextWindowPos(ImVec2(30, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	hiddenLister = !ImGui::Begin("Actor Lister", nullptr, ImGuiWindowFlags_None);
	if (!hiddenLister)
	{
		for (std::shared_ptr<Actor>& actor : updateActors)
		{
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;

			if (selectionActors.find(actor) != selectionActors.end())
			{
				nodeFlags |= ImGuiTreeNodeFlags_Selected;
			}

			ImGui::TreeNodeEx(actor.get(), nodeFlags, actor->GetName());

			if (ImGui::IsItemClicked())
			{
				// �P��I�������Ή����Ă���
				ImGuiIO& io = ImGui::GetIO();
				selectionActors.clear();
				selectionActors.insert(actor);
			}

			ImGui::TreePop();
		}
	}
	ImGui::End();
}

// �ڍו`��
void ActorManager::DrawDetail()
{
	ImGui::SetNextWindowPos(ImVec2(950, 50), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	hiddenDetail = !ImGui::Begin("Actor Detail", nullptr, ImGuiWindowFlags_None);
	if (!hiddenDetail)
	{
		std::shared_ptr<Actor> lastSelected = selectionActors.empty() ? nullptr : *selectionActors.rbegin();
		if (lastSelected != nullptr)
		{
			lastSelected->OnGUI();
		}
	}
	ImGui::End();
}
