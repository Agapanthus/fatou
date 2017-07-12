/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           *******************************
*
* File: abstraction.h
* Purpose: Platform independence. 
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once

#include <fstream>
using std::ifstream;

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         Target            ******************************/

#ifdef _WIN32
	#define OS_WIN
#elif defined(__linux__)
	#pragma GCC diagnostic ignored "-Wswitch"
	#define OS_LIN
	#if !defined(NDEBUG)
		#define _DEBUG
	#endif
#elif defined(__APPLE__) || defined(__MACH__)
	#define OS_MAC
#else
	#error Unknown target OS
#endif

#ifdef _DEBUG
	#define _DEBUGONLY(PARAM) PARAM
#else
	#define _DEBUGONLY(PARAM)
#endif

#if INTPTR_MAX == INT64_MAX
	#define OS_64
#elif INTPTR_MAX == INT32_MAX
	#define OS_32
#else
	#error Unknown pointer size or missing size macros!
#endif


#ifdef OS_WIN
	#define NOMINMAX
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	#include <WinBase.h>
#endif


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          Types            ******************************/

#include <stdint.h>
#include <string>

class ANoncopyable {
public:
	ANoncopyable() {}
private:
	ANoncopyable(const ANoncopyable&);
	ANoncopyable& operator=(const ANoncopyable&);
};

#ifndef int8
typedef int8_t int8;
#endif
#ifndef int16
typedef int16_t int16;
#endif
#ifndef int32
typedef int32_t int32;
#endif
#ifndef int64
typedef int64_t int64;
#endif

#ifndef uint8
typedef uint8_t uint8;
#endif
#ifndef uint16
typedef uint16_t uint16;
#endif
#ifndef uint32
typedef uint32_t uint32;
#endif
#ifndef uint64
typedef uint64_t uint64;
#endif

