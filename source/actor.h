#pragma once

#include <memory>
#include <vector>
#include <set>
#include <string>
#include <DirectXMath.h>
#include "model.h"
#include "model_renderer.h"
// 前方宣言
class Component;

//Actorクラス
//コンポーネント指向の練習で作成してみた
//OBJ3Dに代わるかもしれない新しい汎用クラス
//拡張性に優れているが参照がめんどくさい

// アクター
class Actor : public std::enable_shared_from_this<Actor>
{
public:
	Actor() {}
	virtual ~Actor() {};

	// 開始処理
	virtual void Start();

	// 更新処理
	virtual void Update(float elapsedTime);

	// 行列の更新
	virtual void UpdateTransform();

	// GUI表示
	virtual void OnGUI();

	// 名前の設定
	void SetName(const char* name) { this->name = name; }

	// 名前の取得
	const char* GetName() const { return name.c_str(); }

	// 位置の設定
	void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; }

	// 位置の取得
	const DirectX::XMFLOAT3& GetPosition() const { return position; }

	// 回転の設定
	void SetRotation(const DirectX::XMFLOAT4& rotation) { this->rotation = rotation; }

	// 回転の取得
	const DirectX::XMFLOAT4& GetRotation() const { return rotation; }

	// スケールの設定
	void SetScale(const DirectX::XMFLOAT3& scale) { this->scale = scale; }

	// スケールの取得
	const DirectX::XMFLOAT3& GetScale() const { return scale; }

	// 行列の取得
	const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }

	// モデルのセットアップ
	void SetupModel(std::shared_ptr<ModelResource>& resource);

	// モデルの取得
	Model* GetModel() const { return model.get(); }

	// コンポーネント追加
	template<class T, class... Args>
	std::shared_ptr<T> AddComponent(Args... args)
	{
		std::shared_ptr<T> component = std::make_shared<T>(args...);
		component->SetActor(shared_from_this());
		components.emplace_back(component);
		return component;
	}

	// コンポーネント取得
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

// アクターマネージャー
class ActorManager
{
private:
	ActorManager() {}
	~ActorManager() {};

public:
	// インスタンス取得
	static ActorManager& Instance()
	{
		static ActorManager instance;
		return instance;
	}

	// 作成
	std::shared_ptr<Actor> Create();

	// 削除
	void Destroy(std::shared_ptr<Actor> actor);

	// 更新
	void Update(float elapsedTime);

	// 行列更新
	void UpdateTransform();

	//void Render(ModelRenderer* renderer, Shader* shader, DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view);

	//void Render(ModelRenderer* renderer, Shader* shader, DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor);

	void Render(ModelRenderer* renderer, Shader* shader, DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor, float edgeThershood, float toonThereshood, float shadowbias);

	void Render(ModelRenderer* renderer, DirectX::XMFLOAT4X4& view_projection, DirectX::XMFLOAT4X4& projection, const DirectX::XMFLOAT4& light_direction, std::shared_ptr<Camera> camera, DirectX::XMFLOAT4X4& view);

	// 描画
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
