/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: Lexer.h
* Purpose: Lexer
*
* Copyright 2017 Eric Skaliks
*
*/

#pragma once
#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          Error           ]******************************/

class fHaskellError {
public:
	fHaskellError(const string stage, const string msg, size_t line, size_t column) : stage(stage), msg(msg), line(line), column(column) {}
	string toString() const { return fHaskellError::stage + "@" + std::to_string(fHaskellError::line) + "," + std::to_string(fHaskellError::column) + ": " + fHaskellError::msg; }
private:
	const string msg, stage;
	const size_t line, column;
};
class LexerError : public fHaskellError {
public:
	LexerError(string msg, size_t line, size_t column)
		:fHaskellError("Lexer", msg, line, column)
	{}
};


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          Lexem           ]******************************/

struct Lexem {
	enum LexemType {
		// No more lexems
		none, error,

		// Just a symbol
		symbol,
		// Values
		number, string,

		// Brackets
		parOpen, parClose, curlyOpen, curlyClose,

		// Sugar
		pipe, separate, compose, cons,
		// Keywords, see https://wiki.haskell.org/Keywords#.27
		hdata, hif, helse, hthen, hlet, hin, hwhere, hdo, hcase, hof, hotherwise,

		// Operators
		// Arithmetic
		substract, add, multiply, divide, assign, fracpower, intpower, logpower,
		// Compare
		eq, less, leq, geq, greater, neq,
		// Logic
		or , and,
		// Other Operators
		doublecolon, function, concat,

	} type;

	Lexem(Lexem::LexemType t, const char* start, const char* end, const size_t line, const size_t column) : type(t), start(start), end(end), line(line), column(column) {}
	Lexem &operator=(const Lexem&) = default;

	const char* start;
	const char* end;
	size_t line, column;
};

std::string toString(Lexem::LexemType t) {
	switch (t) {
	case Lexem::none: return "none";
	case Lexem::error: return "error";
	case Lexem::symbol: return "Symbol";
	case Lexem::number: return "Number";
	case Lexem::string: return "String";
	default: return "Something else";
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[          Lexer           ]******************************/

class Lexer {
public:
	Lexer(string code);
	Lexem nextLexem() {
		return Lexer::desugarLayout();
	}
	~Lexer();

private:
	Lexem makeLexem(Lexem::LexemType type, const char* start, const char* end) const {
		if (type == Lexem::error) {
			throw LexerError("Unexpected Character with code " + std::to_string(int(*Lexer::pos)), Lexer::initialLine, Lexer::initialColumn);
		}
		else if (type == Lexem::none) {
			return Lexem(type, start, end, 0,0);
		}
		return Lexem(type, start, end, Lexer::initialLine, Lexer::initialColumn);
	}
	Lexem makeLexem(Lexem::LexemType type, const char* start) const {
		return Lexer::makeLexem(type, start, Lexer::pos);
	}
	static bool isLayoutKeyword(Lexem::LexemType type) {
		return type == Lexem::hlet || type == Lexem::hwhere || type == Lexem::hdo || type == Lexem::hof;
	}


	Lexem nextLexemInternal();


	Lexem desugarLayout();
	Lexem cachedLexem;
	vector<size_t> indentStack;
	bool lastWasLayout, hasSeparator;

	Lexem detectKeyword(Lexem symbol);
	void readUntil(char target);
	void readUntil(char targetC1, char targetC2);
	Lexem lookahead(Lexem::LexemType a, Lexem::LexemType b, char bc);
	char* code;
	const char* pos;
	const char* end;
	size_t line, column, initialLine, initialColumn;
};
