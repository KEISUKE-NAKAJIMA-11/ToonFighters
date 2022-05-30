#include "fighter_base.h"
#include "user.h"
#include "util.h"
#include "framework.h"
#include "judge_manager.h"
#include "myutil.h"







FighterBase::FighterBase(std::shared_ptr<GamePad> pd):pad(pd)
{
    for (int i = 0; i < ATTACKS_MAX; i++)
    {
        attacks[i] = std::make_unique<attack_range>();
    }
}


void FighterBase::LoadData(std::string _file_name, OBJ3D* obj)
{
    fightername = _file_name;

  
    if (Load("./Data/Info/FightersData/" + _file_name + ".bin", "fighters", *this))
    {
        obj->head_size = head;
        obj->body_size = body;
        obj->reg_size = reg;
        obj->size = extrude;
        obj->checksize = checkarea;
        tempcheck_size = obj->checksize;
        harf_checkarea = checkarea / 2;
        if (obj->tag == "PLAYER1")
        {
            obj->color = color1p;
        }

        else if (obj->tag == "PLAYER2")
        {
            obj->color = color2p;
        }
       
    }
    else if (Load("./Data/Info/FightersData/" + _file_name + ".json", "fighters", *this))
    {
        obj->head_size = head;
        obj->body_size = body;
        obj->reg_size = reg;
        obj->size = extrude;
        obj->checksize = checkarea;
        tempcheck_size = obj->checksize;
        harf_checkarea = checkarea / 2;
     
        if (obj->tag == "PLAYER1")
        {
            obj->color = color1p;
        }

        else if (obj->tag == "PLAYER2")
        {
            obj->color = color2p;
        }
 
    }
    else
    {
        obj->size = VECTOR2(71, 435);
        obj->head_size = VECTOR2(20, 45);
        obj->body_size = VECTOR2(64, 107);
        obj->reg_size = VECTOR2(82, 275);
        obj->checksize = VECTOR2(71, 435);
        obj->color = VECTOR4(1, 1, 1, 1);
     
    }

    loadattk();

 }



void FighterBase::init(OBJ3D* obj)
{


    obj->initpos = obj->position;
    obj->oldpos = obj->position;

    


    /*obj->size = VECTOR2(71,435);

    obj->head_size = VECTOR2(20, 45);
    obj->body_size = VECTOR2(64, 107);
    obj->reg_size =  VECTOR2(82, 275);*/

    LoadData(fightername,obj);

    obj->guard_elements = ATTACK_ELEMENT_UPPER | ATTACK_ELEMENT_MIDDLE;

    if (obj->tag == "PLAYER1")
    {
        obj->player_num = PLAYER_NUM::PLAYER1P;
    }

    else if (obj->tag == "PLAYER2")
    {
        obj->player_num = PLAYER_NUM::PLAYER2P;
    }


    obj->is_judge = true;

    obj->is_guard = false;

    logstack = std::make_unique<Stick_log>();

    command = std::make_unique<CommandList>();
    commandstate = Command::NOCOMMAND;

    //obj->model->PlayAnimation(0, true);

    obj->angle.y = 2.0f;

    state = CHARASTATE::STATE_NONE;
    
}


void FighterBase::update(OBJ3D* obj)
{
    float delta_time=framework::Instance().elapased_time;
    float dist = 0.0f;
    //パラメーターセーブ用
       head = obj->head_size;
       body = obj->body_size;
       reg = obj->reg_size;
       extrude = obj->size;
     
       checkarea = obj->checksize;

       if (obj->ishit)
       {
           int a = 0;
       }
       
       //color = obj->color;

       if (obj->tag == "PLAYER1")
       {
           obj->color = color1p;
           dist = judge_manager::Instance().get_1pdistance();
           isEnemy_Hit = judge_manager::Instance().get_2p_hit();
       }

       else if (obj->tag == "PLAYER2")
       {
           obj->color = color2p;
           dist = judge_manager::Instance().get_2pdistance();
           isEnemy_Hit = judge_manager::Instance().get_1p_hit();
       }

       if (!obj->isGround)
       {
           obj->checksize = harf_checkarea;
       }

       else
       {
           obj->checksize = tempcheck_size;
       }

       if (UI_Manager::Instance().Get_Roundstart())
       {

           timer++;

           frametimer++;
         

           if (!UI_Manager::Instance().Get_KOflag())
           {
               if (!isCPU)
               {
                   input_manager();
               }
               else
               {
                   input_cpu_algorithm();
               }




           }

           else
           {


               if (obj->health <= 0)
               {

                   //setanimation(ANIM_STATE::ANIM_DETH);
                   obj->ishit = false;
                   if (damage_timer <= 0)
                   {
                       state = CHARASTATE::STATE_DETH;
                   }


               }
               else
               {
                   state = CHARASTATE::STATE_NONE;
                   stick_dir = STICK_N;
                   wintimer++;
                   if (wintimer >= 60 * 2)
                   {
                       state = CHARASTATE::STATE_WIN;

                   }
                   obj->ishit = false;
                   // setanimation(ANIM_STATE::ANIM_WIN);

               }

               stick_dir = STICKDIRECTION::STICK_N;
               button_num = BUTTONNUM::BUTTON_NO;

               attackentry_point = ATTACKENTRY::NONE;

           }

           if (!UI_Manager::Instance().Get_KOflag())
           {
               areaCheck(obj);

           }

           else
           {


               if (obj->health <= 0)
               {


                   state = CHARASTATE::STATE_DETH;

               }
               else
               {
                   if (obj->isGround)
                   {
                       state = CHARASTATE::STATE_WIN;
                   }
             

               }

           }
          


           if (framework::Instance().useimgui)
           {
               imgui_update(obj);

               imgui_bot_update(obj);

               imgui_attack_update(obj);

               imgui_command_update(obj);
           }



          obj->position +=  obj->velocity * delta_time;

           obj->position.z = 0.0f;
       }

       else if (!UI_Manager::Instance().Get_Roundstart() && UI_Manager::Instance().state == 1)
    {
           state = CHARASTATE::STATE_INTRO;

    }

       behavior(obj);
       animator(obj);
       animationmanagement(obj);

 

        if (dist < 0)
        {
            if (obj->isGround && state!=CHARASTATE::STATE_ATTACK && state != CHARASTATE::STATE_JUMP)
            {
                obj->scale.x = -1.0f;
                obj->angle.y = -2.0f;
            }


        }
        else
        {
            if (obj->isGround && state != CHARASTATE::STATE_ATTACK && state != CHARASTATE::STATE_JUMP)
            {
                obj->scale.x = 1.0f;
                obj->angle.y = 1.6f;
            }

        }
 
}



float ABS_Indicate(float x)
{
    //if (x < 0) return -x;
    //return x;
    return x < 0 ? -x : x;
}



void FighterBase::input_manager()
{
    int i = 0;

    float stick = 0.5f;



    if (pad->GetAxisLX()>=stick)
    {
        stick_dir = STICKDIRECTION::STICK_RIGHT;
    }

    else if (pad->GetAxisLX() <= -stick)
    {
        stick_dir = STICKDIRECTION::STICK_LEFT;
    }

    else  if (pad->GetAxisLY() >= stick)
    {
        stick_dir = STICKDIRECTION::STICK_UP;
    }

     else if (pad->GetAxisLY() <= -stick)
     {
         stick_dir = STICKDIRECTION::STICK_DOWN;
     }

  

      if (pad->GetAxisLX() >= stick && pad->GetAxisLY() >= stick)
     {
         stick_dir = STICKDIRECTION::STICK_RIGHT_UP;
     }

     else if(pad->GetAxisLX() >= stick && pad->GetAxisLY() <= -stick)
     {
         stick_dir = STICKDIRECTION::STICK_RIGHT_DOWN;
     }


     else if (pad->GetAxisLX() <= -stick && pad->GetAxisLY() >= stick)
     {
         stick_dir = STICKDIRECTION::STICK_LEFT_UP;
     }

     else if (pad->GetAxisLX() <= -stick && pad->GetAxisLY() <= -stick)
     {
         stick_dir = STICKDIRECTION::STICK_LEFT_DOWN;
     }

      if (pad->GetAxisLX() == 0 && pad->GetAxisLY() == 0)
      {
          stick_dir = STICKDIRECTION::STICK_N;
      }



      if (pad->GetButtonDown() & GamePad::BTN_A)
      {
          button_num = BUTTONNUM::BUTTON_A;
      }

      else if (pad->GetButtonDown() & GamePad::BTN_B)
      {
          button_num = BUTTONNUM::BUTTON_B;
      }


      else if (pad->GetButtonDown() & GamePad::BTN_X)
      {
          button_num = BUTTONNUM::BUTTON_X;
      }

      else if (pad->GetButtonDown() & GamePad::BTN_Y)
      {
          button_num = BUTTONNUM::BUTTON_Y;
      }

      else if (pad->GetButtonDown() & GamePad::BTN_LEFT_THUMB)
      {
          button_num = BUTTONNUM::BTN_LEFT_THUMB;
      }


      else if (pad->GetButtonDown() & GamePad::BTN_LEFT_SHOULDER)
      {
          button_num = BUTTONNUM::BTN_LEFT_SHOULDER;
      }

      else if (pad->GetButtonDown() & GamePad::BTN_LEFT_TRIGGER)
      {
          button_num = BUTTONNUM::BTN_LEFT_TRIGGER;
      }

      else if (pad->GetButtonDown() & GamePad::BTN_RIGHT_THUMB)
      {
          button_num = BUTTONNUM::BTN_RIGHT_THUMB;
      }


      else if (pad->GetButtonDown() & GamePad::BTN_RIGHT_SHOULDER)
      {
          button_num = BUTTONNUM::BTN_RIGHT_SHOULDER;
      }

      else if (pad->GetButtonDown() & GamePad::BTN_RIGHT_TRIGGER)
      {
          button_num = BUTTONNUM::BTN_RIGHT_TRIGGER;
      }

      else
      {
          button_num = BUTTONNUM::BUTTON_NO;
      }


    logstack->update(stick_dir, command.get(),frametimer);


    for (auto itr = logstack->sticklog.begin(); itr != logstack->sticklog.end(); ++itr)
    {
        histry[i] = *itr;
        i++;
    }

    i = 0;

    for (auto itr = logstack->timerlog.begin(); itr != logstack->timerlog.end(); ++itr)
    {
        histtimer[i] = *itr;
        i++;
    }

}

