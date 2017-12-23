/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: ASExpression.cpp
* Purpose: SExpressions
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "ASExpression.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[     External Symbols      ]*****************************/

AErrorCode ASExpression::outOfMemory("sexpression out of memory"),
ASExpression::invalidParameter("sexpression invalid parameter"),
ASExpression::invalidVirtualAddressSpace("sexpression invalid virtual address space");

const ASExpr_cell _nilTemplate = { ASExpr_register{ uint64(&_nilTemplate) }, ASExpr_register{ uint64(&_nilTemplate) } };
ASExpr_cell * const nil = (ASExpr_cell * const)&_nilTemplate;