/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU         ]********************************
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
/*****************************[         Target           ]******************************/

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
/*****************************[          Types           ]******************************/

#include <stdint.h>

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

template<typename T> inline T maximum(T a, T b) {
	if (a > b) return a;
	else return b;
}
template<typename T> inline T minimum(T a, T b) {
	if (a < b) return a;
	else return b;
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[     Error Handling       ]******************************/

#include <exception>
using std::runtime_error;

typedef std::bad_alloc AAllocException;

#ifdef USE_ASSERTATIONS
#include <assert.h>
#define fassert(expr) assert(expr)
#else
#define fassert(expr)
#endif


#define ALLOC_CHECK(MEMORY) if(MEMORY == nullptr) throw AAllocException();

#ifdef OS_WIN
inline int32 _fatalNote(std::string msg) {
	//switch (
	MessageBoxA(nullptr, (std::string("Fatal Error: ") + msg).c_str(), "Fatou::Error", MB_ICONERROR | MB_SYSTEMMODAL); //) {
	/*case IDIGNORE:
		return 0;
	case IDRETRY:
		return 1;
	default:*/
		exit(-1);
		return -1;
	//}
}
#elif defined(OS_LIN)

#include <unistd.h>
#include <iostream>

inline int32 _fatalNote(std::string msg) {
	std::cout << "FATAL ERROR: " << msg << std::endl;

	pid_t my_pid;
	int status, timeout /* unused ifdef WAIT_FOR_COMPLETION */;
	if (0 == (my_pid = fork())) {
		if (-1 == execlp("zenity", "zenity", "--error", ("--text=" + msg).c_str(), "--title=Error", (char *)0)) {
			perror("child process execve failed [%m]");
			return -1;
		}
	}
        exit(-1);
	return 0;
}
#else
#error IMPL
#endif

#include <string>
#include <sstream>
using std::ostringstream;

class StreamFormatter {
public:
	operator std::string() const {
		return this->stream.str();
	}
	template<typename T> StreamFormatter& operator << (const T& value) {
		this->stream << value;
		return *this;
	}
private:
	std::ostringstream stream;
};

#define fatalNote(TEXT) _fatalNote(StreamFormatter() << TEXT)


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        Geometry          ]******************************/

#include <math.h>

struct AiSize {
	AiSize(int32 w, int32 h) : w(w), h(h) {}
	AiSize() : w(0), h(0) {}
	union { 
		int32 w; 
		int32 x; 
	};
	union {
		int32 h; 
		int32 y;
	};

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

	inline AiSize operator*(const int fac) const {
		return AiSize(AiSize::w*fac, AiSize::h*fac);
	}
	inline AiSize operator/(const int fac) const {
		return AiSize(AiSize::w / fac, AiSize::h / fac);
	}
	inline AiSize operator+(const int sum) const {
		return AiSize(AiSize::w + sum, AiSize::h + sum);
	}
	inline AiSize operator-(const int sum) const {
		return AiSize(AiSize::w - sum, AiSize::h - sum);
	}

	inline AiSize operator-(const AiSize b) const {
		return AiSize(AiSize::w - b.w, AiSize::h - b.h);
	}
	inline AiSize operator+(const AiSize b) const {
		return AiSize(AiSize::w + b.w, AiSize::h + b.h);
	}
	inline AiSize operator/(const AiSize b) const {
		return AiSize(AiSize::w / b.w, AiSize::h / b.h);
	}
	inline AiSize operator*(const AiSize b) const {
		return AiSize(AiSize::w * b.w, AiSize::h * b.h);
	}

	inline float magnitude() {
		return sqrtf(float(AiSize::w*AiSize::w + AiSize::h*AiSize::h));
	}
};

typedef AiSize AiPoint;

struct ASize {
	explicit ASize(AiSize in) : w(float(in.w)), h(float(in.h)) {}
	ASize(float w, float h) : w(w), h(h) {}
	ASize() : w(0.0f), h(0.0f) {}
	union {
		float x;
		float w;
	};
	union {
		float y;
		float h;
	};
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
	
