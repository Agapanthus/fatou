/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[           FATOU           ]******************************
*
* File: fHaskell.cpp
* Purpose: Interpreter, Parser, Compiler
*
* Copyright 2017 Eric Skaliks
*
*/

#include "stdafx.h"
#include "fHaskell.h"


/////////////////////////////////////////////////////////////////////////////////////////
/*****************************[         fHaskell          ]*****************************/


fHaskell::fHaskell() : heap(1024*1024) {
	/*
	// Simple test
	any main = AST.getNew();
	main->cdr.set(42, ASExpr_symbol);
	main->car.set(AST.getNew(5));

	main->car.next()->car.next()->cdr.set(main);
	main->car.next()->car.next()->car.next()->cdr.set(AST.getNew(2));

	any tail = main->car.next()->car.next()->car.next();
	AST.free(tail->car.next());
	tail->car.set(AST.getNew(5));

	cout << AST.print(main) << endl;

	cout << "done!" << endl;

	AST.free(main);

	*/

	// TODO:  Benchmark high_p floating point numbers and low_p!

	// TODO: Haskell-Tests:
	// -> Nested functions, inner symbol with the same name as an outer symbol

	string test1(R"(

zoom = param "Zoom" "The magnification" (Real 64 8)
gp::Precision
gp | fixedParam "Automatic Precision" "Try to automatically choose the smallest possible precision depending on the actual zoom." Toggle 
		= if zoom < 0.00000000001 -- TODO!
			then 64bit
			else 32bit
   | otherwise
        = param "Precision" "Higher precisions increase image quality and zoom-ability but decrease performance" Precision

comma::Real 32 32
comma = 8 -- TODO: Make this a paramter?!

-- algebraic Datatype
data iterState = iterState{ count :: real 32 32, position :: complex gp comma}

-- library function
iterate :: (iterState -> iterState) -> iterState -> iterState
iterate (iterState iteration count position)
	| count > param "Iter." "Maximum Iterations" (Real 32 32) 
		= iterState count position
	| otherwise 
		= iteration iterState count+1 position

data accumState = accumState{ count :: real 32 32, function :: complex gp comma, derived :: complex gp comma, x :: complex gp comma}

sample::Complex 32 -> Colour
sample start = 
		 let z = calculateStart start
		     iteration :: (iterState -> iterState) -> iterState -> iterState
		     iteration (iterState iterator count position) = 
				let accum :: Real 32 32 -> Complex gp comma -> Complex gp comma -- TODO: Is this somehow possible with map or fold, without implementing Monads?
					accum accumState count function derived x = 
						| count > fixedParam "Degree" "The number of coefficients in the polynomial" (real 32 32) 
							= accumState count function derived x
						| otherwise 
							let c = param "c"++count "" (complex gp comma)
								nx = x*z
							in accum accumState count+1 function+nx*c derived+x*count*c nx
					(accumState count function derived x) = accum accumState 1 (param "c0" "" (complex gp comma)) 1
					state = iterState count+1 position-function/derived
				in if magnitude function < param "bias" "" real gp comma then
					iterator state
					else state -- TODO: Calculate Smoothing!
		 in interactiveColouring iterate iteration (iterState 0 z)

-- TODO: Animations are also programmable!
-- TODO: Music is programmable!

)");
	string test2(R"(
doGuessing num = do {
  putStrLn "Enter your guess:"
  guess <- getLine
  case compare (read guess) num of
    LT -> do putStrLn "Too low!"
             doGuessing num
    GT -> do putStrLn "Too high!"
             doGuessing num
    EQ -> putStrLn "You Win!"
}

)");


	fHaskell::setCode(test2);
}

void fHaskell::setCode(const string &code) {
	
	fHaskell::parser.parse(fHaskell::heap, code);

	Sleep(1000000);
}


