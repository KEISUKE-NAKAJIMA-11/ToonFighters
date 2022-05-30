#include <stdlib.h>
#include <functional>
#include <WICTextureLoader.h>


#include "model_resource.h"
#include "misc.h"
#include "encodeing.h"

#include <iostream>



//#undef max
//#undef min

ModelResource::s_property* ModelResource::s_material::GetProperty(e_propaty_name _property)
{
	switch (_property)
	{
	case e_propaty_name::diffuse:
		return &diffuse;
		break;

	case e_propaty_name::ambient:
		return &ambient;
		break;

	case e_propaty_name::speculer:
		return &speculer;
		break;

	case e_propaty_name::bump:
		return &bump;
		break;

	case e_propaty_name::emissive:
		return &emissive;
		break;

	case e_propaty_name::metalrough:
		return &metalrough;
		break;
	}
}



// FbxDouble2 → XMFLOAT2
inline DirectX::XMFLOAT2 FbxDouble2ToFloat2(const FbxDouble2& fbxValue)
{
	return DirectX::XMFLOAT2(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1])
	);
}

// FbxDouble3 → XMFLOAT3
inline DirectX::XMFLOAT3 FbxDouble3ToFloat3(const FbxDouble3& fbxValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2])
	);
}

// FbxDouble4 → XMFLOAT3
inline DirectX::XMFLOAT3 FbxDouble4ToFloat3(const FbxDouble4& fbxValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2])
	);
}

// FbxDouble4 → XMFLOAT4
inline DirectX::XMFLOAT4 FbxDouble4ToFloat4(const FbxDouble4& fbxValue)
{
	return DirectX::XMFLOAT4(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2]),
		static_cast<float>(fbxValue[3])
	);
}

// FbxDouble4 → XMFLOAT4
inline DirectX::XMFLOAT4X4 FbxAMatrixToFloat4x4(const FbxAMatrix& fbxValue)
{
	return DirectX::XMFLOAT4X4(
		static_cast<float>(fbxValue[0][0]),
		static_cast<float>(fbxValue[0][1]),
		static_cast<float>(fbxValue[0][2]),
		static_cast<float>(fbxValue[0][3]),
		static_cast<float>(fbxValue[1][0]),
		static_cast<float>(fbxValue[1][1]),
		static_cast<float>(fbxValue[1][2]),
		static_cast<float>(fbxValue[1][3]),
		static_cast<float>(fbxValue[2][0]),
		static_cast<float>(fbxValue[2][1]),
		static_cast<float>(fbxValue[2][2]),
		static_cast<float>(fbxValue[2][3]),
		static_cast<float>(fbxValue[3][0]),
		static_cast<float>(fbxValue[3][1]),
		static_cast<float>(fbxValue[3][2]),
		static_cast<float>(fbxValue[3][3])
	);
}

// FbxDouble4 → FLOAT4X4
inline FLOAT4X4 FbxAMatrixToFloat4x4Ex(const FbxAMatrix& fbxValue)
{
	FLOAT4X4 dummy;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			dummy.value[i][j] = static_cast<float>(fbxValue[i][j]);
		}
	}
	return dummy;

}


void ModelResource::Init(ID3D11Device* _device, const std::string& _fbx_file_name, const bool _use_cereal, const bool has_fbm)
{
	//ファイルパスのディレクトリ以降のファイル名だけを切り取る(検索に使いたいので拡張子はいらない)
	//指定文字時のインデックスが返ってくるので、切取りの際にその文字を使いたいので+1している

	const char* cursor = _fbx_file_name.c_str();
	while (*cursor)cursor++;
	while (*cursor != '/' && *cursor != '\\')
	{
		cursor--;
	}
	cursor++;

	std::string name = cursor;

	size_t count = name.find_last_of("/") + 1;
	size_t count2 = name.find_last_of(".") + 1;


	resource_name = name.substr(count, count2 - count - 1);




	//cerealファイルを見つけられたかどうかのフラグ
	bool find_file = false;
	if (_use_cereal)
	{
		if (LoadBinary())
		{
			find_file = true;
		}
		else if (LoadJson())
		{
			find_file = true;
		}

		//cerealファイルを読み込めていたらマテリアルの画像をロードする
		//materialの配列に格納してから各メッシュに情報を格納している(ロード回数を減らすため)
		if (find_file)
		{
			for (auto& material : materials)
			{
				LoadMaterial(_device, &material.diffuse, true);
				LoadMaterial(_device, &material.ambient);
				LoadMaterial(_device, &material.speculer);
				LoadMaterial(_device, &material.bump, true);
				LoadMaterial(_device, &material.emissive, true);
				LoadMaterial(_device, &material.metalrough, true);
			}
			for (auto& mesh : meshes)
			{
				LoadBuffer(_device, mesh);
				for (auto& subset : mesh.subsets)
				{
					subset.material = &materials.at(subset.material_index);
				}
			}
		}
	}

	//cerealファイルが見つかってなかったらfbxからロードしてcrealファイルを作成
	if (find_file == false)
	{
		CreateModel(_device, _fbx_file_name, has_fbm);
		//SaveBinary();
		//SaveJson();
	}

}

