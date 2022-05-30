#ifndef RESOURCEMANAGER_H_
#define RESOURCEMANAGER_H_

//�C���N���[�h
#include <fstream>
#include <d3d11.h>
#include <string>
#include <WICTextureLoader.h>
#include "DirectXTex.h"

inline bool ReadBinaryFile(const char* _filename, char** _blob, unsigned int& _size)
{
	std::ifstream input_file(_filename, std::ifstream::binary);
	input_file.seekg(0, std::ifstream::end);
	_size = static_cast<int>(input_file.tellg());
	input_file.seekg(0, std::ifstream::beg);
	*_blob = new char[_size];
	input_file.read(*_blob, _size);
	return true;
}


//���\�[�X�Ǘ��N���X: ��Ƀ��f���̃e�N�X�`�����[�h�Ŏg�p�@��y�ɋ����Ă����������

class resourceManager
{
private:
	//�N���X���Ŏg�p����\����
	struct s_resource_shader_resource_views//shaderResourceView�p
	{
		//variable(�ϐ�)
		int i_ref_num;//���t�@�����X�̊i�[�C���f�b�N�X
		std::wstring path;//���t�@�����X�̃t�@�C����
		 ID3D11ShaderResourceView* shader_resource_view;//shaderResourceView���g���܂킷���߂̕ϐ�

		//function
		//constractor
		s_resource_shader_resource_views() : i_ref_num(0), shader_resource_view(nullptr) { path[0] = '\0'; }
		//�ۑ����Ă���shaderResourceView���������֐�
		void Release(bool _force = false);
	};

	struct s_resource_vertex_shaders//vertexShader�p
	{
		//variable(�ϐ�)
		int i_ref_num;//���t�@�����X�̊i�[�C���f�b�N�X
		std::wstring path;//���t�@�����X�̃t�@�C����
		ID3D11VertexShader* vertex_shader;//vertexShader���g���܂킷�p�̕ϐ�
		ID3D11InputLayout* layout;//�g���܂킷�p��inputLayout

		//fanction
		//constractor
		s_resource_vertex_shaders() : i_ref_num(0), vertex_shader(nullptr), layout(nullptr) { path[0] = '\0'; }

		//����֐�
		void Release(bool _force = false);

	};

	struct s_resource_pixel_shaders//pixelShader�p
	{
		//variable(�ϐ�)
		int i_ref_num;//���t�@�����X�̊i�[�C���f�b�N�X
		std::wstring path;//���t�@�����X�̃t�@�C����
		ID3D11PixelShader* pixel_shader;//pixelShader���g���܂킷�p

		//function
		//constructor
		s_resource_pixel_shaders() : i_ref_num(0), pixel_shader(nullptr) { path[0] = '\0'; }

		//����֐�
		void Release(bool _force = false);
	};

	struct s_resource_compute_shaders//conputeShader�p
	{
		//variable(�ϐ�)
		int i_ref_num;//���t�@�����X�̊i�[�C���f�b�N�X
		std::wstring path;//���t�@�����X�̃t�@�C����
		ID3D11ComputeShader* compute_shader;//computeShader���g���܂킷�p

		//function
		//constructor
		s_resource_compute_shaders() : i_ref_num(0), compute_shader(nullptr) { path[0] = '\0'; }

		//����֐�
		void Release(bool _force = false);
	};

	struct s_resource_geometry_shaders//conputeShader�p
	{
		//variable(�ϐ�)
		int i_ref_num;//���t�@�����X�̊i�[�C���f�b�N�X
		std::wstring path;//���t�@�����X�̃t�@�C����
		ID3D11GeometryShader* geometry_shader;//computeShader���g���܂킷�p

		//function
		//constructor
		s_resource_geometry_shaders() : i_ref_num(0), geometry_shader(nullptr) { path[0] = '\0'; }

		//����֐�
		void Release(bool _force = false);
	};

	enum class e_resource_kind
	{
		SRV,
		Vertex,
		Pixel,
		Compute,
		Geometry,
	};