void FighterBase::input_cpu_algorithm()
{

}



void FighterBase::Jump(float speed, OBJ3D* obj)
{
    // 上方向の力を設定
    obj->velocity.y = speed;
}


void FighterBase::uninit(OBJ3D* obj)
{
}

void FighterBase::move(OBJ3D*obj)
{
    switch (obj->state)
    {
    case STATE::state_INIT:
        init(obj);
        obj->state++;
    case STATE::state_UPDATE:
        update(obj);
        break;
    case STATE::state_UNINIT:
        uninit(obj);
        break;
    }

}

void FighterBase::areaCheck(OBJ3D* obj)
{

    const float correction_velocity = 30.0f;

    float delta_time = framework::Instance().elapased_time;




    float oldX = obj->screen_position.x;   // x座標を一時的に保存
    obj->screen_position.x = clamp(obj->screen_position.x, obj->checksize.x, pSystem.SCREEN_WIDTH - (obj->checksize.x*2));
    if (oldX != obj->screen_position.x)
    {
        VECTOR3 POS;
        
        judge_manager::Instance().ScreenToWorld(obj->screen_position, POS, obj->view, obj->projection);

        obj->position = POS;

        obj->iswall = true;

        obj->velocity.x = 0;  // 保存した値と違えば画面端にあたっている


    }

    else
    {
        obj->iswall = false;
    }

    float oldY = obj->screen_position.y;   // x座標を一時的に保存

    obj->screen_position.y = clamp(obj->screen_position.y, obj->size.y, (pSystem.SCREEN_HEIGHT) - obj->size.y/4);

}


void FighterBase::behavior(OBJ3D* obj)
{
    float delta_time = framework::Instance().elapased_time;
    
    const int GUARD_INVISIVLE = 10;

    float dist = 0.0f;

    if (obj->tag=="PLAYER1")
    {
        dist = judge_manager::Instance().get_1pdistance();
    }

    else if (obj->tag == "PLAYER2")
    {
        dist = judge_manager::Instance().get_2pdistance();
    }

    invincibletime--;
    if (invincibletime <= 0)
    {
        obj->is_invisible = false;
    }

    else
    {
        obj->is_invisible = true;
    }



    if (obj->stanpoint >= 0.1f)
    {
        obj->stanpoint -= 0.5f;
    }

    if (obj->stanpoint <= 0.0f)
    {
        obj->stanpoint = 0.1f;
    }


       switch (state)
       {
       case CHARASTATE::STATE_NONE:

          
           if (!UI_Manager::Instance().Get_Roundstart())
           {
               state = CHARASTATE::STATE_INTRO;
              
               break;
           }


           isEnemy_damegestate = false;

           if (obj->ishit)
           {
               state = CHARASTATE::STATE_DAMAGE;
               //if (obj->isGround)
               //{
               //   // setanimation(ANIM_STATE::ANIM_DAMAGE);
               //}
               //else
               //{
               //    //setanimation(ANIM_STATE::ANIM_DOWN);
               //}
               obj->hitstop_timer = 0;
               damage_timer = 0;
               break;
           }



         

           obj->velocity.x = 0;

           if (obj->success_guard)
           {
               state = CHARASTATE::STATE_GUARD;
               break;
           }
          
      


           rigidity_time--;
           
           if (rigidity_time <= 0)
           {
               rigidity_time = 0;
           }
           

           if (rigidity_time > 0) break;


           if (!isCPU)
           {
               commandstate = command->CommandCheack(pad->GetButtonDown() & GamePad::BTN_A, histry.data(), histtimer.data(), dist, timer);
           }


           if (!UI_Manager::Instance().Get_KOflag())
           {
               if (atack_move(obj)) break;
           }

        

           squat = false;

           obj->emissionEdgeColor = { 0, 0, 0, 1 };

           attackentry_point = ATTACKENTRY::NONE;

       if (obj->isGround && stick_dir == STICKDIRECTION::STICK_LEFT)
       {
           obj->velocity.x = -PL_SPEED;

           if (dist > 0)
           {
               obj->guard_elements = ATTACK_ELEMENT_UPPER | ATTACK_ELEMENT_MIDDLE;
               obj->is_guard = true;
               backword = true;
         
           }

           else
           {
               obj->is_guard = false;
               backword = false;
           }

       }

       else if (obj->isGround && stick_dir == STICKDIRECTION::STICK_RIGHT)
       {
           obj->velocity.x = PL_SPEED;


           if (dist < 0)
           {
               obj->guard_elements = ATTACK_ELEMENT_UPPER | ATTACK_ELEMENT_MIDDLE;
               obj->is_guard = true;
               backword = true;
           }

           else
           {
               obj->is_guard = false;
               backword = false;
           }

     
          

       }

         if (obj->isGround && stick_dir == STICKDIRECTION::STICK_N)
       {
           obj->velocity.x = 0;

           obj->is_guard = false;

        
           //setanimation(ANIM_STATE::ANIM_STANDING);
       }



       if (obj->isGround)
       {
           if (stick_dir == STICKDIRECTION::STICK_DOWN)
           {
          
               obj->velocity.x = 0;
           
               //setanimation(ANIM_STATE::ANIM_SQAUT);

               squat = true;
               
           }

       

           if (stick_dir == STICKDIRECTION::STICK_LEFT_DOWN)
               {
                 
                   obj->velocity.x = 0;
                   
                   squat = true;

                   if (dist > 0)
                   {
                       obj->guard_elements = ATTACK_ELEMENT_LOWER | ATTACK_ELEMENT_MIDDLE;
                       obj->is_guard = true;
                   }

                   else
                   {
                       obj->is_guard = false;
                   }

                   //setanimation(ANIM_STATE::ANIM_SQAUT);
           
               }
       

   

               if (stick_dir == STICKDIRECTION::STICK_RIGHT_DOWN)
               {

                   obj->velocity.x = 0;

                   squat = true;

                   if (dist < 0)
                   {
                       obj->guard_elements = ATTACK_ELEMENT_LOWER | ATTACK_ELEMENT_MIDDLE;
                       obj->is_guard = true;
                   }

                   else
                   {
                       obj->is_guard = false;
                   }

                   //setanimation(ANIM_STATE::ANIM_SQAUT);
                   
               }
           }
       
       if (obj->isGround)
       {
           if (stick_dir == STICKDIRECTION::STICK_UP)
           {
               Jump(JUMP_SPEED, obj);

               Audio::GetSound("jump")->Play();
               //  setanimation(ANIM_STATE::ANIM_JUMP);

               obj->isGround = false;

               state = CHARASTATE::STATE_JUMP;

           }


           else if (stick_dir == STICKDIRECTION::STICK_LEFT_UP)
           {
               Jump(JUMP_SPEED, obj);
               Audio::GetSound("jump")->Play();
               obj->velocity.x = -PL_SPEED;
               // setanimation(ANIM_STATE::ANIM_JUMP);
               obj->isGround = false;
               state = CHARASTATE::STATE_JUMP;
           }

           else if (stick_dir == STICKDIRECTION::STICK_RIGHT_UP)
           {
               Jump(JUMP_SPEED, obj);

               Audio::GetSound("jump")->Play();
               obj->velocity.x = PL_SPEED;
               // setanimation(ANIM_STATE::ANIM_JUMP);
               obj->isGround = false;
               state = CHARASTATE::STATE_JUMP;
           }
       }
     
    
       if (obj->guard_redy)
       {
           obj->velocity.x = 0;

           state = CHARASTATE::STATE_GUARD;

           set_invincible(GUARD_INVISIVLE);

       }


      
       old_attack = ATTACKENTRY::NONE;

       break;

       case CHARASTATE::STATE_ATTACK:

           if (obj->ishit)
           {
               state = CHARASTATE::STATE_DAMAGE;

               attackentry_point = ATTACKENTRY::NONE;


               set_rigidity(0);


               break;
           }

          
           if (isEnemy_Hit)
           {
               if (!isEnemy_damegestate) isEnemy_damegestate = true;
           }
           if (isEnemy_damegestate)
               {

                   cancel_timer++;

                   if (cancel_timer < cancel_limit_time || obj->model->GetCurrentSeconds() > 10)
                   {
                       if (obj->isGround)
                       {

                           atack_move(obj);
                           commandstate = command->CommandCheack(pad->GetButtonDown() & GamePad::BTN_A, histry.data(), histtimer.data(), dist, timer);


                       }


                       else if (!obj->isGround)
                       {
                           if (cancel_timer < cancel_limit_time || obj->model->GetCurrentSeconds() > 10)
                           {
                               commandstate = command->CommandCheack(pad->GetButtonDown() & GamePad::BTN_A, histry.data(), histtimer.data(), dist, timer);

                               air_atack_move(obj);
                           }
                       }

                   }


                  
                       

                  
               }
           

           if (obj->isGround && attackentry_point >= ATTACKENTRY::AIR_A && attackentry_point <= ATTACKENTRY::AIR_Y)
           {
               rigidity_time = 0;
               attackentry_point = ATTACKENTRY::NONE;
               state = CHARASTATE::STATE_LANDING;

               break;
           }

           if (obj->isGround && !obj->model->IsPlayAnimation())
           {
               //cancel_frame = 0;

               
               state = CHARASTATE::STATE_NONE;
               commandstate = Command::NOCOMMAND;
               cancel_timer = 0;
               //obj->velocity.x = 0;

               attackentry_point = ATTACKENTRY::NONE;

               
           }

           break;


       case CHARASTATE::STATE_JUMP:

           if (obj->ishit)
           {
               state = CHARASTATE::STATE_DAMAGE;

               attackentry_point = ATTACKENTRY::NONE;

               
           }


           if (obj->success_guard)
           {
               state = CHARASTATE::STATE_GUARD;
           }


           if (obj->isGround)
           {
               //cancel_frame = 0;
               state = CHARASTATE::STATE_LANDING;

              //commandstate = Command::NOCOMMAND;

               //attackentry_point = ATTACKENTRY::NONE;


           }

           air_atack_move(obj);

           break;


       case CHARASTATE::STATE_DOWN:
           if (obj->ishit)
           {
               damage_timer = 0;
               state = CHARASTATE::STATE_DAMAGE;
           }

           if (obj->isGround && !obj->model->IsPlayAnimation())
           {              
               state = CHARASTATE::STATE_GETUP;
               commandstate = Command::NOCOMMAND;
               obj->is_invisible = true;

           }



           break;

       case CHARASTATE::STATE_FRONT_STEP:


           if (obj->ishit)
           {
               state = CHARASTATE::STATE_DAMAGE;

               attackentry_point = ATTACKENTRY::NONE;


              // obj->velocity.x = 0;

               /*  if (obj->isGround)
                 {
                     setanimation(ANIM_STATE::ANIM_DAMAGE);
                 }
                 else
                 {
                     setanimation(ANIM_STATE::ANIM_ROLL);
                 }*/

               set_rigidity(10);
           }

           step_timer++;
           if (step_timer >=20)
           {
             
               {
                   //cancel_frame = 0;
                   obj->velocity.x = 0;
                   state = CHARASTATE::STATE_NONE;
                   //obj->ishit = false;
                   commandstate = Command::NOCOMMAND;
                   step_timer = 0;


                   set_rigidity(0);
               }
           }


           break;

       case CHARASTATE::STATE_BACK_STEP:


           if (obj->ishit)
           {
               state = CHARASTATE::STATE_DAMAGE;

               attackentry_point = ATTACKENTRY::NONE;


               //obj->velocity.x = 0;

               /*  if (obj->isGround)
                 {
                     setanimation(ANIM_STATE::ANIM_DAMAGE);
                 }
                 else
                 {
                     setanimation(ANIM_STATE::ANIM_ROLL);
                 }*/

               set_rigidity(10);
           }

           step_timer++;
           if (step_timer >= 20)
           {
               //
               {
                   //cancel_frame = 0;
                   obj->velocity.x = 0;
                   state = CHARASTATE::STATE_NONE;
                   //obj->ishit = false;
                   commandstate = Command::NOCOMMAND;
                   step_timer = 0;


                   set_rigidity(5);
               }
           }


           break;


       case CHARASTATE::STATE_LANDING:

           obj->velocity.x = 0;

           land_time -= (1 * delta_time);

           if (!land_time <=0)
           {
               land_time = 10;
             state = CHARASTATE::STATE_NONE;
             commandstate = Command::NOCOMMAND;
             cancel_timer = 0;

           }

           break;

       case CHARASTATE::STATE_GETUP:


           obj->velocity.x = 0;

           


           if (!obj->model->IsPlayAnimation())
           {
               //cancel_frame = 0;
               state = CHARASTATE::STATE_NONE;
               obj->ishit = false;
               obj->is_invisible = false;

           }

           break;

       case CHARASTATE::STATE_DAMAGE:

         
          
               damage_timer++;

               obj->emissionEdgeColor = { 1, 0, 0, 1 };

               if (obj->stanpoint > 400)
               {
                   if (obj->isGround)
                   {
                       Jump(JUMP_SPEED, obj);
                   }
                  
                   state = CHARASTATE::STATE_STAN;
                   obj->stanpoint = 0.1f;
                   obj->ishit = false;

                   stantimer = 0;

                   obj->is_invisible = true;

                   break;
               }


               if (damage_timer >= (15))
               {
                   if (!obj->isGround)
                   {
                      state = CHARASTATE::STATE_DOWN;
                  
                   }

                   else
                   {
                      // if (!obj->model->IsPlayAnimation())
                       {
                           state = CHARASTATE::STATE_NONE;
                         
                       }
                     
                   }
        
                   obj->ishit = false;
                   damage_timer = 0;
             
               }
             
           

           
           break;

       case CHARASTATE::STATE_GUARD:

           if (obj->guard_redy)
           {
               obj->guard_redy = false;
           }


           guard_timer++;

           if (guard_timer >= (1000 * delta_time))
           {
               state = CHARASTATE::STATE_NONE;

               obj->success_guard = false;

               guard_timer = 0;

               obj->velocity.x = 0;
           }


           break;

       case CHARASTATE::STATE_WIN:

           if (!UI_Manager::Instance().Get_KOflag())
           {
               state = CHARASTATE::STATE_NONE;
               win_timer = 0;
           }

           obj->velocity.x = 0;
           //obj->velocity.y = 0;
           break;

       case CHARASTATE::STATE_DETH:

           if (!UI_Manager::Instance().Get_KOflag())
           {
               state = CHARASTATE::STATE_NONE;
           }
           obj->velocity.x = 0;
           break;


       case CHARASTATE::STATE_INTRO:

              obj->stanpoint = 0.1f;

           if (UI_Manager::Instance().Get_Roundstart())
           {
            
               state = CHARASTATE::STATE_NONE;
           }
           obj->velocity.x = 0;
           break;


       case CHARASTATE::STATE_STAN:
           
           obj->velocity.x = 0;


           stantimer++;

           if (obj->isGround)
           {
               obj->is_invisible = false;
           }


           if (obj->ishit)
           {
               state = CHARASTATE::STATE_DAMAGE;
           }

           if (stantimer>=(60*6))
           {
              
               state = CHARASTATE::STATE_NONE;
           }
      

           break;
   

       }

 
}