void ModelResource::SaveBinary()
{
	//std::string temp_resource_name = resource_name + '\0';
	const char* cursor = resource_name.c_str();
	while (*cursor)cursor++;
	while (*cursor != '/' && *cursor != '\\')cursor--; 
	cursor++;

	std::string file_name = "./Data/model/serialized/";
	file_name += cursor;
	file_name +=".bin";

	Save(file_name.c_str(), resource_name.c_str(), *this);

}

void ModelResource::SaveBinary(std::string filename)
{

	size_t count = resource_name.find_last_of("/") + 1;
	size_t count2 = resource_name.find_last_of(".") + 1;
	resource_name = resource_name.substr(count, count2 - count - 1);

	std::string file_name = filename;

	Save(file_name.c_str(), resource_name.c_str(), *this);

}


void ModelResource::SaveJson()
{
	std::string file_name = "./Data/model/serialized/" + resource_name + ".json";

	Save(file_name.c_str(), resource_name.c_str(), *this);

}



void ModelResource::AddAnimation(const char* filename)
{
	// FBXのファイルパスはUTF-8にする必要がある
	char fbxFilename[256];
	Encoding::StringToUTF8(filename, fbxFilename, sizeof(fbxFilename));

	FbxManager* fbxManager = FbxManager::Create();

	// FBXに対する入出力を定義する
	FbxIOSettings* fbxIOS = FbxIOSettings::Create(fbxManager, IOSROOT);	// 特別な理由がない限りIOSROOTを指定
	fbxManager->SetIOSettings(fbxIOS);

	// インポータを生成
	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
	bool result = fbxImporter->Initialize(fbxFilename, -1, fbxManager->GetIOSettings());	// -1でファイルフォーマット自動判定
	_ASSERT_EXPR_A(result, "FbxImporter::Initialize() : Failed!!\n");

	// SceneオブジェクトにFBXファイル内の情報を流し込む
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "scene");
	fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();	// シーンを流し込んだらImporterは解放してOK

	// アニメーション読み込み
	char name[256];
	::_splitpath_s(filename, nullptr, 0, nullptr, 0, name, 256, nullptr, 0);

	CreateAnimations(fbxScene, name);

	// マネージャ解放
	fbxManager->Destroy();		// 関連するすべてのオブジェクトが解放される
}

bool ModelResource::LoadBinary()
{

	std::string file_name = "./Data/model/serialized/" + resource_name + ".bin";

	return Load(file_name.c_str(), resource_name.c_str(), *this);
}

bool ModelResource::LoadJson()
{
	std::string file_name = "./Data/model/serialized/" + resource_name + ".json";

	return Load(file_name.c_str(), resource_name.c_str(), *this);
}

void ModelResource::LoadMaterial(ID3D11Device* _device, s_property* _property, bool _is_srgb)
{
	if (_property->texture_filename.size() > 0)
	{
		const char* cursor = _property->texture_filename.c_str();
		while (*cursor)cursor++;
		while (*cursor != 'D')cursor--;

		std::string file_name = "./";
		file_name += cursor;

		std::wstring dummy(file_name.begin(), file_name.end());
		resourceManager::LoadShaderResourceView(_device, dummy, &_property->shader_resource_view, &tex2d_desc, nullptr, _is_srgb);

	}
}

void ModelResource::LoadBuffer(ID3D11Device* _device, s_mesh& _mesh)
{
	// 頂点バッファ
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(s_vertex) * _mesh.vertices.size());
		//bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		subresourceData.pSysMem = _mesh.vertices.data();
		subresourceData.SysMemPitch = 0;
		subresourceData.SysMemSlicePitch = 0;

		HRESULT hr = _device->CreateBuffer(&bufferDesc, &subresourceData, _mesh.vertex_buffer.GetAddressOf());

	}

	// インデックスバッファ
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		D3D11_SUBRESOURCE_DATA subresourceData = {};

		bufferDesc.ByteWidth = static_cast<UINT>(sizeof(u_int) * _mesh.indices.size());
		//bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		subresourceData.pSysMem = _mesh.indices.data();
		subresourceData.SysMemPitch = 0; //Not use for index buffers.
		subresourceData.SysMemSlicePitch = 0; //Not use for index buffers.
		HRESULT hr = _device->CreateBuffer(&bufferDesc, &subresourceData, _mesh.index_buffer.GetAddressOf());
	}

}

