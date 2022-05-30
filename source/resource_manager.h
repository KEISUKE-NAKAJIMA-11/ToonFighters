#ifndef RESOURCEMANAGER_H_
#define RESOURCEMANAGER_H_

//インクルード
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


//リソース管理クラス: 主にモデルのテクスチャロードで使用　先輩に教えてもらったやり方

class resourceManager
{
private:
	//クラス内で使用する構造体
	struct s_resource_shader_resource_views//shaderResourceView用
	{
		//variable(変数)
		int i_ref_num;//リファレンスの格納インデックス
		std::wstring path;//リファレンスのファイル名
		 ID3D11ShaderResourceView* shader_resource_view;//shaderResourceViewを使いまわすための変数

		//function
		//constractor
		s_resource_shader_resource_views() : i_ref_num(0), shader_resource_view(nullptr) { path[0] = '\0'; }
		//保存しているshaderResourceViewを解放する関数
		void Release(bool _force = false);
	};

	struct s_resource_vertex_shaders//vertexShader用
	{
		//variable(変数)
		int i_ref_num;//リファレンスの格納インデックス
		std::wstring path;//リファレンスのファイル名
		ID3D11VertexShader* vertex_shader;//vertexShaderを使いまわす用の変数
		ID3D11InputLayout* layout;//使いまわす用のinputLayout

		//fanction
		//constractor
		s_resource_vertex_shaders() : i_ref_num(0), vertex_shader(nullptr), layout(nullptr) { path[0] = '\0'; }

		//解放関数
		void Release(bool _force = false);

	};

	struct s_resource_pixel_shaders//pixelShader用
	{
		//variable(変数)
		int i_ref_num;//リファレンスの格納インデックス
		std::wstring path;//リファレンスのファイル名
		ID3D11PixelShader* pixel_shader;//pixelShaderを使いまわす用

		//function
		//constructor
		s_resource_pixel_shaders() : i_ref_num(0), pixel_shader(nullptr) { path[0] = '\0'; }

		//解放関数
		void Release(bool _force = false);
	};

	struct s_resource_compute_shaders//conputeShader用
	{
		//variable(変数)
		int i_ref_num;//リファレンスの格納インデックス
		std::wstring path;//リファレンスのファイル名
		ID3D11ComputeShader* compute_shader;//computeShaderを使いまわす用

		//function
		//constructor
		s_resource_compute_shaders() : i_ref_num(0), compute_shader(nullptr) { path[0] = '\0'; }

		//解放関数
		void Release(bool _force = false);
	};

	struct s_resource_geometry_shaders//conputeShader用
	{
		//variable(変数)
		int i_ref_num;//リファレンスの格納インデックス
		std::wstring path;//リファレンスのファイル名
		ID3D11GeometryShader* geometry_shader;//computeShaderを使いまわす用

		//function
		//constructor
		s_resource_geometry_shaders() : i_ref_num(0), geometry_shader(nullptr) { path[0] = '\0'; }

		//解放関数
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

	//variable(変数)
	static const int resource_max = 256;//リファレンスの上限
	static s_resource_shader_resource_views	shader_resource_views[resource_max];
	static s_resource_vertex_shaders vertex_shaders[resource_max];
	static s_resource_pixel_shaders	pixel_shaders[resource_max];
	static s_resource_compute_shaders	compute_shaders[resource_max];
	static s_resource_geometry_shaders	geometry_shaders[resource_max];

	//function
	//コンストラクタ
	resourceManager() {}
	//デストラクタ
	~resourceManager() 
	{}

public:
	//function
	//シングルトン用関数
	static resourceManager* GetInstance()
	{
		static resourceManager instance;

		return &instance;
	}

	//シェーダーリソースビューのロード
	static bool LoadShaderResourceView(ID3D11Device* _device, std::wstring _file_name,
		ID3D11ShaderResourceView** _shader_resource_view, D3D11_TEXTURE2D_DESC* _tex_desc, ID3D11Texture2D* _tex2d, bool _use_srgb = true)