bool FighterBase::atack_move(OBJ3D* obj)
{


    float delta_time = framework::Instance().elapased_time;



    float dist = 0.0f;

    if (obj->tag == "PLAYER1")
    {
        dist = judge_manager::Instance().get_1pdistance();
    }

    else if (obj->tag == "PLAYER2")
    {
        dist = judge_manager::Instance().get_2pdistance();
    }

    //if (commandstate == Command::RFUJIN && pad->GetButtonDown() & GamePad::BTN_A)
           //{
           //    ANIM_SPEED = 1.0f;
           //    obj->model->PlayAnimation(5, false, ANIM_SPEED);
           //    // commandstate = Command::NOCOMMAND;
           //    judge_manager::Instance()._attack_manager->add_attack_range(
           //        (dist >= 0) ? obj->screen_position.x + 64 : obj->screen_position.x - (32 * 5), obj->screen_position.y - (32 * 3),
           //        (dist >= 0) ? obj->screen_position.x + (32 * 5) : obj->screen_position.x - (32 * 2),
           //        obj->screen_position.y - (32 * 1), 12, obj->player_num,
           //        ATTACK_ELEMENT_MIDDLE, 1, VECTOR2((40 * dist) * delta_time, 100 * delta_time), 1000 * delta_time / ANIM_SPEED, 150);

           //    obj->velocity.x = 0;
           //    command->command_timer = TIMER_LIMIT;
           //    state = CHARASTATE::STATE_ATTACK;


           //}



    switch (commandstate)
    {

#if 1 //行数が多いので折りたためるようにする

    case Command::NOCOMMAND:

        if (obj->isGround)
        {

            if (!squat)
            {
                
                if (button_num == BUTTONNUM::BUTTON_A)
                {
                
                    if (attackentry_point == ATTACKENTRY::GROUND_A) break;
                    now_attack = ATTACKENTRY::GROUND_A;

                    if (old_attack != now_attack)
                    {

                       judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::GA].get());
                       isPursuit = attacks[ATTACKNUM::GA]->isPursuit;
                       //cancel_frame = 20.0f;
                       
                       set_rigidity(skill_rigidity_time[ATTACKNUM::GA]);
                       set_cancel(skill_cancel_limit_time[ATTACKNUM::GA]);
                       set_invincible(skill_invincible_time[ATTACKNUM::GA]*delta_time);
                       
                       obj->velocity.x = (10 * dist);
                       
                       cancel_timer = 0;

                       attackentry_point = ATTACKENTRY::GROUND_A;
                       
                       old_attack = attackentry_point;
                       
                       //  commandstate = Command::NOCOMMAND;
                       command->command_timer = TIMER_LIMIT;
                       state = CHARASTATE::STATE_ATTACK;
                       
                       
                       
                       return true;
                   }
                    break;
                }


                if (button_num == BUTTONNUM::BUTTON_B)
                {

                    //setanimation(ANIM_STATE::ANIM_ATTACK_B);


                 /*   judge_manager::Instance()._attack_manager->add_attack_range(obj,
                        (dist >= 0) ? (32 * 2) : -(32 * 6), -(32 * 10),
                        (dist >= 0) ? (32 * 6) : -(32 * 2),
                        -(32 * 3),
                        500 * delta_time, obj->player_num, ATTACK_ELEMENT_UPPER, 1, (obj->isGround) ? VECTOR2((30 * dist) * delta_time, (400) * delta_time) : VECTOR2((30 * dist) * delta_time, 200 * delta_time), 800 * delta_time / ANIM_SPEED, 150);*/

                    if (attackentry_point == ATTACKENTRY::GROUND_B) break;

                      now_attack = ATTACKENTRY::GROUND_B;

                    if (old_attack != now_attack)
                    {

                        judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::GB].get());
                        isPursuit = attacks[ATTACKNUM::GB]->isPursuit;

                        //cancel_frame = 20.0f;

                        attackentry_point = ATTACKENTRY::GROUND_B;

                        obj->velocity.x = (10 * dist);
                        cancel_timer = 0;
                        set_rigidity(skill_rigidity_time[ATTACKNUM::GB]);
                        set_cancel(skill_cancel_limit_time[ATTACKNUM::GB]);

                        set_invincible(skill_invincible_time[ATTACKNUM::GB]*delta_time);
                       
                        old_attack = now_attack;

                        state = CHARASTATE::STATE_ATTACK;

                        return true;
                    }
                    break;
                }


                if (button_num == BUTTONNUM::BUTTON_X)
                {
              
                    now_attack = ATTACKENTRY::GROUND_X;

                    if (old_attack != now_attack)
                    {

                        judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::GX].get());
                        isPursuit = attacks[ATTACKNUM::GX]->isPursuit;

                        // cancel_frame = 20.0f;
                        cancel_timer = 0;
                        set_rigidity(skill_rigidity_time[ATTACKNUM::GX]);
                        set_cancel(skill_cancel_limit_time[ATTACKNUM::GX]);

                        set_invincible(skill_invincible_time[ATTACKNUM::GX] * delta_time);
                        obj->velocity.x = (10 * dist);

                        old_attack = now_attack;

                        attackentry_point = ATTACKENTRY::GROUND_X;

                        state = CHARASTATE::STATE_ATTACK;

                        return true;
                    }
                    break;
                }


                if (button_num == BUTTONNUM::BUTTON_Y)
                {


                    //setanimation(ANIM_STATE::ANIM_ATTACK_Y);

                   /* judge_manager::Instance()._attack_manager->add_attack_range(obj,
                        (dist >= 0) ? (32 * 3) : -(32 * 5), -(32 * 10),
                        (dist >= 0) ? (32 * 5) : -(32 * 3),
                        -(32 * 7), 12, obj->player_num,
                        ATTACK_ELEMENT_UPPER, 1, VECTOR2((40 * dist) * delta_time, 50 * delta_time), 30 * delta_time / ANIM_SPEED, 150);*/
                    if (attackentry_point == ATTACKENTRY::GROUND_Y) break;


                    now_attack = ATTACKENTRY::GROUND_Y;

                    if (old_attack != now_attack)
                    {
                        attackentry_point = ATTACKENTRY::GROUND_Y;


                        judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::GY].get());
                        isPursuit = attacks[ATTACKNUM::GY]->isPursuit;

                        //cancel_frame = 20.0f;

                        obj->velocity.x = (10 * dist);
                        //Jump(JUMP_SPEED * delta_time, obj);

                        cancel_timer = 0;
                        set_rigidity(skill_rigidity_time[ATTACKNUM::GY]);

                        old_attack = now_attack;

                        set_invincible(skill_invincible_time[ATTACKNUM::GY] * delta_time);
                        set_cancel(skill_cancel_limit_time[ATTACKNUM::GY]);
                        state = CHARASTATE::STATE_ATTACK;


                        return true;
                    }
                    break;
                }
            }
            else
            {
                if (button_num == BUTTONNUM::BUTTON_A)
                {

                    //setanimation(ANIM_STATE::ANIM_ATTACK_A);

                    /*judge_manager::Instance()._attack_manager->add_attack_range(obj,
                        (dist >= 0) ? (32 * 2) : -(32 * 6), -(32 * 10),
                        (dist >= 0) ? (32 * 6) : -(32 * 2),
                        -(32 * 3),
                        500 * delta_time, obj->player_num, ATTACK_ELEMENT_LOWER, 1, VECTOR2((30 * dist) * delta_time, 0), 800 * delta_time / ANIM_SPEED, 150);*/

                    if (attackentry_point == ATTACKENTRY::CROUCH_A) break;

                    now_attack = ATTACKENTRY::CROUCH_A;

                    if (old_attack != now_attack)
                    {

                        judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::CROUCHA].get());
                        isPursuit = attacks[ATTACKNUM::CROUCHA]->isPursuit;

                        //cancel_frame = 20.0f;

                        //obj->velocity.x = (10 * dist);

                        set_rigidity(skill_rigidity_time[ATTACKNUM::CROUCHA]);
                        set_cancel(skill_cancel_limit_time[ATTACKNUM::CROUCHA]);

                        set_invincible(skill_invincible_time[ATTACKNUM::CROUCHA] * delta_time);

                        attackentry_point = ATTACKENTRY::CROUCH_A;
                        cancel_timer = 0;

                        old_attack = now_attack;

                        //  commandstate = Command::NOCOMMAND;
                        command->command_timer = TIMER_LIMIT;
                        state = CHARASTATE::STATE_ATTACK;

                        return true;
                    }

                    break;
                }


                if (button_num == BUTTONNUM::BUTTON_B)
                {

                    //setanimation(ANIM_STATE::ANIM_ATTACK_B);


                 /*   judge_manager::Instance()._attack_manager->add_attack_range(obj,
                        (dist >= 0) ? (32 * 2) : -(32 * 6), -(32 * 10),
                        (dist >= 0) ? (32 * 6) : -(32 * 2),
                        -(32 * 3),
                        500 * delta_time, obj->player_num, ATTACK_ELEMENT_UPPER, 1, (obj->isGround) ? VECTOR2((30 * dist) * delta_time, (400) * delta_time) : VECTOR2((30 * dist) * delta_time, 200 * delta_time), 800 * delta_time / ANIM_SPEED, 150);*/
                    if (attackentry_point == ATTACKENTRY::CROUCH_B) break;

                    now_attack = ATTACKENTRY::CROUCH_B;


                    if (old_attack != now_attack)
                    {

                        judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::CROUCHB].get());
                        isPursuit = attacks[ATTACKNUM::CROUCHB]->isPursuit;

                        //cancel_frame = 20.0f;

                        // obj->velocity.x = (10 * dist) * delta_time;

                        cancel_timer = 0;

                        set_rigidity(skill_rigidity_time[ATTACKNUM::CROUCHB]);
                        set_cancel(skill_cancel_limit_time[ATTACKNUM::CROUCHB]);
                        set_invincible(skill_invincible_time[ATTACKNUM::CROUCHB] * delta_time);

                        attackentry_point = ATTACKENTRY::CROUCH_B;

                        old_attack = now_attack;

                        state = CHARASTATE::STATE_ATTACK;


                        return true;

                    }

                    break;
                }


                if (button_num == BUTTONNUM::BUTTON_X)
                {

                    now_attack = ATTACKENTRY::CROUCH_X;
                    
                    if (old_attack != now_attack)
                    {
                        judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::CROUCHX].get());
                        isPursuit = attacks[ATTACKNUM::CROUCHX]->isPursuit;

                        // cancel_frame = 20.0f;

                         //obj->velocity.x = (10 * dist) * delta_time;

                        set_rigidity(skill_rigidity_time[ATTACKNUM::CROUCHX]);
                        set_cancel(skill_cancel_limit_time[ATTACKNUM::CROUCHX]);
                        set_invincible(skill_invincible_time[ATTACKNUM::CROUCHX] * delta_time);

                        attackentry_point = ATTACKENTRY::CROUCH_X;

                        old_attack = now_attack;

                        state = CHARASTATE::STATE_ATTACK;

                        return true;
                    }

                    break;
                }


                if (button_num == BUTTONNUM::BUTTON_Y)
                {


                    //setanimation(ANIM_STATE::ANIM_ATTACK_Y);

                   /* judge_manager::Instance()._attack_manager->add_attack_range(obj,
                        (dist >= 0) ? (32 * 3) : -(32 * 5), -(32 * 10),
                        (dist >= 0) ? (32 * 5) : -(32 * 3),
                        -(32 * 7), 12, obj->player_num,
                        ATTACK_ELEMENT_UPPER, 1, VECTOR2((40 * dist) * delta_time, 50 * delta_time), 30 * delta_time / ANIM_SPEED, 150);*/
                    if (attackentry_point == ATTACKENTRY::CROUCH_Y) break;

                    now_attack = ATTACKENTRY::CROUCH_Y;

                    if (old_attack != now_attack)
                    {

                        judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::CROUCHY].get());
                        isPursuit = attacks[ATTACKNUM::CROUCHY]->isPursuit;

                        //cancel_frame = 20.0f;

                        //obj->velocity.x = (10 * dist) * delta_time;
                        //Jump(JUMP_SPEED * delta_time, obj);
                        set_rigidity(skill_rigidity_time[ATTACKNUM::CROUCHY]);
                        set_cancel(skill_cancel_limit_time[ATTACKNUM::CROUCHY]);
                        set_invincible(skill_invincible_time[ATTACKNUM::CROUCHY] * delta_time);


                        attackentry_point = ATTACKENTRY::CROUCH_Y;
                        old_attack = now_attack;
                        cancel_timer = 0;

                        state = CHARASTATE::STATE_ATTACK;

                        return true;

                    }

                    break;
                }
            }


            //if (button_num == BUTTONNUM::BTN_RIGHT_SHOULDER)
            //{


            //    //setanimation(ANIM_STATE::ANIM_ATTACK_Y);

            //   /* judge_manager::Instance()._attack_manager->add_attack_range(obj,
            //        (dist >= 0) ? (32 * 3) : -(32 * 5), -(32 * 10),
            //        (dist >= 0) ? (32 * 5) : -(32 * 3),
            //        -(32 * 7), 12, obj->player_num,
            //        ATTACK_ELEMENT_UPPER, 1, VECTOR2((40 * dist) * delta_time, 50 * delta_time), 30 * delta_time / ANIM_SPEED, 150);*/
            //    judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[5].get());

            //    //cancel_frame = 20.0f;

            //    obj->velocity.x = (10 * dist);
            //    //Jump(JUMP_SPEED * delta_time, obj);
            //    attackentry_point = ATTACKENTRY::GROUND_RB;

            //    Jump(JUMP_SPEED,obj);

            //    set_rigidity(skill_rigidity_time[5]);
            //    set_invincible(skill_invincible_time[5]);
            //    set_cancel(skill_cancel_limit_time[5]);
            //    state = CHARASTATE::STATE_ATTACK;

            //    break;
            //}


        }


        
        break;