	inline ASize operator/(const ASize &div) const {
		return ASize(ASize::w / div.w, ASize::h / div.h);
	}
	inline ASize operator+(const ASize &div) const {
		return ASize(ASize::w + div.w, ASize::h + div.h);
	}
	inline ASize operator*(const ASize &div) const {
		return ASize(ASize::w * div.w, ASize::h * div.h);
	}
	inline ASize operator-(const ASize &div) const {
		return ASize(ASize::w - div.w, ASize::h - div.h);
	}

	inline ASize operator/(const float &div) const {
		return ASize(ASize::w / div, ASize::h / div);
	}
	inline ASize operator+(const float &div) const {
		return ASize(ASize::w + div, ASize::h + div);
	}
	inline ASize operator*(const float &div) const {
		return ASize(ASize::w * div, ASize::h * div);
	}
	inline ASize operator-(const float &div) const {
		return ASize(ASize::w - div, ASize::h - div);
	}
};
typedef ASize APoint;

struct ARect {
	ARect() : left(.0f), top(.0f), right(.0f), bottom(.0f) {}
	ARect(float a) : left(a), top(a), right(a), bottom(a) {}
	ARect(float l, float t, float r, float b) : left(l), top(t), right(r), bottom(b) {}
	void set(float l, float t, float r, float b) { ARect::left = l; ARect::top = t; ARect::right = r; ARect::bottom = b; }
	float left, top, right, bottom;
	inline bool operator==(const ARect& compare) const {
		return ((left == compare.left) && (top == compare.top) && (right == compare.right) && (bottom == compare.bottom));
	}
	inline ARect operator*(const float fac) const {
		return ARect(ARect::left*fac, ARect::top*fac, ARect::right*fac, ARect::bottom*fac);
	}
	inline ARect operator/(const float fac) const {
		return ARect(ARect::left/fac, ARect::top/fac, ARect::right/fac, ARect::bottom/fac);
	}
	inline ARect operator+(const float sum) const {
		return ARect(ARect::left+ sum, ARect::top+ sum, ARect::right+ sum, ARect::bottom+ sum);
	}
	inline ARect operator-(const float sum) const {
		return ARect(ARect::left - sum, ARect::top - sum, ARect::right - sum, ARect::bottom - sum);
	}

	inline ARect operator+(const ARect& sum) const {
		return ARect(ARect::left + sum.left, ARect::top + sum.top, ARect::right + sum.right, ARect::bottom + sum.bottom);
	}
	inline ARect operator+(const ASize &sum) const {
		return ARect(ARect::left + sum.w, ARect::top + sum.h, ARect::right + sum.w, ARect::bottom + sum.h);
	}
	inline ARect operator/(const ASize &sum) const {
		return ARect(ARect::left / sum.w, ARect::top / sum.h, ARect::right / sum.w, ARect::bottom / sum.h);
	}
	inline ARect operator*(const ASize &sum) const {
		return ARect(ARect::left * sum.w, ARect::top * sum.h, ARect::right * sum.w, ARect::bottom * sum.h);
	}
	inline ARect operator-(const ASize &sum) const {
		return ARect(ARect::left - sum.w, ARect::top - sum.h, ARect::right - sum.w, ARect::bottom - sum.h);
	}
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



/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         Colors           ]******************************/

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
/*****************************[     Smart Pointers       ]******************************/

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
		if(pointer::c == nullptr) fatalNote("designating nullptr");
		return (*(pointer::c));
	}
	// return pointer to class object
	T *operator->() {
		if(pointer::c == nullptr) fatalNote("addressing nullptr");
		return (pointer::c);
	}
	const T *data() const {
		return c;
	}
	bool empty() const {
		return c == nullptr;
	}
private:
	T *c;
};



/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          Strings         ]******************************/

#include <string>
#include <utf8.h>
#include <iomanip>