	{
		//フルパスからファイル名のみを切り取ったもの
		std::wstring only_file_name{};
		//文字列を切り取るために使用
		size_t slash_start = _file_name.find_last_of('/');
		//文字列を切り取るために使用
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
			//データ探索
		for (int n = 0; n < resource_max; n++)
		{
			 p = &shader_resource_views[n];//	エイリアス

			//データがなかったらその領域にデータを作る
			if (p->i_ref_num == 0)
			{
				//データ作成時のインデックスをセット
				if (no == -1) no = n;
				continue;
			}

				//ファイルパスが違うなら無視
			if (wcscmp(p->path.c_str(), _file_name.c_str()) != 0) continue;

				//同名ファイルが存在したらそれを使用する
			find = p;
			p->shader_resource_view->GetResource(&resource);
			break;
		}

			//データが見つからなかったので新規読み込み
		if (!find)
		{
			//画像読み込みに必要な変数
			DirectX::TexMetadata metadata = DirectX::TexMetadata();
			DirectX::ScratchImage image;

			//拡張子を識別する
			std::wstring dummy = only_file_name;
			size_t start = dummy.find_last_of(L".");
			std::wstring extension = dummy.substr(start, dummy.size() - start);

			//空いているスロットに読み込む
		   p = &shader_resource_views[no];

			//画像をSRGBで読み込むか決める
			int use_state = DirectX::WIC_LOADER_DEFAULT;
			if (_use_srgb)
				use_state = DirectX::WIC_LOADER_FORCE_SRGB;

			if (L".png" == extension || L".PNG" == extension)//pngの場合
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

		//	最終処理
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

	//頂点シェーダーのロード
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

		//	データ検索
		for (int n = 0; n < resource_max; n++) {
			s_resource_vertex_shaders* p = &vertex_shaders[n];

			if (p->i_ref_num == 0) {
				if (no == -1) no = n;
				continue;
			}

			if (wcscmp(p->path.c_str(), filename.c_str()) != 0) continue;

			//	同名ファイルが存在した
			find = p;
			break;
		}

		//	データが見つからなかった
		if (!find) {
			s_resource_vertex_shaders* p = &vertex_shaders[no];
			char* blob;
			unsigned int size;

			//	コンパイル済みピクセルシェーダーオブジェクトの読み込み
			if (!ReadBinaryFile(_cso_file_name.c_str(), &blob, size)) return false;

			//	頂点シェーダーオブジェクトの生成
			if (FAILED(_device->CreateVertexShader(blob, size, nullptr, &p->vertex_shader)))
				return false;

			HRESULT hr = _device->CreateInputLayout(_input_element_desc, _num_elements, blob, size, &p->layout);
			//	入力レイアウトの作成
			delete[] blob;
			if (FAILED(hr)) return false;

			//	新規データの保存
			find = p;
			find->path = filename;
		}

		//	最終処理
		*_vertex_shader = find->vertex_shader;
		*_input_layout = find->layout;
		find->i_ref_num++;

		return true;
	}

	//ピクセルシェーダーのロード
	static bool LoadPixelShader(ID3D11Device* _device, std::string _cso_file_name, ID3D11PixelShader** _pixel_shader)
	{
		//引数のピクセルシェーダーをリセットする
		*_pixel_shader = nullptr;
		s_resource_pixel_shaders* find = nullptr;
		//データ検索の際に使用
		int no = -1;

		//char型からw_char型に変換
		std::wstring filename(_cso_file_name.begin(), _cso_file_name.end());

		//	データ検索
		for (int n = 0; n < resource_max; n++) {
			s_resource_pixel_shaders* p = &pixel_shaders[n];
			if (p->i_ref_num == 0) {
				if (no == -1) no = n;
				continue;
			}
			if (wcscmp(p->path.c_str(), filename.c_str()) != 0) continue;

			//	同名ファイルが存在した
			find = p;
			break;
		}

		//	新規作成
		if (!find) {
			s_resource_pixel_shaders* p = &pixel_shaders[no];
			char* blob;
			unsigned int size;

			//	コンパイル済みピクセルシェーダーオブジェクトの読み込み
			if (!ReadBinaryFile(_cso_file_name.c_str(), &blob, size)) return false;

			HRESULT hr = _device->CreatePixelShader(blob, size, nullptr, &p->pixel_shader);
			delete[] blob;
			if (FAILED(hr)) return false;

			find = p;
			find->path = filename;
		}

		//	最終処理（参照渡しでデータを渡す）
		*_pixel_shader = find->pixel_shader;
		find->i_ref_num++;

		return true;
	}

	//コンピュートシェーダーのロード
	static bool LoadComputeShader(ID3D11Device* _device, std::string _cso_file_name, ID3D11ComputeShader** _compute_shader)
	{
		//引数のコンピュートシェーダーをリセットする
		*_compute_shader = nullptr;
		s_resource_compute_shaders* find = nullptr;
		//データ検索の際に使用
		int no = -1;

		//char型からw_char型に変換
		std::wstring filename(_cso_file_name.begin(), _cso_file_name.end());

		//	データ検索
		for (int n = 0; n < resource_max; n++) {
			s_resource_compute_shaders* p = &compute_shaders[n];
			if (p->i_ref_num == 0) {
				if (no == -1) no = n;
				continue;
			}
			if (wcscmp(p->path.c_str(), filename.c_str()) != 0) continue;

			//	同名ファイルが存在した
			find = p;
			break;
		}

		//	新規作成
		if (!find) {
			s_resource_compute_shaders* p = &compute_shaders[no];
			char* blob;
			unsigned int size;

			//	コンパイル済みコンピュートシェーダーオブジェクトの読み込み
			if (!ReadBinaryFile(_cso_file_name.c_str(), &blob, size)) return false;

			HRESULT hr = _device->CreateComputeShader(blob, size, nullptr, &p->compute_shader);
			delete[] blob;
			if (FAILED(hr)) return false;

			find = p;
			find->path = filename;
		}

		//	最終処理（参照渡しでデータを渡す）
		*_compute_shader = find->compute_shader;
		find->i_ref_num++;

		return true;
	}

	//ジオメトリーシェーダーのロード
	static bool LoadGeometryShader(ID3D11Device* _device, std::string _cso_file_name, ID3D11GeometryShader** _geometry_shader)
	{
		//引数のコンピュートシェーダーをリセットする
		*_geometry_shader = nullptr;
		s_resource_geometry_shaders* find = nullptr;
		//データ検索の際に使用
		int no = -1;

		//char型からw_char型に変換
		std::wstring filename(_cso_file_name.begin(), _cso_file_name.end());

		//	データ検索
		for (int n = 0; n < resource_max; n++) {
			s_resource_geometry_shaders* p = &geometry_shaders[n];
			if (p->i_ref_num == 0) {
				if (no == -1) no = n;
				continue;
			}
			if (wcscmp(p->path.c_str(), filename.c_str()) != 0) continue;

			//	同名ファイルが存在した
			find = p;
			break;
		}

		//	新規作成
		if (!find) {
			s_resource_geometry_shaders* p = &geometry_shaders[no];
			char* blob;
			unsigned int size;

			//	コンパイル済みコンピュートシェーダーオブジェクトの読み込み
			if (!ReadBinaryFile(_cso_file_name.c_str(), &blob, size)) return false;

			HRESULT hr = _device->CreateGeometryShader(blob, size, nullptr, &p->geometry_shader);
			delete[] blob;
			if (FAILED(hr)) return false;

			find = p;
			find->path = filename;
		}

		//	最終処理（参照渡しでデータを渡す）
		*_geometry_shader = find->geometry_shader;
		find->i_ref_num++;

		return true;
	}

	//全リソースの解放用関数
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


	//指定シェーダーリソースビューの解放関数
	static void ReleaseShaderResourceView(ID3D11ShaderResourceView* _shader_resource_view)
	{
		if (!_shader_resource_view) return;
		for (int n = 0; n < resource_max; n++) {
			s_resource_shader_resource_views* p = &shader_resource_views[n];

			//	データが無いなら無視
			if (p->i_ref_num == 0) continue;

			//	データが違うなら無視
			if (_shader_resource_view != p->shader_resource_view) continue;

			//	データが存在した
			p->Release();
			break;
		}
	}

	//指定の頂点シェーダーの解放
	static void ReleaseVertexShader(ID3D11VertexShader* _vertex_shader, ID3D11InputLayout* _input_layout)
	{
		if (!_vertex_shader) return;
		if (!_input_layout) return;

		for (int n = 0; n < resource_max; n++) {
			s_resource_vertex_shaders* p = &vertex_shaders[n];
			if (p->i_ref_num == 0) continue;
			if (_vertex_shader != p->vertex_shader) continue;
			if (_input_layout != p->layout) continue;

			//	データが存在した
			p->Release();
			break;
		}
	}

	//指定のピクセルシェーダーの解放
	static void ReleasePixelShader(ID3D11PixelShader* _pixel_shader)
	{
		if (!_pixel_shader) return;
		for (int n = 0; n < resource_max; n++)
		{
			s_resource_pixel_shaders* p = &pixel_shaders[n];

			if (p->i_ref_num == 0) continue;

			if (_pixel_shader != p->pixel_shader) continue;

			//	データが存在した
			p->Release();
			break;
		}
	}

	//指定のコンピュートシェーダーの解放
	static void ReleaseComputeShader(ID3D11ComputeShader* _compute_shader)
	{
		if (!_compute_shader) return;
		for (int n = 0; n < resource_max; n++)
		{
			s_resource_compute_shaders* p = &compute_shaders[n];

			if (p->i_ref_num == 0) continue;

			if (_compute_shader != p->compute_shader) continue;

			//	データが存在した
			p->Release();
			break;
		}
	}

	//指定のジオメトリーシェーダーの解放
	static void ReleaseGeometryShader(ID3D11GeometryShader* _geometry_shader)
	{
		if (!_geometry_shader) return;
		for (int n = 0; n < resource_max; n++)
		{
			s_resource_geometry_shaders* p = &geometry_shaders[n];

			if (p->i_ref_num == 0) continue;

			if (_geometry_shader != p->geometry_shader) continue;

			//	データが存在した
			p->Release();
			break;
		}
	}

};


#endif // !RESOURCEMANAGER_H_