#endif // 1
#if 1
    case Command::RHURF:

        if (obj->isGround)
        {

            if (button_num == BUTTONNUM::BUTTON_A)
            {

                //setanimation(ANIM_STATE::ANIM_ATTACK_236_A);

                state = CHARASTATE::STATE_ATTACK;
                attackentry_point = ATTACKENTRY::HDOU_A;

                /*judge_manager::Instance()._attack_manager->add_attack_range(obj,
                    (dist >= 0) ? (32 * 2) : -(32 * 6), -(32 * 10),
                    (dist >= 0) ? (32 * 6) : -(32 * 2),
                    (32 * 3),
                    500 * delta_time, obj->player_num, ATTACK_ELEMENT_MIDDLE, 1, VECTOR2((30 * dist) * delta_time, 0), 800 * delta_time / ANIM_SPEED, 150);*/

                judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::HDOUA].get());
                isPursuit = attacks[ATTACKNUM::HDOUA]->isPursuit;
                // cancel_frame = 20.0f;

                set_rigidity(skill_rigidity_time[ATTACKNUM::HDOUA]);
                set_cancel(skill_cancel_limit_time[ATTACKNUM::HDOUA]);
                set_invincible(skill_invincible_time[ATTACKNUM::HDOUA] * delta_time);

                obj->velocity.x = (10 * dist) * delta_time;
                cancel_timer = 0;

                //  commandstate = Command::NOCOMMAND;
                command->command_timer = TIMER_LIMIT;

                return true;

            }
        }

        break;