	//variable(�ϐ�)
	static const int resource_max = 256;//���t�@�����X�̏��
	static s_resource_shader_resource_views	shader_resource_views[resource_max];
	static s_resource_vertex_shaders vertex_shaders[resource_max];
	static s_resource_pixel_shaders	pixel_shaders[resource_max];
	static s_resource_compute_shaders	compute_shaders[resource_max];
	static s_resource_geometry_shaders	geometry_shaders[resource_max];

	//function
	//�R���X�g���N�^
	resourceManager() {}
	//�f�X�g���N�^
	~resourceManager() 
	{}

public:
	//function
	//�V���O���g���p�֐�
	static resourceManager* GetInstance()
	{
		static resourceManager instance;

		return &instance;
	}

	//�V�F�[�_�[���\�[�X�r���[�̃��[�h
	static bool LoadShaderResourceView(ID3D11Device* _device, std::wstring _file_name,
		ID3D11ShaderResourceView** _shader_resource_view, D3D11_TEXTURE2D_DESC* _tex_desc, ID3D11Texture2D* _tex2d, bool _use_srgb = true)

	{
		//�t���p�X����t�@�C�����݂̂�؂���������
		std::wstring only_file_name{};
		//�������؂��邽�߂Ɏg�p
		size_t slash_start = _file_name.find_last_of('/');
		//�������؂��邽�߂Ɏg�p
		size_t back_slash_start = _file_name.find_last_of('\\');
		s_resource_shader_resource_views* p;

		if (slash_start != std::wstring::npos)
		{
			only_file_name = _file_name.substr(slash_start);
		}

		if (back_slash_start != std::wstring::npos)
		{
			only_file_name = _file_name.substr(back_slash_start);
		}

		HRESULT hr = 0;
		int no = -1;
		s_resource_shader_resource_views* find = nullptr;
		ID3D11Resource* resource = nullptr;
		bool use_SRGB = false;
			//�f�[�^�T��
		for (int n = 0; n < resource_max; n++)
		{
			 p = &shader_resource_views[n];//	�G�C���A�X

			//�f�[�^���Ȃ������炻�̗̈�Ƀf�[�^�����
			if (p->i_ref_num == 0)
			{
				//�f�[�^�쐬���̃C���f�b�N�X���Z�b�g
				if (no == -1) no = n;
				continue;
			}

				//�t�@�C���p�X���Ⴄ�Ȃ疳��
			if (wcscmp(p->path.c_str(), _file_name.c_str()) != 0) continue;

				//�����t�@�C�������݂����炻����g�p����
			find = p;
			p->shader_resource_view->GetResource(&resource);
			break;
		}

			//�f�[�^��������Ȃ������̂ŐV�K�ǂݍ���
		if (!find)
		{
			//�摜�ǂݍ��݂ɕK�v�ȕϐ�
			DirectX::TexMetadata metadata = DirectX::TexMetadata();
			DirectX::ScratchImage image;

			//�g���q�����ʂ���
			std::wstring dummy = only_file_name;
			size_t start = dummy.find_last_of(L".");
			std::wstring extension = dummy.substr(start, dummy.size() - start);

			//�󂢂Ă���X���b�g�ɓǂݍ���
		   p = &shader_resource_views[no];

			//�摜��SRGB�œǂݍ��ނ����߂�
			int use_state = DirectX::WIC_LOADER_DEFAULT;
			if (_use_srgb)
				use_state = DirectX::WIC_LOADER_FORCE_SRGB;

			if (L".png" == extension || L".PNG" == extension)//png�̏ꍇ
			{
				if (FAILED(DirectX::CreateWICTextureFromFileEx
				(_device, _file_name.c_str(), 0,
					D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, use_state,
					&resource, &p->shader_resource_view)))
					return false;
			}
			else
			{
				if (L".jpeg" == extension || L".jpg" == extension || L".jpe" == extension || L".gif" == extension || L".tiff" == extension || L".tif" == extension || L".bmp" == extension)
				{
					if (FAILED(DirectX::LoadFromWICFile(_file_name.c_str(), 0, &metadata, image)))
						return false;
				}
				else if (L".dds" == extension || L".DDS" == extension)
				{
					if (FAILED(DirectX::LoadFromDDSFile(_file_name.c_str(), 0, &metadata, image)))
						return false;
				}
				else if (L".tga" == extension || L".vda" == extension || L".icb" == extension || L".vst" == extension || L".TGA" == extension)
				{
					use_SRGB = true;

					if (FAILED(DirectX::LoadFromTGAFile(_file_name.c_str(), &metadata, image)))
					{
						return false;
					}

				
				}

				hr = DirectX::CreateShaderResourceViewEx
				(_device, image.GetImages(), image.GetImageCount(), metadata, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, D3D11_RESOURCE_MISC_TEXTURECUBE, use_state, &(p->shader_resource_view));
			
				p->shader_resource_view->GetResource(&resource);
			}
			find = p;
			p->path = _file_name;
			//p->shader_resource_view->GetResource(&resource);
		}

		//	�ŏI����
		if (_tex2d != nullptr)
		{
			resource->QueryInterface(&_tex2d);
			*_shader_resource_view = find->shader_resource_view;
			_tex2d->GetDesc(_tex_desc);
			find->i_ref_num++;
			resource->Release();
		}
		else
		{
			ID3D11Texture2D* tex2d;
			resource->QueryInterface(&tex2d);
			*_shader_resource_view = find->shader_resource_view;
			tex2d->GetDesc(_tex_desc);
			find->i_ref_num++;
			tex2d->Release();
			resource->Release();
		
		}
		return true;
	}

