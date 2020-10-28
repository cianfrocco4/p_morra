//
//  p_morra.cpp
//  p_morra
//
//  Created by Anthony Cianfrocco on 10/26/20.
//  Copyright © 2020 Anthony Cianfrocco. All rights reserved.
//

#include "p_morra.hpp"
#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <time.h>
#include <chrono>

p_morra::p_morra(const int anPlayerNum, const int anNumPlayers, const int anNumGames)
{
  this->mnPlayer = anPlayerNum;
  this->mnNumPlayers = anNumPlayers;
  this->mnNumGames = anNumGames;
  this->mnNumWon = 0;
}

void p_morra::run()
{
  for(int lnGame = 0; lnGame < mnNumGames; lnGame ++)
  {
    //std::cout << "I am " << anPlayerNum << " out of " << anNumPlayers << " playing " << anNumGames << " games\n\n";
    //
    // 1.) Pick number of fingers to broadcast (1-5)
    // 2.) Pick a guess 
    // 3.) Broadcast number of fingers and guess
    // 4.) Wait for all other players
    // 5.) Sum up ALL players fingers and determine if winner
    // 6.) Print output
    //
    
//    // 1.) Pick a number of fingers
//    const int lnNumFingers = rand() % 5 + 1;  // Random number between 1 and 5
//    
//    // 2.) Pick a guess greater than my own num fingers 
//    const int lnGuess = lnNumFingers + (mnNumPlayers * (rand() % 5 + 1));    
//
    int msg[2] = {0, 0};

    // init sum to my num fingers
    int lnSum = 0;//lnNumFingers;

    // Wait for players before me to broadcast
    for(int lnSrc = 0;  lnSrc < mnPlayer; lnSrc ++)
    {
      MPI_Status lcStatus{};
      int lnErr = MPI_Recv(msg, 2, MPI_INT, lnSrc, lnGame, MPI_COMM_WORLD, &lcStatus);

      if(lnErr != MPI_SUCCESS)
      {
        std::cout << "ERROR receiving message\n";
        return;
      }

      lnSum += msg[0];
    }
  
    // 1.) Pick a number of fingers
    const int lnNumFingers = rand() % 5 + 1;  // Random number between 1 and 5

    // 2.) Pick a guess greater than my own num fingers 
    const int lnGuess = lnNumFingers + (mnNumPlayers * (rand() % 5 + 1));

    msg[0] = lnNumFingers;
    msg[1] = lnGuess;

    // update sum to my num fingers
    lnSum += lnNumFingers; 
    std::cout << "I am " << mnPlayer << ". For run " << lnGame << ", I extend " <<
	    lnNumFingers << " and my guess is " << lnGuess << ".\n";

    // 3.) Broadcast number of fingers and guess to ALL other players
    for(int lnDest = 0; lnDest < mnNumPlayers; lnDest ++)
    {
       if(lnDest != mnPlayer)
       {
         int lnErr = MPI_Send(msg, 2, MPI_INT, lnDest, lnGame, MPI_COMM_WORLD); 
	 
	 if(lnErr != MPI_SUCCESS)
	 {
           std::cout << "ERROR sending message\n";
	   return;
	 }
       }
    }

    // 4.) Wait for all other players after me to broadcast their info
    for(int lnSrc = mnPlayer + 1; lnSrc < mnNumPlayers; lnSrc ++)
    {
      MPI_Status lcStatus{};
      int lnErr = MPI_Recv(msg, 2, MPI_INT, lnSrc, lnGame, MPI_COMM_WORLD, &lcStatus);
      
      if(lnErr != MPI_SUCCESS)
      {
        std::cout << "ERROR receiving message\n";
	return;
      }

      lnSum += msg[0];
    }
    
    MPI_Barrier(MPI_COMM_WORLD);

    bool lbWon = lnSum == lnGuess;
    bool lbFoundAnother = false;
    // Wait for all previous players to braodcast score
    for(int lnSrc = 0; lnSrc < mnPlayer; lnSrc ++)
    {
      MPI_Status lcStatus{};
      int lnErr = MPI_Recv(msg, 2, MPI_INT, lnSrc, lnGame, MPI_COMM_WORLD, &lcStatus);

      if(lnErr != MPI_SUCCESS)
      {
        std::cout << "ERROR receiving message\n";
        return;
      }

      if(msg[0] == lnGuess && lbWon)
      {
        lbFoundAnother = true;
      }
    }
    
    msg[0] = lnGuess;

    // Send score to all players waiting
    for(int lnDest = 0; lnDest < mnNumPlayers; lnDest ++)
    {
       if(lnDest != mnPlayer)
       {
         int lnErr = MPI_Send(msg, 2, MPI_INT, lnDest, lnGame, MPI_COMM_WORLD);

         if(lnErr != MPI_SUCCESS)
         {
           std::cout << "ERROR sending message\n";
           return;
         }
       }
    }

    // Wait for all other players after me to broadcast their guess
    for(int lnSrc = mnPlayer + 1; lnSrc < mnNumPlayers; lnSrc ++)
    {
      MPI_Status lcStatus{};
      int lnErr = MPI_Recv(msg, 2, MPI_INT, lnSrc, lnGame, MPI_COMM_WORLD, &lcStatus);

      if(lnErr != MPI_SUCCESS)
      {
        std::cout << "ERROR receiving message\n";
        return;
      }

      if(msg[0] == lnGuess && lbWon)
      {
        lbFoundAnother = true;
      }

    }

    // 5/6.) Sum is complete. Determine if winner and print output.
    if(lbWon && !lbFoundAnother)
    {
      std::cout << "I am " << mnPlayer << ", and I won the run " << lnGame << ".\n";
      mnNumWon ++;
    }
    else if(lbWon && lbFoundAnother)
    {
      std::cout << "I am " << mnPlayer << ", and I almost won the run " << lnGame << ".\n";
    }
    
    // Wait for all other players
    MPI_Barrier(MPI_COMM_WORLD);
  }

  // Calculate how I did compared to other players (processes)
  

  std::cout << "Hey! I am " << mnPlayer << ", and I won " << mnNumWon << " time(s). Overall, I played better than\n"; 
}

