#pragma once

#include "vector.h"



//==========================================================================
    //
    //      ViewSettingsクラス
    //
    //==========================================================================
class ViewSettings
{
private:
    float   scaleFactor;    // 画面に描画されるイメージのスケール
    VECTOR2 vCenter;        // 画面の中心座標
    VECTOR2 vLeftTop;       // 画面左上の座標
    VECTOR2 vLeftBottom;    // 画面左下の座標

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
    //  入力マネージャの初期化
    //--------------------------------------------------------------
    void init();

    //--------------------------------------------------------------
    //  スケールを設定
    //--------------------------------------------------------------
    void setScale(float);

    //--------------------------------------------------------------
    //  スケールを取得
    //--------------------------------------------------------------
    float getScale();

    //--------------------------------------------------------------
    //  中央のワールド座標を設定
    //--------------------------------------------------------------
    void setCenter(float, float);
    void setCenter(VECTOR2);

    //--------------------------------------------------------------
    //  画面左上のワールド座標を設定
    //--------------------------------------------------------------
    void setLeftTop(float, float);
    void setLeftTop(VECTOR2);

    //--------------------------------------------------------------
    //  画面左下のワールド座標を設定
    //--------------------------------------------------------------
    void setLeftBottom(float, float);
    void setLeftTop(VECTOR2);

    //--------------------------------------------------------------
    //  画面中央のワールド座標を取得
    //--------------------------------------------------------------
    VECTOR2 getCenter();

    //--------------------------------------------------------------
    //  画面左上のワールド座標を取得
    //--------------------------------------------------------------
    VECTOR2 getLeftTop();

    //--------------------------------------------------------------
    //  画面左下のワールド座標を取得
    //--------------------------------------------------------------
    VECTOR2 getLeftBottom();

    //--------------------------------------------------------------
    //  画面の任意の座標にワールド座標を設定
    //--------------------------------------------------------------
    void setByPosition(VECTOR2, VECTOR2);

    //--------------------------------------------------------------
    //  Box2Dの座標をワールド座標に変換
    //--------------------------------------------------------------
    // VECTOR2 box2dToWorld(b2Vec2);

    //--------------------------------------------------------------
    //  ワールド座標をスクリーン座標へ変換
    //--------------------------------------------------------------
    VECTOR2 worldToScreen(VECTOR2);
    VECTOR3 worldToScreen(VECTOR3);

    //--------------------------------------------------------------
    //  スクリーン座標をワールド座標に変換
    //--------------------------------------------------------------
    VECTOR2 screenToWorld(VECTOR2);

    //--------------------------------------------------------------
    //  ワールド座標をBox2Dの座標に変換
    //--------------------------------------------------------------
    // b2Vec2 worldToBox2d(VECTOR2);

}
