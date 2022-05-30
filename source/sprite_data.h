#pragma once
#include "texture.h"
#include "sprite.h"
#include "user.h"
#include "obj2d.h"

enum TEXNO
{
    HP_GAUGE,
    GAUGE_COVER,
    KO,
    STAN,
    END,

};
extern LoadTexture loadTexture[];

extern SpriteData spr_HP;
extern SpriteData spr_HPCOVER;
extern SpriteData spr_KO;