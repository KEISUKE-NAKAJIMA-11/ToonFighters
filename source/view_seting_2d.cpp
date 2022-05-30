#include "view_seting_2d.h"
#include "framework.h"
//--------------------------------------------------------------
    //  �R���X�g���N�^
    //--------------------------------------------------------------
ViewSettings::ViewSettings()
{
    initialize();
}

//--------------------------------------------------------------
//  �f�X�g���N�^
//--------------------------------------------------------------
ViewSettings::~ViewSettings()
{
}

//--------------------------------------------------------------
//  �����ݒ�
//--------------------------------------------------------------
void ViewSettings::initialize()
{
    scaleFactor = 1.0f;
    setViewLeftTop(VECTOR2(0, 0));
}

//--------------------------------------------------------------
//  �X�P�[����ݒ�
//--------------------------------------------------------------
void ViewSettings::setScaleFactor(float s)
{
    scaleFactor = s;
}

//--------------------------------------------------------------
//  ��ʒ����̃��[���h���W��ݒ�
//--------------------------------------------------------------
void ViewSettings::setViewCenter(VECTOR2 c)
{
    vCenter = c;
#ifdef GAMELIB_PLUS_UP
    vLeftTop = vCenter + VECTOR2(-system::SCREEN_WIDTH / 2, system::SCREEN_HEIGHT / 2) / scaleFactor;
    vLeftBottom = vCenter + VECTOR2(-system::SCREEN_WIDTH / 2, -system::SCREEN_HEIGHT / 2) / scaleFactor;
#else
    vLeftTop = vCenter + VECTOR2(-pSystem.SCREEN_WIDTH / 2.0f, (float)-pSystem.SCREEN_HEIGHT / 2.0f) / scaleFactor;
    vLeftBottom = vCenter + VECTOR2(-pSystem.SCREEN_WIDTH / 2.0f, pSystem.SCREEN_HEIGHT / 2.0f) / scaleFactor;
#endif
}

//--------------------------------------------------------------
//  ��ʍ���̃��[���h���W��ݒ�
//--------------------------------------------------------------
void ViewSettings::setViewLeftTop(VECTOR2 lt)
{
    vLeftTop = lt;
#ifdef GAMELIB_PLUS_UP
    vCenter = vLeftTop + VECTOR2(system::SCREEN_WIDTH / 2, -system::SCREEN_HEIGHT / 2) / scaleFactor;
    vLeftBottom = vLeftTop + VECTOR2(0, -system::SCREEN_HEIGHT) / scaleFactor;
#else
    vCenter = vLeftTop + VECTOR2(pSystem.SCREEN_WIDTH / 2.0f, pSystem.SCREEN_HEIGHT / 2.0f) / scaleFactor;
    vLeftBottom = vLeftTop + VECTOR2(0, (float)pSystem.SCREEN_HEIGHT) / scaleFactor;
#endif
}

//--------------------------------------------------------------
//  ��ʍ����̃��[���h���W��ݒ�
//--------------------------------------------------------------
void ViewSettings::setViewLeftBottom(VECTOR2 lb)
{
    vLeftBottom = lb;
#ifdef GAMELIB_PLUS_UP
    vCenter = vLeftBottom + VECTOR2(system::SCREEN_WIDTH / 2, system::SCREEN_HEIGHT / 2) / scaleFactor;
    vLeftTop = vLeftBottom + VECTOR2(0, system::SCREEN_HEIGHT) / scaleFactor;
#else
    vCenter = vLeftBottom - VECTOR2(pSystem.SCREEN_WIDTH / 2.0f, pSystem.SCREEN_HEIGHT / 2.0f) / scaleFactor;
    vLeftTop = vLeftBottom - VECTOR2(0, (float)pSystem.SCREEN_HEIGHT) / scaleFactor;
#endif
}

//--------------------------------------------------------------
//  ��ʓ��̔C�ӂ̍��W�̃��[���h���W��ݒ�
//--------------------------------------------------------------
void ViewSettings::setViewByPosition(VECTOR2 screenPos, VECTOR2 worldPos)
{
#ifdef GAMELIB_PLUS_UP
    screenPos.y = system::SCREEN_HEIGHT - screenPos.y;
    VECTOR2 lb = worldPos - screenPos / scaleFactor;
    setViewLeftBottom(lb);
#else
    VECTOR2 lt = worldPos - screenPos / scaleFactor;
    setViewLeftTop(lt);
#endif
}

//--------------------------------------------------------------
//  ���[���h���W���X�N���[�����W�ɕϊ�
//--------------------------------------------------------------
VECTOR2 ViewSettings::worldToScreen(const VECTOR2& wPos)
{
#ifdef GAMELIB_PLUS_UP
    return (wPos - vLeftBottom) * scaleFactor;
#else
    return (wPos - vLeftTop) * scaleFactor;
#endif
}

