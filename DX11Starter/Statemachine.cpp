#include <assert.h>
#include "StateMachine.h"

StateMachine::StateMachine(unsigned char maxStates) :
	_maxStates(maxStates),
	currentState(0),
	_eventGenerated(false),
	_pEventData(NULL)
{
}

// generates an external event. called once per external event 
// to start the state machine executing
void StateMachine::ExternalEvent(unsigned char newState, EventData* pData)
{
	
}

// generates an internal event. called from within a state 
// function to transition to a new state
void StateMachine::InternalEvent(unsigned char newState, EventData* pData)
{
	
}

// the state engine executes the state machine states
void StateMachine::StateEngine(void)
{
	
}