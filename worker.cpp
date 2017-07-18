/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: worker.cpp
* Purpose: Thread for progressive rendering.
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "worker.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          worker         ]*******************************/


worker::worker() {
	
}

worker::~worker() {

}

void worker::render() {
	// Get messages
	static workerMsg msg(true);
	while(AQueue::pop(msg, false)) {
		switch (msg.type) {
		case workerMsg::sizeChange:
			break;
		case workerMsg::cancel:
			break;
		}
	}

	// Render a


}