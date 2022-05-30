#pragma once

#include "OBJ3D.h"
#include "command.h"
#include "cereal.h"
#include "collision.h"

enum CHARASTATE
{
    STATE_NONE,
    STATE_ATTACK,
    STATE_DOWN,
    STATE_GUARD,
    STATE_JUMP,
    STATE_DAMAGE,
    STATE_GETUP,
    STATE_LANDING,
    STATE_DETH,
    STATE_WIN,
    STATE_FRONT_STEP,
    STATE_BACK_STEP,
    STATE_STAN,
    STATE_INTRO,
};


//FighterBaseクラス
//全ファイターの中核になる予定のクラス
//今は1キャラのみだが卒業制作ではアートの友人に協力してもらい
//キャラを増やす予定
//仮想関数にすることで必要な部分や固有使用以外は使い回しが聞くように設計
//攻撃編集ツールとしての側面もある


class FighterBase  {

private:

    enum STATE
    {
        state_INIT,
        state_UPDATE,
        state_UNINIT,
    };

    enum ANIM_STATE : int
    {
        ANIM_STANDING,
        ANIM_FOWERD,
        ANIM_BACKWERD,
        ANIM_JUMP,
        ANIM_SQAUT,
        ANIM_SQGUARD,
        ANIM_GUARD,
        ANIM_AIRGUARD,
        ANIM_ATTACK_A,
        ANIM_ATTACK_B,
        ANIM_ATTACK_X,
        ANIM_ATTACK_Y,
        ANIM_ATTACK_RB,

        ANIM_ATTACK_A_AIR,
        ANIM_ATTACK_B_AIR,
        ANIM_ATTACK_X_AIR,
        ANIM_ATTACK_Y_AIR,

        ANIM_ATTACK_A_CROUCH, 
        ANIM_ATTACK_B_CROUCH,
        ANIM_ATTACK_X_CROUCH,
        ANIM_ATTACK_Y_CROUCH,

        ANIM_ATTACK_236_A,
        ANIM_DAMAGE,
        ANIM_DOWN,
        ANIM_DETH,
        ANIM_ROLL,
        ANIM_WIN,
        ANIM_GETUP,
        ANIM_INTRO,

        ANIM_STAN,

        ANIM_FRONT_STEP,
        ANIM_BACK_STEP,

        ANIM_LANDING,

        ANIM_END

    };

    enum ATTACKENTRY : int
    {
        NONE,
        GROUND_A,
        GROUND_B,
        GROUND_X,
        GROUND_Y,

         AIR_A,
         AIR_B,
         AIR_X,
         AIR_Y,

         CROUCH_A,
         CROUCH_B,
         CROUCH_X,
         CROUCH_Y,

         GROUND_RB,
         AIR_RB,


        HDOU_A,
        END
    };

    enum  ATTACKNUM : int
    {
        GA,
        GB,
        GX,
        GY,

        HDOUA,
        RB,
        LB,
        AIRA,
        AIRB,
        AIRX,
        AIRY,

        CROUCHA,
        CROUCHB,
        CROUCHX,
        CROUCHY,



       
        UPPERCUT,
        END_
    };


private:
    
    std::unique_ptr<Stick_log> logstack;
    std::unique_ptr<CommandList> command;

    std::unique_ptr <attack_range> attacks[30] = {};

    ComParse com[20];

    std::shared_ptr<GamePad> pad;

    std::array<int, 20> 	histry = {};	//listのデータを引数で受け取る
    std::array<int, 20> 	histtimer = {};	//listのデータを引数で受け取る
    int timer = 0;
    int damage_timer = 0;
    int guard_timer = 0;
    int wintimer = 0;
    bool animtriger = false;

    bool backword = false;
    bool squat = false;

    int frametimer = 0;

    
    int stick_dir = STICKDIRECTION::STICK_N;
    int button_num = BUTTONNUM::BUTTON_NO;
    int old_attack = ATTACKENTRY::NONE;

    int now_attack = ATTACKENTRY::NONE;

   // int attackbits[30];


    int anim_state = ANIM_STATE::ANIM_STANDING;

    ATTACKENTRY attackentry_point = ATTACKENTRY::NONE;

    const float PL_SPEED = 100.0f;

    const float MAX_SPEED = 0.5f;

     float JUMP_SPEED = 450.0f;

    const int ATTACKS_MAX = 30;


