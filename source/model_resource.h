#pragma once

#include <string>
#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <fbxsdk.h>

#include "DirectXTex.h"
#include <sstream>
#include <fstream>
#include <tchar.h>
#include <functional>
#include <vector>
#include <memory>
#include "resource_manager.h"

#include "cereal.h"

//cerealでXMシリーズが使えないから自作クラスを用意
struct FLOAT4X4
{
	float value[4][4] = { 0 };
	friend class cereal::access;
	template<class Archive>
	void serialize(Archive& archive, std::uint32_t version)
	{
		if (version >= 0)
		{
			archive
			(
				CEREAL_NVP(value[0][0]),
				CEREAL_NVP(value[0][1]),
				CEREAL_NVP(value[0][2]),
				CEREAL_NVP(value[0][3]),
				CEREAL_NVP(value[1][0]),
				CEREAL_NVP(value[1][1]),
				CEREAL_NVP(value[1][2]),
				CEREAL_NVP(value[1][3]),
				CEREAL_NVP(value[2][0]),
				CEREAL_NVP(value[2][1]),
				CEREAL_NVP(value[2][2]),
				CEREAL_NVP(value[2][3]),
				CEREAL_NVP(value[3][0]),
				CEREAL_NVP(value[3][1]),
				CEREAL_NVP(value[3][2]),
				CEREAL_NVP(value[3][3])
			);
		}
	}

	inline DirectX::XMFLOAT4X4 ConvertXMFLOAT4x4()
	{
		return DirectX::XMFLOAT4X4(
			value[0][0], value[0][1], value[0][2], value[0][3],
			value[1][0], value[1][1], value[1][2], value[1][3],
			value[2][0], value[2][1], value[2][2], value[2][3],
			value[3][0], value[3][1], value[3][2], value[3][3]);
	}
};
CEREAL_CLASS_VERSION(FLOAT4X4, 0)

enum class e_propaty_name
{
	diffuse,
	ambient,
	speculer,
	bump,
	emissive,
	metalrough,
};


//FBXや独自でシリアライズしたモデルを読み込むクラス
//FBXを読み込むと自動でシリアライズしてくれる


class ModelResource
{
public:
	static const int MAX_BONE_INFLUENCES = 4;
	static const int MAX_BONES = 1000;

	struct s_node
	{
		std::string			name;
		int					parent_index;
		DirectX::XMFLOAT3	scale;
		DirectX::XMFLOAT4	rotate;
		DirectX::XMFLOAT3	translate;
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(name),
					CEREAL_NVP(parent_index),
					CEREAL_NVP(scale.x),
					CEREAL_NVP(scale.y),
					CEREAL_NVP(scale.z),
					CEREAL_NVP(rotate.x),
					CEREAL_NVP(rotate.y),
					CEREAL_NVP(rotate.z),
					CEREAL_NVP(rotate.w),
					CEREAL_NVP(translate.x),
					CEREAL_NVP(translate.y),
					CEREAL_NVP(translate.z)
				);
			}
		}

	};

	struct s_node_key_data
	{
		DirectX::XMFLOAT3 scale;
		DirectX::XMFLOAT4 rotate;
		DirectX::XMFLOAT3 translate;
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(scale.x),
					CEREAL_NVP(scale.y),
					CEREAL_NVP(scale.z),
					CEREAL_NVP(rotate.x),
					CEREAL_NVP(rotate.y),
					CEREAL_NVP(rotate.z),
					CEREAL_NVP(rotate.w),
					CEREAL_NVP(translate.x),
					CEREAL_NVP(translate.y),
					CEREAL_NVP(translate.z)
				);
			}
		}

	};

	struct s_key_frame
	{
		float						seconds;
		std::vector<s_node_key_data>	node_keys;
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(seconds),
					CEREAL_NVP(node_keys)
				);
			}
		}

	};

	struct s_animation
	{
		float seconds_length = 0;
		std::vector<s_key_frame> keyframes;
		std::string					name;
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(seconds_length),
					CEREAL_NVP(keyframes),
					CEREAL_NVP(name)
				);
			}
		}

	};

	struct s_vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;
		FLOAT bone_weights[MAX_BONE_INFLUENCES] = { 1,0 };
		INT bone_indices[MAX_BONE_INFLUENCES] = {};

		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(position.x),
					CEREAL_NVP(position.y),
					CEREAL_NVP(position.z),
					CEREAL_NVP(normal.x),
					CEREAL_NVP(normal.y),
					CEREAL_NVP(normal.z),
					CEREAL_NVP(texcoord.x),
					CEREAL_NVP(texcoord.y),
					CEREAL_NVP(bone_weights),
					CEREAL_NVP(bone_indices)
				);
			}
		}
	};

	struct s_property
	{
		float factor = 1;
		DirectX::XMFLOAT4 color = { 0.8f, 0.8f, 0.8f, 1.0f }; // w channel is used as shininess by only specular.
		std::string texture_filename;
		float scale_u = 1.0;
		float scale_v = 1.0;

		DirectX::TexMetadata metadata = {};
		ID3D11ShaderResourceView* shader_resource_view;

		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(factor),
					CEREAL_NVP(color.x),
					CEREAL_NVP(color.y),
					CEREAL_NVP(color.z),
					CEREAL_NVP(color.w),
					CEREAL_NVP(texture_filename),
					CEREAL_NVP(scale_u),
					CEREAL_NVP(scale_v)
				);
			}
		}

	};

	struct s_material
	{
		s_property diffuse;
		s_property ambient;
		s_property speculer;
		s_property bump;
		s_property emissive;
		s_property metalrough;
		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(diffuse),
					CEREAL_NVP(ambient),
					CEREAL_NVP(speculer),
					CEREAL_NVP(bump),
					CEREAL_NVP(emissive),
					CEREAL_NVP(metalrough)
				);
			}
		}

		s_property* GetProperty(e_propaty_name _property);
	};

	struct s_subset
	{
		u_int index_start = 0;//index bufferの開始位置
		u_int index_count = 0;//vertexの個数
		int material_index = 0;//materialの配列から情報を参照してくる際に使用
		s_material* material;

		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(index_start),
					CEREAL_NVP(index_count),
					CEREAL_NVP(material_index)
				);
			}
		}
	};

	struct s_mesh
	{

		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;

		std::vector<s_vertex> vertices;
		std::vector<int> indices;
		std::vector<s_subset> subsets;

		int node_index;
		std::vector<int> node_indices;
		std::vector<FLOAT4X4> inverseTransforms;

		friend class cereal::access;
		template<class Archive>
		void serialize(Archive& archive, std::uint32_t version)
		{
			if (version >= 0)
			{
				archive
				(
					CEREAL_NVP(vertices),
					CEREAL_NVP(indices),
					CEREAL_NVP(subsets),
					CEREAL_NVP(node_index),
					CEREAL_NVP(node_indices),
					CEREAL_NVP(inverseTransforms)
				);
			}
		}

	};
