#include "misc.h"
#include "Model.h"

void Model::Init(std::shared_ptr<ModelResource>& resource)
{
	this->resource = resource;

	// ノード
	const std::vector<ModelResource::s_node>& res_nodes = resource->GetNodes();

	nodes.resize(res_nodes.size());
	for (size_t nodeIndex = 0; nodeIndex < nodes.size(); ++nodeIndex)
	{
		auto&& src = res_nodes.at(nodeIndex);
		auto&& dst = nodes.at(nodeIndex);

		dst.name = src.name.c_str();
		dst.parent = src.parent_index >= 0 ? &nodes.at(src.parent_index) : nullptr;
		dst.scale = src.scale;
		dst.rotate = src.rotate;
		dst.translate = src.translate;

		if (dst.parent != nullptr)
		{
			dst.parent->children.emplace_back(&dst);
		}
	}
	status[0].resize(res_nodes.size());
	status[1].resize(res_nodes.size());
}

// アニメーション再生
void Model::PlayAnimation(int animationIndex, bool loop,float _animrate, float _changeSeconds)
{
	if (_changeSeconds > 0)
	{
		nextSeconds = 0;
		nextAnimation = animationIndex;
		if (currentAnimation < 0)
		{
			currentAnimation = animationIndex;
			currentSeconds = 0.0f;
		}
		//float frame_seconds = resource->GetAnimations().at(animationIndex).keyframes.at().seconds - resource->GetAnimations().at(animationIndex).keyframes.at(0).seconds;
		changeSeconds = _changeSeconds;
		changeCount = _changeSeconds;
		changeRate = (changeSeconds - changeCount) / _changeSeconds;
	}
	else
	{
		currentAnimation = animationIndex;
		currentSeconds = 0.0f;
		changeCount = -1;
		changeRate = -1;
		endAnimation = false;
	}

	animRate = _animrate;
	loopAnimation = loop;
}

