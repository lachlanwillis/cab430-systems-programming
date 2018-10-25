#ifndef __MINESWEEPERCLIENT_H__
#define __MINESWEEPERCLIENT_H__

// define functions here
void StartMinesweeper(int);
int ReceiveData(int, char*, short);
int SendData(int, char*, short);
int DisplayMenu();
void PlayMinesweeper(int);
void ShowLeaderboard(int);
void DrawGame();
int ReceiveLeaderboard(int, int);
char * ReceiveGameState(int);
void SendGameChoice(int, char*, int);
int GetTileCoordinates();

#endif //__MINESWEEPERCLIENT_H__