	//���_�V�F�[�_�[�̃��[�h
	static bool LoadVertexShader(
		ID3D11Device* _device, std::string _cso_file_name,
		D3D11_INPUT_ELEMENT_DESC* _input_element_desc, int _num_elements,
		ID3D11VertexShader** _vertex_shader, ID3D11InputLayout** _input_layout)

	{
		*_vertex_shader = nullptr;
		*_input_layout = nullptr;

		s_resource_vertex_shaders* find = nullptr;
		int no = -1;

		//	const char * -> wchar_t *
		std::wstring filename(_cso_file_name.begin(), _cso_file_name.end());

		//	�f�[�^����
		for (int n = 0; n < resource_max; n++) {
			s_resource_vertex_shaders* p = &vertex_shaders[n];

			if (p->i_ref_num == 0) {
				if (no == -1) no = n;
				continue;
			}

			if (wcscmp(p->path.c_str(), filename.c_str()) != 0) continue;

			//	�����t�@�C�������݂���
			find = p;
			break;
		}

		//	�f�[�^��������Ȃ�����
		if (!find) {
			s_resource_vertex_shaders* p = &vertex_shaders[no];
			char* blob;
			unsigned int size;

			//	�R���p�C���ς݃s�N�Z���V�F�[�_�[�I�u�W�F�N�g�̓ǂݍ���
			if (!ReadBinaryFile(_cso_file_name.c_str(), &blob, size)) return false;

			//	���_�V�F�[�_�[�I�u�W�F�N�g�̐���
			if (FAILED(_device->CreateVertexShader(blob, size, nullptr, &p->vertex_shader)))
				return false;

			HRESULT hr = _device->CreateInputLayout(_input_element_desc, _num_elements, blob, size, &p->layout);
			//	���̓��C�A�E�g�̍쐬
			delete[] blob;
			if (FAILED(hr)) return false;

			//	�V�K�f�[�^�̕ۑ�
			find = p;
			find->path = filename;
		}

		//	�ŏI����
		*_vertex_shader = find->vertex_shader;
		*_input_layout = find->layout;
		find->i_ref_num++;

		return true;
	}