// アニメーション計算
void Model::UpdateAnimation(float elapsedTime)
{
	if (currentAnimation < 0)
	{
		return;
	}

	if (resource->GetAnimations().empty())
	{
		return;
	}

	//現在のアニメーション
	ModelResource::s_animation* animations[2] = { &resource->GetAnimations().at(currentAnimation) ,&resource->GetAnimations().at(nextAnimation >= 0 ? nextAnimation : 0) };

	const std::vector<ModelResource::s_key_frame>* keyframes[2] = { &animations[0]->keyframes,&animations[1]->keyframes };
	int keyCount[2] = { static_cast<int>(keyframes[0]->size()), static_cast<int>(keyframes[1]->size()) };

	for (int keyIndex = 0; keyIndex < keyCount[0] - 1; ++keyIndex)
	{
		// 現在の時間がどのキーフレームの間にいるか判定する
		const ModelResource::s_key_frame& keyframe0 = keyframes[0]->at(keyIndex);
		const ModelResource::s_key_frame& keyframe1 = keyframes[0]->at(keyIndex + 1);
		if (currentSeconds >= keyframe0.seconds && currentSeconds < keyframe1.seconds)
		{
			float rate = (currentSeconds - keyframe0.seconds / keyframe1.seconds - keyframe0.seconds);

			assert(nodes.size() == keyframe0.node_keys.size());
			assert(nodes.size() == keyframe1.node_keys.size());
			int nodeCount = static_cast<int>(nodes.size());
			for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
			{
				// ２つのキーフレーム間の補完計算
				const ModelResource::s_node_key_data& key0 = keyframe0.node_keys.at(nodeIndex);
				const ModelResource::s_node_key_data& key1 = keyframe1.node_keys.at(nodeIndex);

				s_node& node = nodes[nodeIndex];

				DirectX::XMVECTOR s0 = DirectX::XMLoadFloat3(&key0.scale);
				DirectX::XMVECTOR s1 = DirectX::XMLoadFloat3(&key1.scale);
				DirectX::XMVECTOR r0 = DirectX::XMLoadFloat4(&key0.rotate);
				DirectX::XMVECTOR r1 = DirectX::XMLoadFloat4(&key1.rotate);
				DirectX::XMVECTOR t0 = DirectX::XMLoadFloat3(&key0.translate);
				DirectX::XMVECTOR t1 = DirectX::XMLoadFloat3(&key1.translate);

				status[0].at(nodeIndex).s = DirectX::XMVectorLerp(s0, s1, rate);
				status[0].at(nodeIndex).r = DirectX::XMQuaternionSlerp(r0, r1, rate);
				status[0].at(nodeIndex).t = DirectX::XMVectorLerp(t0, t1, rate);

			}
			break;
		}
	}

	if (changeCount >= 0)
	{
		for (int keyIndex = 0; keyIndex < keyCount[1] - 1; ++keyIndex)
		{
			// 現在の時間がどのキーフレームの間にいるか判定する
			const ModelResource::s_key_frame& keyframe0 = keyframes[1]->at(keyIndex);
			const ModelResource::s_key_frame& keyframe1 = keyframes[1]->at(keyIndex + 1);
			if (currentSeconds >= keyframe0.seconds && currentSeconds < keyframe1.seconds)
			{
				float rate = (currentSeconds - keyframe0.seconds / keyframe1.seconds - keyframe0.seconds);

				assert(nodes.size() == keyframe0.node_keys.size());
				assert(nodes.size() == keyframe1.node_keys.size());
				int nodeCount = static_cast<int>(nodes.size());
				for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
				{
					// ２つのキーフレーム間の補完計算
					const ModelResource::s_node_key_data& key0 = keyframe0.node_keys.at(nodeIndex);
					const ModelResource::s_node_key_data& key1 = keyframe1.node_keys.at(nodeIndex);

					s_node& node = nodes[nodeIndex];

					DirectX::XMVECTOR s0 = DirectX::XMLoadFloat3(&key0.scale);
					DirectX::XMVECTOR s1 = DirectX::XMLoadFloat3(&key1.scale);
					DirectX::XMVECTOR r0 = DirectX::XMLoadFloat4(&key0.rotate);
					DirectX::XMVECTOR r1 = DirectX::XMLoadFloat4(&key1.rotate);
					DirectX::XMVECTOR t0 = DirectX::XMLoadFloat3(&key0.translate);
					DirectX::XMVECTOR t1 = DirectX::XMLoadFloat3(&key1.translate);

					status[1].at(nodeIndex).s = DirectX::XMVectorLerp(s0, s1, rate);
					status[1].at(nodeIndex).r = DirectX::XMQuaternionSlerp(r0, r1, rate);
					status[1].at(nodeIndex).t = DirectX::XMVectorLerp(t0, t1, rate);

				}
				break;
			}
		}

	}

	for (size_t i = 0; i < status[0].size() - 1; i++)
	{
		s_node& node = nodes[i];
		s_info& info1 = status[0][i];
		s_info* info2 = nullptr;
		s_info result;
		if (changeCount > 0)
		{
			info2 = &status[1][i];

			result.s = DirectX::XMVectorLerp(info1.s, info2->s, changeRate);
			result.r = DirectX::XMQuaternionSlerp(info1.r, info2->r, changeRate);
			result.t = DirectX::XMVectorLerp(info1.t, info2->t, changeRate);

		}
		else
		{
			result = info1;
		}
		DirectX::XMStoreFloat3(&node.scale, result.s);
		DirectX::XMStoreFloat4(&node.rotate, result.r);
		DirectX::XMStoreFloat3(&node.translate, result.t);
	}

	// 最終フレーム処理
	if (endAnimation)
	{
		endAnimation = false;
		currentAnimation = -1;
		return;
	}

	// 時間経過
	currentSeconds += elapsedTime * animRate;
	nextSeconds += elapsedTime * animRate;
	if (currentSeconds >= animations[0]->seconds_length)
	{
		if (loopAnimation)
		{
			currentSeconds -= animations[0]->seconds_length;
		}
		else
		{
			currentSeconds = animations[0]->seconds_length;
			endAnimation = true;
		}
	}
	if (changeCount >= 0)
		if (nextSeconds >= animations[1]->seconds_length)
		{
			nextSeconds -= animations[1]->seconds_length;
		}

	if (changeCount > 0)
	{
		changeCount -= elapsedTime;
		if (changeCount < 0)changeCount = 0;
		changeRate = (changeSeconds - changeCount) / changeSeconds;
	}
	if (changeCount <= .0f && changeSeconds > 0)
	{
		changeCount = -1;
		changeSeconds = -1;
		currentAnimation = nextAnimation;
		nextAnimation = -1;
		currentSeconds = nextSeconds;
	}
}

