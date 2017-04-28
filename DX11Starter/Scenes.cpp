#include "Scenes.h"



Scenes::Scenes()
{
}
Scenes::Scenes(int l, int n, int u, Entity ** e, UI **uu)
{
	level = l;
	objectNum = n;
	uiNum = u;
	gameObject = new Entity*[objectNum];
	gameObject = e;
	uiObject = new UI*[uiNum];
	uiObject = uu;
}

Scenes::~Scenes()
{
	delete[] gameObject;
	delete[] uiObject;
}

int Scenes::getLevel() {
	return level;
}
int Scenes::getobjectNum() {
	return objectNum;
}
int Scenes::getuiNum() {
	return uiNum;
}

void Scenes::drawScene() {
	
	
}