VECTOR3 ViewSettings::worldToScreen(const VECTOR3& wPos)
{
#ifdef GAMELIB_PLUS_UP
    return (wPos - vLeftBottom) * scaleFactor;
#else
    return (wPos - vLeftTop) * scaleFactor;
#endif
}

//--------------------------------------------------------------
//  �X�N���[�����W�����[���h���W�ɕϊ�
//--------------------------------------------------------------
VECTOR2 ViewSettings::screenToWorld(VECTOR2 sPos)
{
#ifdef GAMELIB_PLUS_UP
    sPos.y = system::SCREEN_HEIGHT - sPos.y;
    return vLeftBottom + sPos / scaleFactor;
#else
    return vLeftTop + sPos / scaleFactor;
#endif
}

namespace view_set2d
{

    //--------------------------------------------------------------
    //  ViewSettings�̏�����
    //--------------------------------------------------------------
    void init()
    {
        pSystem.viewset->initialize();
    }

    //--------------------------------------------------------------
    //  �X�P�[����ݒ�
    //--------------------------------------------------------------
    void setScale(float scale)
    {
        pSystem.viewset->setScaleFactor(scale);
    }

    //--------------------------------------------------------------
    //  �X�P�[�����擾
    //--------------------------------------------------------------
    float getScale()
    {
        return pSystem.viewset->getScaleFactor();
    }

    //--------------------------------------------------------------
    //  �����̃��[���h���W��ݒ�
    //--------------------------------------------------------------
    void setCenter(float cx, float cy)
    {
        pSystem.viewset->setViewCenter(VECTOR2(cx, cy));
    }

    void setCenter(VECTOR2 v)
    {
        pSystem.viewset->setViewCenter(v);
    }

    //--------------------------------------------------------------
    //  ��ʍ���̃��[���h���W��ݒ�
    //--------------------------------------------------------------
    void setLeftTop(float ltx, float lty)
    {
        pSystem.viewset->setViewLeftTop(VECTOR2(ltx, lty));
    }

    void setLeftTop(VECTOR2 v)
    {
        pSystem.viewset->setViewLeftTop(v);
    }

    //--------------------------------------------------------------
    //  ��ʍ����̃��[���h���W��ݒ�
    //--------------------------------------------------------------
    void setLeftBottom(float lbx, float lby)
    {
        pSystem.viewset->setViewLeftBottom(VECTOR2(lbx, lby));
    }

    void setLeftBottom(VECTOR2 v)
    {
        pSystem.viewset->setViewLeftBottom(v);
    }

    //--------------------------------------------------------------
    //  ��ʒ����̃��[���h���W���擾
    //--------------------------------------------------------------
    VECTOR2 getCenter()
    {
        return pSystem.viewset->getViewCenter();
    }

    //--------------------------------------------------------------
    //  ��ʍ���̃��[���h���W���擾
    //--------------------------------------------------------------
    VECTOR2 getLeftTop()
    {
        return pSystem.viewset->getViewLeftTop();
    }

    //--------------------------------------------------------------
    //  ��ʍ����̃��[���h���W���擾
    //--------------------------------------------------------------
    VECTOR2 getLeftBottom()
    {
        return pSystem.viewset->getViewLeftBottom();
    }

    //--------------------------------------------------------------
    //  ��ʂ̔C�ӂ̍��W�Ƀ��[���h���W��ݒ�
    //--------------------------------------------------------------
    void setByPosition(VECTOR2 screen, VECTOR2 world)
    {
        pSystem.viewset->setViewByPosition(screen, world);
    }

    //--------------------------------------------------------------
    //  Box2D�̍��W�����[���h���W�ɕϊ�
    //--------------------------------------------------------------
    //VECTOR2 box2dToWorld(b2Vec2 b2Pos)
    //{
    //    return pSystem.viewset->box2dToWorld(b2Pos);
    //}

    //--------------------------------------------------------------
    //  ���[���h���W���X�N���[�����W�֕ϊ�
    //--------------------------------------------------------------
    VECTOR2 worldToScreen(VECTOR2 wPos)
    {
        return pSystem.viewset->worldToScreen(wPos);
    }

    VECTOR3 worldToScreen(VECTOR3 wPos)
    {
        return pSystem.viewset->worldToScreen(wPos);
    }

    //--------------------------------------------------------------
    //  �X�N���[�����W�����[���h���W�ɕϊ�
    //--------------------------------------------------------------
    VECTOR2 screenToWorld(VECTOR2 sPos)
    {
        return pSystem.viewset->screenToWorld(sPos);
    }

    //--------------------------------------------------------------
    //  ���[���h���W��Box2D�̍��W�ɕϊ�
    //--------------------------------------------------------------
    //b2Vec2 worldToBox2d(VECTOR2 wPos)
    //{
    //    return pSystem.viewset->worldToBox2d(wPos);
    //}

}