void Model::InverseKinematicks(DirectX::XMFLOAT3 _target_pos, std::string _tip, std::string _root)
{
	//動かしたい先端のボーンを見つける
	Model::s_node* now_bone = nullptr;
	Model::s_node* parent_bone = nullptr;
	for (auto& node : nodes)
	{
		if (_tip.compare(node.name) != 0)
			continue;

		now_bone = &node;
		//target_posをモデル空間に移動
		//DirectX::XMFLOAT4 _pos = { _target_pos.x, _target_pos.y, _target_pos.z ,1.0f };

		//DirectX::XMMATRIX _inverse_world = DirectX::XMMatrixInverse(nullptr, DirectX::XMLoadFloat4x4(&now_bone->worldTransform));
		//DirectX::XMFLOAT4X4 _inverse;
		//DirectX::XMStoreFloat4x4(&_inverse, _inverse_world);

		//_target_pos = {
		//	_pos.x * _inverse._11 + _pos.y * _inverse._21 + _pos.z * _inverse._31 + _pos.w * _inverse._41,
		//	_pos.x * _inverse._12 + _pos.y * _inverse._22 + _pos.z * _inverse._32 + _pos.w * _inverse._42,
		//	_pos.x * _inverse._13 + _pos.y * _inverse._23 + _pos.z * _inverse._33 + _pos.w * _inverse._43, };
		break;
	}
	if (now_bone == nullptr)return;
	parent_bone = now_bone->parent;
	std::vector<s_bone> bones;
	//1つ目のボーンの親は絶対いるので決め打ちで書く
	bones.emplace_back();

	//bones.back().tip_translate = { now_bone->translate.x,now_bone->translate.y,now_bone->translate.z };
	//bones.back().root_translate =
	//{ parent_bone->translate.x,parent_bone->translate.y,parent_bone->translate.z };

	bones.back().tip_translate = { now_bone->worldTransform._41,now_bone->worldTransform._42,now_bone->worldTransform._43 };
	bones.back().root_translate = { parent_bone->worldTransform._41,parent_bone->worldTransform._42,parent_bone->worldTransform._43 };

	bones.back().bone = now_bone;
	//そのあとは検索する
	while (1)
	{
		now_bone = parent_bone;
		parent_bone = now_bone->parent;
		if (parent_bone == nullptr)break;
		bones.emplace_back();
		//bones.back().tip_translate = { now_bone->translate.x,now_bone->translate.y,now_bone->translate.z };
		//bones.back().root_translate =
		//{ parent_bone->translate.x,parent_bone->translate.y,parent_bone->translate.z };
		bones.back().tip_translate = { now_bone->worldTransform._41,now_bone->worldTransform._42,now_bone->worldTransform._43 };
		bones.back().root_translate =
		{ parent_bone->worldTransform._41,parent_bone->worldTransform._42,parent_bone->worldTransform._43 };
		bones.back().bone = now_bone;
		if (_root.compare(parent_bone->name) == 0)break;
	}

	//ターゲットへの移動
	DirectX::XMFLOAT3 target = _target_pos;

	for (auto& node : nodes)
	{
		if (_tip.compare(node.name) == 0)
		{
			now_bone = &node;
			break;
		}
	}

	for (auto& bone : bones)
	{
		//ターゲット向きのベクトル(移動値にも使用)
		DirectX::XMFLOAT4 vec = { target.x - bone.tip_translate.x,target.y - bone.tip_translate.y, target.z - bone.tip_translate.z,1.0f };
		DirectX::XMVECTOR to_target = DirectX::XMLoadFloat4(&vec);
		to_target = DirectX::XMVector4Normalize(to_target);
		//ボーンをターゲット向きになるように回転する
		//TODO 警告直す(nullptrの逆参照 now_boneにparent_nodeが入ってる)
		DirectX::XMQuaternionSlerp(DirectX::XMVectorSet(now_bone->rotate.x, now_bone->rotate.y, now_bone->rotate.z, now_bone->rotate.w), to_target, .0f);

		//移動
		now_bone->worldTransform._41 = target.x;
		now_bone->worldTransform._42 = target.y;
		now_bone->worldTransform._43 = target.z;
		bone.tip_translate = target;
		bone.root_translate.x += vec.x;
		bone.root_translate.y += vec.y;
		bone.root_translate.z += vec.z;

		target = bone.root_translate;
		now_bone = now_bone->parent;
	}

	//根元のボーンも回転させる
	if (now_bone != nullptr)
	{
		DirectX::XMFLOAT4 vec = { target.x - bones.back().tip_translate.x,target.y - bones.back().tip_translate.y, target.z - bones.back().tip_translate.z,1.0f };
		DirectX::XMVECTOR to_target = DirectX::XMLoadFloat4(&vec);
		to_target = DirectX::XMVector4Normalize(to_target);
		//ボーンをターゲット向きになるように回転する
		DirectX::XMFLOAT4 rotate = now_bone->rotate;
		DirectX::XMQuaternionSlerp(DirectX::XMVectorSet(rotate.x, rotate.y, rotate.z, rotate.w), to_target, .0f);
	}

	//離れている分を戻す
	Model::s_bone* bone = &bones.back();
	for (int i = bones.size(); i > 0; i--)
	{
		now_bone = bone->bone;
		target = { now_bone->parent->worldTransform._41,now_bone->parent->worldTransform._42,now_bone->parent->worldTransform._43 };
		//ボーンの根元から親ボーンまでのベクトル
		DirectX::XMFLOAT3 vec = { target.x - bone->root_translate.x,target.y - bone->root_translate.y, target.z - bone->root_translate.z };

		//移動
		now_bone->worldTransform._41 = target.x;
		now_bone->worldTransform._42 = target.y;
		now_bone->worldTransform._43 = target.z;


		bone--;
	}


	//CalculateLocalTransform();
}

