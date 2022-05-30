#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "vector.h"

const int PRIMITIVE_VERTEX_NUM = 130;
const int PRIMITIVE_CIRCLE_SIDE_NUM = 32;

//==========================================================================
//
//      Primitiveクラス
//
//==========================================================================
class Primitive
{

    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* buffer;
    ID3D11RasterizerState* rasterizerState;
    ID3D11DepthStencilState* depthStencilState;

    ID3D11Device* device;
public:
    Primitive(ID3D11Device*);
    ~Primitive();

    //--------------------------------------------------------------
    //  矩形描画
    //--------------------------------------------------------------
    void rect(ID3D11DeviceContext*, const VECTOR2&, const VECTOR2&,
        const VECTOR2&, float, const VECTOR4&) const;

    //--------------------------------------------------------------
    //  線描画
    //--------------------------------------------------------------
    void line(ID3D11DeviceContext*, const VECTOR2&, const VECTOR2&,
        const VECTOR4&, float) const;

    //--------------------------------------------------------------
    //  円描画
    //--------------------------------------------------------------
    void circle(ID3D11DeviceContext*, const VECTOR2&, float,
        const VECTOR4&, int) const;

    //--------------------------------------------------------------
    //  四角ポリゴン描画（頂点指定）
    //--------------------------------------------------------------
    void quad(ID3D11DeviceContext*, const VECTOR2(&)[4],
        const VECTOR4&) const;

    void draw_triangle_fan(ID3D11DeviceContext*,
        const VECTOR2[], const VECTOR4&, int) const;

    //--------------------------------------------------------------
    //  構造体定義
    //--------------------------------------------------------------
    struct vertex { VECTOR3 position; VECTOR4 color; };

};

//==========================================================================
//
//      PrimitiveBatchクラス
//
//==========================================================================
class PrimitiveBatch
{
private:

    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* buffer;
    ID3D11RasterizerState* rasterizerState;
    ID3D11DepthStencilState* depthStencilState;

    size_t MAX_INSTANCES;
    struct instance
    {
        DirectX::XMFLOAT4X4 ndcTransform;
        VECTOR4 color;
    };
    ID3D11Buffer* instanceBuffer;

public:

    struct vertex
    {
        VECTOR3 position;
    };

    PrimitiveBatch(ID3D11Device*, size_t maxInstance = (256));
    ~PrimitiveBatch();

    //--------------------------------------------------------------
    //  前処理
    //--------------------------------------------------------------
    void begin(ID3D11DeviceContext*);

    //--------------------------------------------------------------
    //  矩形描画
    //--------------------------------------------------------------
    void rect(const VECTOR2&, const VECTOR2&,
        const VECTOR2&, float,
        const VECTOR4&);

    //--------------------------------------------------------------
    //  線描画
    //--------------------------------------------------------------
    void line(const VECTOR2&, const VECTOR2&,
        const VECTOR4&, float width);

    //--------------------------------------------------------------
    //  後処理
    //--------------------------------------------------------------
    void end(ID3D11DeviceContext*);

private:
    D3D11_VIEWPORT viewport;

    UINT instanceCount = 0;
    instance* instances = nullptr;
};




//==========================================================================
    //
    //		primitive
    //
    //==========================================================================

namespace primitive
{

