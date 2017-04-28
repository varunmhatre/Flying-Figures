#pragma once
//#include "Game.h"
#include "UI.h"
#include "Entity.h"
using namespace std;
class Scenes 
{
	int level;
	int objectNum;
	int uiNum;
	Entity * * gameObject;   
	UI ** uiObject;
public:
	Scenes();
	Scenes(int, int,int, Entity **, UI**);
	int getLevel();
	int getobjectNum();
	int getuiNum();
	~Scenes();
	void drawScene();
};

