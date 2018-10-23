#ifndef __MINESWEEPERSERVER_H__
#define __MINESWEEPERSERVER_H__

// define functions here
int ReceiveData(int, char*, short);
int SendData(int, char*, short);
struct GameState PlaceMines();
int TileContainsMine(int, int, struct GameState);

#endif //__MINESWEEPERSERVER_H__
