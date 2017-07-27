/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: worker.h
* Purpose: Thread for progressive rendering.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"
#include "GLHelpers.h"
#include "parallelctx.h"
#include "pRenderer.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        messages         ]*******************************/

struct cancelMessage {
	cancelMessage(bool cancel) : cancel(cancel) {}
	bool cancel;
};
struct sizeChangeMessage {
	sizeChangeMessage(AiSize size) : size(size) {}
	AiSize size;
};

struct workerMsg {
	enum {
		sizeChange,
		cancel,
		end
	} type;

	workerMsg(cancelMessage msg) :
		type(cancel) {
		workerMsg::data.cancel = msg;
	}
	workerMsg(sizeChangeMessage msg) :
		type(sizeChange) {
		workerMsg::data.sizeChange = msg;
	}
	union workerMsgData {
		workerMsgData() {}
		cancelMessage cancel;
		sizeChangeMessage sizeChange;
	} data;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          worker         ]*******************************/

class worker : public abstractWorker<workerMsg> {
public:
	worker(int dummy);
	~worker();
	void render();
	void draw();

private:
	pointer<pRenderer> pR;
	pointer<shader> program;
	struct {
		GLint iter, screenTexture, c, zoom, pos, coec, coe;
		GLint layer;
	} uniform;
	
	float cx, cy, zoomx, zoomy, posx, posy;
	int iter;
	size_t coec;
	float coe[MAX_POLY], coet[MAX_POLY];
	
	pointer<texture> colorMap;

	pointer<syncBuffer> buf;
	pointer<shader>texprogram;
};
