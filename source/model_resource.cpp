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



// FbxDouble2 �� XMFLOAT2
inline DirectX::XMFLOAT2 FbxDouble2ToFloat2(const FbxDouble2& fbxValue)
{
	return DirectX::XMFLOAT2(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1])
	);
}

// FbxDouble3 �� XMFLOAT3
inline DirectX::XMFLOAT3 FbxDouble3ToFloat3(const FbxDouble3& fbxValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2])
	);
}

// FbxDouble4 �� XMFLOAT3
inline DirectX::XMFLOAT3 FbxDouble4ToFloat3(const FbxDouble4& fbxValue)
{
	return DirectX::XMFLOAT3(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2])
	);
}

// FbxDouble4 �� XMFLOAT4
inline DirectX::XMFLOAT4 FbxDouble4ToFloat4(const FbxDouble4& fbxValue)
{
	return DirectX::XMFLOAT4(
		static_cast<float>(fbxValue[0]),
		static_cast<float>(fbxValue[1]),
		static_cast<float>(fbxValue[2]),
		static_cast<float>(fbxValue[3])
	);
}

// FbxDouble4 �� XMFLOAT4
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

// FbxDouble4 �� FLOAT4X4
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
	//�t�@�C���p�X�̃f�B���N�g���ȍ~�̃t�@�C����������؂���(�����Ɏg�������̂Ŋg���q�͂���Ȃ�)
	//�w�蕶�����̃C���f�b�N�X���Ԃ��Ă���̂ŁA�؎��̍ۂɂ��̕������g�������̂�+1���Ă���

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




	//cereal�t�@�C����������ꂽ���ǂ����̃t���O
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

		//cereal�t�@�C����ǂݍ��߂Ă�����}�e���A���̉摜�����[�h����
		//material�̔z��Ɋi�[���Ă���e���b�V���ɏ����i�[���Ă���(���[�h�񐔂����炷����)
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

	//cereal�t�@�C�����������ĂȂ�������fbx���烍�[�h����creal�t�@�C�����쐬
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
	// FBX�̃t�@�C���p�X��UTF-8�ɂ���K�v������
	char fbxFilename[256];
	Encoding::StringToUTF8(filename, fbxFilename, sizeof(fbxFilename));

	FbxManager* fbxManager = FbxManager::Create();

	// FBX�ɑ΂�����o�͂��`����
	FbxIOSettings* fbxIOS = FbxIOSettings::Create(fbxManager, IOSROOT);	// ���ʂȗ��R���Ȃ�����IOSROOT���w��
	fbxManager->SetIOSettings(fbxIOS);

	// �C���|�[�^�𐶐�
	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
	bool result = fbxImporter->Initialize(fbxFilename, -1, fbxManager->GetIOSettings());	// -1�Ńt�@�C���t�H�[�}�b�g��������
	_ASSERT_EXPR_A(result, "FbxImporter::Initialize() : Failed!!\n");

	// Scene�I�u�W�F�N�g��FBX�t�@�C�����̏��𗬂�����
	FbxScene* fbxScene = FbxScene::Create(fbxManager, "scene");
	fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();	// �V�[���𗬂����񂾂�Importer�͉������OK

	// �A�j���[�V�����ǂݍ���
	char name[256];
	::_splitpath_s(filename, nullptr, 0, nullptr, 0, name, 256, nullptr, 0);

	CreateAnimations(fbxScene, name);

	// �}�l�[�W�����
	fbxManager->Destroy();		// �֘A���邷�ׂẴI�u�W�F�N�g����������
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
	// ���_�o�b�t�@
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

	// �C���f�b�N�X�o�b�t�@
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
	//�}�l�[�W���[���\������
	FbxManager* manager = FbxManager::Create();

	// FBX�ɑ΂�����o�͂��`����
	manager->SetIOSettings(FbxIOSettings::Create(manager, IOSROOT));

	// �C���|�[�^�𐶐�
	FbxImporter* importer = FbxImporter::Create(manager, "");
	bool import_status = false;
	// -1�Ńt�@�C���t�H�[�}�b�g��������
	import_status = importer->Initialize(_fbx_file_name.c_str(), -1, manager->GetIOSettings());
	//_ASSERT_EXPR_A(import_status, importer->GetStatus().GetErrorString());

	//Scene�I�u�W�F�N�g��FBX�t�@�C���̏��𗬂�����
	FbxScene* scene = FbxScene::Create(manager, "scene");
	import_status = importer->Import(scene);
	//_ASSERT_EXPR_A(import_status, importer->GetStatus().GetErrorString());
	//Scene�𗬂����񂾂�Importor�͉������
	importer->Destroy();

	//�W�I���g�����O�p������
	fbxsdk::FbxGeometryConverter geometry_converter(manager);
	geometry_converter.Triangulate(scene, true);
	geometry_converter.RemoveBadPolygonsFromMeshes(scene);

	// �f�B���N�g���p�X�擾
	char dirname[256];
	::_splitpath_s(_fbx_file_name.c_str(), nullptr, 0, dirname, 256, nullptr, 0, nullptr, 0);

	// ���f���\�z
	FbxNode* fbx_root_node = scene->GetRootNode();

	CreateMaterials(_device, dirname, scene, has_fbm);
	CreateNodes(fbx_root_node, -1);
	CreateMeshes(_device, fbx_root_node);

	char name[256];
	::_splitpath_s(_fbx_file_name.c_str(), nullptr, 0, nullptr, 0, name, 256, nullptr, 0);
	CreateAnimations(scene,name);

	// �}�l�[�W�����
	manager->Destroy();
	// �֘A���邷�ׂẴI�u�W�F�N�g����������

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
					assert(0 && "�e�N�X�`���摜�ǂݍ��ݎ��s");
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

	// �ċA�I�Ɏq�m�[�h����������
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

	// �ċA�I�Ɏq�m�[�h����������
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

	// �T�u�Z�b�g�̃}�e���A���ݒ�
	for (int fbxMaterialIndex = 0; fbxMaterialIndex < fbxMaterialCount; ++fbxMaterialIndex)
	{
		const FbxSurfaceMaterial* fbxSurfaceMaterial = _root_node->GetMaterial(fbxMaterialIndex);

		s_subset& subset = mesh.subsets.at(fbxMaterialIndex);
		subset.material_index = FindMaterialIndex(_root_node->GetScene(), fbxSurfaceMaterial);
		subset.material = &materials.at(subset.material_index);
	}

	// �T�u�Z�b�g�̒��_�C���f�b�N�X�͈͐ݒ�
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

	// ���_�e���̓f�[�^
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
				return;//4�ڂ܂ł̌Ăяo�����͊i�[���邾��
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
			return;//4�ڈȍ~�͔�r���Ă��Ԃꂽ����0�Ԗ�(�e���l���ő�̂��̂ɉ��Z����)

		}
	};
	// ���_�e���̓f�[�^�𒊏o����
	std::vector<BoneInfluence> boneInfluences;
	{
		boneInfluences.resize(fbxControlPointsCount);

		FbxAMatrix fbxGeometricTransform(
			_root_node->GetGeometricTranslation(FbxNode::eSourcePivot),
			_root_node->GetGeometricRotation(FbxNode::eSourcePivot),
			_root_node->GetGeometricScaling(FbxNode::eSourcePivot)
		);

		// �X�L�j���O�ɕK�v�ȏ����擾����
		int fbxDeformerCount = _fbx_mesh->GetDeformerCount(FbxDeformer::eSkin);
		for (int fbxDeformerIndex = 0; fbxDeformerIndex < fbxDeformerCount; ++fbxDeformerIndex)
		{
			FbxSkin* fbxSkin = static_cast<FbxSkin*>(_fbx_mesh->GetDeformer(fbxDeformerIndex, FbxDeformer::eSkin));

			int fbxClusterCount = fbxSkin->GetClusterCount();
			for (int fbxClusterIndex = 0; fbxClusterIndex < fbxClusterCount; ++fbxClusterIndex)
			{
				FbxCluster* fbxCluster = fbxSkin->GetCluster(fbxClusterIndex);

				// ���_�e���̓f�[�^�𒊏o����
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

				// �{�[���ϊ��s��p�̋t�s��̌v�Z������
				{
					// ���b�V����Ԃ��烏�[���h��Ԃւ̕ϊ��s��
					FbxAMatrix fbxMeshSpaceTransform;
					fbxCluster->GetTransformMatrix(fbxMeshSpaceTransform);

					// �{�[����Ԃ��烏�[���h��Ԃւ̕ϊ��s��
					FbxAMatrix fbxBoneSpaceTransform;
					fbxCluster->GetTransformLinkMatrix(fbxBoneSpaceTransform);

					// �{�[���t�s����v�Z����
					FbxAMatrix fbxInverseTransform = fbxBoneSpaceTransform.Inverse() * fbxMeshSpaceTransform * fbxGeometricTransform;

					FLOAT4X4 inverseTransform = FbxAMatrixToFloat4x4Ex(fbxInverseTransform);
					mesh.inverseTransforms.emplace_back(inverseTransform);

					int nodeIndex = FindNodeIndex(fbxCluster->GetLink()->GetName());
					mesh.node_indices.emplace_back(nodeIndex);
				}
			}
		}
	}

	// UV�Z�b�g��
	FbxStringList fbxUVSetNames;
	_fbx_mesh->GetUVSetNames(fbxUVSetNames);

	// ���_�f�[�^
	mesh.vertices.resize(fbxPolygonCount * 3);
	mesh.indices.resize(fbxPolygonCount * 3);

	int vertexCount = 0;
	const FbxVector4* fbxControlPoints = _fbx_mesh->GetControlPoints();
	for (int fbxPolygonIndex = 0; fbxPolygonIndex < fbxPolygonCount; ++fbxPolygonIndex)
	{
		// �|���S���ɓK�p����Ă���}�e���A���C���f�b�N�X���擾����
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
	// ���ׂẴA�j���[�V���������擾
	FbxArray<FbxString*> fbxAnimStackNames;
	_fbx_scene->FillAnimStackNameArray(fbxAnimStackNames);

	int fbxAnimationCount = fbxAnimStackNames.Size();
	for (int fbxAnimationIndex = 0; fbxAnimationIndex < fbxAnimationCount; ++fbxAnimationIndex)
	{
		animations.emplace_back(s_animation());
		s_animation& animation = animations.back();

		// �A�j���[�V�����f�[�^�̃T���v�����O�ݒ�
		FbxTime::EMode fbxTimeMode = _fbx_scene->GetGlobalSettings().GetTimeMode();
		FbxTime fbxFrameTime;
		fbxFrameTime.SetTime(0, 0, 0, 1, 0, fbxTimeMode);

		float samplingRate = static_cast<float>(fbxFrameTime.GetFrameRate(fbxTimeMode));
		float samplingTime = 1.0f / samplingRate;

		FbxString* fbxAnimStackName = fbxAnimStackNames.GetAt(fbxAnimationIndex);
		FbxAnimStack* fbxAnimStack = _fbx_scene->FindMember<FbxAnimStack>(fbxAnimStackName->Buffer());

		// �Đ�����A�j���[�V�������w�肷��B
		_fbx_scene->SetCurrentAnimationStack(fbxAnimStack);

		// �A�j���[�V�����̍Đ��J�n���ԂƍĐ��I�����Ԃ��擾����
		FbxTakeInfo* fbxTakeInfo = _fbx_scene->GetTakeInfo(fbxAnimStackName->Buffer());
		FbxTime fbxStartTime = fbxTakeInfo->mLocalTimeSpan.GetStart();
		FbxTime fbxEndTime = fbxTakeInfo->mLocalTimeSpan.GetStop();

		// ���o����f�[�^��60�t���[����ŃT���v�����O����
		FbxTime fbxSamplingStep;
		fbxSamplingStep.SetTime(0, 0, 1, 0, 0, fbxTimeMode);
		fbxSamplingStep = static_cast<FbxLongLong>(fbxSamplingStep.Get() * samplingTime);

		int startFrame = static_cast<int>(fbxStartTime.Get() / fbxSamplingStep.Get());
		int endFrame = static_cast<int>(fbxEndTime.Get() / fbxSamplingStep.Get());
		int frameCount = static_cast<int>((fbxEndTime.Get() - fbxStartTime.Get()) / fbxSamplingStep.Get());

		// �A�j���[�V�����̑ΏۂƂȂ�m�[�h��񋓂���
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

		// �A�j���[�V������
		char animationName[256];
		::sprintf_s(animationName, "%s/%s", name, fbxAnimStackName->Buffer());
		animation.name = animationName;

		// �A�j���[�V�����f�[�^�𒊏o����
		animation.seconds_length = frameCount * samplingTime;
		animation.keyframes.resize(frameCount + 1);

		float seconds = 0.0f;
		s_key_frame* keyframe = animation.keyframes.data();
		size_t fbxNodeCount = fbxNodes.size();
		FbxTime fbxCurrentTime = fbxStartTime;
		for (FbxTime fbxCurrentTime = fbxStartTime; fbxCurrentTime < fbxEndTime; fbxCurrentTime += fbxSamplingStep, ++keyframe)
		{
			// �L�[�t���[�����̎p���f�[�^�����o���B
			keyframe->seconds = seconds;
			keyframe->node_keys.resize(fbxNodeCount);
			for (size_t fbxNodeIndex = 0; fbxNodeIndex < fbxNodeCount; ++fbxNodeIndex)
			{
				s_node_key_data& keyData = keyframe->node_keys.at(fbxNodeIndex);
				FbxNode* fbxNode = fbxNodes.at(fbxNodeIndex);

				// �w�莞�Ԃ̃��[�J���s�񂩂�X�P�[���l�A��]�l�A�ړ��l�����o���B
				const FbxAMatrix& fbxLocalTransform = fbxNode->EvaluateLocalTransform(fbxCurrentTime);

				keyData.scale = FbxDouble4ToFloat3(fbxLocalTransform.GetS());
				keyData.rotate = FbxDouble4ToFloat4(fbxLocalTransform.GetQ());
				keyData.translate = FbxDouble4ToFloat3(fbxLocalTransform.GetT());
			}
			seconds += samplingTime;
		}
	}

	// ��n��
	for (int i = 0; i < fbxAnimationCount; i++)
	{
		delete fbxAnimStackNames[i];
	}
}
void ModelResource::LoadAnimations(FbxScene* fbxScene, const char* name)
{
	// ���ׂẴA�j���[�V���������擾
	FbxArray<FbxString*> fbxAnimStackNames;
	fbxScene->FillAnimStackNameArray(fbxAnimStackNames);

	int fbxAnimationCount = fbxAnimStackNames.Size();
	for (int fbxAnimationIndex = 0; fbxAnimationIndex < fbxAnimationCount; ++fbxAnimationIndex)
	{
		animations.emplace_back(s_animation());
		s_animation& animation = animations.back();

		// �A�j���[�V�����f�[�^�̃T���v�����O�ݒ�
		FbxTime::EMode fbxTimeMode = fbxScene->GetGlobalSettings().GetTimeMode();
		FbxTime fbxFrameTime;
		fbxFrameTime.SetTime(0, 0, 0, 1, 0, fbxTimeMode);

		float samplingRate = static_cast<float>(fbxFrameTime.GetFrameRate(fbxTimeMode));
		float samplingTime = 1.0f / samplingRate;

		FbxString* fbxAnimStackName = fbxAnimStackNames.GetAt(fbxAnimationIndex);
		FbxAnimStack* fbxAnimStack = fbxScene->FindMember<FbxAnimStack>(fbxAnimStackName->Buffer());

		// �Đ�����A�j���[�V�������w�肷��B
		fbxScene->SetCurrentAnimationStack(fbxAnimStack);

		// �A�j���[�V�����̍Đ��J�n���ԂƍĐ��I�����Ԃ��擾����
		FbxTakeInfo* fbxTakeInfo = fbxScene->GetTakeInfo(fbxAnimStackName->Buffer());
		FbxTime fbxStartTime = fbxTakeInfo->mLocalTimeSpan.GetStart();
		FbxTime fbxEndTime = fbxTakeInfo->mLocalTimeSpan.GetStop();

		// ���o����f�[�^��60�t���[����ŃT���v�����O����
		FbxTime fbxSamplingStep;
		fbxSamplingStep.SetTime(0, 0, 1, 0, 0, fbxTimeMode);
		fbxSamplingStep = static_cast<FbxLongLong>(fbxSamplingStep.Get() * samplingTime);

		int startFrame = static_cast<int>(fbxStartTime.Get() / fbxSamplingStep.Get());
		int endFrame = static_cast<int>(fbxEndTime.Get() / fbxSamplingStep.Get());
		int frameCount = static_cast<int>((fbxEndTime.Get() - fbxStartTime.Get()) / fbxSamplingStep.Get());

		// �A�j���[�V�����̑ΏۂƂȂ�m�[�h��񋓂���
		std::vector<FbxNode*> fbxNodes;
#if 0
		// �m�[�h�����r���đΏۃm�[�h��񋓂���
		// �����O���d�����Ă���Ǝ��s����ꍇ������
		FbxNode* fbxRootNode = fbxScene->GetRootNode();
		for (Node& node : nodes)
		{
			FbxNode* fbxNode = fbxRootNode->FindChild(node.name.c_str(), true, true);
			fbxNodes.emplace_back(fbxNode);
		}
#else
		// �m�[�h�̊��S�p�X���r���đΏۃm�[�h��񋓂���i�d���j
		// ���K�����f���ƃA�j���[�V������FBX�̃m�[�h�c���[�\������v���Ă���K�v������
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
		// �A�j���[�V������
		char animationName[256];
		::sprintf_s(animationName, "%s/%s", name, fbxAnimStackName->Buffer());
		animation.name = animationName;

		// �A�j���[�V�����f�[�^�𒊏o����
		animation.seconds_length = frameCount * samplingTime;
		animation.keyframes.resize(frameCount + 1);

		float seconds = 0.0f;
		s_key_frame* keyframe = animation.keyframes.data();
		size_t fbxNodeCount = fbxNodes.size();
		FbxTime fbxCurrentTime = fbxStartTime;
		for (FbxTime fbxCurrentTime = fbxStartTime; fbxCurrentTime < fbxEndTime; fbxCurrentTime += fbxSamplingStep, ++keyframe)
		{
			// �L�[�t���[�����̎p���f�[�^�����o���B
			keyframe->seconds = seconds;
			keyframe->node_keys.resize(fbxNodeCount);
			for (size_t fbxNodeIndex = 0; fbxNodeIndex < fbxNodeCount; ++fbxNodeIndex)
			{
				s_node_key_data& keyData = keyframe->node_keys.at(fbxNodeIndex);
				FbxNode* fbxNode = fbxNodes.at(fbxNodeIndex);
				if (fbxNode == nullptr)
				{
					// �A�j���[�V�����Ώۂ̃m�[�h���Ȃ������̂Ń_�~�[�f�[�^��ݒ�
					s_node& node = nodes.at(fbxNodeIndex);
					keyData.scale = node.scale;
					keyData.rotate = node.rotate;
					keyData.translate = node.translate;
				}
				else if (fbxNodeIndex == rootMotionNodeIndex)
				{
					// ���[�g���[�V�����͖�������
					s_node& node = nodes.at(fbxNodeIndex);
					keyData.scale = DirectX::XMFLOAT3(1, 1, 1);
					keyData.rotate = DirectX::XMFLOAT4(0, 0, 0, 1);
					keyData.translate = DirectX::XMFLOAT3(0, 0, 0);
				}
				else
				{
					// �w�莞�Ԃ̃��[�J���s�񂩂�X�P�[���l�A��]�l�A�ړ��l�����o���B
					const FbxAMatrix& fbxLocalTransform = fbxNode->EvaluateLocalTransform(fbxCurrentTime);

					keyData.scale = FbxDouble4ToFloat3(fbxLocalTransform.GetS());
					keyData.rotate = FbxDouble4ToFloat4(fbxLocalTransform.GetQ());
					keyData.translate = FbxDouble4ToFloat3(fbxLocalTransform.GetT());
				}
			}
			seconds += samplingTime;
		}
	}

	// ��n��
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



