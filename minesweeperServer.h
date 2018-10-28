#ifndef __MINESWEEPERSERVER_H__
#define __MINESWEEPERSERVER_H__

#define MAXDATASIZE 256

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
void SendLeaderboard(int, struct LeaderboardEntry*);
void FlipTile(struct GameState*, int loc);
void GameOverMsg(int, int);

#endif //__MINESWEEPERSERVER_H__