bool ModelResource::CreateModel(ID3D11Device* _device, const std::string& _fbx_file_name, const bool has_fbm)
{
	//マネージャーを構成する
	FbxManager* manager = FbxManager::Create();

	// FBXに対する入出力を定義する
	manager->SetIOSettings(FbxIOSettings::Create(manager, IOSROOT));

	// インポータを生成
	FbxImporter* importer = FbxImporter::Create(manager, "");
	bool import_status = false;
	// -1でファイルフォーマット自動判定
	import_status = importer->Initialize(_fbx_file_name.c_str(), -1, manager->GetIOSettings());
	//_ASSERT_EXPR_A(import_status, importer->GetStatus().GetErrorString());

	//SceneオブジェクトにFBXファイルの情報を流し込む
	FbxScene* scene = FbxScene::Create(manager, "scene");
	import_status = importer->Import(scene);
	//_ASSERT_EXPR_A(import_status, importer->GetStatus().GetErrorString());
	//Sceneを流し込んだらImportorは解放する
	importer->Destroy();

	//ジオメトリを三角化する
	fbxsdk::FbxGeometryConverter geometry_converter(manager);
	geometry_converter.Triangulate(scene, true);
	geometry_converter.RemoveBadPolygonsFromMeshes(scene);

	// ディレクトリパス取得
	char dirname[256];
	::_splitpath_s(_fbx_file_name.c_str(), nullptr, 0, dirname, 256, nullptr, 0, nullptr, 0);

	// モデル構築
	FbxNode* fbx_root_node = scene->GetRootNode();

	CreateMaterials(_device, dirname, scene, has_fbm);
	CreateNodes(fbx_root_node, -1);
	CreateMeshes(_device, fbx_root_node);

	char name[256];
	::_splitpath_s(_fbx_file_name.c_str(), nullptr, 0, nullptr, 0, name, 256, nullptr, 0);
	CreateAnimations(scene,name);

	// マネージャ解放
	manager->Destroy();
	// 関連するすべてのオブジェクトが解放される

	return true;
}

void ModelResource::CreateMaterials(ID3D11Device* _device, const char* _dire_name, FbxScene* _fbx_scene, const bool has_fbm)
{
	int fbxMaterialCount = _fbx_scene->GetMaterialCount();

	if (fbxMaterialCount > 0)
	{
		for (int fbxMaterialIndex = 0; fbxMaterialIndex < fbxMaterialCount; ++fbxMaterialIndex)
		{
			FbxSurfaceMaterial* fbxSurfaceMaterial = _fbx_scene->GetMaterial(fbxMaterialIndex);

			CreateMaterial(_device, _dire_name, fbxSurfaceMaterial, has_fbm);
		}
	}
	else
	{
		s_material material;
		material.diffuse.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material.ambient.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		material.speculer.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		materials.emplace_back(material);
	}
}

void ModelResource::CreateMaterial(ID3D11Device* _device, const char* _dire_name, FbxSurfaceMaterial* _fbx_surface_material, const bool has_fbm)
{
	s_material material = { 0 };

	//diffse material
	FetchProperty(_device, material.diffuse, FbxSurfaceMaterial::sDiffuse, FbxSurfaceMaterial::sDiffuseFactor, _dire_name, _fbx_surface_material, has_fbm);

	//ambient material
	FetchProperty(_device, material.ambient, FbxSurfaceMaterial::sAmbient, FbxSurfaceMaterial::sAmbientFactor, _dire_name, _fbx_surface_material, has_fbm);

	//speculer material
	FetchProperty(_device, material.speculer, FbxSurfaceMaterial::sSpecular, FbxSurfaceMaterial::sSpecularFactor, _dire_name, _fbx_surface_material, has_fbm);

	//bump material
	FetchProperty(_device, material.bump, FbxSurfaceMaterial::sBump, FbxSurfaceMaterial::sNormalMap, _dire_name, _fbx_surface_material, has_fbm);

	materials.emplace_back(material);
}

void ModelResource::FetchProperty(ID3D11Device* _device, s_property& _material, const char* _property_name, const char* _factor_name, const char* _dire_name, FbxSurfaceMaterial* _fbx_surface_material, const bool has_fbm)
{
	const FbxProperty property = _fbx_surface_material->FindProperty(_property_name);
	const FbxProperty factor = _fbx_surface_material->FindProperty(_factor_name);
	if (property.IsValid() && factor.IsValid())
	{
		FbxDouble3 colour = property.Get<FbxDouble3>();
		double f = factor.Get<FbxDouble>();
		_material.color.x = static_cast<float>(colour[0]);
		_material.color.y = static_cast<float>(colour[1]);
		_material.color.z = static_cast<float>(colour[2]);
		_material.color.w = 1.0f;
	}

	if (property.IsValid())
	{
		const int number_of_textures = property.GetSrcObjectCount<FbxFileTexture>();
		if (number_of_textures > 0)
		{
			const FbxFileTexture* file_texture = property.GetSrcObject<FbxFileTexture>();
			if (file_texture)
			{
				//material.texture_filename = file_texture->GetFileName();
				std::string relativeName(file_texture->GetRelativeFileName());
				if (has_fbm == false)
				{
					size_t s_pos = relativeName.find_last_of("\\") + 1;
					size_t e_pos = relativeName.size() - s_pos;
					relativeName = relativeName.substr(s_pos, e_pos);
				}
				_material.texture_filename = _dire_name + relativeName;
				std::wstring NAME(_material.texture_filename.begin(), _material.texture_filename.end());

				// Create "diffuse.shader_resource_view" from "filename".
				if (!resourceManager::LoadShaderResourceView(_device, NAME.c_str(), &_material.shader_resource_view, &tex2d_desc, nullptr))
				{
					assert(0 && "テクスチャ画像読み込み失敗");
					return;
				}

			}
		}
	}

}