private:

	std::string resource_name;
	std::vector<s_node>		nodes;
	std::vector<s_material>	materials;
	std::vector<s_mesh>		meshes;
	std::vector<s_animation>animations;

	int						rootMotionNodeIndex = -1;
	friend class cereal::access;
	template<class Archive>
	void serialize(Archive& archive, std::uint32_t version)
	{
		if (version >= 0)
		{
			archive
			(
				CEREAL_NVP(nodes),
				CEREAL_NVP(materials),
				CEREAL_NVP(meshes),
				CEREAL_NVP(animations)
			);
		}
		if (version >= 1)
		{
			archive
			(
				CEREAL_NVP(resource_name)
			);

		}
	}

	D3D11_TEXTURE2D_DESC tex2d_desc;


public:
	std::vector<s_mesh>& GetMeshes() { return meshes; }
	std::vector<s_node>& GetNodes() { return nodes; }
	std::vector<s_animation>& GetAnimations() { return animations; }
	std::vector<s_material>& GetMaterials() { return materials; }
	std::string GetResouRcename() { return resource_name; }
	//FBXファイルの読み込み時に使う関数
	void Init(ID3D11Device* _device, const std::string& _objFileName, const bool _use_cereal, const bool has_fbm = true);

	void AddAnimation(std::vector<ModelResource::s_animation> _anim)
	{
		//for(s_animation* _anim = animations.begin;_anim != animations.();_anim++)
		for (auto& itr : _anim)
		{
			animations.emplace_back();
			animations.back() = itr;
		}
	}

	void AddAnimation(const char* filename);

private:

	//cereal関連関数
	//保存用

	//読み込み用
	bool LoadBinary();
	bool LoadJson();

	//画像情報はcerealizeできないので別でロード
	void LoadMaterial(ID3D11Device* _device, s_property* _property, bool is_srgb = false);
	//バッファもceralizeできないので別でロード
	void LoadBuffer(ID3D11Device* _device, s_mesh& _mesh);

	//FBXファイルからモデル情報を読み取る(内部でマネージャーも作成している)
	bool CreateModel(ID3D11Device* _device, const std::string& _fbx_file_name, const bool has_fbm);

	//マテリアル情報を作成
	void CreateMaterials(ID3D11Device* _device, const char* _dire_name, FbxScene* _fbx_scene, const bool has_fbm);
	void CreateMaterial(ID3D11Device* _device, const char* _dire_name, FbxSurfaceMaterial* _fbx_surface_material, const bool has_fbm);
	void FetchProperty(ID3D11Device* _device, s_property& _material, const char* _property_name, const char* _factor_name, const char* _dire_name, FbxSurfaceMaterial* _fbx_surface_material, const bool has_fbm);

	//FBXノードを再帰的に辿ってデータを構築する
	void CreateNodes(FbxNode* _root_node, int parent_node_index);
	void CreateNode(FbxNode* _root_node, int parent_node_index);

	//メッシュ情報を作成
	void CreateMeshes(ID3D11Device* _device, FbxNode* _root_node);
	void CreateMesh(ID3D11Device* _device, FbxNode* _root_node, FbxMesh* _fbx_mesh);

	//アニメーションデータを作成
	void CreateAnimations(FbxScene* _fbx_scene, const char* name);
	void LoadAnimations(FbxScene* fbxScene, const char* name);
public:
	int FindNodeIndex(const char* _name);
	int FindMaterialIndex(FbxScene* _fbx_scene, const FbxSurfaceMaterial* _fbx_surface_material);

	std::string GetNodePath(FbxNode* fbxNode) const;

	void SaveBinary();
	void SaveBinary(std::string filename);


	void SaveJson();


	void UploadMaterials(ID3D11Device* _Device, std::string file_name, const int index, e_propaty_name propaty);

};
CEREAL_CLASS_VERSION(ModelResource, 0)