int main(int argc, char* argv[])
{
  if(argc != 2)
  {
    std::cout << "Usage: p_morra <num_games>\n";
    return 0;
  }

  auto start = std::chrono::high_resolution_clock::now();

  // Initialize the MPI environment. The two arguments to MPI Init are not
  // currently used by MPI implementations, but are there in case future
  // implementations might need the arguments.
  int lnErr = MPI_Init(&argc, &argv);

  if(lnErr != MPI_SUCCESS)
  {
    std::cout << "MPI initialization failed!\n";
    return 0;
  }

  // Get the number of processes
  int lnWorldSize;
  MPI_Comm_size(MPI_COMM_WORLD, &lnWorldSize);

  // Get the rank of the process
  int lnWorldRank;
  MPI_Comm_rank(MPI_COMM_WORLD, &lnWorldRank);

  srand(time(NULL) + lnWorldRank);

  // Get the name of the processor
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  int name_len;
  MPI_Get_processor_name(processor_name, &name_len);

  p_morra lcSim(lnWorldRank, lnWorldSize, atoi(argv[1]));

  auto lrTime = MPI_Wtime();
  lcSim.run();
  auto lrEndTime = MPI_Wtime() - lrTime;

  MPI_Barrier(MPI_COMM_WORLD);
 
  // Find the min start time between all procs and the max end time 
  // b/w all procs and then calc the total exec time
  auto lrMinStartTime = lrTime;
  auto lrMaxEndTime = lrTime;
  MPI_Reduce(&lrTime, &lrMinStartTime, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  MPI_Reduce(&lrEndTime, &lrMaxEndTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  
  if( lnWorldRank == 0)
  {
    std::cout << "Execution Time: " << lrMaxEndTime - lrMinStartTime << " seconds.\n";
  }

  // Finalize the MPI environment. No more MPI calls can be made after this
  MPI_Finalize();
}
