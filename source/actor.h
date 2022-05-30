#pragma once

#include <memory>
#include <vector>
#include <set>
#include <string>
#include <DirectXMath.h>
#include "model.h"
#include "model_renderer.h"
// �O���錾
class Component;

//Actor�N���X
//�R���|�[�l���g�w���̗��K�ō쐬���Ă݂�
//OBJ3D�ɑ��邩������Ȃ��V�����ėp�N���X
//�g�����ɗD��Ă��邪�Q�Ƃ��߂�ǂ�����

// �A�N�^�[
class Actor : public std::enable_shared_from_this<Actor>
{
public:
	Actor() {}
	virtual ~Actor() {};

	// �J�n����
	virtual void Start();

	// �X�V����
	virtual void Update(float elapsedTime);

	// �s��̍X�V
	virtual void UpdateTransform();

	// GUI�\��
	virtual void OnGUI();

	// ���O�̐ݒ�
	void SetName(const char* name) { this->name = name; }

	// ���O�̎擾
	const char* GetName() const { return name.c_str(); }

	// �ʒu�̐ݒ�
	void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; }

	// �ʒu�̎擾
	const DirectX::XMFLOAT3& GetPosition() const { return position; }

	// ��]�̐ݒ�
	void SetRotation(const DirectX::XMFLOAT4& rotation) { this->rotation = rotation; }

	// ��]�̎擾
	const DirectX::XMFLOAT4& GetRotation() const { return rotation; }

	// �X�P�[���̐ݒ�
	void SetScale(const DirectX::XMFLOAT3& scale) { this->scale = scale; }

	// �X�P�[���̎擾
	const DirectX::XMFLOAT3& GetScale() const { return scale; }

	// �s��̎擾
	const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }

	// ���f���̃Z�b�g�A�b�v
	void SetupModel(std::shared_ptr<ModelResource>& resource);

	// ���f���̎擾
	Model* GetModel() const { return model.get(); }

	// �R���|�[�l���g�ǉ�
	template<class T, class... Args>
	std::shared_ptr<T> AddComponent(Args... args)
	{
		std::shared_ptr<T> component = std::make_shared<T>(args...);
		component->SetActor(shared_from_this());
		components.emplace_back(component);
		return component;
	}

	// �R���|�[�l���g�擾
	template<class T>
	std::shared_ptr<T> GetComponent()
	{
		for (std::shared_ptr<Component>& component : components)
		{
			std::shared_ptr<T> p = std::dynamic_pointer_cast<T>(component);
			if (p == nullptr) continue;
			return p;
		}
		return nullptr;
	}


	DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };

private:
	std::string			name;
	DirectX::XMFLOAT3	position = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT4	rotation = DirectX::XMFLOAT4(0, 0, 0, 1);
	DirectX::XMFLOAT3	scale = DirectX::XMFLOAT3(1, 1, 1);
	DirectX::XMFLOAT4X4	transform = DirectX::XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);



	std::unique_ptr<Model>	model;

	std::vector<std::shared_ptr<Component>>	components;
};

// �A�N�^�[�}�l�[�W���[
class ActorManager
{
private:
	ActorManager() {}
	~ActorManager() {};

public:
	// �C���X�^���X�擾
	static ActorManager& Instance()
	{
		static ActorManager instance;
		return instance;
	}

	// �쐬
	std::shared_ptr<Actor> Create();

	// �폜
	void Destroy(std::shared_ptr<Actor> actor);

	// �X�V
	void Update(float elapsedTime);

	// �s��X�V
	void UpdateTransform();

	//void Render(ModelRenderer* renderer, Shader* shader, DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view);

	//void Render(ModelRenderer* renderer, Shader* shader, DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor);

	void Render(ModelRenderer* renderer, Shader* shader, DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor, float edgeThershood, float toonThereshood, float shadowbias);

	void Render(ModelRenderer* renderer, DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view);

	// �`��
	//void Render(ModelRenderer* renderer, DirectX::XMFLOAT4X4& view_projection, DirectX::XMMATRIX& world_transform, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera);

private:
	void DrawLister();
	void DrawDetail();

private:
	std::vector<std::shared_ptr<Actor>>		startActors;
	std::vector<std::shared_ptr<Actor>>		updateActors;
	std::set<std::shared_ptr<Actor>>		selectionActors;

	bool					hiddenLister = false;
	bool					hiddenDetail = false;
};
