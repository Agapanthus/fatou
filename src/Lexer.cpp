/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: Lexer.cpp
* Purpose: Lexer
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "Lexer.h"

/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           Lexer           ]*****************************/



Lexer::Lexer(string code) : line(1), column(1), cachedLexem(Lexem::none, nullptr, nullptr, 0, 0) {
	const std::string::size_type size = code.size();
	Lexer::code = new char[size + 1];
	if (!Lexer::code) throw AAllocException();
	memcpy(Lexer::code, code.c_str(), code.size() + 1);

	Lexer::end = Lexer::code + code.length();

	Lexer::pos = Lexer::code;

	Lexer::lastWasLayout = true; // Top-level
	Lexer::hasSeparator = false;
}
Lexer::~Lexer() {
	delete[] Lexer::code;
}

Lexem Lexer::lookahead(Lexem::LexemType a, Lexem::LexemType b, char bc) {
	fassert(b != '\n');
	fassert(a != '\n');

	if (Lexer::pos + 1 < Lexer::end) {
		if (*(Lexer::pos + 1) == bc) {
			Lexer::pos += 2;
			Lexer::column += 2;
			return Lexer::makeLexem(b, Lexer::pos - 2);
		}
	}
	Lexer::pos++;
	Lexer::column++;
	return Lexer::makeLexem(a, Lexer::pos - 1);
}

void Lexer::readUntil(char target) {
	const char* start = Lexer::pos;
	while (Lexer::pos < Lexer::end) {
		if (*(Lexer::pos) == '\n') {
			Lexer::column = 1;
			Lexer::line++;
		}
		if (*(Lexer::pos) == target) {
			Lexer::pos++;
			Lexer::column++;
			return;
		}
		Lexer::pos++;
		Lexer::column++;
	}
	Lexer::pos = start; // Not found
}
void Lexer::readUntil(char targetC1, char targetC2) {
	const char* start = Lexer::pos;
	while (Lexer::pos + 1 < Lexer::end) {
		if (*(Lexer::pos + 1) == '\n') {
			Lexer::column = 1;
			Lexer::line++;
		}
		if (*(Lexer::pos) == targetC1 && *(Lexer::pos + 1) == targetC2) {
			Lexer::pos += 2;
			Lexer::column += 2;
			return;
		};
		Lexer::pos++;
		Lexer::column++;
	}
	Lexer::pos = start; // Not found
}



Lexem Lexer::desugarLayout() {

	Lexem l = Lexer::cachedLexem;

	if (Lexer::cachedLexem.type == Lexem::none) {
		l = Lexer::nextLexemInternal();
		Lexer::hasSeparator = false;
	}

	string a(l.start, l.end);
	size_t r = l.column;

	if (Lexer::lastWasLayout) {
		Lexer::hasSeparator = true;
		Lexer::lastWasLayout = false;
		if (l.type != Lexem::curlyOpen) {
			Lexer::cachedLexem = l;
			if (indentStack.size() > 0) if (l.column == indentStack[indentStack.size() - 1]) throw LexerError("Invalid indentation", l.line, l.column);
			indentStack.push_back(l.column);
			static const char*const copen = "{";
			return Lexem(Lexem::curlyOpen, copen, copen + 1, 0, 0);
		}
	}
	else {
		if (indentStack.size() > 0) {
			if (indentStack[indentStack.size() - 1] > l.column  // Less indented
																// || (indentStack[indentStack.size() - 1] == l.column && l.type==Lexem::hwhere) // where in list TODO: Step 4: https://en.wikibooks.org/wiki/Haskell/Indentation#Explicit_characters_in_place_of_indentation WHAT?!!
				) {
				Lexer::cachedLexem = l;
				indentStack.pop_back();
				static const char*const close = "}";
				return Lexem(Lexem::curlyClose, close, close + 1, 0, 0);
			}
			if (indentStack[indentStack.size() - 1] == l.column && (!Lexer::hasSeparator))
			{
				Lexer::cachedLexem = l;
				Lexer::hasSeparator = true;
				static const char*const separator = ";";
				return Lexem(Lexem::separate, separator, separator + 1, 0, 0);
			}
		}
	}

	Lexer::cachedLexem.type = Lexem::none;

	// Layout-keywords
	if (isLayoutKeyword(l.type)) {
		Lexer::lastWasLayout = true;
	}
	if (l.type == Lexem::none) {
		// Close all brackets
		if (indentStack.size() > 0) {
			indentStack.pop_back();
			static const char*const close = "}";
			return Lexem(Lexem::curlyClose, close, close + 1, 0, 0);
		}
	}

	return l;
}

