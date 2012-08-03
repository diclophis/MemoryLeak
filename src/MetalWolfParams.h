#ifndef METALWOLFPARAMS_H
#define METALWOLFPARAMS_H

#include "AncientDawn.h"


namespace MWParams
{
    //Player
    static float kPlayerStartX = 0.0f;
    static float kPlayerStartY = -400.0f;
    static float kPlayerStartHeatlh = 100.0f;
    static float kPlayerBulletDamage = 5.0f;
    static int kPlayerGunMLFileIndex = 1; //EPlayerGunsMLFileName_LVL1;
    
    //Enemy
    static float kEnemyStartX = 0.0f;
    static float kEnemyStartY = 365.0f;
    static float kEnemyHalfPixelDimX = 381.0f; //2*381 = 768
    static float kEnemyHalfPixelDimY = 100.0f;
    static float kEnemyStartingHealth = 3000.0f; //This will need to be retrieved from the server at some point
    static float kEnemyBulletDamageAmount = 1.0f;
    
    //Level
    static float kNextLevelTime = 120.0f;
    
    //Weapon Damage Data


}

#endif