void ModelResource::UploadMaterials(ID3D11Device* _device, std::string file_name, const int index, e_propaty_name propaty)
{

	s_material* material = &materials[index];
	std::wstring path(file_name.begin(), file_name.end());
	if (material == nullptr)return;
	switch (propaty)
	{
	case e_propaty_name::diffuse:
		material->diffuse.texture_filename = file_name;
		resourceManager::LoadShaderResourceView(_device, path, &material->diffuse.shader_resource_view, &tex2d_desc, nullptr);
		break;
	case e_propaty_name::ambient:
		material->ambient.texture_filename = file_name;
		resourceManager::LoadShaderResourceView(_device, path, &material->ambient.shader_resource_view, &tex2d_desc, nullptr);
		break;
	case e_propaty_name::speculer:
		material->speculer.texture_filename = file_name;
		resourceManager::LoadShaderResourceView(_device, path, &material->speculer.shader_resource_view, &tex2d_desc, nullptr);
		break;
	case e_propaty_name::bump:
		material->bump.texture_filename = file_name;
		resourceManager::LoadShaderResourceView(_device, path, &material->bump.shader_resource_view, &tex2d_desc, nullptr);
		break;
	case e_propaty_name::emissive:
		material->emissive.texture_filename = file_name;
		resourceManager::LoadShaderResourceView(_device, path, &material->emissive.shader_resource_view, &tex2d_desc, nullptr);
		break;
	case e_propaty_name::metalrough:
		material->metalrough.texture_filename = file_name;
		resourceManager::LoadShaderResourceView(_device, path, &material->metalrough.shader_resource_view, &tex2d_desc, nullptr);
		break;

	}
}

void ModelResource::CreateNodes(FbxNode* _root_node, int _parent_node_index)
{
	FbxNodeAttribute* fbxNodeAttribute = _root_node->GetNodeAttribute();
	FbxNodeAttribute::EType fbxNodeAttributeType = FbxNodeAttribute::EType::eUnknown;

	if (fbxNodeAttribute != nullptr)
	{
		fbxNodeAttributeType = fbxNodeAttribute->GetAttributeType();
	}

	switch (fbxNodeAttributeType)
	{
	case FbxNodeAttribute::eUnknown:
	case FbxNodeAttribute::eNull:
	case FbxNodeAttribute::eMarker:
	case FbxNodeAttribute::eMesh:
	case FbxNodeAttribute::eSkeleton:
		CreateNode(_root_node, _parent_node_index);
		break;
	}

	// 再帰的に子ノードを処理する
	_parent_node_index = static_cast<int>(nodes.size() - 1);
	for (int i = 0; i < _root_node->GetChildCount(); ++i)
	{
		CreateNodes(_root_node->GetChild(i), _parent_node_index);
	}

}

void ModelResource::CreateNode(FbxNode* _fbx_node, int _parent_node_index)
{
	FbxAMatrix& fbxLocalTransform = _fbx_node->EvaluateLocalTransform();
	s_node node;
	node.name = _fbx_node->GetName();
	node.parent_index = _parent_node_index;
	node.scale = FbxDouble4ToFloat3(fbxLocalTransform.GetS());
	node.rotate = FbxDouble4ToFloat4(fbxLocalTransform.GetQ());
	node.translate = FbxDouble4ToFloat3(fbxLocalTransform.GetT());

	nodes.emplace_back(node);
}

void ModelResource::CreateMeshes(ID3D11Device* _device, FbxNode* _root_node)
{
	FbxNodeAttribute* fbxNodeAttribute = _root_node->GetNodeAttribute();
	FbxNodeAttribute::EType fbxNodeAttributeType = FbxNodeAttribute::EType::eUnknown;

	if (fbxNodeAttribute != nullptr)
	{
		fbxNodeAttributeType = fbxNodeAttribute->GetAttributeType();
	}

	switch (fbxNodeAttributeType)
	{
	case FbxNodeAttribute::eMesh:
		CreateMesh(_device, _root_node, static_cast<FbxMesh*>(fbxNodeAttribute));
		break;
	}

	// 再帰的に子ノードを処理する
	for (int i = 0; i < _root_node->GetChildCount(); ++i)
	{
		CreateMeshes(_device, _root_node->GetChild(i));
	}
}

