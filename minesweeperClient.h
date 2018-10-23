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

#endif //__MINESWEEPERCLIENT_H__ 
