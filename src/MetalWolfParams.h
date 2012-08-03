#ifndef METALWOLFPARAMS_H
#define METALWOLFPARAMS_H

#include "AncientDawn.h"


namespace MWParams
{
    //Player
    static float kPlayerStartX = 0.0f;
    static float kPlayerStartY = -400.0f;
    static float kPlayerStartArmorType1 = 10.0f;
    static float kPlayerStartArmorType2 = 20.0f;
    static float kPlayerStartArmorType3 = 30.0f;
    static EPlayerGunType kPlayerGun = EPlayerGunType_GUNS_LVL3;
    
    static float kPlayerStartHealth [] = {
        100.0f,
        200.0f,
        300.0f,
        };
    
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
    static float kGunBulletDamage[] = {
        5.0f, //EPlayerGunType_LASER_LVL1
//        5.0f, //EPlayerGunType_LASER_LVL2
//        5.0f, //EPlayerGunType_LASER_LVL3
//        5.0f, //EPlayerGunType_LASER_LVL4
//        5.0f, //EPlayerGunType_LASER_LVL5
        5.0f, //EPlayerGunType_GUNS_LVL1
        5.0f, //EPlayerGunType_GUNS_LVL2
        5.0f, //EPlayerGunType_GUNS_LVL3
        5.0f, //EPlayerGunType_GUNS_LVL4
        5.0f, //EPlayerGunType_GUNS_LVL5
        5.0f, //EPlayerGunType_MISSLE_LVL1
        5.0f, //EPlayerGunType_MISSLE_LVL2
        5.0f, //EPlayerGunType_MISSLE_LVL3
        5.0f, //EPlayerGunType_MISSLE_LVL4
        5.0f, //EPlayerGunType_MISSLE_LVL5
        };


}

#endif