	//�s�N�Z���V�F�[�_�[�̃��[�h
	static bool LoadPixelShader(ID3D11Device* _device, std::string _cso_file_name, ID3D11PixelShader** _pixel_shader)
	{
		//�����̃s�N�Z���V�F�[�_�[�����Z�b�g����
		*_pixel_shader = nullptr;
		s_resource_pixel_shaders* find = nullptr;
		//�f�[�^�����̍ۂɎg�p
		int no = -1;

		//char�^����w_char�^�ɕϊ�
		std::wstring filename(_cso_file_name.begin(), _cso_file_name.end());

		//	�f�[�^����
		for (int n = 0; n < resource_max; n++) {
			s_resource_pixel_shaders* p = &pixel_shaders[n];
			if (p->i_ref_num == 0) {
				if (no == -1) no = n;
				continue;
			}
			if (wcscmp(p->path.c_str(), filename.c_str()) != 0) continue;

			//	�����t�@�C�������݂���
			find = p;
			break;
		}

		//	�V�K�쐬
		if (!find) {
			s_resource_pixel_shaders* p = &pixel_shaders[no];
			char* blob;
			unsigned int size;

			//	�R���p�C���ς݃s�N�Z���V�F�[�_�[�I�u�W�F�N�g�̓ǂݍ���
			if (!ReadBinaryFile(_cso_file_name.c_str(), &blob, size)) return false;

			HRESULT hr = _device->CreatePixelShader(blob, size, nullptr, &p->pixel_shader);
			delete[] blob;
			if (FAILED(hr)) return false;

			find = p;
			find->path = filename;
		}

		//	�ŏI�����i�Q�Ɠn���Ńf�[�^��n���j
		*_pixel_shader = find->pixel_shader;
		find->i_ref_num++;

		return true;
	}

	//�R���s���[�g�V�F�[�_�[�̃��[�h
	static bool LoadComputeShader(ID3D11Device* _device, std::string _cso_file_name, ID3D11ComputeShader** _compute_shader)
	{
		//�����̃R���s���[�g�V�F�[�_�[�����Z�b�g����
		*_compute_shader = nullptr;
		s_resource_compute_shaders* find = nullptr;
		//�f�[�^�����̍ۂɎg�p
		int no = -1;

		//char�^����w_char�^�ɕϊ�
		std::wstring filename(_cso_file_name.begin(), _cso_file_name.end());

		//	�f�[�^����
		for (int n = 0; n < resource_max; n++) {
			s_resource_compute_shaders* p = &compute_shaders[n];
			if (p->i_ref_num == 0) {
				if (no == -1) no = n;
				continue;
			}
			if (wcscmp(p->path.c_str(), filename.c_str()) != 0) continue;

			//	�����t�@�C�������݂���
			find = p;
			break;
		}

		//	�V�K�쐬
		if (!find) {
			s_resource_compute_shaders* p = &compute_shaders[no];
			char* blob;
			unsigned int size;

			//	�R���p�C���ς݃R���s���[�g�V�F�[�_�[�I�u�W�F�N�g�̓ǂݍ���
			if (!ReadBinaryFile(_cso_file_name.c_str(), &blob, size)) return false;

			HRESULT hr = _device->CreateComputeShader(blob, size, nullptr, &p->compute_shader);
			delete[] blob;
			if (FAILED(hr)) return false;

			find = p;
			find->path = filename;
		}

		//	�ŏI�����i�Q�Ɠn���Ńf�[�^��n���j
		*_compute_shader = find->compute_shader;
		find->i_ref_num++;

		return true;
	}

	//�W�I���g���[�V�F�[�_�[�̃��[�h
	static bool LoadGeometryShader(ID3D11Device* _device, std::string _cso_file_name, ID3D11GeometryShader** _geometry_shader)
	{
		//�����̃R���s���[�g�V�F�[�_�[�����Z�b�g����
		*_geometry_shader = nullptr;
		s_resource_geometry_shaders* find = nullptr;
		//�f�[�^�����̍ۂɎg�p
		int no = -1;

		//char�^����w_char�^�ɕϊ�
		std::wstring filename(_cso_file_name.begin(), _cso_file_name.end());

		//	�f�[�^����
		for (int n = 0; n < resource_max; n++) {
			s_resource_geometry_shaders* p = &geometry_shaders[n];
			if (p->i_ref_num == 0) {
				if (no == -1) no = n;
				continue;
			}
			if (wcscmp(p->path.c_str(), filename.c_str()) != 0) continue;

			//	�����t�@�C�������݂���
			find = p;
			break;
		}

		//	�V�K�쐬
		if (!find) {
			s_resource_geometry_shaders* p = &geometry_shaders[no];
			char* blob;
			unsigned int size;

			//	�R���p�C���ς݃R���s���[�g�V�F�[�_�[�I�u�W�F�N�g�̓ǂݍ���
			if (!ReadBinaryFile(_cso_file_name.c_str(), &blob, size)) return false;

			HRESULT hr = _device->CreateGeometryShader(blob, size, nullptr, &p->geometry_shader);
			delete[] blob;
			if (FAILED(hr)) return false;

			find = p;
			find->path = filename;
		}

		//	�ŏI�����i�Q�Ɠn���Ńf�[�^��n���j
		*_geometry_shader = find->geometry_shader;
		find->i_ref_num++;

		return true;
	}

