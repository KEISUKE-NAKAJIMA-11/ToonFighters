#pragma once

#include <memory>
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "model_resource.h"
#include <wrl.h>
#include <string>

class ModelResource;

//ModelResourceクラスで読み込んだ情報を使って3Dモデルをセットするクラス
//アニメーション管理も行う

class Model
{
public:
	Model() {}
	~Model() {}

	struct s_info
	{
		DirectX::XMVECTOR s, r, t;
	};

	struct s_node
	{
		const char* name;
		s_node* parent;
		DirectX::XMFLOAT3	scale;
		DirectX::XMFLOAT4	rotate;
		DirectX::XMFLOAT3	translate;
		DirectX::XMFLOAT4X4	localTransform;
		DirectX::XMFLOAT4X4	worldTransform;

		std::vector<s_node*>	children;
	};

	struct s_bone
	{
		Model::s_node* bone;
		DirectX::XMFLOAT3	tip_translate;
		DirectX::XMFLOAT3	root_translate;
	};
	void Init(std::shared_ptr<ModelResource>& resource);
	// アニメーション
	bool IsPlayAnimation() const { return currentAnimation >= 0; }
	int GeyAnimationindex() const { return currentAnimation; }
	void PlayAnimation(int animationIndex, bool loop = false,float anim_rate = 1.0f ,float _changeSeconds = .0f);
	void UpdateAnimation(float elapsedTime);

	//IK
	void InverseKinematicks(DirectX::XMFLOAT3 _target_pos, std::string _tip, std::string _root);

	//ボーン位置をもらう
	DirectX::XMFLOAT3 GetBonePosition(const char* _node_name);

	DirectX::XMFLOAT4X4 GetBoneTranceform(const char* _node_name);



	// 行列計算
	void CalculateLocalTransform();
	void CalculateWorldTransform(const DirectX::XMMATRIX& world_transform);

	const std::vector<s_node>& GetNodes() const { return nodes; }
	std::vector<s_node>& GetNodes() { return nodes; }
	ModelResource* GetResource() const { return resource.get(); }
	int GetCurrentAinmIndex() { return currentAnimation; }
	int GetNextAinmIndex() { return nextAnimation; }
	float GetCurrentSeconds() { return currentSeconds; }
	bool GetEndAnimation() { return endAnimation; }
	float GetNextSeconds() { return nextSeconds; }
	float GetChangeSeconds() { return changeSeconds; }
	float GetChangeCount() { return changeCount; }
	float GetChangeRate() { return changeRate; }
	std::wstring GetName() { return name; }
	void SetName(const std::wstring _name) { name = _name; }

	void SetAnimationSeconds(float seconds) { currentSeconds = seconds; }
	bool isloopAnimation() { return loopAnimation; }

	std::vector<DirectX::XMFLOAT3> bonepos;

private:
	std::shared_ptr<ModelResource>	resource;
	std::vector<s_node>				nodes;
	std::vector<s_info> status[2];
	int								currentAnimation = -1;
	int								nextAnimation = -1;
	float							currentSeconds = 0.0f;
	float							nextSeconds = 0.0f;
	float							changeSeconds = 0.0f;
	float							changeCount = 0.0f;
	float							changeRate = 0.0f;
	float                           animRate = 0.0f;
	bool							loopAnimation = false;
	bool							endAnimation = false;
	std::wstring					name = L"no name";
};