#include "sprite_data.h"
#include "obj2d_data.h"


LoadTexture loadTexture[] = {
    { TEXNO::HP_GAUGE,    L"./Data/Image/hp2.png",    10U },
   
    { TEXNO::GAUGE_COVER,    L"./Data/Image/gaugecover2.png",    10U },
    { TEXNO::KO,    L"./Data/Image/KOtest2.png",    10U },
     { TEXNO::STAN,    L"./Data/Image/stan.png",    10U },

    { -1, nullptr }	// èIóπÉtÉâÉO

};

#define SPRITE_CENTER(texno,left,top,width,height)	{ (texno),(left),(top),(width),(height),(width)/2,(height)/2 }
#define SPRITE_BOTTOM(texno,left,top,width,height)	{ (texno),(left),(top),(width),(height),(width)/2,(height)   }
#define SPRITE_LEFT_TOP(texno,left,top,width,height)	{ (texno),(left),(top),(width),(height),(width),(height) }

SpriteData spr_HP = SPRITE_LEFT_TOP (TEXNO::HP_GAUGE, 0, 0, 1600, 112);
SpriteData spr_HPCOVER = SPRITE_LEFT_TOP(TEXNO::GAUGE_COVER, 0, 0, 1600, 163);
SpriteData spr_KO = SPRITE_LEFT_TOP(TEXNO::KO, 0, 0, 1920, 1080);

SpriteData spr_STAN = SPRITE_LEFT_TOP(TEXNO::STAN, 0, 0, 1600, 112);