void ModelResource::CreateMesh(ID3D11Device* _device, FbxNode* _root_node, FbxMesh* _fbx_mesh)
{
	int fbxControlPointsCount = _fbx_mesh->GetControlPointsCount();

	int fbxMaterialCount = _root_node->GetMaterialCount();
	int fbxPolygonCount = _fbx_mesh->GetPolygonCount();

	meshes.emplace_back(s_mesh());
	s_mesh& mesh = meshes.back();
	mesh.subsets.resize(fbxMaterialCount > 0 ? fbxMaterialCount : 1);
	mesh.node_index = FindNodeIndex(_root_node->GetName());

	// サブセットのマテリアル設定
	for (int fbxMaterialIndex = 0; fbxMaterialIndex < fbxMaterialCount; ++fbxMaterialIndex)
	{
		const FbxSurfaceMaterial* fbxSurfaceMaterial = _root_node->GetMaterial(fbxMaterialIndex);

		s_subset& subset = mesh.subsets.at(fbxMaterialIndex);
		subset.material_index = FindMaterialIndex(_root_node->GetScene(), fbxSurfaceMaterial);
		subset.material = &materials.at(subset.material_index);
	}

	// サブセットの頂点インデックス範囲設定
	if (fbxMaterialCount > 0)
	{
		for (int fbxPolygonIndex = 0; fbxPolygonIndex < fbxPolygonCount; ++fbxPolygonIndex)
		{
			int fbxMaterialIndex = _fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(fbxPolygonIndex);
			mesh.subsets.at(fbxMaterialIndex).index_count += 3;
		}

		int offset = 0;
		for (s_subset& subset : mesh.subsets)
		{
			subset.index_start = offset;
			offset += subset.index_count;

			subset.index_count = 0;
		}
	}

	// 頂点影響力データ
	struct BoneInfluence
	{
		int useCount = 0;
		int indices[MAX_BONE_INFLUENCES] = { 0 };
		float weights[MAX_BONE_INFLUENCES] = { 1.0f,0.0f };


		void Add(int index, float weight)
		{
			int dummy_i = -1;
			float dummy_w = -1;

			if (useCount < MAX_BONE_INFLUENCES)
			{
				{
					indices[useCount] = index;
					weights[useCount] = weight;
				}
				useCount++;
				return;//4つ目までの呼び出し時は格納するだけ
			}

			else
			{
				for (int i = 0; i < MAX_BONE_INFLUENCES; i++)
				{
					if (dummy_i == -1)
					{
						if (weights[i] < weight)
						{
							dummy_i = indices[i];
							dummy_w = weights[i];
							indices[i] = index;
							weights[i] = weight;

						}
					}
					else
					{
						if (weights[i] < dummy_w)
						{
							int dummy_i2 = indices[i];
							float dummy_w2 = weights[i];
							indices[i] = dummy_i;
							weights[i] = dummy_w;
							dummy_i = dummy_i2;
							dummy_w = dummy_w2;
						}
					}
				}

				if (dummy_i == -1)
				{
					weights[0] += weight;
				}
				else
				{
					weights[0] += dummy_w;
				}
			}
			return;//4つ目以降は比較してあぶれた分を0番目(影響値が最大のものに加算する)

		}
	};
	// 頂点影響力データを抽出する
	std::vector<BoneInfluence> boneInfluences;
	{
		boneInfluences.resize(fbxControlPointsCount);

		FbxAMatrix fbxGeometricTransform(
			_root_node->GetGeometricTranslation(FbxNode::eSourcePivot),
			_root_node->GetGeometricRotation(FbxNode::eSourcePivot),
			_root_node->GetGeometricScaling(FbxNode::eSourcePivot)
		);

		// スキニングに必要な情報を取得する
		int fbxDeformerCount = _fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int fbxDeformerIndex = 0; fbxDeformerIndex < fbxDeformerCount; ++fbxDeformerIndex)
		{
			FbxSkin* fbxSkin = static_cast<FbxSkin*>(_fbx_mesh->GetDeformer(fbxDeformerIndex, FbxDeformer::eSkin));

			int fbxClusterCount = fbxSkin->GetClusterCount();
			for (int fbxClusterIndex = 0; fbxClusterIndex < fbxClusterCount; ++fbxClusterIndex)
			{
				FbxCluster* fbxCluster = fbxSkin->GetCluster(fbxClusterIndex);

				// 頂点影響力データを抽出する
				{
					int fbxClusterControlPointIndicesCount = fbxCluster->GetControlPointIndicesCount();
					const int* fbxControlPointIndices = fbxCluster->GetControlPointIndices();
					const double* fbxControlPointWeights = fbxCluster->GetControlPointWeights();

					for (int i = 0; i < fbxClusterControlPointIndicesCount; ++i)
					{
						BoneInfluence& boneInfluence = boneInfluences.at(fbxControlPointIndices[i]);

						boneInfluence.Add(fbxClusterIndex, static_cast<float>(fbxControlPointWeights[i]));
					}
				}

				// ボーン変換行列用の逆行列の計算をする
				{
					// メッシュ空間からワールド空間への変換行列
					FbxAMatrix fbxMeshSpaceTransform;
					fbxCluster->GetTransformMatrix(fbxMeshSpaceTransform);

					// ボーン空間からワールド空間への変換行列
					FbxAMatrix fbxBoneSpaceTransform;
					fbxCluster->GetTransformLinkMatrix(fbxBoneSpaceTransform);

					// ボーン逆行列を計算する
					FbxAMatrix fbxInverseTransform = fbxBoneSpaceTransform.Inverse() * fbxMeshSpaceTransform * fbxGeometricTransform;

					FLOAT4X4 inverseTransform = FbxAMatrixToFloat4x4Ex(fbxInverseTransform);
					mesh.inverseTransforms.emplace_back(inverseTransform);

					int nodeIndex = FindNodeIndex(fbxCluster->GetLink()->GetName());
					mesh.node_indices.emplace_back(nodeIndex);
				}
			}
		}
	}

	// UVセット名
	FbxStringList fbxUVSetNames;
	_fbx_mesh->GetUVSetNames(fbxUVSetNames);

	// 頂点データ
	mesh.vertices.resize(fbxPolygonCount * 3);
	mesh.indices.resize(fbxPolygonCount * 3);

	int vertexCount = 0;
	const FbxVector4* fbxControlPoints = _fbx_mesh->GetControlPoints();
	for (int fbxPolygonIndex = 0; fbxPolygonIndex < fbxPolygonCount; ++fbxPolygonIndex)
	{
		// ポリゴンに適用されているマテリアルインデックスを取得する
		int fbxMaterialIndex = 0;
		if (fbxMaterialCount > 0)
		{
			fbxMaterialIndex = _fbx_mesh->GetElementMaterial()->GetIndexArray().GetAt(fbxPolygonIndex);
		}

		s_subset& subset = mesh.subsets.at(fbxMaterialIndex);
		const int indexOffset = subset.index_start + subset.index_count;

		for (int fbxVertexIndex = 0; fbxVertexIndex < 3; ++fbxVertexIndex)
		{
			s_vertex vertex;

			int fbxControlPointIndex = _fbx_mesh->GetPolygonVertex(fbxPolygonIndex, fbxVertexIndex);
			// Position
			{
				vertex.position = FbxDouble4ToFloat3(fbxControlPoints[fbxControlPointIndex]);
			}

			// Weight
			{
				BoneInfluence& boneInfluence = boneInfluences.at(fbxControlPointIndex);
				for (int i = 0; i < MAX_BONE_INFLUENCES; i++)
				{
					vertex.bone_indices[i] = boneInfluence.indices[i];
					vertex.bone_weights[i] = boneInfluence.weights[i];

				}
				//vertex.bone_weights[3] = 1.0f - (vertex.bone_weights[0] + vertex.bone_weights[1] + vertex.bone_weights[2]);
			}

			// Normal
			if (_fbx_mesh->GetElementNormalCount() > 0)
			{
				FbxVector4 fbxNormal;
				_fbx_mesh->GetPolygonVertexNormal(fbxPolygonIndex, fbxVertexIndex, fbxNormal);
				vertex.normal = FbxDouble4ToFloat3(fbxNormal);
			}
			else
			{
				vertex.normal = DirectX::XMFLOAT3(0, 0, 0);
			}

			// Texcoord
			if (_fbx_mesh->GetElementUVCount() > 0)
			{
				bool fbxUnmappedUV;
				FbxVector2 fbxUV;
				_fbx_mesh->GetPolygonVertexUV(fbxPolygonIndex, fbxVertexIndex, fbxUVSetNames[0], fbxUV, fbxUnmappedUV);
				fbxUV[1] = 1.0 - fbxUV[1];
				vertex.texcoord = FbxDouble2ToFloat2(fbxUV);
			}
			else
			{
				vertex.texcoord = DirectX::XMFLOAT2(0, 0);
			}

			mesh.indices.at(indexOffset + fbxVertexIndex) = vertexCount;
			mesh.vertices.at(vertexCount) = vertex;
			vertexCount++;
		}

		subset.index_count += 3;
	}

	LoadBuffer(_device, mesh);

}

