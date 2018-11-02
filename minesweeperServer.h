#ifndef __MINESWEEPERSERVER_H__
#define __MINESWEEPERSERVER_H__

#define MAXDATASIZE 256

#include <stdbool.h>

struct LeaderboardEntry {
	char username[MAXDATASIZE];
	int time;
	int won;
	int played;
};

// define functions here
int ReceiveData(int, char*, short);
int SendData(int, char*, short);
struct GameState PlaceMines();
int TileContainsMine(int, int, struct GameState);
void MinesweeperMenu(int);
void FormatGameState(struct GameState, char*);
void SendLeaderboard(int);
void FlipTile(struct GameState*, int, int, int);
void FlagTile(struct GameState*, int, int, int);
void FlipSurrounds(struct GameState*, int, int);
void GameOverMsg(int, int, bool);
void SortLeaderboard(struct LeaderboardEntry*);
void AddLeaderboardEntry(char[MAXDATASIZE], int, bool);


#endif //__MINESWEEPERSERVER_H__
