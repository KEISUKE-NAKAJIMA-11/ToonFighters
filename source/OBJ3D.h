#pragma once
#include "model.h"

#include "camera.h"

#include "model_renderer.h"
#include <list>
#include <map>
#include <memory>
#include "Vector.h"
#include "assain.h"
#include "debug_primitive.h"
#include "obj2d_data.h"


class OBJ3D;
//#define BURUBON

enum character_part //�����蔻��쐬���Ɏg�p
{
    HEAD,
    BODY,
    REG,
    PART_MAX,

};


enum class attack_types:unsigned int //�U���̑�����`
{
    upper = (1 << 0),
    middle = (1 << 1),
    lower = (1 << 2),

    end,
};




const unsigned int ATTACK_ELEMENT_UPPER = (1 << 0); // 0000 0000 0000 0001
const unsigned int ATTACK_ELEMENT_MIDDLE = (1 << 1); // 0000 0000 0000 0010
const unsigned int ATTACK_ELEMENT_LOWER = (1 << 2); // 0000 0000 0000 0100
const unsigned int ATTACK_ELEMENT_AIR = (1 << 3); // 0000 0000 0000 1000



class MoveAlg
{
public:
    virtual void move(OBJ3D* obj) = 0;
    enum STATE
    {
        state_INIT,
        state_UPDATE,
        state_UNINIT,
    };
};

class OBJ3D:public std::enable_shared_from_this<OBJ3D>
{
public:

    MoveAlg* movealg = nullptr;
    //std::unique_ptr<Model> model;
    //std::shared_ptr<ModelResource>	model_resource;
    //std::unique_ptr<ModelData>	model_data;
    //std::unique_ptr<ModelRenderer> model_renderer;
    const char* filename_temp;
    std::shared_ptr<Model> model;
    std::shared_ptr<ModelResource>	model_resource;

    std::shared_ptr<Model> blade_model;
    std::shared_ptr<ModelResource>	blade_model_resource;

    std::shared_ptr<ModelRenderer> model_renderer;


    //���炢����p�f�o�b�O�v���~�e�B�u
    std::unique_ptr<DebugRenderer>  debugRenderer;


    //����͍l����������Ȃ̂Ńr���[�ƃv���W�F�N�V�����s��������Ă���
    DirectX::XMFLOAT4X4 view;
    DirectX::XMFLOAT4X4 projection;



    VECTOR3 position = { 0.0f,0.0f,0.0f };

    //�ȉ��A�X�N���[�����W�ł̃|�W�V����
    VECTOR3 screen_position = { 0.0f,0.0f,0.0f };

    VECTOR3 head_position = {};//��
    VECTOR3 body_position = {};//�� 
    VECTOR3 reg_position = {};//��

    DirectX::XMFLOAT3 head = {};
    DirectX::XMFLOAT3 body = {};
    DirectX::XMFLOAT3 reg = {};

    VECTOR3 root= {};
    VECTOR3 vector = { 0.0f,0.0f,0.0f };
    VECTOR3 initvector = { 0.0f,0.0f,0.0f };
    VECTOR3 initpos = { 0.0f,0.0f,0.0f };
    VECTOR3 oldpos = { 0.0f,0.0f,0.0f };
    VECTOR3 scale = { 0.0f,0.0f,0.0f };
    VECTOR3 angle = { 0.0f,0.0f,0.0f };
    VECTOR4 color = { 0.0f,0.0f,0.0f,0.0f };



    std::string tag;//IMGUI�p�̃^�O

    int player_num;//1p��2p��

    int state = 0;
    int timer = 0;
    int discovery_timer = 0;
    int damage_timer = 0;
    int goal_timer = 0;
    int waittimer = 0;
    int count = 0;
    int distance = 0;
    int type = 0;
    float fasterspeed = 1.0f;
    bool exist = true;
    bool hitcheck = true;

    bool iswall = false;

    bool ishit = false;
    bool inthehitrect = false;

    bool is_guard = true;

    bool success_guard = false;

    bool guard_redy = false;


    bool is_judge;//�����߂����邩�ǂ���
    bool is_invisible;//���㖳�G�Z���o�����p

    VECTOR3 velocity = { 0,0,0 };
    bool	isGround = true;

    int		health = 1000;
    int		maxHealth = 1000;
    float	invincibleTimer = 1.0f;
    float	stepOffset = 30.0f;
    float	jumpSpeed = 200.0f;
    float	gravity = -1000.5f;
    float	stanpoint = 0.1f;

    float	slopeRate = 1.0f;
    float	friction = 0.5f;
    float	acceleration = 1.0f;
    float	maxMoveSpeed = 5.0f;
    float	moveVecX = 0.0f;
    float	moveVecZ = 0.0f;
    float	airControl = 0.3f;

    float hitstop_timer = 0.0f;
    float hitstop_limit = 0.0f;

    VECTOR2                 size;                    // ������p�T�C�Y �����2D
    VECTOR2                 head_size;
    VECTOR2                 body_size;
    VECTOR2                 reg_size;

    VECTOR2                 checksize;                    // �G���A�`�F�b�N�T�C�Y �����2D
   

    DirectX::XMFLOAT4 emissionEdgeColor = { 0, 0, 0, 1 };
    DirectX::XMFLOAT4 emissionColor = { 0, 0, 0, 1 };

    unsigned int guard_elements;


    fRECT hit_rect[character_part::PART_MAX] = {};                          // ���炢����p ���A���́A���ő�܂��ɕ�����   



    fRECT extrude_rect = {}; //�����o������
    fRECT extrude_rect_center = {}; //�����o������
    fRECT areacheck_rect = {}; //�����o����

    fRECT guard_rect = {};//�@�K�[�h���m����

    fRECT attk_rect[30] = {}; // �U������p

    OBJ3D();
    //void Init() {}
    void Clear();
    void Update();
    void Draw();
};

class OBJManager
{
    std::list<std::shared_ptr<OBJ3D>> objList;

public:
    virtual void init()
    {
        objList.clear();

    }

    virtual void update(DirectX::XMFLOAT4X4 view, DirectX::XMFLOAT4X4 projection);
    virtual void draw(ID3D11DeviceContext* immediate_context, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time);
    virtual void draw(ID3D11DeviceContext* immediate_context, Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor, float edgeThershood, float toonThereshood, float shadowbias);
    //virtual void draw(ID3D11DeviceContext* immediate_context, Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time, DirectX::XMFLOAT4 brightColor, DirectX::XMFLOAT4 darkColor);
   // virtual void draw(ID3D11DeviceContext* immediate_context,Shader* shader, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4X4 projection, DirectX::XMFLOAT4 light_direction, std::shared_ptr<Camera> camera, float elapsed_time);
    //void draw(ID3D11DeviceContext* immediate_context, DirectX::XMFLOAT4X4 view_projection, DirectX::XMFLOAT4 light_direction,Camera* camera, float elapsed_time);
   // void draw(ID3D11DeviceContext* immediate_context, DirectX::XMFLOAT4X4 view_projection);
    std::map<std::string, std::shared_ptr<OBJ3D>> objects;
    virtual void uninit();
    std::shared_ptr<OBJ3D> Add(ID3D11Device* device, MoveAlg* movealg, VECTOR3 pos, const char* filename, std::string tag, bool has_fbm,bool has_blade);



public:
    //std::shared_ptr<OBJ3D> Add(ID3D11Device* device, MoveAlg* movealg, VECTOR3 pos, const char* filename,  std::string tag);
    std::list<std::shared_ptr<OBJ3D>>& getList() { return objList; }                // objList���擾����
    DirectX::XMFLOAT4X4 view;



};