using std::string;
#ifdef OS_WIN
class string16 : public std::wstring {
public: string16(const char16_t * input) : std::wstring((wchar_t*)input) {}
		string16() {}
		template <typename T>string16(T t) : std::wstring(t) {}
};
typedef std::basic_string<char32_t, std::char_traits<char32_t>, std::allocator<char32_t>> string32;
#elif defined(OS_LIN)
typedef std::basic_string<char16_t, std::char_traits<char16_t>, std::allocator<char16_t>> string16;
typedef std::wstring string32;
#else
#error IMPL
#endif


inline string16 toUTF16(const string &str) {
	string16 tmp;
	utf8::utf8to16(str.begin(), str.end(), back_inserter(tmp));
	return tmp;
}
inline string toUTF8(const string16 &str) {
	string tmp;
	utf8::utf16to8(str.begin(), str.end(), back_inserter(tmp));
	return tmp;
}

inline string toUTF8(const string32 &str) {
	string tmp;
	utf8::utf32to8(str.begin(), str.end(), back_inserter(tmp));
	return tmp;
}


template<typename T> string toString(T t) {
	return std::to_string(t);
}
inline string toString(ASize size) {
	return string("(") + toString(size.w) + ", " + toString(size.h) + ")";
}
inline string toString(AiSize size) {
	return string("(") + toString(size.w) + ", " + toString(size.h) + ")";
}
inline string toString(AiRect rect) {
	return string("(") + toString(rect.left) + ", " + toString(rect.top) + ", " + toString(rect.right) + ", " + toString(rect.bottom) + ")";
}
inline string toString(ARect rect) {
	return string("(") + toString(rect.left) + ", " + toString(rect.top) + ", " + toString(rect.right) + ", " + toString(rect.bottom) + ")";
}
inline string toString(string in) {
	return in;
}
inline string toString(string16 in) {
	return toUTF8(in);
}
inline string toString(const char* in) {
	return string(in);
}
inline string toString(float in, int precision) {
	std::stringstream stream;
	stream << std::fixed << std::setprecision(precision) << in;
	return stream.str();
}
template<typename T> string int_to_hex(T t) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(sizeof(T) * 2)
		<< std::hex << t;
	return stream.str();
}

template<typename t> string16 toString16(t T) {
#ifdef OS_WIN
	return std::to_wstring(T);
#elif defined(OS_LIN)
	return toUTF16(toString(T));
#else 
#error IMPL
#endif
}
inline string16 toString16(AiSize size) {
	return string16(u"(") + toString16(size.w) + string16(u", ") + toString16(size.h) + string16(u")");
}
inline string16 toString16(AiRect rect) {
	return string16(u"(") + toString16(rect.left) + string16(u", ") + toString16(rect.top) + string16(u", ") + toString16(rect.right) + string16(u", ") + toString16(rect.bottom) + string16(u")");
}
inline string16 toString16(ARect rect) {
	return string16(u"(") + toString16(rect.left) + string16(u", ") + toString16(rect.top) + string16(u", ") + toString16(rect.right) + string16(u", ") + toString16(rect.bottom) + string16(u")");
}
inline string16 toString16(string in) {
	return toUTF16(in);
}



/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         picopng          ]******************************/

int decodePNG(std::vector<unsigned char>& out_image, unsigned long& image_width, unsigned long& image_height, const unsigned char* in_png, size_t in_size, bool convert_to_rgba32 = true);


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[        debugging         ]******************************/

#ifdef USE_CONSOLE
#include <iostream>
#ifdef OS_WIN
	#include <io.h>
	#include <fcntl.h>
#endif

using std::cout;
using std::endl;
using std::cerr;