using std::string;


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[     Error Handling        ******************************/

#include <exception>
using std::exception;

typedef std::bad_alloc AAllocException;

#define ALLOC_CHECK(MEMORY) if(MEMORY == nullptr) throw AAllocException();

#ifdef OS_WIN
inline int32 fatalNote(string msg) {
	switch (MessageBoxA(nullptr, (string("Fatal Error: ") + msg).c_str(), "Fatou::Error",  MB_ICONERROR | MB_SYSTEMMODAL)) {
	case IDIGNORE:
		return 0;
	case IDRETRY:
		return 1;
	default:
		exit(-1);
		return -1;
	}
}
#elif defined(OS_LIN)

#include <unistd.h>

// TODO: funktioniert das Ordnungsgemaess?
// Create terminal and redirect output to it, returns 0 on success,
// -1 otherwise.
inline int make_terminal() {
	/*char  pidarg[256]; // the '--pid=' argument of tail
	pid_t child;       // the pid of the child proc
	pid_t parent;      // the pid of the parent proc
	FILE* fp;          // file to which output is redirected
	int   fn;          // file no of fp

	// Open file for redirection
	fp = fopen("/tmp/asdf.log","w");
	fn = fileno(fp);

	// Get pid of current process and create string with argument for tail
	parent = getpid();
	sprintf( pidarg, "--pid=%d", parent );

	// Create child process
	child = fork();
	if( child == 0 ) {
	// CHILD PROCESS

	// Replace child process with a gnome-terminal running:
	//      tail -f /tmp/asdf.log --pid=<parent_pid>
	// This prints the lines outputed in asdf.log and exits when
	// the parent process dies.
	execlp( "gnome-terminal", "gnome-terminal", "-x", "tail","-f","/tmp/asdf.log", pidarg, nullptr );

	// if there's an error, print out the message and exit
	perror("execlp()");
	exit( -1 );
	} else {
	// PARENT PROCESS
	close(1);      // close stdout
	int ok = dup2( fn, 1 ); // replace stdout with the file

	if( ok != 1 ) {
	perror("dup2()");
	return -1;
	}

	// Make stdout flush on newline, doesn't happen by default
	// since stdout is actually a file at this point.
	setvbuf( stdout, nullptr, _IONBF, BUFSIZ );
	}
	*/
	return 0;
}
inline int32 fatalNote(string msg) {
	/*if(make_terminal()<0) {
	exit(-1); // Wenn was schief geht, keine Hoffnung mehr!
	}
	printf(msg.c_str(),"");*/
	//system(("zenity --error --text=\"" + escapeshellarg(msg) + "\" --title=\"Error\"").c_str());
	//printf((msg+"\n").c_str(),"");
	//execlp("zenity", "zenity", "--error", ("--text=" + msg).c_str(), "--title=Error");  
	std::cout << msg << std::endl;

	pid_t my_pid;
	int status, timeout /* unused ifdef WAIT_FOR_COMPLETION */;
	if (0 == (my_pid = fork())) {
		// if (-1 == execve(argv[0], (char **)argv , nullptr)) {
		if (-1 == execlp("zenity", "zenity", "--error", ("--text=" + msg).c_str(), "--title=Error", (char *)0)) {
			perror("child process execve failed [%m]");
			return -1;
		}
	}
	sleep(5); // Damit man den Dialog sieht!
	return 0;
}
#else
#error IMPL
#endif

inline int32 fatalNote(bool condition, string msg) {
	if (condition) return fatalNote(msg);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        Geometry           ******************************/

struct AiSize {
	AiSize(int32 w, int32 h) : w(w), h(h) {}
	AiSize() : w(0), h(0) {}
	int32 w, h;

	inline bool operator==(const AiSize& compare) const {
		return ((w == compare.w) && (h == compare.h));
	}
	inline bool operator!=(const AiSize& compare) const {
		return ((w != compare.w) || (h != compare.h));
	}
	inline bool operator<=(const AiSize& compare) const {
		return ((w <= compare.w) && (h <= compare.h));
	}
	inline bool operator>(const AiSize& compare) const {
		return ((w > compare.w) && (h > compare.h));
	}
};

struct ASize {
	ASize(float w, float h) : w(w), h(h) {}
	ASize() : w(0.0f), h(0.0f) {}
	float w, h;

	inline bool operator==(const ASize& compare) const {
		return ((w == compare.w) && (h == compare.h));
	}
	inline bool operator!=(const ASize& compare) const {
		return ((w != compare.w) || (h != compare.h));
	}
	inline bool operator<=(const ASize& compare) const {
		return ((w <= compare.w) && (h <= compare.h));
	}
	inline bool operator>(const ASize& compare) const {
		return ((w > compare.w) && (h > compare.h));
	}
};

struct ARect {
	ARect(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
	void set(float l, float t, float r, float b) { ARect::left = l; ARect::top = t; ARect::right = r; ARect::bottom = b; }
	float left, top, right, bottom;
};

// int32 - Rect 
struct AiRect {
	AiRect() : left(0), top(0), right(0), bottom(0) {}
	AiRect(int32 a) : left(a), top(a), right(a), bottom(a) {}
	AiRect(int32 l, int32 t, int32 r, int32 b) : left(l), top(t), right(r), bottom(b) {}
	inline void set(int32 l, int32 t, int32 r, int32 b) { AiRect::left = l; AiRect::top = t; AiRect::right = r; AiRect::bottom = b; }
	int32 left, top, right, bottom;
	inline bool operator==(const AiRect& compare) const {
		return ((left == compare.left) && (top == compare.top) && (right == compare.right) && (bottom == compare.bottom));
	}
};

// DST is the smallest rect containing both A and B.
// If A's or B's height or width is zero, this dimension is ignored.
#define ARect_OR(DST, A, B)\
	if((A.left < A.right) && (B.left < B.right)) { DST.left = A.left < B.left ? A.left : B.left; DST.right = A.right > B.right ? A.right : B.right; }\
	else if(A.left < A.right) { DST.left = A.left; DST.right = A.right; }\
	else if(B.left < B.right) { DST.left = B.left; DST.right = B.right; }\
	else { DST.left = 0; DST.right = 0; }\
	if((A.top < A.bottom) && (B.top < B.bottom)) { DST.top = A.top < B.top ? A.top : B.top; DST.bottom = A.bottom > B.bottom ? A.bottom : B.bottom; }\
	else if(A.top < A.bottom) { DST.top = A.top; DST.bottom = A.bottom; }\
	else if(B.top < B.bottom) { DST.top = B.top; DST.bottom = B.bottom; }\
	else { DST.top = 0; DST.bottom = 0; }
//  DST is the smallest rect containing both A and B.
#define ARect_ORQ(DST, A, B) DST.left = A.left < B.left ? A.left : B.left; DST.right = A.right > B.right ? A.right : B.right; DST.top = A.top < B.top ? A.top : B.top; DST.bottom = A.bottom > B.bottom ? A.bottom : B.bottom;
// DST is the intersection of A and B
#define ARect_AND(DST, A, B) DST.left = A.left > B.left ? A.left : B.left; DST.right = A.right < B.right ? A.right : B.right; DST.top = A.top > B.top ? A.top : B.top; DST.bottom = A.bottom < B.bottom ? A.bottom : B.bottom;
// Evaluates to true, if A and B intersect
#define ARect_Overlap(A, B) (((A.left < B.right) && (A.right > B.left) && (A.top < B.bottom) && (A.bottom > B.top )))

// Evaluates to true, if (a, b) is inside the rect r
#define AB_INSIDE_AR(a, b, r) (((a)<(r).right) && ((a)>=(r).left) && ((b)<(r).bottom) && ((b)>=(r).top))


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         Colors            ******************************/

#define AColor_black 0xFF000000
#define AColor_dgray 0xFF444444
#define AColor_grey 0xFF888888
#define AColor_lgrey 0xFFCCCCCC
#define AColor_white 0xFFFFFFFF
#define AColor_red 0xFFFF0000
#define AColor_green 0xFF00FF00
#define AColor_blue 0xFF0000FF
#define AColor_yellow 0xFFFFFF00
#define AColor_cyan 0xFF00FFFF
#define AColor_magenta 0xFFFF00FF
#define AColor_transparent 0x00000000

// Union von a,r,g,b und einem uint32
struct AColor {
	enum {
		AColor_a = 3,
		AColor_r = 2,
		AColor_g = 1,
		AColor_b = 0
	};
	union {
		uint8 c[4];
		uint32 u;
	};
	AColor(uint8 red, uint8 green, uint8 blue, uint8 alpha = 255) { c[AColor_a] = alpha; c[AColor_r] = red; c[AColor_g] = green; c[AColor_b] = blue; }
	AColor(uint32 uint) : u(uint) {}
	AColor() : u(0) {}
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[     Smart Pointers        ******************************/

#include <vector>
using std::vector;

// Smart Pointer - data is deallocated automatically
template<typename T> class pointer : public ANoncopyable {
	//static_assert(!std::is_base_of<ARefcounted, T>::value, "Don't use smartPointer with ARefcounted! Use AReference instead!");
public:
	explicit pointer() : pointer(nullptr, false) {}
	explicit pointer(T *child, const bool check = true) : c(child) {
		if (check) ALLOC_CHECK(pointer::c);
	}
	pointer(pointer<T> &right) : c(right.release()) { }
	~pointer() {
		if (pointer::c)
			delete pointer::c;
	}
	void reset(T *child = nullptr, bool check = true) {
		if (pointer::c)
			delete pointer::c;
		pointer::c = child;
		if (check) ALLOC_CHECK(pointer::c);
	}
	T *release(T *child = nullptr) {
		T *tmp = pointer::c;
		pointer::c = child;
		return tmp;
	}
	// assign compatible _Right (assume pointer)
	pointer<T>& operator=(pointer<T>& right) {
		reset(right.release());
		return (*this);
	}
	// return designated value
	T& operator*() {
		fatalNote(pointer::c == nullptr, "designating nullptr");
		return (*(pointer::c));
	}
	// return pointer to class object
	T *operator->() {
		fatalNote(pointer::c == nullptr, "addressing nullptr");
		return (pointer::c);
	}
	const T *data() const {
		return c;
	}
private:
	T *c;
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         picopng           ******************************/

int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          Macros           ******************************/


// Jim Balter, https://stackoverflow.com/questions/5256313/c-c-macro-string-concatenation

/*
* Concatenate preprocessor tokens A and B without expanding macro definitions
* (however, if invoked from a macro, macro arguments are expanded).
*/
#define PPCAT_NX(A, B) A ## B

/*
* Concatenate preprocessor tokens A and B after macro-expanding them.
*/
#define PPCAT(A, B) PPCAT_NX(A, B)

/*
* Turn A into a string literal without expanding macro definitions
* (however, if invoked from a macro, macro arguments are expanded).
*/
#define STRINGIZE_NX(A) #A

/*
* Turn A into a string literal after macro-expanding it.
*/
#define STRINGIZE(A) STRINGIZE_NX(A)