	//�S���\�[�X�̉���p�֐�
	static void Release()
	{
		for (int i = 0; i < resource_max; i++) {
			shader_resource_views[i].Release(true);
			vertex_shaders[i].Release(true);
			pixel_shaders[i].Release(true);
			compute_shaders[i].Release(true);
			geometry_shaders[i].Release(true);
		}
	}


	//�w��V�F�[�_�[���\�[�X�r���[�̉���֐�
	static void ReleaseShaderResourceView(ID3D11ShaderResourceView* _shader_resource_view)
	{
		if (!_shader_resource_view) return;
		for (int n = 0; n < resource_max; n++) {
			s_resource_shader_resource_views* p = &shader_resource_views[n];

			//	�f�[�^�������Ȃ疳��
			if (p->i_ref_num == 0) continue;

			//	�f�[�^���Ⴄ�Ȃ疳��
			if (_shader_resource_view != p->shader_resource_view) continue;

			//	�f�[�^�����݂���
			p->Release();
			break;
		}
	}

	//�w��̒��_�V�F�[�_�[�̉��
	static void ReleaseVertexShader(ID3D11VertexShader* _vertex_shader, ID3D11InputLayout* _input_layout)
	{
		if (!_vertex_shader) return;
		if (!_input_layout) return;

		for (int n = 0; n < resource_max; n++) {
			s_resource_vertex_shaders* p = &vertex_shaders[n];
			if (p->i_ref_num == 0) continue;
			if (_vertex_shader != p->vertex_shader) continue;
			if (_input_layout != p->layout) continue;

			//	�f�[�^�����݂���
			p->Release();
			break;
		}
	}

	//�w��̃s�N�Z���V�F�[�_�[�̉��
	static void ReleasePixelShader(ID3D11PixelShader* _pixel_shader)
	{
		if (!_pixel_shader) return;
		for (int n = 0; n < resource_max; n++)
		{
			s_resource_pixel_shaders* p = &pixel_shaders[n];

			if (p->i_ref_num == 0) continue;

			if (_pixel_shader != p->pixel_shader) continue;

			//	�f�[�^�����݂���
			p->Release();
			break;
		}
	}

	//�w��̃R���s���[�g�V�F�[�_�[�̉��
	static void ReleaseComputeShader(ID3D11ComputeShader* _compute_shader)
	{
		if (!_compute_shader) return;
		for (int n = 0; n < resource_max; n++)
		{
			s_resource_compute_shaders* p = &compute_shaders[n];

			if (p->i_ref_num == 0) continue;

			if (_compute_shader != p->compute_shader) continue;

			//	�f�[�^�����݂���
			p->Release();
			break;
		}
	}

	//�w��̃W�I���g���[�V�F�[�_�[�̉��
	static void ReleaseGeometryShader(ID3D11GeometryShader* _geometry_shader)
	{
		if (!_geometry_shader) return;
		for (int n = 0; n < resource_max; n++)
		{
			s_resource_geometry_shaders* p = &geometry_shaders[n];

			if (p->i_ref_num == 0) continue;

			if (_geometry_shader != p->geometry_shader) continue;

			//	�f�[�^�����݂���
			p->Release();
			break;
		}
	}

};


#endif // !RESOURCEMANAGER_H_