Lexem Lexer::nextLexemInternal() {

	// TODO: Assuming there are no tab-characters! Please make sure to replace all tab-characters with spaces before parsing!

	// Skip whitespace
	while (Lexer::pos < Lexer::end) {
		switch (*Lexer::pos) {
		case '\n':
			Lexer::column = 1;
			Lexer::line++;
			Lexer::pos++;
			break;
		case ' ':
		case '\r':
		//case '\t':
			Lexer::pos++;
			Lexer::column++;
			break;
		default:
			goto nextLexemLoopDone;
		}
	}
nextLexemLoopDone:

	// Read Lexem
	if (Lexer::pos >= Lexer::end) return Lexer::makeLexem(Lexem::none, Lexer::pos);

	Lexer::initialLine = Lexer::line;
	Lexer::initialColumn = Lexer::column;

	switch (*Lexer::pos) {
	case '\0': return Lexer::makeLexem(Lexem::none, Lexer::pos);
	case '|': return Lexer::lookahead(Lexem::pipe, Lexem:: or , '|');
	case '(': Lexer::pos++;	Lexer::column++; return Lexer::makeLexem(Lexem::parOpen, Lexer::pos - 1);
	case ')': Lexer::pos++; Lexer::column++; return Lexer::makeLexem(Lexem::parClose, Lexer::pos - 1);
	case '-':
		if (Lexer::pos + 1 < Lexer::end) {
			if (*(Lexer::pos + 1) == '>') {
				Lexer::pos += 2;
				Lexer::column += 2;
				return Lexer::makeLexem(Lexem::function, Lexer::pos - 2);
			}
			if (*(Lexer::pos + 1) == '-') { // Skip Comment
				Lexer::pos += 2;
				Lexer::column += 2;
				Lexer::readUntil('\n');
				return Lexer::nextLexemInternal();
			}
		}
		Lexer::pos++;
		Lexer::column++;
		return Lexer::makeLexem(Lexem::substract, Lexer::pos - 1);
	case '{':
		if (Lexer::pos + 1 < Lexer::end) {
			if (*(Lexer::pos + 1) == '-') {
				Lexer::readUntil('-', '}');
				return Lexer::nextLexemInternal();
			}
		}
		Lexer::pos++;
		Lexer::column++;
		return Lexer::makeLexem(Lexem::curlyOpen, Lexer::pos - 1);
	case '}': Lexer::pos++; Lexer::column++; return Lexer::makeLexem(Lexem::curlyClose, Lexer::pos - 1);
	case '+': return Lexer::lookahead(Lexem::add, Lexem::concat, '+');
	case '*': return Lexer::lookahead(Lexem::multiply, Lexem::logpower, '*');
	case '/': return Lexer::lookahead(Lexem::divide, Lexem::neq, '=');
	case '^': return Lexer::lookahead(Lexem::intpower, Lexem::fracpower, '^');
	case '=': return Lexer::lookahead(Lexem::assign, Lexem::eq, '=');
	case '>': return Lexer::lookahead(Lexem::greater, Lexem::geq, '=');
	case '<': return Lexer::lookahead(Lexem::less, Lexem::leq, '=');
	case '&': return Lexer::lookahead(Lexem::error, Lexem::and, '&');
	case ':': return Lexer::lookahead(Lexem::cons, Lexem::doublecolon, ':');
	case '.': Lexer::pos++; Lexer::column++; return Lexer::makeLexem(Lexem::compose, Lexer::pos - 1);
	case ';': Lexer::pos++; Lexer::column++; return Lexer::makeLexem(Lexem::separate, Lexer::pos - 1);
	case '\"': {
		const char * start = Lexer::pos;
		Lexer::pos++; Lexer::column++;
		Lexer::readUntil('"');
		if ((Lexer::pos - start) >= 2) return Lexer::makeLexem(Lexem::string, start + 1, Lexer::pos - 1);
		else {
			throw LexerError("Unclosed string. \" expected.", Lexer::line, Lexer::column);
			return Lexer::makeLexem(Lexem::error, start);
		}
	}
	default: {
		bool isNumber = false, isSymbol = false, dotOccured = false, eOccured = false;
		const char * start = Lexer::pos;
		while (Lexer::pos < Lexer::end) {
			const char c = *(Lexer::pos);
			if (c >= '0' && c <= '9') { // Digits

			}
			else if (c == 'E' || c == 'e') { // Ee
				if (!isSymbol) {
					// TODO: Allow + and - directly following e!
				}
				if (isNumber && eOccured) return Lexer::makeLexem(Lexem::separate, start);
				eOccured = true;
			}
			else if (c == '.') { // .
				if (isSymbol) return Lexer::detectKeyword(Lexer::makeLexem(Lexem::symbol, start));
				if (isNumber && dotOccured) return Lexer::makeLexem(Lexem::number, start);
				dotOccured = true;
				isNumber = true;
			}
			else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_' || c == '\'') { // Other letters, _ and '
				if (isNumber) return Lexer::makeLexem(Lexem::number, start);
				isSymbol = true;
			}
			else {
				if (start == Lexer::pos) {
					Lexer::pos++;
					return Lexer::makeLexem(Lexem::error, start);
				}
				if (isSymbol) return Lexer::detectKeyword(Lexer::makeLexem(Lexem::symbol, start));
				return Lexer::makeLexem(Lexem::number, start); // Something else... Here it ends.
			}
			Lexer::pos++; Lexer::column++;
		}
		return Lexer::makeLexem(Lexem::none, start);
	}
	}
}

Lexem Lexer::detectKeyword(Lexem symbol) {
	string s(symbol.start, symbol.end);
	if (s == "data") symbol.type = Lexem::hdata;
	else if (s == "if") symbol.type = Lexem::hif;
	else if (s == "then") symbol.type = Lexem::hthen;
	else if (s == "else") symbol.type = Lexem::helse;
	else if (s == "let") symbol.type = Lexem::hlet;
	else if (s == "in") symbol.type = Lexem::hin;
	else if (s == "where") symbol.type = Lexem::hwhere;
	else if (s == "do") symbol.type = Lexem::hdo;
	else if (s == "case") symbol.type = Lexem::hcase;
	else if (s == "of") symbol.type = Lexem::hof;
	else if (s == "otherwise") symbol.type = Lexem::hotherwise;
	return symbol;
}