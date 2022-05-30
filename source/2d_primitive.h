#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "vector.h"

const int PRIMITIVE_VERTEX_NUM = 130;
const int PRIMITIVE_CIRCLE_SIDE_NUM = 32;

//==========================================================================
//
//      Primitive�N���X
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
    //  ��`�`��
    //--------------------------------------------------------------
    void rect(ID3D11DeviceContext*, const VECTOR2&, const VECTOR2&,
        const VECTOR2&, float, const VECTOR4&) const;

    //--------------------------------------------------------------
    //  ���`��
    //--------------------------------------------------------------
    void line(ID3D11DeviceContext*, const VECTOR2&, const VECTOR2&,
        const VECTOR4&, float) const;

    //--------------------------------------------------------------
    //  �~�`��
    //--------------------------------------------------------------
    void circle(ID3D11DeviceContext*, const VECTOR2&, float,
        const VECTOR4&, int) const;

    //--------------------------------------------------------------
    //  �l�p�|���S���`��i���_�w��j
    //--------------------------------------------------------------
    void quad(ID3D11DeviceContext*, const VECTOR2(&)[4],
        const VECTOR4&) const;

    void draw_triangle_fan(ID3D11DeviceContext*,
        const VECTOR2[], const VECTOR4&, int) const;

    //--------------------------------------------------------------
    //  �\���̒�`
    //--------------------------------------------------------------
    struct vertex { VECTOR3 position; VECTOR4 color; };

};