class AConsole {
public:
	AConsole(int consoleLines) {
#ifdef OS_WIN
		// allocate a console for this app

		AllocConsole();

		// set the screen buffer to be big enough to let us scroll text

		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
		coninfo.dwSize.Y = consoleLines;
		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

		// redirect unbuffered STDOUT to the console

		lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
		AConsole::fp[0] = _fdopen(hConHandle, "w");
		ALLOC_CHECK(AConsole::fp[0]);
		*stdout = *AConsole::fp[0];
		setvbuf(stdout, nullptr, _IONBF, 0);

		// cout umleiten

		AConsole::of[0] = new std::ofstream(AConsole::fp[0]);
		ALLOC_CHECK(AConsole::of[0])
			std::cout.rdbuf(AConsole::of[0]->rdbuf());

		// redirect unbuffered STDIN to the console

		lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
		hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
		AConsole::fp[1] = _fdopen(hConHandle, "r");
		ALLOC_CHECK(AConsole::fp[1]);
		*stdin = *AConsole::fp[1];
		setvbuf(stdin, nullptr, _IONBF, 0);

		AConsole::of[1] = new std::ofstream(AConsole::fp[1]);
		ALLOC_CHECK(AConsole::of[1])
			std::cin.rdbuf(AConsole::of[1]->rdbuf());

		// redirect unbuffered STDERR to the console

		lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
		hConHandle = _open_osfhandle((intptr_t)lStdHandle, _O_TEXT);
		AConsole::fp[2] = _fdopen(hConHandle, "w");
		ALLOC_CHECK(AConsole::fp[2]);
		*stderr = *AConsole::fp[2];
		setvbuf(stderr, nullptr, _IONBF, 0);

		AConsole::of[2] = new std::ofstream(AConsole::fp[2]);
		ALLOC_CHECK(AConsole::of[2])
			std::cerr.rdbuf(AConsole::of[2]->rdbuf());

		// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well

		std::ios::sync_with_stdio(true);
#endif
	}
	~AConsole() {
#ifdef OS_WIN
		if (AConsole::of[0]) {
			std::cout.rdbuf(nullptr);
			delete AConsole::of[0];
			AConsole::of[0] = nullptr;
		}
		if (AConsole::of[1]) {
			std::cin.rdbuf(nullptr);
			delete AConsole::of[1];
			AConsole::of[1] = nullptr;
		}
		if (AConsole::of[2]) {
			std::cerr.rdbuf(nullptr);
			delete AConsole::of[2];
			AConsole::of[2] = nullptr;
		}
#endif
	}

private:
#ifdef OS_WIN
	int hConHandle;
	HANDLE lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp[3];
	std::ofstream *of[3];

#endif

};

#else

#endif


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           QPC            ]******************************/

class QPC {
#ifdef OS_WIN
public:
	QPC() {
		LARGE_INTEGER li;

		if (!QueryPerformanceFrequency(&li))
			fatalNote("QueryPerformanceFrequency failed!");

		QPC::PCFreq = double(li.QuadPart) / 1000.0;

		QueryPerformanceCounter(&li);
		QPC::CounterStart = li.QuadPart;

	}
	double get() {
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		double dif = double(li.QuadPart - CounterStart) / QPC::PCFreq;
		QPC::CounterStart = li.QuadPart;
		return dif;
	}
	void sleep(int32 millis) {
		Sleep(millis);
	}
	~QPC() {

	}
private:
	double PCFreq = 0.0;
	int64 CounterStart = 0;
#elif defined(OS_LIN)
public:
	QPC() {
		clock_gettime(CLOCK_REALTIME, &CounterStart);
	}
	double get() {
		struct timespec tv;
		clock_gettime(CLOCK_REALTIME, &tv);
		double dif = (tv.tv_nsec - CounterStart.tv_nsec) / 1000000.0 + (tv.tv_sec - CounterStart.tv_sec) * 1000.0;
		CounterStart = tv;
		return dif;
	}
	void sleep(int32 millis) {
		cout << millis << endl;
		struct timespec ts;
		ts.tv_sec = millis / 1000;
		ts.tv_nsec = (millis % 1000) * 1000000;
		nanosleep(&ts, NULL);
	}
	~QPC() {

	}
private:
	struct timespec CounterStart;

    
#else
#error IMPL
#endif
};