#endif // 1

    case Command::RSTEP:
        if (obj->isGround)
        {
            obj->velocity.x = PL_SPEED * 2.5f;


            if (dist < 0)
            {
                obj->guard_elements = ATTACK_ELEMENT_UPPER | ATTACK_ELEMENT_MIDDLE;
                obj->is_guard = true;
                backword = true;
         

                state = CHARASTATE::STATE_BACK_STEP;
            }

            else
            {
                obj->is_guard = false;
                backword = false;
            

                state = CHARASTATE::STATE_FRONT_STEP;
            }

            //set_rigidity(5);

            set_invincible(5);


            return true;
         
        }
            break;

    case Command::LSTEP:
        if (obj->isGround)
        {
              
            obj->velocity.x = -PL_SPEED * 2.5f;
                if (dist > 0)
                {
                    obj->guard_elements = ATTACK_ELEMENT_UPPER | ATTACK_ELEMENT_MIDDLE;
                    obj->is_guard = true;
                    backword = true;
              

                    state = CHARASTATE::STATE_BACK_STEP;
                }

                else
                {
                    obj->is_guard = false;
                    backword = false;
              
                    state = CHARASTATE::STATE_FRONT_STEP;
                }

                //set_rigidity(5);
                set_invincible(5);

                return true;
        }

        break;


    case Command::RFUJIN:
        if (obj->isGround)
        {

            if (button_num == BUTTONNUM::BUTTON_X)
            {


                //setanimation(ANIM_STATE::ANIM_ATTACK_Y);

               /* judge_manager::Instance()._attack_manager->add_attack_range(obj,
                    (dist >= 0) ? (32 * 3) : -(32 * 5), -(32 * 10),
                    (dist >= 0) ? (32 * 5) : -(32 * 3),
                    -(32 * 7), 12, obj->player_num,
                    ATTACK_ELEMENT_UPPER, 1, VECTOR2((40 * dist) * delta_time, 50 * delta_time), 30 * delta_time / ANIM_SPEED, 150);*/
                judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::UPPERCUT].get());
                isPursuit = attacks[ATTACKNUM::UPPERCUT]->isPursuit;

                //cancel_frame = 20.0f;

                obj->velocity.x = (30 * dist);
                //Jump(JUMP_SPEED * delta_time, obj);
                attackentry_point = ATTACKENTRY::GROUND_RB;

                Jump(JUMP_SPEED * 1.2f, obj);

                cancel_timer = 0;

                set_rigidity(skill_rigidity_time[ATTACKNUM::UPPERCUT]);
                set_invincible(skill_invincible_time[ATTACKNUM::UPPERCUT] * delta_time);
                set_cancel(skill_cancel_limit_time[ATTACKNUM::UPPERCUT]);
                state = CHARASTATE::STATE_ATTACK;

                return true;
            }
        }

        break;


    }


    return false;
}

void FighterBase::air_atack_move(OBJ3D* obj)
{

    float delta_time = framework::Instance().elapased_time;



    float dist = 0.0f;

    if (obj->tag == "PLAYER1")
    {
        dist = judge_manager::Instance().get_1pdistance();
    }

    else if (obj->tag == "PLAYER2")
    {
        dist = judge_manager::Instance().get_2pdistance();
    }

    //if (commandstate == Command::RFUJIN && pad->GetButtonDown() & GamePad::BTN_A)
           //{
           //    ANIM_SPEED = 1.0f;
           //    obj->model->PlayAnimation(5, false, ANIM_SPEED);
           //    // commandstate = Command::NOCOMMAND;
           //    judge_manager::Instance()._attack_manager->add_attack_range(
           //        (dist >= 0) ? obj->screen_position.x + 64 : obj->screen_position.x - (32 * 5), obj->screen_position.y - (32 * 3),
           //        (dist >= 0) ? obj->screen_position.x + (32 * 5) : obj->screen_position.x - (32 * 2),
           //        obj->screen_position.y - (32 * 1), 12, obj->player_num,
           //        ATTACK_ELEMENT_MIDDLE, 1, VECTOR2((40 * dist) * delta_time, 100 * delta_time), 1000 * delta_time / ANIM_SPEED, 150);

           //    obj->velocity.x = 0;
           //    command->command_timer = TIMER_LIMIT;
           //    state = CHARASTATE::STATE_ATTACK;


           //}



    switch (commandstate)
    {

#if 1 //行数が多いので折りたためるようにする

    case Command::NOCOMMAND:

        if (!obj->isGround)
        {
            if (button_num == BUTTONNUM::BUTTON_A)
            {
                if (attackentry_point == ATTACKENTRY::AIR_A) break;
                now_attack = ATTACKENTRY::AIR_A;

                if (old_attack != now_attack)
                {

                    //setanimation(ANIM_STATE::ANIM_ATTACK_A);

                    /*judge_manager::Instance()._attack_manager->add_attack_range(obj,
                        (dist >= 0) ? (32 * 2) : -(32 * 6), -(32 * 10),
                        (dist >= 0) ? (32 * 6) : -(32 * 2),
                        -(32 * 3),
                        500 * delta_time, obj->player_num, ATTACK_ELEMENT_LOWER, 1, VECTOR2((30 * dist) * delta_time, 0), 800 * delta_time / ANIM_SPEED, 150);*/

                    judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::AIRA].get());
                    isPursuit = attacks[ATTACKNUM::AIRA]->isPursuit;

                    set_rigidity(skill_rigidity_time[ATTACKNUM::AIRA]);
                    set_cancel(skill_cancel_limit_time[ATTACKNUM::AIRA]);
                    // cancel_frame = 20.0f;

                     //obj->velocity.x = (10 * dist);

                    attackentry_point = ATTACKENTRY::AIR_A;

                    old_attack = now_attack;

                    //  commandstate = Command::NOCOMMAND;
                    command->command_timer = TIMER_LIMIT;
                    state = CHARASTATE::STATE_ATTACK;
                }
                    break;
              
            }


            if (button_num == BUTTONNUM::BUTTON_B)
            {

                //setanimation(ANIM_STATE::ANIM_ATTACK_B);

                if (attackentry_point == ATTACKENTRY::AIR_B) break;
                now_attack = ATTACKENTRY::AIR_B;

                if (old_attack != now_attack)
                {

                    /*   judge_manager::Instance()._attack_manager->add_attack_range(obj,
                           (dist >= 0) ? (32 * 2) : -(32 * 6), -(32 * 10),
                           (dist >= 0) ? (32 * 6) : -(32 * 2),
                           -(32 * 3),
                           500 * delta_time, obj->player_num, ATTACK_ELEMENT_UPPER, 1, (obj->isGround) ? VECTOR2((30 * dist) * delta_time, (400) * delta_time) : VECTOR2((30 * dist) * delta_time, 200 * delta_time), 800 * delta_time / ANIM_SPEED, 150);*/


                    judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::AIRB].get());
                    isPursuit = attacks[ATTACKNUM::AIRB]->isPursuit;

                    set_rigidity(skill_rigidity_time[ATTACKNUM::AIRB]);
                    set_cancel(skill_cancel_limit_time[ATTACKNUM::AIRB]);
                    //cancel_frame = 20.0f;

                    // obj->velocity.x = (10 * dist) * delta_time;

                    attackentry_point = ATTACKENTRY::AIR_B;
                    old_attack = now_attack;

                    state = CHARASTATE::STATE_ATTACK;
                }

                break;
            }


            if (button_num == BUTTONNUM::BUTTON_X)
            {


                //setanimation(ANIM_STATE::ANIM_ATTACK_X);

                if (attackentry_point == ATTACKENTRY::AIR_X) break;
                now_attack = ATTACKENTRY::AIR_X;

                if (old_attack != now_attack)
                {

                    /*  judge_manager::Instance()._attack_manager->add_attack_range(obj,
                          (dist >= 0) ? (32 * 3) : -(32 * 5), -(32 * 10),
                          (dist >= 0) ? (32 * 5) : -(32 * 3),
                          -(32 * 7), 12, obj->player_num,
                          ATTACK_ELEMENT_UPPER, 1, VECTOR2((40 * dist) * delta_time, 20 * delta_time), 100 * delta_time / ANIM_SPEED, 150);*/

                    judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::AIRX].get());
                    isPursuit = attacks[ATTACKNUM::AIRX]->isPursuit;



                    set_rigidity(skill_rigidity_time[ATTACKNUM::AIRX]);
                    set_cancel(skill_cancel_limit_time[ATTACKNUM::AIRX]);
                    //cancel_frame = 20.0f;

                    //obj->velocity.x = (10 * dist) * delta_time;

                    attackentry_point = ATTACKENTRY::AIR_X;
                    old_attack = now_attack;

                    state = CHARASTATE::STATE_ATTACK;
                }

                break;
            }


            if (button_num == BUTTONNUM::BUTTON_Y)
            {


                //setanimation(ANIM_STATE::ANIM_ATTACK_Y);

                if (attackentry_point == ATTACKENTRY::AIR_Y) break;
                now_attack = ATTACKENTRY::AIR_Y;

                if (old_attack != now_attack)
                {

                    /* judge_manager::Instance()._attack_manager->add_attack_range(obj,
                         (dist >= 0) ? (32 * 3) : -(32 * 5), -(32 * 10),
                         (dist >= 0) ? (32 * 5) : -(32 * 3),
                         -(32 * 7), 12, obj->player_num,
                         ATTACK_ELEMENT_UPPER, 1, VECTOR2((40 * dist) * delta_time, 50 * delta_time), 30 * delta_time / ANIM_SPEED, 150);*/
                    judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::AIRY].get());
                    isPursuit = attacks[ATTACKNUM::AIRY]->isPursuit;

                    //cancel_frame = 20.0f;


                    set_rigidity(skill_rigidity_time[ATTACKNUM::AIRY]);
                    set_cancel(skill_cancel_limit_time[ATTACKNUM::AIRY]);
                    //obj->velocity.x = (10 * dist) * delta_time;
                    //Jump(JUMP_SPEED * delta_time, obj);
                    attackentry_point = ATTACKENTRY::AIR_Y;

                    old_attack = now_attack;

                    state = CHARASTATE::STATE_ATTACK;
                }

                break;
            }
        }

        break;

