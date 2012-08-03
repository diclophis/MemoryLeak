#ifndef METALWOLFPARAMS_H
#define METALWOLFPARAMS_H

#include "AncientDawn.h"


namespace MWParams
{
    //Player
    static float kPlayerStartX = 0.0f;
    static float kPlayerStartY = -400.0f;
    static EPlayerGunType kPlayerGun = EPlayerGunType_GUNS_LVL3;
    
    static float kPlayerStartHealth [] = {
        25.0f, //Level 1 Health
        50.0f, //Level 2 Health
        100.0f, //Level 3 Health
        150.0f, //Level 4 Health
        200.0f, //Level 5 Health
        };
        
    //static int kNumArmorLevels = 5;
    static int kNumArmorTypes = 3;
    static float kPlayerStartArmor [] = {
        10.0f, //Level 1 Armor Light
        20.0f, //Level 2 Armor Light
        30.0f, //Level 3 Armor Light
        20.0f, //Level 1 Armor Medium
        30.0f, //Level 2 Armor Medium
        40.0f, //Level 3 Armor Medium
        30.0f, //Level 1 Armor Heavy
        40.0f, //Level 2 Armor Heavy
        50.0f, //Level 3 Armor Heavy
        };
    
    //Enemy
    static float kEnemyStartX = 0.0f;
    static float kEnemyStartY = 365.0f;
    static float kEnemyHalfPixelDimX = 384.0f; //2*381 = 768
    static float kEnemyHalfPixelDimY = 100.0f;
    static float kEnemyStartingHealth = 230000.0f; //This will need to be retrieved from the server at some point
    static float kEnemyBulletDamageAmount = 5.0f;
    
    //Level
    static float kNextLevelTime = 150.0f;
    
    //Weapon Damage Data
    static float kGunBulletDamage[] = {
        24.0f, //EPlayerGunType_LASER_LVL1
        48.0f, //EPlayerGunType_LASER_LVL2
        72.0f, //EPlayerGunType_LASER_LVL3
        96.0f, //EPlayerGunType_LASER_LVL4
        120.0f, //EPlayerGunType_LASER_LVL5
        10.0f, //EPlayerGunType_GUNS_LVL1
        20.0f, //EPlayerGunType_GUNS_LVL2
        30.0f, //EPlayerGunType_GUNS_LVL3
        40.0f, //EPlayerGunType_GUNS_LVL4
        50.0f, //EPlayerGunType_GUNS_LVL5
        250.0f, //EPlayerGunType_MISSLE_LVL1
        300.0f, //EPlayerGunType_MISSLE_LVL2
        350.0f, //EPlayerGunType_MISSLE_LVL3
        400.0f, //EPlayerGunType_MISSLE_LVL4
        450.0f, //EPlayerGunType_MISSLE_LVL5
        };
        
    //Coin Params
    static float kCoinFallSpeed = 7.0f;
    static const char* kCoinAmount = "25";


}

#endif
