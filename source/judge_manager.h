#pragma once
#include "OBJ3D.h"
#include "collision.h"




class judge_manager {
    //player�ŕʃI�u�W�F�N�g���Q�Ƃ������Ȃ��̂Ŕ���p�N���X��p��
private:

    float distance1p = 0;
    float distance2p = 0;

    const float correction_velocity = 410.0f;
    const float correction_velocity_air = 20.0f;
    const float correction_velocity_y = 450.0f;


    float COLLECTIONAIR = 300.0f;

    float STANPOINT = 250.0f;

  

    static constexpr float ADJUST_Y = 0.125f;               // �����蔻��ł̈ʒu�����p�i�c�j
    static constexpr float ADJUST_X = 0.0125f;              // �����蔻��ł̈ʒu�����p�i���j

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

           //�U���p
           //���炢�p

    void init()
    {
        _attack_manager = std::make_unique<atack_manager>();
        _attack_manager->init();
    }
    
    //�����蔻��A�v���C���[�̌����X�V
    void update(OBJManager* object1, OBJManager* object2);
    //�����蔻�菈��
    void collision_update(OBJManager* object1, OBJManager* object2);
    void player1_collision_update(OBJManager* object1, OBJManager* object2);
    void player2_collision_update(OBJManager* object1, OBJManager* object2);

    void player_collision_update(OBJManager* object1,int combo);

    void extrude_update(OBJManager* object1, OBJManager* object2);

    void extrude_Right(OBJ3D*);                 // �E�����␳����
    void extrude_Left(OBJ3D*);                  // �������␳����



    //�����蔻��̕`��
    void drawhit(OBJManager* object1, OBJManager* object2);



    float get_1pdistance() { return this->distance1p; }
    float get_2pdistance() { return this->distance2p; }
    bool get_1p_hit() { return this->hit1p; }
    bool get_2p_hit() { return this->hit2p; }

    // ���[���h���W(3D)����X�N���[�����W(2D)�֕ϊ�
    void WorldToScreen(VECTOR3* screenPosition, const VECTOR3& worldPosition, DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection);
    // �X�N���[�����W(2D)���烏�[���h���W(3D)�֕ϊ�
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
