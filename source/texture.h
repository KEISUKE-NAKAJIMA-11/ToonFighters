#pragma once
// UNIT.04
#include <d3d11.h>
#include "Vector.h"
#include "sprite.h"

HRESULT load_texture_from_file(ID3D11Device* device,
    const wchar_t* file_name,
    ID3D11ShaderResourceView** shader_resource_view,
    D3D11_TEXTURE2D_DESC* texture2d_desc);
HRESULT make_dummy_texture(ID3D11Device* device, ID3D11ShaderResourceView** shader_resource_view);

HRESULT load_texture_from_file_model(ID3D11Device* device,
    const wchar_t* file_name,
    ID3D11ShaderResourceView** shader_resource_view);

void combine_resource_path(wchar_t(&combined_resource_path)[256], const wchar_t* referrer_filename, const wchar_t* referent_filename);

// �e�N�X�`�����[�h�f�[�^
struct LoadTexture
{
    int             texNum;         // �e�N�X�`���ԍ�
    const wchar_t* fileName;       // �t�@�C����
    UINT            maxInstance;    // �ő吔�iSpriteBatch�g�p���j
};

//==========================================================================
//
//      �e�N�X�`���N���X
//
//==========================================================================
class Texture
{
private:
    sprite_batch* sprBat;    // �X�v���C�g�o�b�`

public:
    Texture();
    ~Texture();

    bool load(ID3D11Device*, const wchar_t*, size_t);
    void release();

    void begin(ID3D11DeviceContext*);
    void end(ID3D11DeviceContext*);

    void draw(
        float, float, float, float,
        float, float, float, float,
        float, float, float,
        float, float, float, float,
        bool);

    void draw(
        const VECTOR2&, const VECTOR2&,
        const VECTOR2&, const VECTOR2&,
        const VECTOR2&, float,
        const VECTOR4&, bool);

    bool isEmpty();	// �e�N�X�`���f�[�^�̗L���itrue : ��, false : �L�j
};






//class TextureShader
//{
//protected:
//    ComPtr<ID3D11ShaderResourceView> ShaderResourceView = nullptr;
//    ComPtr<ID3D11RenderTargetView> RenderTargetView = nullptr;
//    ComPtr<ID3D11DepthStencilView> DepthStencilView = nullptr;
//
//    //// Cube�Œǉ�
//    //ComPtr<ID3D11Texture2D> Texture2D = nullptr;
//
//    // �e�N�X�`�����
//    D3D11_TEXTURE2D_DESC texture2d_desc = {};
//    void HDR_CheckHeader(FILE* fp);
//    bool HDR_ReadLine(BYTE* scanline, FILE* fp);
//    bool HDR_ReadPixels(FILE* fp, float* buf);
//
//public:
//    TextureShader();
//    virtual ~TextureShader();
//    bool Load(const wchar_t* filename = nullptr);
//    bool LoadMipMap(const wchar_t* filename = nullptr);
//    bool LoadHDR(const wchar_t* filename);
//
//    void Set(UINT Slot = 0, BOOL flg = TRUE);
//
//    UINT GetWidth() { return texture2d_desc.Width; }
//    UINT GetHeight() { return texture2d_desc.Height; }
//    bool Create(u_int width, u_int height, DXGI_FORMAT format);
//    bool CreateMipMap(u_int width, u_int height, DXGI_FORMAT format);
//    // �[�x�X�e���V���o�b�t�@�쐬
//    bool CreateDepth(u_int width, u_int height, DXGI_FORMAT format = DXGI_FORMAT_R24G8_TYPELESS);
//
//
//    ID3D11RenderTargetView* GetRenderTargetView() { return RenderTargetView.Get(); }
//    ID3D11DepthStencilView* GetDepthStencilView() { return DepthStencilView.Get(); }
//
//    //CubeMap�p
//    bool CreateCube(u_int width, u_int height, DXGI_FORMAT format, int miplevel = 1);
//    bool CreateCubeDepthStencil(u_int width, u_int height);
//
//
//};
//
//
//






//==========================================================================
//
//		�e�N�X�`���}�l�[�W��
//
//==========================================================================
class TextureManager
{
private:
    size_t      maxNum;
    Texture* texture;
    UINT        current;

public:
    TextureManager();
    ~TextureManager();

public:
    void init(size_t max);
    void uninit();

    void load(ID3D11Device*, int, const wchar_t*, size_t);  // �ԍ����w�肵�ă��[�h
    int load(ID3D11Device*, const wchar_t*, size_t);        // �󂫔ԍ���T���ă��[�h�i�߂�l�F�e�N�X�`���ԍ��A-1�F���[�h�G���[�j
    void load(ID3D11Device*, const LoadTexture*);           // ���[�h�f�[�^�ɂ�郍�[�h

    void release(int);
    void release(const LoadTexture*);
    void releaseAll();

    void begin(ID3D11DeviceContext*, int);
    void end(ID3D11DeviceContext*, int);

    void draw(int,
        float, float, float, float,
        float, float, float, float,
        float, float, float,
        float, float, float, float, bool);
    void draw(int,
        const VECTOR2&, const VECTOR2&,
        const VECTOR2&, const VECTOR2&,
        const VECTOR2&, float angle,
        const VECTOR4&, bool);
};


namespace texture
{


    void init(size_t);


    void uninit();


    int load(const wchar_t*, size_t maxInstance = (1U));


    void load(int, const wchar_t*, size_t maxInstance = (1U));


    void load(const LoadTexture*);


    void release(int);



    void release(const LoadTexture*);


    void releaseAll();


    void begin(int);


    void end(int);

    //--------------------------------------------------------------
    //  �e�N�X�`���̕`��
    //--------------------------------------------------------------
    //  int texNo                          �e�N�X�`���̃C���f�b�N�X
    //  float x, float y                   �ʒu
    //  float scaleX, float scaleY         �X�P�[��
    //  float left, float top              �ؔ��ʒu
    //  float width, float height          �ؔ��T�C�Y
    //  float centerX, float centerY       ���S
    //  float angle                        �p�x(radian)
    //  float r, float g, float b, float a �F
    //  bool  world                   true:���[���h���W�ɕ`�� false:�X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void draw(int,
        float, float, float scaleX = (1), float scaleY = (1),
        float left = (0), float top = (0), float width = (0), float height = (0),
        float centerX = (0), float centerY = (0), float angle = (0),
        float r = (1), float g = (1), float b = (1), float a = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  �e�N�X�`���̕`��
    //--------------------------------------------------------------
    //  int texNo                  �e�N�X�`���̃C���f�b�N�X
    //  const VECTOR2& position    �ʒu
    //  const VECTOR2& scale       �X�P�[��
    //  const VECTOR2& texPos      �ؔ��ʒu
    //  const VECTOR2& texSize     �ؔ��T�C�Y
    //  const VECTOR2& center      ���S
    //  float angle                �p�x(radian)
    //  const VECTOR4& color       �F
    //  bool  world           true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void draw(int,
        const VECTOR2&, const VECTOR2& scale = (VECTOR2(1, 1)),
        const VECTOR2& texPos = (VECTOR2(0, 0)), const VECTOR2& texSize = (VECTOR2(0, 0)),
        const VECTOR2& center = (VECTOR2(0, 0)), float angle = 0,
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)),
        bool world = (false));

}
