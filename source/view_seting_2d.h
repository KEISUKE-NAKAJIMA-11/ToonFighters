#pragma once

#include "vector.h"



//==========================================================================
    //
    //      ViewSettings�N���X
    //
    //==========================================================================
class ViewSettings
{
private:
    float   scaleFactor;    // ��ʂɕ`�悳���C���[�W�̃X�P�[��
    VECTOR2 vCenter;        // ��ʂ̒��S���W
    VECTOR2 vLeftTop;       // ��ʍ���̍��W
    VECTOR2 vLeftBottom;    // ��ʍ����̍��W

public:
    ViewSettings();
    ~ViewSettings();

    void initialize();

    void setScaleFactor(float s);
    void setViewCenter(VECTOR2);
    void setViewLeftTop(VECTOR2);
    void setViewLeftBottom(VECTOR2);
    void setViewByPosition(VECTOR2, VECTOR2);

    float getScaleFactor() { return scaleFactor; }
    VECTOR2 getViewCenter() { return vCenter; }
    VECTOR2 getViewLeftTop() { return vLeftTop; }
    VECTOR2 getViewLeftBottom() { return vLeftBottom; }

    VECTOR2 worldToScreen(const VECTOR2&);
    VECTOR3 worldToScreen(const VECTOR3&);
    VECTOR2 screenToWorld(VECTOR2);
    void WorldToScreen(VECTOR3* screenPosition, const VECTOR3& worldPosition, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection);
    void ScreenToWorld(VECTOR3 screenPosition, VECTOR3& worldPosition, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection);
};





namespace view_set2d
{

    //--------------------------------------------------------------
    //  ���̓}�l�[�W���̏�����
    //--------------------------------------------------------------
    void init();

    //--------------------------------------------------------------
    //  �X�P�[����ݒ�
    //--------------------------------------------------------------
    void setScale(float);

    //--------------------------------------------------------------
    //  �X�P�[�����擾
    //--------------------------------------------------------------
    float getScale();

    //--------------------------------------------------------------
    //  �����̃��[���h���W��ݒ�
    //--------------------------------------------------------------
    void setCenter(float, float);
    void setCenter(VECTOR2);

    //--------------------------------------------------------------
    //  ��ʍ���̃��[���h���W��ݒ�
    //--------------------------------------------------------------
    void setLeftTop(float, float);
    void setLeftTop(VECTOR2);

    //--------------------------------------------------------------
    //  ��ʍ����̃��[���h���W��ݒ�
    //--------------------------------------------------------------
    void setLeftBottom(float, float);
    void setLeftTop(VECTOR2);

    //--------------------------------------------------------------
    //  ��ʒ����̃��[���h���W���擾
    //--------------------------------------------------------------
    VECTOR2 getCenter();

    //--------------------------------------------------------------
    //  ��ʍ���̃��[���h���W���擾
    //--------------------------------------------------------------
    VECTOR2 getLeftTop();

    //--------------------------------------------------------------
    //  ��ʍ����̃��[���h���W���擾
    //--------------------------------------------------------------
    VECTOR2 getLeftBottom();

    //--------------------------------------------------------------
    //  ��ʂ̔C�ӂ̍��W�Ƀ��[���h���W��ݒ�
    //--------------------------------------------------------------
    void setByPosition(VECTOR2, VECTOR2);

    //--------------------------------------------------------------
    //  Box2D�̍��W�����[���h���W�ɕϊ�
    //--------------------------------------------------------------
    // VECTOR2 box2dToWorld(b2Vec2);

    //--------------------------------------------------------------
    //  ���[���h���W���X�N���[�����W�֕ϊ�
    //--------------------------------------------------------------
    VECTOR2 worldToScreen(VECTOR2);
    VECTOR3 worldToScreen(VECTOR3);

    //--------------------------------------------------------------
    //  �X�N���[�����W�����[���h���W�ɕϊ�
    //--------------------------------------------------------------
    VECTOR2 screenToWorld(VECTOR2);

    //--------------------------------------------------------------
    //  ���[���h���W��Box2D�̍��W�ɕϊ�
    //--------------------------------------------------------------
    // b2Vec2 worldToBox2d(VECTOR2);

}
