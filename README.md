* account for user is created if not found
* main screen
 * background-image
 * title bar copy
 * flavor copy
 * store button upgrade cost coins
  * weapon level
  * armor level
  * shield level
  * drive level
* social button
 * fuel gifts
 * fuel recovers at 5 min loop
* play button
 * b. l. m. - specific sub type for each, describe what projectels damage, etc
 * heavy. medium. light heavier armer makes ship slower, absorbes damage does not recharge
 * 1 2 3 4 5 higher shield makes drive charge slower, absorbs damage, recharges over time
 * larger drive makes charge faster, effects (time-dialation) duration
 * lose X fuel
 * parallax scroll
 * small hit box for ship, full size bullet box
 * enemy hit box is single large box
 * bullets animate explosion on hit
 * explosion sprite
 * coin sprite
 * bullet sprite
 * ship sprite
 * enemy sprite
 * laser sprite
 * missile sprite
 * particle effects in bulletml
  * 10 for enemy
  * 1 for each bullet type
 * shield constantly charging over time, by the rate of the drive
 * player hit
  * apply damange from bullet to shield, then armor animate explosion
  * if armor less than 0 game over lose screen
 * enemy hit
  * apply damage to the ships armor
  * randomly drop coin that player must collect
  * never blows up ship from multiplayer event or fake with random timer
 * Xs game time, when passed game over, win only if enemy hp less than 0 or survived
  * some amount of awards are applied
 * money sink button

* player is striving to collect coins during gameplay, all coins are kept, coins allow then to upgrade
* player is string to be the player that blows up enemy
* player is tring to upgrade ship, after feeling of hopelessless at the size of the ship after they die X times at first 
* player is trying to expand their network of friends to upgrade faster

