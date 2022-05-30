#pragma once
#include "OBJ3D.h"
#include "collision.h"




class judge_manager {
    //playerで別オブジェクトを参照したくないので判定用クラスを用意
private:

    float distance1p = 0;
    float distance2p = 0;

    const float correction_velocity = 410.0f;
    const float correction_velocity_air = 20.0f;
    const float correction_velocity_y = 450.0f;


    float COLLECTIONAIR = 300.0f;

    float STANPOINT = 250.0f;

  

    static constexpr float ADJUST_Y = 0.125f;               // あたり判定での位置調整用（縦）
    static constexpr float ADJUST_X = 0.0125f;              // あたり判定での位置調整用（横）

    const float attenuation_rate = 0.8f;

    bool hit1p = false;
    bool hit2p = false;
public:
    static judge_manager& Instance()
    {
        static judge_manager instance;
        return instance;
    }


    float MAX_STANPOINT = 400.0f;
    std::unique_ptr <atack_manager> _attack_manager;

           //攻撃用
           //くらい用

    void init()
    {
        _attack_manager = std::make_unique<atack_manager>();
        _attack_manager->init();
    }
    
    //当たり判定、プレイヤーの向き更新
    void update(OBJManager* object1, OBJManager* object2);
    //当たり判定処理
    void collision_update(OBJManager* object1, OBJManager* object2);
    void player1_collision_update(OBJManager* object1, OBJManager* object2);
    void player2_collision_update(OBJManager* object1, OBJManager* object2);

    void player_collision_update(OBJManager* object1,int combo);

    void extrude_update(OBJManager* object1, OBJManager* object2);

    void extrude_Right(OBJ3D*);                 // 右方向補正処理
    void extrude_Left(OBJ3D*);                  // 左方向補正処理



    //当たり判定の描画
    void drawhit(OBJManager* object1, OBJManager* object2);



    float get_1pdistance() { return this->distance1p; }
    float get_2pdistance() { return this->distance2p; }
    bool get_1p_hit() { return this->hit1p; }
    bool get_2p_hit() { return this->hit2p; }

    // ワールド座標(3D)からスクリーン座標(2D)へ変換
    void WorldToScreen(VECTOR3* screenPosition, const VECTOR3& worldPosition, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection);
    // スクリーン座標(2D)からワールド座標(3D)へ変換
    void ScreenToWorld(VECTOR3 screenPosition, VECTOR3& worldPosition, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection);


};



//template<template<class> class ArrayType>
//class Rect_Manager
//{
//private:
//    ArrayType<fRECT> rects;
//
//public:
//    auto begin()
//    {
//        return rects.begin();
//    }
//
//    auto end()
//    {
//        return rects.end();
//    }
//
//    auto& Add()
//    {
//        return rects.emplace_back();
//    }
//};
