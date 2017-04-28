#pragma once
#include "DXCore.h"
class Input 
{
public:
	Input();
	//Input(Camera *);
	~Input();

	void OnMouseDown(WPARAM buttonState, int x, int y);
	void OnMouseUp(WPARAM buttonState, int x, int y);
	void OnMouseMove(WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta, int x, int y);

	POINT prevMousePos;

	//Camera *cc;

};

