/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: ASExpression.h
* Purpose: SExpressions
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[       Definitions         ]*****************************/

enum ASExpr_type {
	ASExpr_intern = 0,
	ASExpr_symbol = 1,
	ASExpr_string = 2,
	ASExpr_instruction = 3,
};

#ifdef USE_17BYTE_CELL
#include "ASExpression_17Byte.h"
#else
#include "ASExpression_16Byte.h"
#endif 
