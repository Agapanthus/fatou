/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: ASExpression_16Byte.cpp
* Purpose: Symbolic Expression implementation for systems with "regular" virtual memory 
*			(e.g., it is possible to store flags in the MSBs of pointers)
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"

// Detect some cases, when one tries to use a cell which is not allocated at this moment.
#define CHECK_FOR_MISSUSED_CELLS


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Definitions         ]*****************************/

#ifdef OS_WIN
#define A_MEMORY_ALIGN16 __declspec(align(16)) 
#define A_ALLOC_ALIGN16(SIZE) _aligned_malloc(SIZE, 16)
#define A_FREE_ALIGN16(MEM) _aligned_free(MEM)
#else
#error "TODO: Implement"
#endif

#define UINT64_ONE UINT64_C(1)
#define ASExprFlags_cycle (UINT64_ONE << 63)
#define ASExprFlags_garbage (UINT64_ONE << 62)

#define ASExpr_FLAG_MASK UINT64_C(0xC000000000000000)
#define ASExpr_TYPE_MASK UINT64_C(0x3F00000000000000)
#define ASExpr_DATA_MASK UINT64_C(0x00FFFFFFFFFFFFFF)
#define ASExpr_FT_MASK (ASExpr_TYPE_MASK | ASExpr_FLAG_MASK)

struct ASExpr_cell;
typedef ASExpr_cell* any;
struct ASExpr_register {
	inline bool check(ASExpr_type type) {
		return ASExpr_register::dataType() == type;
	}
	inline ASExpr_type dataType() {
		return ASExpr_type((ASExpr_register::ptr & ASExpr_TYPE_MASK) >> 56);
	}
	inline uint64 get() {
#ifdef CHECK_FOR_MISSUSED_CELLS
		fassert((ASExpr_register::ptr & ASExprFlags_garbage) == 0); // You must not use a cell which is not allocated!
#endif
		return ASExpr_register::ptr & ASExpr_DATA_MASK;
	}
	inline any next() {
		fassert(ASExpr_type::ASExpr_intern == 0);
		fassert(ASExpr_register::check(ASExpr_intern));
		return any(ASExpr_register::ptr);
		//return any(ASExpr_half::get());
	}
	inline void set(any target) {
		fassert(ASExpr_type::ASExpr_intern == 0);
		fassert((ASExpr_FT_MASK & uint64(target)) == 0);
		ASExpr_register::ptr = uint64(target);
	//	ASExpr_half::set(uint64(target), ASExpr_type::ASExpr_intern);
	}
	inline void set(uint64 target, ASExpr_type type) {
#ifdef CHECK_FOR_MISSUSED_CELLS
		fassert((ASExpr_register::ptr & ASExprFlags_garbage) == 0);
#endif
		fassert((ASExpr_FT_MASK & uint64(target)) == 0);
		fassert(uint64(type) < (1 << 6));
		ASExpr_register::ptr = (uint64(type) << 56) | target;
	}
	uint64 ptr;
};

A_MEMORY_ALIGN16 struct ASExpr_cell {
	ASExpr_register car, cdr;
};
	
extern const ASExpr_cell _nilTemplate;
extern ASExpr_cell * const nil;


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         Allocator         ]*****************************/

class ASExpression {
public:
	static AErrorCode outOfMemory, invalidParameter, invalidVirtualAddressSpace;
	class ASExpressionException : public AException { public: ASExpressionException(const AErrorCode &code, const string &details = "") : AException(code, details) {} };

	ASExpression(size_t size) {
		const char *addressTester = "A";
		if(! ((uint64(addressTester) & ASExpr_FLAG_MASK) == UINT64_C(0))) throw ASExpressionException(invalidVirtualAddressSpace);
		if(! ((uint64(addressTester) & ASExpr_DATA_MASK) != UINT64_C(0))) throw ASExpressionException(invalidVirtualAddressSpace);

		// Allocation
		if (size < 3) throw ASExpressionException(invalidParameter, "allocate at least 3 cells");

#ifdef USE_17BYTE_CELL
		ASExpression::first = new ASExpr_cell[size];
#else
		ASExpression::first = (ASExpr_cell*)A_ALLOC_ALIGN16(size * sizeof(ASExpr_cell));
#endif
		if (!ASExpression::first) throw AAllocException();

#ifndef USE_17BYTE_CELL
		if (0 != (ASExpr_FLAG_MASK & uint64(ASExpression::first))) throw ASExpressionException(invalidVirtualAddressSpace);
		if (0 != (ASExpr_FLAG_MASK & uint64(ASExpression::first + size))) throw ASExpressionException(invalidVirtualAddressSpace);
#endif

		// Initialize the data structure!
		ASExpression::last = ASExpression::first + size - 1;
		ASExpression::avail = ASExpression::first; // Point to the first cell
		fassert(ASExpr_type::ASExpr_intern == 0); // If this is true, we can simply leave all the flags zero to init the structure!
		ASExpression::avail->car.set(nil);

		any tmp = ASExpression::avail;
		while (tmp < ASExpression::last) {
			tmp->car.set(tmp+1); 
#ifdef CHECK_FOR_MISSUSED_CELLS
			tmp->car.ptr |= ASExprFlags_garbage;
#endif
			tmp++;
		}
		ASExpression::last->car.set(nil);
		ASExpression::last->cdr.set(nil);
	}
	~ASExpression() {
#ifdef USE_17BYTE_CELL
		delete[] ASExpression::first;
#else
		A_FREE_ALIGN16(ASExpression::first);
#endif
	}

