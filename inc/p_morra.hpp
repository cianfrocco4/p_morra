//
//  p_morra.hpp
//  p_morra
//
//  Created by Anthony Cianfrocco on 10/26/20.
//  Copyright © 2020 Anthony Cianfrocco. All rights reserved.
//

#ifndef p_morra_hpp
#define p_morra_hpp

class p_morra
{

public:
  p_morra(const int anPlayerNum, const int anNumPlayers, const int anNumGames);
  void run();
private:
  int mnPlayer;
  int mnNumPlayers;
  int mnNumGames;
  int mnNumWon;
};

#endif /* p_morra_hpp */

