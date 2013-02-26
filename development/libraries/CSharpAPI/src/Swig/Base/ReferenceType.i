/*
 * Copyright (C) 2006 Shell International Exploration & Production.
 * All rights reserved. 
 * 
 * Developed under licence for Shell by PDS B.V.
 *
 * Confidential and proprietary source code of Shell. 
 * Do not distribute without written permission from Shell 
 */


// ----------------------------------------
// Passing primitive datatypes by reference
// ----------------------------------------

/*
 * To pass a (mutable) primitive datatype by reference an object needs 
 * to be created. Use %pointer_class( type, name ) to define the class
 * wrapper. The progra, in the target language can then create an 
 * instance of this class and pass it to the function. For example  :
 *
 *	  The method to test in C++
 *
 *	void someMethod( unsigned int& retVal );
 *
 *
 *    In the project SWIG file
 *
 *  %module ModuleName
 *  %include Common.i
 *
 *   In Python
 * 
 *  # (someMethod should set myVal to 123456)
 *	myVal = ModuleName.uintP()
 *	ModuleName.someMethod( myVal )
 *  assert myVal.value() == 123456
 */

%{
%}


%include <cpointer.i>
%include <std_string.i>

%pointer_class( int , intP )
%pointer_class( unsigned int , uintP )
%pointer_class( unsigned char , ucharP )
%pointer_class( float , floatP )
%pointer_class( double , doubleP )
%pointer_class( bool , boolP )
%pointer_class( std::string, stringP )