#endif // 1
#if 1
    case Command::RHURF:

        if (button_num == BUTTONNUM::BUTTON_A)
        {

            //setanimation(ANIM_STATE::ANIM_ATTACK_236_A);

            if (attackentry_point == ATTACKENTRY::HDOU_A) break;
            now_attack = ATTACKENTRY::HDOU_A;

            if (old_attack != now_attack)
            {

                state = CHARASTATE::STATE_ATTACK;
                attackentry_point = ATTACKENTRY::HDOU_A;

                /*judge_manager::Instance()._attack_manager->add_attack_range(obj,
                    (dist >= 0) ? (32 * 2) : -(32 * 6), -(32 * 10),
                    (dist >= 0) ? (32 * 6) : -(32 * 2),
                    (32 * 3),
                    500 * delta_time, obj->player_num, ATTACK_ELEMENT_MIDDLE, 1, VECTOR2((30 * dist) * delta_time, 0), 800 * delta_time / ANIM_SPEED, 150);*/

                old_attack = now_attack;

                judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::HDOUA].get());
                isPursuit = attacks[ATTACKNUM::HDOUA]->isPursuit;
                //cancel_frame = 20.0f;
                set_cancel(skill_cancel_limit_time[ATTACKNUM::HDOUA]);
                set_rigidity(skill_rigidity_time[ATTACKNUM::HDOUA]);

                obj->velocity.x = (10 * dist) * delta_time;
                //  commandstate = Command::NOCOMMAND;
                command->command_timer = TIMER_LIMIT;
                break;
            }
        }

        break;

    case Command::RFUJIN:
       
        {

            if (button_num == BUTTONNUM::BUTTON_X)
            {
                if (attackentry_point == ATTACKENTRY::GROUND_RB) break;
                now_attack = ATTACKENTRY::GROUND_RB;

                if (old_attack != now_attack)
                {

                    //setanimation(ANIM_STATE::ANIM_ATTACK_Y);

                   /* judge_manager::Instance()._attack_manager->add_attack_range(obj,
                        (dist >= 0) ? (32 * 3) : -(32 * 5), -(32 * 10),
                        (dist >= 0) ? (32 * 5) : -(32 * 3),
                        -(32 * 7), 12, obj->player_num,
                        ATTACK_ELEMENT_UPPER, 1, VECTOR2((40 * dist) * delta_time, 50 * delta_time), 30 * delta_time / ANIM_SPEED, 150);*/
                    judge_manager::Instance()._attack_manager->add_attack_range(obj, attacks[ATTACKNUM::UPPERCUT].get());
                    isPursuit = attacks[ATTACKNUM::UPPERCUT]->isPursuit;

                    //cancel_frame = 20.0f;

                    obj->velocity.x = (30 * dist);
                    //Jump(JUMP_SPEED * delta_time, obj);
                    attackentry_point = ATTACKENTRY::GROUND_RB;

                    Jump(JUMP_SPEED * 1.2f, obj);

                    cancel_timer = 0;

                    old_attack = now_attack;

                    set_rigidity(skill_rigidity_time[ATTACKNUM::UPPERCUT]);
                    set_invincible(skill_invincible_time[ATTACKNUM::UPPERCUT] * delta_time);
                    set_cancel(skill_cancel_limit_time[ATTACKNUM::UPPERCUT]);
                    state = CHARASTATE::STATE_ATTACK;
                }
                break;
            }
        }

        break;


#endif // 1

    }

}

void FighterBase::loadattk()
{
    FILE* f;
    fopen_s(&f, "./Data/info/FightersData/TestFighter.txt", "r");
    if (f!=nullptr)
    {
        for (int i = 0; i < ATTACKS_MAX; i++)
        {

            fscanf_s(f, "%f ", &(float)attacks[i]->a_left);
            fscanf_s(f, "%f ", &(float)attacks[i]->a_top);
            fscanf_s(f, "%f ", &(float)attacks[i]->a_right);
            fscanf_s(f, "%f ", &(float)attacks[i]->a_bottom);
            fscanf_s(f, "%d ", &attacks[i]->attack_life_time);
            fscanf_s(f, "%d ", &attacks[i]->attack_standby_frame);
            fscanf_s(f, "%d ", &attacks[i]->attack_types);
            fscanf_s(f, "%d ", &attacks[i]->hitcount);
            fscanf_s(f, "%d ", &attacks[i]->damage);
            fscanf_s(f, "%f ", &(float)attacks[i]->vector.x);
            fscanf_s(f, "%f ", &(float)attacks[i]->vector.y);
            fscanf_s(f, "%f ", &ANIMSPEED[i]);
            fscanf_s(f, "%d ", &(int)skill_rigidity_time[i]);
            fscanf_s(f, "%d ", &(int)skill_invincible_time[i]);
            fscanf_s(f, "%d ", &attacks[i]->air);
            fscanf_s(f, "%d ", &(int)skill_cancel_limit_time[i]);
            fscanf_s(f, "%f ", &(float)attacks[i]->attenuation_rate);
            fscanf_s(f, "%f ", &(float)attacks[i]->stanpoint);
            fscanf_s(f, "%f ", &(float)attacks[i]->hitstop);
            fscanf_s(f, "%d ", &attacks[i]->isPursuit);
            
        }
        fclose(f);

    }
    
   

}

void FighterBase::saveattk()
{
    FILE* f;
    fopen_s(&f, "./Data/info/FightersData/TestFighter.txt", "w");
    for (int i = 0; i < ATTACKS_MAX; i++)
    {
        fprintf_s(f, "%f ", attacks[i]->a_left);
        fprintf_s(f, "%f ", attacks[i]->a_top);
        fprintf_s(f, "%f ", attacks[i]->a_right);
        fprintf_s(f, "%f ", attacks[i]->a_bottom);
        fprintf_s(f, "%d ", attacks[i]->attack_life_time);
        fprintf_s(f, "%d ", attacks[i]->attack_standby_frame);
        fprintf_s(f, "%d ", attacks[i]->attack_types);
        fprintf_s(f, "%d ", attacks[i]->hitcount);
        fprintf_s(f, "%d ", attacks[i]->damage);
        fprintf_s(f, "%f ", attacks[i]->vector.x);
        fprintf_s(f, "%f ", attacks[i]->vector.y);
        fprintf_s(f, "%f ", ANIMSPEED[i]);

        fprintf_s(f, "%d ",skill_rigidity_time[i]);
        fprintf_s(f, "%d ",skill_invincible_time[i]);
        fprintf_s(f, "%p ",(int*)attacks[i]->air);
        fprintf_s(f, "%d ", skill_cancel_limit_time[i]);
        fprintf_s(f, "%f ", attacks[i]->attenuation_rate);
        fprintf_s(f, "%f ", attacks[i]->stanpoint);
        fprintf_s(f, "%f ", attacks[i]->hitstop);

        fprintf_s(f, "%p ", (int*)attacks[i]->isPursuit);
    }

    fclose(f);
}


int anim = 0;


void FighterBase::imgui_update(OBJ3D* obj)
{

    

#ifdef USE_IMGUI

    const char* xd = obj->tag.c_str();

    ImGui::Begin(xd);

    if (ImGui::Button(u8"パラメーターをセーブ"))
    {
        Save("./Data/Info/FightersData/" + fightername + ".bin", "fighters", *this);
        Save("./Data/Info/FightersData/" + fightername + ".json", "fighters", *this);
     
    }

    if (ImGui::Button(u8"パラメーターをロード"))
    {
        LoadData(fightername, obj);

    }
  


    ImGui::Checkbox(u8"ボットメニュー表示", &isCPU);
    ImGui::Checkbox(u8"攻撃編集メニュー", &isAttack_EDIT);
    ImGui::Checkbox(u8"入力確認メニュー", &iscommand);


    ImGui::Text(u8"コマンド入力の受付時間");

    ImGui::Text(u8"timer::%f", command->command_timer);

    ImGui::Text(u8"どのコマンドが入力されたか");
   
    ImGui::Text(u8"anim::%d", anim_state);


    ImGui::Text(u8"state::%d", commandstate);

    ImGui::Text(u8"キャンセルフレーム::%d", cancel_timer/60);

    ImGui::Text(u8"移動ベクトルX::%f", obj->velocity.x);
    ImGui::Text(u8"移動ベクトルY::%f", obj->velocity.y);

    ImGui::DragFloat(u8"位置X::%f", &obj->position.x);
    ImGui::DragFloat(u8"位置Y::%f", &obj->position.y);


    ImGui::InputInt(u8"HP::%f", &obj->health);
    ImGui::InputInt(u8"アニメーション番号::%d", &anim);
    if (ImGui::Button(u8"アニメーション再生"))
    {
        obj->model->PlayAnimation(anim, true);
    }

    if (obj->tag == "PLAYER1")
    {
        ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color1p));
    }

    else if (obj->tag == "PLAYER2")
    {
        ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color2p));
    }


    ImGui::DragFloat(u8"頭のくらい判定x::%f", &obj->head_size.x);
    ImGui::DragFloat(u8"頭のくらい判定y::%f", &obj->head_size.y);

    ImGui::DragFloat(u8"体のくらい判定x::%f", &obj->body_size.x);
    ImGui::DragFloat(u8"体のくらい判定y::%f", &obj->body_size.y);

    ImGui::DragFloat(u8"足のくらい判定x", &obj->reg_size.x);
    ImGui::DragFloat(u8"足のくらい判定y", &obj->reg_size.y);

    ImGui::DragFloat(u8"押し返し判定x::%f", &obj->size.x);
    ImGui::DragFloat(u8"押し返し判定y::%f", &obj->size.y);


    ImGui::DragFloat(u8"エリア押し返し判定x::%f", &obj->checksize.x);
    ImGui::DragFloat(u8"エリア押し返し判定y::%f", &obj->checksize.y);

    ImGui::DragFloat(u8"ジャンプ力", &JUMP_SPEED);

    ImGui::Checkbox("trigger", &command->trigger);


    ImGui::Checkbox(u8"ガード中か", &obj->is_guard);

    ImGui::End();