    //--------------------------------------------------------------
    //  矩形描画
    //--------------------------------------------------------------
    //  float x         描画位置x     float y        描画位置y
    //  float w         幅            float h        高さ
    //  float centerX   基準点x       float centerY  基準点y
    //  float angle     角度 (radian)
    //  float r         色 r (0.0f ~ 1.0f)
    //  float g         色 g (0.0f ~ 1.0f)
    //  float b         色 b (0.0f ~ 1.0f)
    //  float a         色 a (0.0f ~ 1.0f)
    //  bool world true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void rect(float, float, float, float,
        float cx = (0), float cy = (0), float angle = (0),
        float r = (1), float g = (1), float b = (1), float a = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  矩形描画
    //--------------------------------------------------------------
    //  const VECTOR2& pos      描画位置 (x, y)
    //  const VECTOR2& size     幅高さ   (w, h)
    //  const VECTOR2& center   基準点   (x, y)
    //  float angle             角度     (radian)
    //  const VECTOR4& color    色       (r, g, b, a) (0.0f ~ 1.0f)
    //  bool  world        true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void rect(const VECTOR2&, const VECTOR2&,
        const VECTOR2& center = (VECTOR2(0, 0)), float angle = (0),
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)),
        bool world = (false));

    //--------------------------------------------------------------
    //  線描画
    //--------------------------------------------------------------
    //  float x1        始点 x    float y1    // 始点 y
    //  float x2        終点 x    float y2    // 終点 y
    //  float r         色 r (0.0f ~ 1.0f)
    //  float g         色 g (0.0f ~ 1.0f)
    //  float b         色 b (0.0f ~ 1.0f)
    //  float a         色 a (0.0f ~ 1.0f)
    //  float width     幅
    //  bool world true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void line(float, float, float, float,
        float r = (1), float g = (1), float b = (1), float a = (1),
        float width = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  線描画
    //--------------------------------------------------------------
    //  const VECTOR2& from     始点 (x, y)
    //  const VECTOR2& to       終点 (x, y)
    //  const VECTOR4& color    色   (r, g, b, a)
    //  float width             幅
    //  bool world         true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void line(const VECTOR2&, const VECTOR2&,
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)), float width = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  円描画
    //--------------------------------------------------------------
    //  float x         中心位置 x    float y      // 中心位置 y
    //  float radius    半径
    //  float r         色 r (0.0f ~ 1.0f)
    //  float g         色 g (0.0f ~ 1.0f)
    //  float b         色 b (0.0f ~ 1.0f)
    //  float a         色 a (0.0f ~ 1.0f)
    //  int n           何角形か
    //  bool world true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void circle(float, float, float,
        float r = (1), float g = (1), float b = (1), float a = (1),
        int n = (32),
        bool world = (false));

    //--------------------------------------------------------------
    //  円描画
    //--------------------------------------------------------------
    //  const VECTOR2& pos      中心位置 (x, y)
    //  float radius            半径
    //  const VECTOR4& color    色   (r, g, b, a)
    //  int n                   何角形か
    //  bool world         true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void circle(const VECTOR2&, float,
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)), int n = (PRIMITIVE_CIRCLE_SIDE_NUM),
        bool world = (false));

    //--------------------------------------------------------------
    //  四角ポリゴン描画（頂点指定）
    //--------------------------------------------------------------
    //  const VECTOR2(&v)[4]    頂点の配列（大きさ4）
    //  float r         色 r (0.0f ~ 1.0f)
    //  float g         色 g (0.0f ~ 1.0f)
    //  float b         色 b (0.0f ~ 1.0f)
    //  float a         色 a (0.0f ~ 1.0f)
    //  bool world true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void quad(const VECTOR2(&)[4],
        float r = (1), float g = (1), float b = (1), float a = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  四角ポリゴン描画（頂点指定）
    //--------------------------------------------------------------
    //  const VECTOR2(&v)[4]    頂点の配列（大きさ4）
    //  const VECTOR4& color    色 (r, g, b, a)
    //  bool  world        true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void quad(const VECTOR2(&)[4],
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)),
        bool world = (false));

    void draw_triangle_fan(VECTOR2[], const VECTOR4&, int nPolygon,
        bool world = (false));
}

//==========================================================================
//
//		primitive_batch
//
//==========================================================================

namespace primitive_batch
{

    //--------------------------------------------------------------
    //	前処理
    //--------------------------------------------------------------
    void begin();

    //--------------------------------------------------------------
    //  後処理
    //--------------------------------------------------------------
    void end();

    //--------------------------------------------------------------
    //  矩形描画
    //--------------------------------------------------------------
    //  float x         float y     中心位置 (x, y)
    //  float w         float h     幅・高さ (w, h)
    //  float cx        float cy    中心 (cx, cy)
    //  float angle     角度 (radian)
    //  float r         色r  (0.0f ~ 1.0f)
    //  float g         色g  (0.0f ~ 1.0f)
    //  float b         色b  (0.0f ~ 1.0f)
    //  float a         色a  (0.0f ~ 1.0f)
    //  bool world true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void rect(float, float, float, float,
        float cx = (0), float cy = (0), float angle = (0),
        float r = (1), float g = (1), float b = (1), float a = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  矩形描画
    //--------------------------------------------------------------
    //  const VECTOR2& pos      中心位置 (x, y)
    //  const VECTOR2& size     幅高さ   (w, h)
    //  const VECTOR2& center   中心     (x, y)
    //  float angle             角度     (radian)
    //  const VECTOR4& color    色       (r, g, b, a)
    //  bool world         true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void rect(const VECTOR2&, const VECTOR2&,
        const VECTOR2& center = (VECTOR2(0, 0)), float angle = (0),
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)),
        bool world = (false));

    //--------------------------------------------------------------
    //  線描画
    //--------------------------------------------------------------
    //  float x1        始点 x
    //  float y1        始点 y
    //  float x2        終点 x
    //  float y2        終点 y
    //  float r         色 r (0.0f ~ 1.0f)
    //  float g         色 g (0.0f ~ 1.0f)
    //  float b         色 b (0.0f ~ 1.0f)
    //  float a         色 a (0.0f ~ 1.0f)
    //  float width     幅
    //  bool world true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void line(float, float, float, float,
        float r = (1), float g = (1), float b = (1), float a = (1),
        float width = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  線描画
    //--------------------------------------------------------------
    //  const VECTOR2& from     始点 (x, y)
    //  const VECTOR2& to       終点 (x, y)
    //  const VECTOR4& color    色   (r, g, b, a)
    //  float width             幅
    //  bool world         true:ワールド座標に描画 false : スクリーン座標に描画
    //--------------------------------------------------------------
    void line(const VECTOR2&, const VECTOR2&,
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)), float width = (1),
        bool world = (false));

}





