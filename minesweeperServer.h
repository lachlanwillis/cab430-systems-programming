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
char *FormatGameState(struct GameState);
void SendLeaderboard(int, struct LeaderboardEntry*);


#endif //__MINESWEEPERSERVER_H__