	// Get a new cell - both car and cdr are nil
	inline any getNew() {			
		// Are there any free cells available?
		if(ASExpression::avail->car.next() == nil) throw ASExpressionException(outOfMemory);
		// Get a new cell!
		any tmp = ASExpression::avail;
		ASExpression::avail = ASExpression::avail->car.next();
		tmp->car.set(nil);
		tmp->cdr.set(nil);
		return tmp;
	}

	// Get count new cells - they are initialized, pointing to each other, the last car and every cdr pointing to nil
	inline any getNew(size_t count) {	
		fassert(count > 0);
		any result = ASExpression::avail;
		any end = result;
		for(size_t i = 0; i < count; i++) {		
#ifdef CHECK_FOR_MISSUSED_CELLS
			end->car.ptr &= ~ASExprFlags_garbage;
#endif
			// Are there any free cells available?
			if (ASExpression::avail->car.next() == nil) throw ASExpressionException(outOfMemory);
			// initialize those cells
			end = ASExpression::avail;
			end->cdr.set(nil);
			ASExpression::avail = ASExpression::avail->car.next();
		}
		end->car.set(nil);
		return result;
	}

	// recursively deletes all connected via car or cdr connected cells. recursion is ok.
	inline void free(any parent) {
		if (parent == nil) return;
		parent->cdr.ptr |= ASExprFlags_cycle;
		if (parent->cdr.dataType() == ASExpr_type::ASExpr_intern)
			if((parent->cdr.next()->cdr.ptr & ASExprFlags_cycle) == 0)
				ASExpression::free(parent->cdr.next());
		if (parent->car.dataType() == ASExpr_type::ASExpr_intern)
			if((parent->car.next()->cdr.ptr & ASExprFlags_cycle) == 0)
				ASExpression::free(parent->car.next());
	
		// Free it
		parent->car.set(ASExpression::avail);
#ifdef CHECK_FOR_MISSUSED_CELLS
		fassert((parent->car.ptr & ASExprFlags_garbage) == 0); // Double free!
		parent->car.ptr |= ASExprFlags_garbage;
#endif
		ASExpression::avail = parent;
	}


	string indent(const string &in, const string &step) {
		std::stringstream ss(in);
		string to;
		string result;
		while (std::getline(ss, to, '\n')) {
			result += step + to + "\n";
		}
		return result;
	}

	// print the tree to a string
	string print(any parent) {
		if (parent == nil) return "# Nil\n";
		static const char* indStep = "  ";
		string result = "# " + toString(uint64(parent - ASExpression::first)) + "\n";
		uint64 backup = parent->cdr.ptr;
		parent->cdr.ptr |= ASExprFlags_cycle;

#ifdef CHECK_FOR_MISSUSED_CELLS
		fassert((parent->car.ptr & ASExprFlags_garbage) == 0); // Trying to print non-allocated cells!
#endif

		if (parent->cdr.dataType() == ASExpr_type::ASExpr_intern)
			if ((parent->cdr.next()->cdr.ptr & ASExprFlags_cycle) == 0)
				result += indent(print(parent->cdr.next()), indStep);
			else result += toString(indStep) + "-> " + ((parent->cdr.next() == nil) ? "nil" : toString(uint64(parent->cdr.next() - ASExpression::first))) + "\n";
		else result += toString(indStep) + ":" + toString(uint64(parent->cdr.dataType())) + " " + toString(parent->cdr.get()) + "\n";


		if (parent->car.dataType() == ASExpr_type::ASExpr_intern)
			if ((parent->car.next()->cdr.ptr & ASExprFlags_cycle) == 0)
				result += print(parent->car.next());
			else result += "-> " + ((parent->car.next() == nil) ? "nil" : toString(uint64(parent->car.next() - ASExpression::first))) + "\n";
		else result += ":" + toString(uint64(parent->cdr.dataType())) + " " + toString(parent->cdr.get()) + "\n";

		parent->cdr.ptr = backup;
		return result;
	}

private:
	ASExpr_cell *first, *last;
	// Last available cell. It car-points to the next available cell etc...
	ASExpr_cell *avail;	

};