void ModelResource::CreateAnimations(FbxScene* _fbx_scene, const char* name)

{
	// すべてのアニメーション名を取得
	FbxArray<FbxString*> fbxAnimStackNames;
	_fbx_scene->FillAnimStackNameArray(fbxAnimStackNames);

	int fbxAnimationCount = fbxAnimStackNames.Size();
	for (int fbxAnimationIndex = 0; fbxAnimationIndex < fbxAnimationCount; ++fbxAnimationIndex)
	{
		animations.emplace_back(s_animation());
		s_animation& animation = animations.back();

		// アニメーションデータのサンプリング設定
		FbxTime::EMode fbxTimeMode = _fbx_scene->GetGlobalSettings().GetTimeMode();
		FbxTime fbxFrameTime;
		fbxFrameTime.SetTime(0, 0, 0, 1, 0, fbxTimeMode);

		float samplingRate = static_cast<float>(fbxFrameTime.GetFrameRate(fbxTimeMode));
		float samplingTime = 1.0f / samplingRate;

		FbxString* fbxAnimStackName = fbxAnimStackNames.GetAt(fbxAnimationIndex);
		FbxAnimStack* fbxAnimStack = _fbx_scene->FindMember<FbxAnimStack>(fbxAnimStackName->Buffer());

		// 再生するアニメーションを指定する。
		_fbx_scene->SetCurrentAnimationStack(fbxAnimStack);

		// アニメーションの再生開始時間と再生終了時間を取得する
		FbxTakeInfo* fbxTakeInfo = _fbx_scene->GetTakeInfo(fbxAnimStackName->Buffer());
		FbxTime fbxStartTime = fbxTakeInfo->mLocalTimeSpan.GetStart();
		FbxTime fbxEndTime = fbxTakeInfo->mLocalTimeSpan.GetStop();

		// 抽出するデータは60フレーム基準でサンプリングする
		FbxTime fbxSamplingStep;
		fbxSamplingStep.SetTime(0, 0, 1, 0, 0, fbxTimeMode);
		fbxSamplingStep = static_cast<FbxLongLong>(fbxSamplingStep.Get() * samplingTime);

		int startFrame = static_cast<int>(fbxStartTime.Get() / fbxSamplingStep.Get());
		int endFrame = static_cast<int>(fbxEndTime.Get() / fbxSamplingStep.Get());
		int frameCount = static_cast<int>((fbxEndTime.Get() - fbxStartTime.Get()) / fbxSamplingStep.Get());

		// アニメーションの対象となるノードを列挙する
		std::vector<FbxNode*> fbxNodes;
		std::function<void(FbxNode*)> traverse = [&](FbxNode* fbxNode)
		{
			FbxNodeAttribute* fbxNodeAttribute = fbxNode->GetNodeAttribute();
			FbxNodeAttribute::EType fbxNodeAttributeType = FbxNodeAttribute::EType::eUnknown;
			if (fbxNodeAttribute != nullptr)
			{
				fbxNodeAttributeType = fbxNodeAttribute->GetAttributeType();
			}

			switch (fbxNodeAttributeType)
			{
			case FbxNodeAttribute::eUnknown:
			case FbxNodeAttribute::eNull:
			case FbxNodeAttribute::eMarker:
			case FbxNodeAttribute::eMesh:
			case FbxNodeAttribute::eSkeleton:
				fbxNodes.emplace_back(fbxNode);
				break;
			}

			for (int i = 0; i < fbxNode->GetChildCount(); i++)
			{
				traverse(fbxNode->GetChild(i));
			}
		};
		traverse(_fbx_scene->GetRootNode());

		// アニメーション名
		char animationName[256];
		::sprintf_s(animationName, "%s/%s", name, fbxAnimStackName->Buffer());
		animation.name = animationName;

		// アニメーションデータを抽出する
		animation.seconds_length = frameCount * samplingTime;
		animation.keyframes.resize(frameCount + 1);

		float seconds = 0.0f;
		s_key_frame* keyframe = animation.keyframes.data();
		size_t fbxNodeCount = fbxNodes.size();
		FbxTime fbxCurrentTime = fbxStartTime;
		for (FbxTime fbxCurrentTime = fbxStartTime; fbxCurrentTime < fbxEndTime; fbxCurrentTime += fbxSamplingStep, ++keyframe)
		{
			// キーフレーム毎の姿勢データを取り出す。
			keyframe->seconds = seconds;
			keyframe->node_keys.resize(fbxNodeCount);
			for (size_t fbxNodeIndex = 0; fbxNodeIndex < fbxNodeCount; ++fbxNodeIndex)
			{
				s_node_key_data& keyData = keyframe->node_keys.at(fbxNodeIndex);
				FbxNode* fbxNode = fbxNodes.at(fbxNodeIndex);

				// 指定時間のローカル行列からスケール値、回転値、移動値を取り出す。
				const FbxAMatrix& fbxLocalTransform = fbxNode->EvaluateLocalTransform(fbxCurrentTime);

				keyData.scale = FbxDouble4ToFloat3(fbxLocalTransform.GetS());
				keyData.rotate = FbxDouble4ToFloat4(fbxLocalTransform.GetQ());
				keyData.translate = FbxDouble4ToFloat3(fbxLocalTransform.GetT());
			}
			seconds += samplingTime;
		}
	}

	// 後始末
	for (int i = 0; i < fbxAnimationCount; i++)
	{
		delete fbxAnimStackNames[i];
	}
}
void ModelResource::LoadAnimations(FbxScene* fbxScene, const char* name)
{
	// すべてのアニメーション名を取得
	FbxArray<FbxString*> fbxAnimStackNames;
	fbxScene->FillAnimStackNameArray(fbxAnimStackNames);

	int fbxAnimationCount = fbxAnimStackNames.Size();
	for (int fbxAnimationIndex = 0; fbxAnimationIndex < fbxAnimationCount; ++fbxAnimationIndex)
	{
		animations.emplace_back(s_animation());
		s_animation& animation = animations.back();

		// アニメーションデータのサンプリング設定
		FbxTime::EMode fbxTimeMode = fbxScene->GetGlobalSettings().GetTimeMode();
		FbxTime fbxFrameTime;
		fbxFrameTime.SetTime(0, 0, 0, 1, 0, fbxTimeMode);

		float samplingRate = static_cast<float>(fbxFrameTime.GetFrameRate(fbxTimeMode));
		float samplingTime = 1.0f / samplingRate;

		FbxString* fbxAnimStackName = fbxAnimStackNames.GetAt(fbxAnimationIndex);
		FbxAnimStack* fbxAnimStack = fbxScene->FindMember<FbxAnimStack>(fbxAnimStackName->Buffer());

		// 再生するアニメーションを指定する。
		fbxScene->SetCurrentAnimationStack(fbxAnimStack);

		// アニメーションの再生開始時間と再生終了時間を取得する
		FbxTakeInfo* fbxTakeInfo = fbxScene->GetTakeInfo(fbxAnimStackName->Buffer());
		FbxTime fbxStartTime = fbxTakeInfo->mLocalTimeSpan.GetStart();
		FbxTime fbxEndTime = fbxTakeInfo->mLocalTimeSpan.GetStop();

		// 抽出するデータは60フレーム基準でサンプリングする
		FbxTime fbxSamplingStep;
		fbxSamplingStep.SetTime(0, 0, 1, 0, 0, fbxTimeMode);
		fbxSamplingStep = static_cast<FbxLongLong>(fbxSamplingStep.Get() * samplingTime);

		int startFrame = static_cast<int>(fbxStartTime.Get() / fbxSamplingStep.Get());
		int endFrame = static_cast<int>(fbxEndTime.Get() / fbxSamplingStep.Get());
		int frameCount = static_cast<int>((fbxEndTime.Get() - fbxStartTime.Get()) / fbxSamplingStep.Get());

		// アニメーションの対象となるノードを列挙する
		std::vector<FbxNode*> fbxNodes;
#if 0
		// ノード名を比較して対象ノードを列挙する
		// ※名前が重複していると失敗する場合がある
		FbxNode* fbxRootNode = fbxScene->GetRootNode();
		for (Node& node : nodes)
		{
			FbxNode* fbxNode = fbxRootNode->FindChild(node.name.c_str(), true, true);
			fbxNodes.emplace_back(fbxNode);
		}
#else
		// ノードの完全パスを比較して対象ノードを列挙する（重い）
		// ※必ずモデルとアニメーションのFBXのノードツリー構成が一致している必要がある
		for (s_node& node : nodes)
		{
			FbxNode* fbxAnimationNode = nullptr;
			for (int fbxNodeIndex = 0; fbxNodeIndex < fbxScene->GetNodeCount(); ++fbxNodeIndex)
			{
				FbxNode* fbxNode = fbxScene->GetNode(fbxNodeIndex);
				std::string& nodePath = GetNodePath(fbxNode);
				if (node.name == nodePath)
				{
					fbxAnimationNode = fbxNode;
					break;
				}
			}
			fbxNodes.emplace_back(fbxAnimationNode);
		}
#endif
		// アニメーション名
		char animationName[256];
		::sprintf_s(animationName, "%s/%s", name, fbxAnimStackName->Buffer());
		animation.name = animationName;

		// アニメーションデータを抽出する
		animation.seconds_length = frameCount * samplingTime;
		animation.keyframes.resize(frameCount + 1);

		float seconds = 0.0f;
		s_key_frame* keyframe = animation.keyframes.data();
		size_t fbxNodeCount = fbxNodes.size();
		FbxTime fbxCurrentTime = fbxStartTime;
		for (FbxTime fbxCurrentTime = fbxStartTime; fbxCurrentTime < fbxEndTime; fbxCurrentTime += fbxSamplingStep, ++keyframe)
		{
			// キーフレーム毎の姿勢データを取り出す。
			keyframe->seconds = seconds;
			keyframe->node_keys.resize(fbxNodeCount);
			for (size_t fbxNodeIndex = 0; fbxNodeIndex < fbxNodeCount; ++fbxNodeIndex)
			{
				s_node_key_data& keyData = keyframe->node_keys.at(fbxNodeIndex);
				FbxNode* fbxNode = fbxNodes.at(fbxNodeIndex);
				if (fbxNode == nullptr)
				{
					// アニメーション対象のノードがなかったのでダミーデータを設定
					s_node& node = nodes.at(fbxNodeIndex);
					keyData.scale = node.scale;
					keyData.rotate = node.rotate;
					keyData.translate = node.translate;
				}
				else if (fbxNodeIndex == rootMotionNodeIndex)
				{
					// ルートモーションは無視する
					s_node& node = nodes.at(fbxNodeIndex);
					keyData.scale = DirectX::XMFLOAT3(1, 1, 1);
					keyData.rotate = DirectX::XMFLOAT4(0, 0, 0, 1);
					keyData.translate = DirectX::XMFLOAT3(0, 0, 0);
				}
				else
				{
					// 指定時間のローカル行列からスケール値、回転値、移動値を取り出す。
					const FbxAMatrix& fbxLocalTransform = fbxNode->EvaluateLocalTransform(fbxCurrentTime);

					keyData.scale = FbxDouble4ToFloat3(fbxLocalTransform.GetS());
					keyData.rotate = FbxDouble4ToFloat4(fbxLocalTransform.GetQ());
					keyData.translate = FbxDouble4ToFloat3(fbxLocalTransform.GetT());
				}
			}
			seconds += samplingTime;
		}
	}

	// 後始末
	for (int i = 0; i < fbxAnimationCount; i++)
	{
		delete fbxAnimStackNames[i];
	}
}
int ModelResource::FindNodeIndex(const char* _name)
{
	for (size_t i = 0; i < nodes.size(); ++i)
	{
		if (nodes[i].name == _name)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}







int ModelResource::FindMaterialIndex(FbxScene* _fbx_scene, const FbxSurfaceMaterial* _fbx_surface_material)
{
	int fbxMaterialCount = _fbx_scene->GetMaterialCount();

	for (int i = 0; i < fbxMaterialCount; ++i)
	{
		if (_fbx_scene->GetMaterial(i) == _fbx_surface_material)
		{
			return i;
		}
	}
	return -1;
}


std::string ModelResource::GetNodePath(FbxNode* fbxNode) const
{
	std::string parentNodeName;

	FbxNode* fbxParentNode = fbxNode->GetParent();
	if (fbxParentNode != nullptr)
	{
		parentNodeName = GetNodePath(fbxParentNode);
		return parentNodeName + "/" + fbxNode->GetName();
	}

	return fbxNode->GetName();
}