DirectX::XMFLOAT3 Model::GetBonePosition(const char* _node_name)
{
	int index = resource->FindNodeIndex(_node_name);
	DirectX::XMFLOAT4X4 w = nodes[index].worldTransform;
	DirectX::XMFLOAT3 wpos = { w._41,w._42,w._43 };
	return wpos;
}


DirectX::XMFLOAT4X4 Model::GetBoneTranceform(const char* _node_name)
{
	int index = resource->FindNodeIndex(_node_name);
	DirectX::XMFLOAT4X4 w = nodes[index].worldTransform;

	return w;
}


// ローカル変換行列計算
void Model::CalculateLocalTransform()
{
	using DirectX::operator*=;

	for (auto& node : nodes)
	{
		//DirectX::XMMATRIX scale, rotate, translate;
		//scale = DirectX::XMMatrixScaling(node.scale.x, node.scale.y, node.scale.z);
		//rotate = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(node.rotate.x, node.rotate.y, node.rotate.z, node.rotate.w));
		//translate = DirectX::XMMatrixTranslation(node.translate.x, node.translate.y, node.translate.z);
		//DirectX::XMStoreFloat4x4(&node.localTransform, scale * rotate * translate);
		DirectX::XMMATRIX local_matrix = DirectX::XMMatrixRotationQuaternion(DirectX::XMVectorSet(node.rotate.x, node.rotate.y, node.rotate.z, node.rotate.w));
		local_matrix.r[0] *= node.scale.x;
		local_matrix.r[1] *= node.scale.y;
		local_matrix.r[2] *= node.scale.z;
		local_matrix.r[3] = DirectX::XMVectorSet(node.translate.x, node.translate.y, node.translate.z, 1.0f);
		DirectX::XMStoreFloat4x4(&node.localTransform, local_matrix);
	}
}

// ワールド変換行列計算
void Model::CalculateWorldTransform(const DirectX::XMMATRIX& world_transform)
{

	  //bonepos.clear();

	for (auto& node : nodes)
	{
		DirectX::XMMATRIX localTransform = DirectX::XMLoadFloat4x4(&node.localTransform);
		if (node.parent != nullptr)
		{
			DirectX::XMMATRIX parentTransform = DirectX::XMLoadFloat4x4(&node.parent->worldTransform);
			DirectX::XMStoreFloat4x4(&node.worldTransform, localTransform * parentTransform);
		}
		else
		{
			DirectX::XMStoreFloat4x4(&node.worldTransform, localTransform * world_transform);
		}

		
		//bonepos.emplace_back() = GetBonePosition(node.name);


	}
}

