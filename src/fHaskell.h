/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: fHaskell.h
* Purpose: Interpreter, Parser, Compiler
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"
#include "ASExpression.h"
#include "Lexer.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         fHaskell          ]*****************************/

class ParserError : public fHaskellError {
public:
	ParserError(const string &msg, const Lexem &l) : fHaskellError("Parser", msg, l.line, l.column){

	}
};

class Parser {
public:
	Parser() {}
	any parse(ASExpression &heap, const string &code) {
		lexer.reset(new Lexer(code));

		// TODO: Use LRStar to parse fHaskell
		lexer->nextLexem();

		return nil;
	}

private:
	pointer<Lexer> lexer;
};

class fHaskell {
public:
	fHaskell();

	// Sets a Fatou-Configuration File
	void setCode(const string &code);

	// Returns the ready-to-use shader
	string getShader();

	// Returns some 
	void trace(float x, float y, void **points);


private:
	Parser parser;
	ASExpression heap;
};