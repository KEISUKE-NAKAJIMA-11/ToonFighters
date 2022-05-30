#pragma once
#include "texture.h"
#include "sprite.h"
#include "sprite_data.h"
#include "framework.h"

constexpr float init_pos = 80.0f;

class UI_Manager
{
private:
    float hitpoint_1p = 0;
    float hitpoint_2p = 0;

    float stanpoint[2] = {};

   float hpgauge1p = 0.0f;
   float hpgauge2p = 0.0f;

   bool koflag = false;
   bool roundstart = false;
   bool win1p = false;
   bool win2p = false;
  

  
   int combo1Ptimer = 0;
   int combo2Ptimer = 0;
 


   float size = 0;

   int wincountp1 = 2;
   int wincountp2 = 2;
   float alpha = 0.5f;
public:

    int timer = 0;
    float fade = 5.0;
    int combo[2] = {};

    int round = 1;

    UI_Manager() {}
     ~UI_Manager() {}

     static UI_Manager& Instance()
     {
         static UI_Manager instance;
         return instance;
     }

     void init() 
     {
         koflag = false;
         win1p =  false;
         win2p = false;
         wincountp1 = 2;
         wincountp2 = 2;
         size = 0.0f;
         state = 0;
         timer = 0;
         fade = 5.0;
         roundstart = false;
         round = 1;


         combo[0] = 0;
         combo[1] = 0;

         stanpoint[0] = 0.1f;
         stanpoint[1] = 0.1f;


         combo1Ptimer = 0;
         combo2Ptimer = 0;

         alpha = 0.5f;
     }

     int state = 0;

   // void update(OBJManager* object1, OBJManager* object2);
    void update(OBJManager* object1, OBJManager* object2, Camera* camera);
    void render(Font* font);

    bool Get_KOflag() { return koflag; }
    bool Get_Roundstart() { return roundstart; }
    bool Get_1pwinflag() { return win1p; }
    bool Get_2pwinflag() { return win2p; }
    int Get_1Pcombo() { return combo[0]; }
    int Get_2Pcombo() { return combo[1]; }
    int Get_Timer() { return timer; }
    void Combo_TimerReset1p() { combo1Ptimer = 0; }
    void Combo_TimerReset2p() { combo2Ptimer = 0; }

};