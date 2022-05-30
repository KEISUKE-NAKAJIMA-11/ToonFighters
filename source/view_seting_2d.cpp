#include "view_seting_2d.h"
#include "framework.h"
//--------------------------------------------------------------
    //  コンストラクタ
    //--------------------------------------------------------------
ViewSettings::ViewSettings()
{
    initialize();
}

//--------------------------------------------------------------
//  デストラクタ
//--------------------------------------------------------------
ViewSettings::~ViewSettings()
{
}

//--------------------------------------------------------------
//  初期設定
//--------------------------------------------------------------
void ViewSettings::initialize()
{
    scaleFactor = 1.0f;
    setViewLeftTop(VECTOR2(0, 0));
}

//--------------------------------------------------------------
//  スケールを設定
//--------------------------------------------------------------
void ViewSettings::setScaleFactor(float s)
{
    scaleFactor = s;
}

//--------------------------------------------------------------
//  画面中央のワールド座標を設定
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
//  画面左上のワールド座標を設定
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
//  画面左下のワールド座標を設定
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
//  画面内の任意の座標のワールド座標を設定
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
//  ワールド座標をスクリーン座標に変換
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
//  スクリーン座標をワールド座標に変換
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
    //  ViewSettingsの初期化
    //--------------------------------------------------------------
    void init()
    {
        pSystem.viewset->initialize();
    }

    //--------------------------------------------------------------
    //  スケールを設定
    //--------------------------------------------------------------
    void setScale(float scale)
    {
        pSystem.viewset->setScaleFactor(scale);
    }

    //--------------------------------------------------------------
    //  スケールを取得
    //--------------------------------------------------------------
    float getScale()
    {
        return pSystem.viewset->getScaleFactor();
    }

    //--------------------------------------------------------------
    //  中央のワールド座標を設定
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
    //  画面左上のワールド座標を設定
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
    //  画面左下のワールド座標を設定
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
    //  画面中央のワールド座標を取得
    //--------------------------------------------------------------
    VECTOR2 getCenter()
    {
        return pSystem.viewset->getViewCenter();
    }

    //--------------------------------------------------------------
    //  画面左上のワールド座標を取得
    //--------------------------------------------------------------
    VECTOR2 getLeftTop()
    {
        return pSystem.viewset->getViewLeftTop();
    }

    //--------------------------------------------------------------
    //  画面左下のワールド座標を取得
    //--------------------------------------------------------------
    VECTOR2 getLeftBottom()
    {
        return pSystem.viewset->getViewLeftBottom();
    }

    //--------------------------------------------------------------
    //  画面の任意の座標にワールド座標を設定
    //--------------------------------------------------------------
    void setByPosition(VECTOR2 screen, VECTOR2 world)
    {
        pSystem.viewset->setViewByPosition(screen, world);
    }

    //--------------------------------------------------------------
    //  Box2Dの座標をワールド座標に変換
    //--------------------------------------------------------------
    //VECTOR2 box2dToWorld(b2Vec2 b2Pos)
    //{
    //    return pSystem.viewset->box2dToWorld(b2Pos);
    //}

    //--------------------------------------------------------------
    //  ワールド座標をスクリーン座標へ変換
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
    //  スクリーン座標をワールド座標に変換
    //--------------------------------------------------------------
    VECTOR2 screenToWorld(VECTOR2 sPos)
    {
        return pSystem.viewset->screenToWorld(sPos);
    }

    //--------------------------------------------------------------
    //  ワールド座標をBox2Dの座標に変換
    //--------------------------------------------------------------
    //b2Vec2 worldToBox2d(VECTOR2 wPos)
    //{
    //    return pSystem.viewset->worldToBox2d(wPos);
    //}

}