//==========================================================================
//
//      PrimitiveBatch�N���X
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
    //  �O����
    //--------------------------------------------------------------
    void begin(ID3D11DeviceContext*);

    //--------------------------------------------------------------
    //  ��`�`��
    //--------------------------------------------------------------
    void rect(const VECTOR2&, const VECTOR2&,
        const VECTOR2&, float,
        const VECTOR4&);

    //--------------------------------------------------------------
    //  ���`��
    //--------------------------------------------------------------
    void line(const VECTOR2&, const VECTOR2&,
        const VECTOR4&, float width);

    //--------------------------------------------------------------
    //  �㏈��
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
    //  ��`�`��
    //--------------------------------------------------------------
    //  float x         �`��ʒux     float y        �`��ʒuy
    //  float w         ��            float h        ����
    //  float centerX   ��_x       float centerY  ��_y
    //  float angle     �p�x (radian)
    //  float r         �F r (0.0f ~ 1.0f)
    //  float g         �F g (0.0f ~ 1.0f)
    //  float b         �F b (0.0f ~ 1.0f)
    //  float a         �F a (0.0f ~ 1.0f)
    //  bool world true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void rect(float, float, float, float,
        float cx = (0), float cy = (0), float angle = (0),
        float r = (1), float g = (1), float b = (1), float a = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  ��`�`��
    //--------------------------------------------------------------
    //  const VECTOR2& pos      �`��ʒu (x, y)
    //  const VECTOR2& size     ������   (w, h)
    //  const VECTOR2& center   ��_   (x, y)
    //  float angle             �p�x     (radian)
    //  const VECTOR4& color    �F       (r, g, b, a) (0.0f ~ 1.0f)
    //  bool  world        true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void rect(const VECTOR2&, const VECTOR2&,
        const VECTOR2& center = (VECTOR2(0, 0)), float angle = (0),
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)),
        bool world = (false));

    //--------------------------------------------------------------
    //  ���`��
    //--------------------------------------------------------------
    //  float x1        �n�_ x    float y1    // �n�_ y
    //  float x2        �I�_ x    float y2    // �I�_ y
    //  float r         �F r (0.0f ~ 1.0f)
    //  float g         �F g (0.0f ~ 1.0f)
    //  float b         �F b (0.0f ~ 1.0f)
    //  float a         �F a (0.0f ~ 1.0f)
    //  float width     ��
    //  bool world true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void line(float, float, float, float,
        float r = (1), float g = (1), float b = (1), float a = (1),
        float width = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  ���`��
    //--------------------------------------------------------------
    //  const VECTOR2& from     �n�_ (x, y)
    //  const VECTOR2& to       �I�_ (x, y)
    //  const VECTOR4& color    �F   (r, g, b, a)
    //  float width             ��
    //  bool world         true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void line(const VECTOR2&, const VECTOR2&,
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)), float width = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  �~�`��
    //--------------------------------------------------------------
    //  float x         ���S�ʒu x    float y      // ���S�ʒu y
    //  float radius    ���a
    //  float r         �F r (0.0f ~ 1.0f)
    //  float g         �F g (0.0f ~ 1.0f)
    //  float b         �F b (0.0f ~ 1.0f)
    //  float a         �F a (0.0f ~ 1.0f)
    //  int n           ���p�`��
    //  bool world true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void circle(float, float, float,
        float r = (1), float g = (1), float b = (1), float a = (1),
        int n = (32),
        bool world = (false));

    //--------------------------------------------------------------
    //  �~�`��
    //--------------------------------------------------------------
    //  const VECTOR2& pos      ���S�ʒu (x, y)
    //  float radius            ���a
    //  const VECTOR4& color    �F   (r, g, b, a)
    //  int n                   ���p�`��
    //  bool world         true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void circle(const VECTOR2&, float,
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)), int n = (PRIMITIVE_CIRCLE_SIDE_NUM),
        bool world = (false));

    //--------------------------------------------------------------
    //  �l�p�|���S���`��i���_�w��j
    //--------------------------------------------------------------
    //  const VECTOR2(&v)[4]    ���_�̔z��i�傫��4�j
    //  float r         �F r (0.0f ~ 1.0f)
    //  float g         �F g (0.0f ~ 1.0f)
    //  float b         �F b (0.0f ~ 1.0f)
    //  float a         �F a (0.0f ~ 1.0f)
    //  bool world true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void quad(const VECTOR2(&)[4],
        float r = (1), float g = (1), float b = (1), float a = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  �l�p�|���S���`��i���_�w��j
    //--------------------------------------------------------------
    //  const VECTOR2(&v)[4]    ���_�̔z��i�傫��4�j
    //  const VECTOR4& color    �F (r, g, b, a)
    //  bool  world        true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
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
    //	�O����
    //--------------------------------------------------------------
    void begin();

    //--------------------------------------------------------------
    //  �㏈��
    //--------------------------------------------------------------
    void end();

    //--------------------------------------------------------------
    //  ��`�`��
    //--------------------------------------------------------------
    //  float x         float y     ���S�ʒu (x, y)
    //  float w         float h     ���E���� (w, h)
    //  float cx        float cy    ���S (cx, cy)
    //  float angle     �p�x (radian)
    //  float r         �Fr  (0.0f ~ 1.0f)
    //  float g         �Fg  (0.0f ~ 1.0f)
    //  float b         �Fb  (0.0f ~ 1.0f)
    //  float a         �Fa  (0.0f ~ 1.0f)
    //  bool world true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void rect(float, float, float, float,
        float cx = (0), float cy = (0), float angle = (0),
        float r = (1), float g = (1), float b = (1), float a = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  ��`�`��
    //--------------------------------------------------------------
    //  const VECTOR2& pos      ���S�ʒu (x, y)
    //  const VECTOR2& size     ������   (w, h)
    //  const VECTOR2& center   ���S     (x, y)
    //  float angle             �p�x     (radian)
    //  const VECTOR4& color    �F       (r, g, b, a)
    //  bool world         true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void rect(const VECTOR2&, const VECTOR2&,
        const VECTOR2& center = (VECTOR2(0, 0)), float angle = (0),
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)),
        bool world = (false));

    //--------------------------------------------------------------
    //  ���`��
    //--------------------------------------------------------------
    //  float x1        �n�_ x
    //  float y1        �n�_ y
    //  float x2        �I�_ x
    //  float y2        �I�_ y
    //  float r         �F r (0.0f ~ 1.0f)
    //  float g         �F g (0.0f ~ 1.0f)
    //  float b         �F b (0.0f ~ 1.0f)
    //  float a         �F a (0.0f ~ 1.0f)
    //  float width     ��
    //  bool world true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void line(float, float, float, float,
        float r = (1), float g = (1), float b = (1), float a = (1),
        float width = (1),
        bool world = (false));

    //--------------------------------------------------------------
    //  ���`��
    //--------------------------------------------------------------
    //  const VECTOR2& from     �n�_ (x, y)
    //  const VECTOR2& to       �I�_ (x, y)
    //  const VECTOR4& color    �F   (r, g, b, a)
    //  float width             ��
    //  bool world         true:���[���h���W�ɕ`�� false : �X�N���[�����W�ɕ`��
    //--------------------------------------------------------------
    void line(const VECTOR2&, const VECTOR2&,
        const VECTOR4& color = (VECTOR4(1, 1, 1, 1)), float width = (1),
        bool world = (false));

}