    float ANIM_SPEED = 1.0f;

    float ANIMSPEED[30] = {};

    int skill_rigidity_time[30] = {};
    int skill_cancel_limit_time[30] = {};

    float skill_attenuation_rate[30] = {};

    int rigidity_time = 0;

    int cancel_limit_time = 0;
    int cancel_timer = 0;

    int skill_invincible_time[30] = {};

     int  land_time = 10;

    int invincibletime = 0;

    int win_timer = 0;
    int step_timer = 0;

    float cancel_frame = 0.0f; //キャンセル受付

    int buttonState[2] = { 0 };

    bool isCPU = false;
    bool isPursuit = false;
    bool isAttack_EDIT = false;
    bool isEnemy_Hit = false;
    bool isEnemy_damegestate = false;

    bool iscommand = false;


    std::string fightername = "Test_fighter";


    VECTOR2                 extrude;
    VECTOR2                 head;
    VECTOR2                 body;
    VECTOR2                 reg;
    VECTOR2                 checkarea;
    VECTOR2                 tempcheck_size;
    VECTOR2                 harf_checkarea;
    VECTOR4                 color1p;
    VECTOR4                 color2p;


    void LoadData(std::string _file_name, OBJ3D* obj);


    friend class cereal::access;
    template<class Archive>
    void serialize(Archive& archive, std::uint32_t version)
    {
        if (version >= 0)
        {
            archive
            (
               
                
                
              
                CEREAL_NVP(extrude.x),
                CEREAL_NVP(extrude.y),
                CEREAL_NVP(head.x),
                CEREAL_NVP(head.y),
                CEREAL_NVP(body.x),
                CEREAL_NVP(body.y),
                CEREAL_NVP(reg.x),
                CEREAL_NVP(reg.y),
                CEREAL_NVP(checkarea.x),
                CEREAL_NVP(checkarea.y),
                CEREAL_NVP(color1p.x),
                CEREAL_NVP(color1p.y),
                CEREAL_NVP(color1p.z),
                CEREAL_NVP(color1p.w),
                CEREAL_NVP(color2p.x),
                CEREAL_NVP(color2p.y),
                CEREAL_NVP(color2p.z),
                CEREAL_NVP(color2p.w)
            );
        }
        if (version >= 1)
        {
            archive
            (
                CEREAL_NVP(fightername)
            );

        }
    }
   

public:
    FighterBase(std::shared_ptr<GamePad> pd);
  virtual void init(OBJ3D* obj);
  virtual void update(OBJ3D* obj);

  virtual void uninit(OBJ3D* obj);
  virtual void move(OBJ3D* obj);
  virtual void areaCheck(OBJ3D* obj);
  virtual void imgui_update(OBJ3D* obj);
  virtual void imgui_bot_update(OBJ3D* obj);
  virtual void imgui_command_update(OBJ3D* obj);



  void imgui_attack_update(OBJ3D* obj);

  virtual void animationmanagement(OBJ3D* obj);
  virtual void animator(OBJ3D* obj);
  virtual void atkanimator(OBJ3D* obj);
  virtual bool atack_move(OBJ3D* obj);

  virtual void air_atack_move(OBJ3D* obj);

  virtual void saveattk();
  virtual void loadattk();



  virtual void setanimation(int _anim_state) 
  {
      if (anim_state != _anim_state  )
      {
          animtriger = false;
      }

      anim_state = _anim_state;
   


  }

  virtual void set_rigidity(int rigidity)
  {

      rigidity_time = rigidity;


  }

  virtual void set_cancel(int cancel)
  {

      cancel_limit_time = cancel;


  }


  virtual void set_invincible(int invincible)
  {

      invincibletime = invincible;


  }


  virtual void  behavior(OBJ3D* obj);
 // virtual void  botbehavior(OBJ3D* obj);
  virtual void input_manager();
  virtual void input_cpu_algorithm();

   // ジャンプ処理
  void Jump(float speed, OBJ3D* obj);

  int stantimer = 0;

   Command commandstate;

   CHARASTATE state;

};



class player1 : public OBJManager {
public:

private:
    void init();
};

//インスタンス取得 *現在の設計上の都合で各プレイヤーをこれでしか参照できない
#define pl1 player1::Instance().getList().begin()->get()


class player2 : public OBJManager {
public:
private:
    void init();
};

#define pl2 player1::Instance().getList().begin()->get()



