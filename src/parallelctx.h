/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: parallelctx.h
* Purpose: Offscreen Context for parallel rendering.
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once

#include "stdafx.h"
#include "GLHelpers.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[    concurrent queue     ]*******************************/

template <typename T> class AQueue {
public:
	bool pop(T& item, bool wait) {
		std::unique_lock<std::mutex> mlock(mutex_);
		if (wait) 
			while (queue_.empty())
				cond_.wait(mlock);
		else if (queue_.empty())
			return false;
		item = queue_.front();
		queue_.pop();
		return true;
	}

	void push(const T& item) {
		std::unique_lock<std::mutex> mlock(mutex_);
		queue_.push(item);
		mlock.unlock();
		cond_.notify_one();
	}

	void push(T&& item) {
		std::unique_lock<std::mutex> mlock(mutex_);
		queue_.push(std::move(item));
		mlock.unlock();
		cond_.notify_one();
	}

private:
	std::queue<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[    Critical Section     ]*******************************/

static std::mutex criticalSectionMutex;
class criticalSection{
public: 
	criticalSection() {
		criticalSectionMutex.lock();
	}
	~criticalSection() {
		criticalSectionMutex.unlock();
	}
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[     offscreenctx        ]*******************************/

template <typename T> class abstractWorker : public AQueue<T>, ANoncopyable {
public:
	virtual void render() = 0;
	virtual void draw() = 0;
};

template <class tworker, typename T> class offscreenctx : ANoncopyable {
	static_assert(std::is_base_of<abstractWorker<T>, tworker>::value, "tworker must derive from abstractWorker");

public:
	offscreenctx(function<tworker*(void)> creator, GLFWwindow * sharedWindow) :
		shouldDie(false), worker(nullptr), creator(creator) {
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		offscreenctx::window = glfwCreateWindow(10, 10, "", NULL, sharedWindow);
		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
	}
	~offscreenctx() {
		offscreenctx::shouldDie = true;
		if(offscreenctx::rendererThread.joinable())
			offscreenctx::rendererThread.join();
		glfwDestroyWindow(offscreenctx::window);
	}
	void start() {
		rendererThread = thread(&offscreenctx::loop, this);
	}
	AQueue<T>* queue() {
		fassert(worker);
		return worker;
	}
	void draw() {
		fassert(worker);
		worker->draw();
	}

protected:
	void use() {
		glfwMakeContextCurrent(offscreenctx::window);
	}
	void loop() {
		{
			criticalSection cx;
			offscreenctx::use();
			glbinding::Binding::initialize(false); // only resolve functions that are actually used (lazy)
			offscreenctx::worker = creator();
		}
		glQueryCreator glqc;
		while (!offscreenctx::shouldDie) {
			glfwPollEvents();
			{
				glQuery glq = glqc.create();
				offscreenctx::worker->render();
			}
		//	cout << glqc.getTime() << endl;
#ifdef OS_WIN
			Sleep(0);
#else
#error IMPL
#endif
		}
		if (worker) delete worker;
	}


private:
	GLFWwindow *window;
	thread rendererThread;
	volatile bool shouldDie;
	tworker *worker;
	function<tworker*(void)> creator;
};