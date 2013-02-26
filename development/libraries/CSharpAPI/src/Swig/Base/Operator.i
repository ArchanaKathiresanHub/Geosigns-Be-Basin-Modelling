/*
 * Copyright (C) 2006 Shell International Exploration & Production.
 * All rights reserved. 
 * 
 * Developed under licence for Shell by PDS B.V.
 *
 * Confidential and proprietary source code of Shell. 
 * Do not distribute without written permission from Shell 
 */

// --------------------------------------------------
// Rename overloaded operators not supported by SWIG.
// --------------------------------------------------

/*
 * If there is no automatic mapping from source to target language
 * then the operators are renamed
 */

#ifdef SWIGCSHARP
  %rename(Equals) operator==;
  %rename(NotEquals) operator!=;

  %rename(LessThan) operator<;
  %rename(LessThanOrEquals) operator<=;

  %rename(GreaterThan) operator>;
  %rename(GreaterThanOrEquals) operator>=;

  %rename(Plus) operator+;
  %rename(PlusEquals) operator+=;

  %rename(Minus) operator-;
  %rename(MinusEquals) operator-=;

  %rename(LogicalAnd) operator&;
  %rename(LogicalAndEquals) operator&=;

  %rename(LogicalOr) operator|;
  %rename(LogicalOrEquals) operator|=;

  %rename(LeftShift) operator<<;
  %rename(RightShift) operator>>;

  %rename(Assign) operator=;
  %rename(MoveNext) operator++();
  %ignore operator++(int);

  %rename(PseudoFunction) operator();
  %rename(Subscript) operator[];
  %rename(IsSet) operator bool;

#endif


#ifdef SWIGPYTHON
  %rename(assignFrom)        *::operator=;
  %rename(preIncrement)      *::operator++();
  %rename(postIncrement)     *::operator++(int);

  // Note this renames the operator[] methods 
  // (and removes the warnings) but it may not be
  // useful for methods that return primitive types. 

  %rename(subscriptOperator) *::operator[];
  %rename(boolOperator)      *::operator bool;

#endif

