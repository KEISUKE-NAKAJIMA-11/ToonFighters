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

// テクスチャロードデータ
struct LoadTexture
{
    int             texNum;         // テクスチャ番号
    const wchar_t* fileName;       // ファイル名
    UINT            maxInstance;    // 最大数（SpriteBatch使用時）
};

//==========================================================================
//
//      テクスチャクラス
//
//==========================================================================
class Texture
{
private:
    sprite_batch* sprBat;    // スプライトバッチ

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

    bool isEmpty();	// テクスチャデータの有無（true : 無, false : 有）
};






//class TextureShader
//{
//protected:
//    ComPtr<ID3D11ShaderResourceView> ShaderResourceView = nullptr;
//    ComPtr<ID3D11RenderTargetView> RenderTargetView = nullptr;
//    ComPtr<ID3D11DepthStencilView> DepthStencilView = nullptr;
//
//    //// Cubeで追加
//    //ComPtr<ID3D11Texture2D> Texture2D = nullptr;
//
//    // テクスチャ情報
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
//    // 深度ステンシルバッファ作成
//    bool CreateDepth(u_int width, u_int height, DXGI_FORMAT format = DXGI_FORMAT_R24G8_TYPELESS);
//
//
//    ID3D11RenderTargetView* GetRenderTargetView() { return RenderTargetView.Get(); }
//    ID3D11DepthStencilView* GetDepthStencilView() { return DepthStencilView.Get(); }
//
//    //CubeMap用
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
//		テクスチャマネージャ
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

    void load(ID3D11Device*, int, const wchar_t*, size_t);  // 番号を指定してロード
    int load(ID3D11Device*, const wchar_t*, size_t);        // 空き番号を探してロード（戻り値：テクスチャ番号、-1：ロードエラー）
    void load(ID3D11Device*, const LoadTexture*);           // ロードデータによるロード

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
    //  テクスチャの描画
    //--------------------------------------------------------------
    //  int texNo                          テクスチャのインデックス
    //  float x, float y                   位置
    //  float scaleX, float scaleY         スケール
    //  float left, float top              切抜位置
    //  float width, float height          切抜サイズ
    //  float centerX, float centerY       中心
    //  float angle                        角度(radian)
    //  float r, float g, float b, float a 色
    //  bool  world                   true:ワールド座標に描画 false:スクリーン座標に描画
    //--------------------------------------------------------------
    void draw(int,
        float, float, float scaleX = (1), float scaleY = (1),
        float left = (0), float top = (0), float width = (0), float height = (0),
        float centerX = (0), float centerY = (0), float angle = (0),
        float r = (1), float g = (1), float b = (1), float a = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  テクスチャの描画
    //--------------------------------------------------------------
    //  int texNo                  テクスチャのインデックス
    //  const VECTOR2& position    位置
    //  const VECTOR2& scale       スケール
    //  const VECTOR2& texPos      切抜位置
    //  const VECTOR2& texSize     切抜サイズ
    //  const VECTOR2& center      中心
    //  float angle                角度(radian)
    //  const VECTOR4& color       色
    //  bool  world           true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void draw(int,
        const VECTOR2&, const VECTOR2& scale = (VECTOR2(1, 1)),
        const VECTOR2& texPos = (VECTOR2(0, 0)), const VECTOR2& texSize = (VECTOR2(0, 0)),
        const VECTOR2& center = (VECTOR2(0, 0)), float angle = 0,
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)),
        bool world = (false));

}