#endif

}

void FighterBase::imgui_bot_update(OBJ3D* obj)
{
#ifdef USE_IMGUI
    const char *xd = obj->tag.c_str();
    std::string xx = "MOVEMENT" + obj->tag;

    
    if (isCPU)
    {
        ImGui::Begin(xx.c_str());

        const char* stick_numbers[] = { " STICK_N", "  STICK_UP", " STICK_RIGHT_UP", "  STICK_RIGHT", "STICK_RIGHT_DOWN","STICK_DOWN","STICK_LEFT_DOWN","STICK_LEFT"," STICK_LEFT_UP" };
        ImGui::Combo(u8"敵の動く方向", reinterpret_cast<int*>(&stick_dir), stick_numbers, IM_ARRAYSIZE(stick_numbers));

        const char* behavior_numbers[] = {"NONE","ATTACK_ABUTTON","ATTACK_BBUTTON","ATTACK_XBUTTON","ATTACK_YBUTTON"};
        ImGui::Combo(u8"敵の攻撃ボタン", reinterpret_cast<int*>(&button_num), behavior_numbers, IM_ARRAYSIZE(behavior_numbers));

        ImGui::End();
    }

#endif

}


void FighterBase::imgui_command_update(OBJ3D* obj)
{
#ifdef USE_IMGUI
    const char* xd = obj->tag.c_str();
    std::string xx = u8"コマンド入力ログ" + obj->tag;
    std::string xxx = u8"コマンド入力計測ログ" + obj->tag;

    if (iscommand)
    {
        ImGui::Begin(xx.c_str());
        ImGui::Text(u8"入力履歴");

        for (int i = 0; i < MAX_BUFFER; i++)
        {
            switch (command->history.at(i))
            {
            case STICKDIRECTION::STICK_N:
                ImGui::Text(u8"入力::☆");
                break;

            case STICKDIRECTION::STICK_UP:
                ImGui::Text(u8"入力::↑");
                break;

            case STICKDIRECTION::STICK_RIGHT_UP:
                ImGui::Text(u8"入力:: →↑");
                break;

            case STICKDIRECTION::STICK_RIGHT:
                ImGui::Text(u8"入力:: →");
                break;

            case STICKDIRECTION::STICK_RIGHT_DOWN:
                ImGui::Text(u8"入力::→↓");
                break;
            case STICKDIRECTION::STICK_DOWN:
                ImGui::Text(u8"入力::↓");
                break;
            case STICKDIRECTION::STICK_LEFT_DOWN:
                ImGui::Text(u8"入力:: ←↓");
                break;
            case STICKDIRECTION::STICK_LEFT:
                ImGui::Text(u8"入力:: ←");
                break;

            case STICKDIRECTION::STICK_LEFT_UP:
                ImGui::Text(u8"入力:: ←↑");
                break;

            
            }

        
        }

        ImGui::Text(u8"%d", command->interbal);

        ImGui::End();


        ImGui::Begin(xxx.c_str());

        ImGui::Text(u8"入力フレーム履歴");
        for (int i = 0; i < MAX_BUFFER; i++)
        {
            ImGui::Text(u8"何フレーム目に入力されたか::%d", command->timer.at(i));
        }

        ImGui::End();

    }

#endif

}


void FighterBase::imgui_attack_update(OBJ3D* obj)
{
#ifdef USE_IMGUI
    const char* xd = obj->tag.c_str();
    std::string xx = u8"攻撃エディター" + obj->tag;


    if (isAttack_EDIT)
    {
        ImGui::Begin(xx.c_str());

        if (ImGui::Button(u8"パラメーターをセーブ"))
        {
            saveattk();
        }

        if (ImGui::Button(u8"パラメーターをロード"))
        {
            loadattk();
        }

        static int datanum;
        const char* data_nums[] = { u8" Aボタン攻撃", u8"Bボタン攻撃", u8"Xボタン攻撃", u8"Yボタン攻撃", u8"必殺技236A",u8"RBボタン攻撃",u8"LBボタン攻撃",
            u8"空中Aボタン攻撃",u8" 空中Bボタン攻撃",u8"空中Xボタン攻撃",u8"空中Yボタン攻撃",u8"しゃがみAボタン攻撃",u8" しゃがみBボタン攻撃",u8"しゃがみXボタン攻撃",u8"しゃがみYボタン攻撃" ,u8"必殺技6N23X"};
        ImGui::Combo(u8"攻撃の配列", reinterpret_cast<int*>(&datanum), data_nums, IM_ARRAYSIZE(data_nums));
        ImGui::DragFloat(u8"攻撃判定左::%f", &attacks[datanum]->a_left);
        ImGui::DragFloat(u8"攻撃判定右::%f", &attacks[datanum]->a_right);
        ImGui::DragFloat(u8"攻撃判定頂点::%f", &attacks[datanum]->a_top);
        ImGui::DragFloat(u8"攻撃判定底辺::%f", &attacks[datanum]->a_bottom);

 
 

        ImGui::DragInt(u8"攻撃の発生値", &attacks[datanum]->attack_standby_frame);
        ImGui::Text(u8"実際の攻撃の発生フレーム:%d", attacks[datanum]->attack_standby_frame / 60);

        ImGui::DragInt(u8"攻撃の持続値", &attacks[datanum]->attack_life_time);
        ImGui::Text(u8"実際の攻撃の持続フレーム:%d", attacks[datanum]->attack_life_time / 60);
      
        ImGui::DragFloat(u8"攻撃のヒットストップ値", &attacks[datanum]->hitstop);
        ImGui::Text(u8"実際の攻撃のヒットストップフレーム:%f", attacks[datanum]->hitstop / 60);
  

        ImGui::DragInt(u8"攻撃のダメージ::%f", &attacks[datanum]->damage);
        ImGui::DragInt(u8"攻撃のヒット数::%f", &attacks[datanum]->hitcount);
        ImGui::DragFloat(u8"攻撃のスピード", &ANIMSPEED[datanum]);

        ImGui::DragFloat(u8"吹っ飛ばしベクトルX::%f", &attacks[datanum]->vector.x);
        ImGui::DragFloat(u8"吹っ飛ばしベクトルY::%f", &attacks[datanum]->vector.y);
        ImGui::DragFloat(u8"吹っ飛ばし減衰率", &attacks[datanum]->attenuation_rate);
     
        ImGui::DragInt(u8"攻撃の後の硬直時間", &skill_rigidity_time[datanum]);
        ImGui::DragInt(u8"攻撃の無敵判定", &skill_invincible_time[datanum]);
        ImGui::Text(u8"実際の攻撃の無敵フレーム:%d", skill_invincible_time[datanum] / 60);

        ImGui::DragInt(u8"攻撃キャンセル受付時間", &skill_cancel_limit_time[datanum]);
        ImGui::Text(u8"実際の攻撃のキャンセルフレーム:%d", skill_cancel_limit_time[datanum] / 60);

        ImGui::DragFloat(u8"攻撃のスタン値", &attacks[datanum]->stanpoint);

        ImGui::Checkbox(u8"空中攻撃かどうか", &attacks[datanum]->air);
        ImGui::Checkbox(u8"追撃可能かどうか", &attacks[datanum]->isPursuit);

        static int attackbits[30] = {};
        //while (attacks[datanum]->attack_types >> attackbits)attackbits++;
        //attackbits--;

        attackbits[datanum] = attacks[datanum]->attack_types;

        const char* types[] = { u8"上段", u8"中段", u8"下段",u8"空中" };
        ImGui::Combo(u8"攻撃の属性", &attackbits[datanum], types, IM_ARRAYSIZE(types));

        if (attackbits[datanum]!= attacks[datanum]->attack_types)
        {
            attacks[datanum]->attack_types = attackbits[datanum];
        }
        


        ImGui::End();
    }

#endif


}

void FighterBase::animationmanagement(OBJ3D* obj)
{
    switch (anim_state)
    {
    case  ANIM_STATE::ANIM_STANDING:
       
        if (!animtriger)
        {
            obj->model->PlayAnimation(0, true, 1.0f, 0.1f);
            animtriger = true;
        }
   
        break;
    case  ANIM_STATE::ANIM_FOWERD:

        if (!animtriger)
        {
            obj->model->PlayAnimation(1, true, 1.0f, 0.01f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_BACKWERD:
        if (!animtriger)
        {
            obj->model->PlayAnimation(2, true, 1.0f, 0.01f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_JUMP:
        if (!animtriger)
        {
        obj->model->PlayAnimation(3, false, 1.0f, 0.001f);
        animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_SQAUT:
        if (!animtriger)
        {
            obj->model->PlayAnimation(12, true, 1.0f, 0.1f);
           animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_SQGUARD:
       //if (!animtriger)
        {
            obj->model->PlayAnimation(14, false, 1.0f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_LANDING:
        if (!animtriger)
        {
            obj->model->PlayAnimation(12, false, 1.0f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_GUARD:
       if (!animtriger)
        {
            obj->model->PlayAnimation(13, false, 1.0f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_AIRGUARD:
        if (!animtriger)
        {
            obj->model->PlayAnimation(14, false, 1.0f);
           animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_ATTACK_A:
        
        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(20, false, ANIMSPEED[ATTACKNUM::GA],0.01f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_ATTACK_B:

        

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            if (!obj->isGround)
            {
                ANIM_SPEED = 0.4f;
            }
            obj->model->PlayAnimation(40, false, ANIMSPEED[ATTACKNUM::GB], 0.01f);
            animtriger = true;
        }
 

        break;

    case  ANIM_STATE::ANIM_ATTACK_X:

        if (!animtriger)
        {
            ANIM_SPEED = 1.0f;
            obj->model->PlayAnimation(19, false, ANIMSPEED[ATTACKNUM::GX], 0.01f);
            animtriger = true;
        }

        break;

    case  ANIM_STATE::ANIM_ATTACK_Y:
      
        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(44, false, ANIMSPEED[ATTACKNUM::GY], 0.01f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_ATTACK_RB:

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(26, false, ANIMSPEED[ATTACKNUM::UPPERCUT], 0.01f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_ATTACK_236_A:
 
        if (!animtriger)
        {
            ANIM_SPEED = 0.9f;
            obj->model->PlayAnimation(32, false, ANIMSPEED[ATTACKNUM::HDOUA], 0.01f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_ATTACK_A_AIR:

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(30, false, ANIMSPEED[ATTACKNUM::AIRA]);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_ATTACK_B_AIR:

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(21, false, ANIMSPEED[ATTACKNUM::AIRB]);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_ATTACK_X_AIR:

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(44, false, ANIMSPEED[ATTACKNUM::AIRX]);
            animtriger = true;
        }
        break;
    case  ANIM_STATE::ANIM_ATTACK_Y_AIR:

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(22, false, ANIMSPEED[ATTACKNUM::AIRY]);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_ATTACK_A_CROUCH:

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(23, false, ANIMSPEED[ATTACKNUM::CROUCHA], 0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_ATTACK_B_CROUCH:

        if (!animtriger)
        {
         
            obj->model->PlayAnimation(30, false, ANIMSPEED[ATTACKNUM::CROUCHB],0.1f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_ATTACK_X_CROUCH:

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(24, false, ANIMSPEED[ATTACKNUM::CROUCHX], 0.1f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_ATTACK_Y_CROUCH:

        if (!animtriger)
        {
            ANIM_SPEED = 0.5f;
            obj->model->PlayAnimation(44, false, ANIMSPEED[ATTACKNUM::CROUCHY], 0.1f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_DAMAGE:
        if (!animtriger)
        {
            obj->model->PlayAnimation(11, false,1.0f,0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_DOWN:
        if (!animtriger)
        {
            obj->model->PlayAnimation(11, false, 1.0f,0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_DETH:
        if (!animtriger)
        {
            obj->model->PlayAnimation(45, false, 1.0f, 0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_WIN:
        if (!animtriger)
        {
            obj->model->PlayAnimation(37, false,1.0f,0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_ROLL:
        if (!animtriger)
        {
            obj->model->PlayAnimation(21, true, 1.0f, 0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_GETUP:
        if (!animtriger)
        {
            obj->model->PlayAnimation(43, false, 1.0f, 0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_INTRO:
        if (!animtriger)
        {
            obj->model->PlayAnimation(35, false, 1.0f, 0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_FRONT_STEP:
        if (!animtriger)
        {
            obj->model->PlayAnimation(16, false, 1.2f,0.1f);
            animtriger = true;
        }
        break;

    case  ANIM_STATE::ANIM_BACK_STEP:
        if (!animtriger)
        {
            obj->model->PlayAnimation(17, false, 1.2f,0.1f);
            animtriger = true;
        }
        break;


    case  ANIM_STATE::ANIM_STAN:
        if (!animtriger)
        {
            obj->model->PlayAnimation(29, true, 1.0f, 0.1f);
            animtriger = true;
        }
        break;

    }

   
 }




void FighterBase::animator(OBJ3D* obj)
{
    float dist = 0.0f;

    if (obj->tag == "PLAYER1")
    {
        dist = judge_manager::Instance().get_1pdistance();
    }

    else if (obj->tag == "PLAYER2")
    {
        dist = judge_manager::Instance().get_2pdistance();
    }

    switch (state)
    {
    case STATE_NONE:

        if (!obj->guard_redy)
        {
            if (obj->isGround)
            {

                if (!squat)
                {
                    if (obj->velocity.x > 0)
                    {
                        if (dist < 0)
                        {
                            setanimation(ANIM_STATE::ANIM_FOWERD);
                        }

                        else
                        {
                            setanimation(ANIM_STATE::ANIM_BACKWERD);
                        }
                    }

                    else  if (obj->velocity.x < 0)
                    {
                        if (dist < 0)
                        {
                            setanimation(ANIM_STATE::ANIM_BACKWERD);
                        }

                        else
                        {
                            setanimation(ANIM_STATE::ANIM_FOWERD);
                        }
                    }

                    else
                    {
                        setanimation(ANIM_STATE::ANIM_STANDING);
                    }
                }

                else
                {
                    setanimation(ANIM_STATE::ANIM_SQAUT);
                }
            }

        }


        else
        {
            setanimation(ANIM_STATE::ANIM_GUARD);
        }

        break;
    case STATE_ATTACK:

        atkanimator(obj);

        break;
    case STATE_DOWN:
        break;

    case STATE_STAN:

        if (obj->isGround)
        {
            setanimation(ANIM_STATE::ANIM_STAN);
        }

        break;

    case STATE_GUARD:
        if (!squat)
        {
            setanimation(ANIM_STATE::ANIM_GUARD);
        }
        else
        {
            setanimation(ANIM_STATE::ANIM_SQGUARD);
        }
        break;
    case STATE_JUMP:

        setanimation(ANIM_STATE::ANIM_JUMP);

        break;
    case STATE_DAMAGE:
        if (obj->isGround)
        {
            setanimation(ANIM_STATE::ANIM_DAMAGE);
        }
        else
        {
            setanimation(ANIM_STATE::ANIM_DOWN);
        }

        break;
    case STATE_GETUP:

        setanimation(ANIM_STATE::ANIM_GETUP);

        break;
    case STATE_LANDING:

        setanimation(ANIM_STATE::ANIM_LANDING);

        break;
    case STATE_DETH:

        setanimation(ANIM_STATE::ANIM_DETH);

        break;
    case STATE_WIN:
        win_timer++;
        if (win_timer >= (60 * 3))
        {
            setanimation(ANIM_STATE::ANIM_WIN);
        }
        else  if (win_timer <= (60 * 3))
        {
            setanimation(ANIM_STATE::ANIM_STANDING);
        }

        break;
    case STATE_FRONT_STEP:

        setanimation(ANIM_STATE::ANIM_FRONT_STEP);

        break;
    case STATE_BACK_STEP:

        setanimation(ANIM_STATE::ANIM_BACK_STEP);

        break;
    case STATE_INTRO:

        if (UI_Manager::Instance().Get_Timer() <= 0)
        {
            setanimation(ANIM_STATE::ANIM_INTRO);

        }

        else if (!obj->model->IsPlayAnimation())
        {
            setanimation(ANIM_STATE::ANIM_STANDING);
        }

        break;
    }

}

void FighterBase::atkanimator(OBJ3D* obj)
{
    if (!obj->ishit)
    {
        switch (attackentry_point)
        {
        case ATTACKENTRY::GROUND_A:
            setanimation(ANIM_STATE::ANIM_ATTACK_A);
            break;
        case ATTACKENTRY::GROUND_B:
            setanimation(ANIM_STATE::ANIM_ATTACK_B);
            break;
        case ATTACKENTRY::GROUND_X:
            setanimation(ANIM_STATE::ANIM_ATTACK_X);
            break;
        case ATTACKENTRY::GROUND_Y:
            setanimation(ANIM_STATE::ANIM_ATTACK_Y);
            break;
        case ATTACKENTRY::AIR_A:
            setanimation(ANIM_STATE::ANIM_ATTACK_A_AIR);
            break;
        case ATTACKENTRY::AIR_B:
            setanimation(ANIM_STATE::ANIM_ATTACK_B_AIR);
            break;
        case ATTACKENTRY::AIR_X:
            setanimation(ANIM_STATE::ANIM_ATTACK_X_AIR);
            break;
        case ATTACKENTRY::AIR_Y:
            setanimation(ANIM_STATE::ANIM_ATTACK_Y_AIR);
            break;
        case ATTACKENTRY::CROUCH_A:
            setanimation(ANIM_STATE::ANIM_ATTACK_A_CROUCH);
            break;
        case ATTACKENTRY::CROUCH_B:
            setanimation(ANIM_STATE::ANIM_ATTACK_B_CROUCH);
            break;
        case ATTACKENTRY::CROUCH_X:
            setanimation(ANIM_STATE::ANIM_ATTACK_X_CROUCH);
            break;
        case ATTACKENTRY::CROUCH_Y:
            setanimation(ANIM_STATE::ANIM_ATTACK_Y_CROUCH);
            break;
        case ATTACKENTRY::GROUND_RB:
            setanimation(ANIM_STATE::ANIM_ATTACK_RB);
            break;
        case ATTACKENTRY::AIR_RB:
            setanimation(ANIM_STATE::ANIM_ATTACK_RB);
            break;
        case ATTACKENTRY::HDOU_A:
            setanimation(ANIM_STATE::ANIM_ATTACK_B);
            break;
        }

    }

}
    










void player1::init()
{
    OBJManager::init();
}


void player2::init()
{
    OBJManager::init();
}

