// ---------------------------------------
// Helper for classes in the std namespace
// ---------------------------------------
/*
  * Note %template(<empty>) indicates to swig that it does not
 * need to generate wrapper code.
 */

// General name for vector of strings

%include <stl.i>
%include <std_except.i>
%include <exception.i>

%template(BoolVector)		      std::vector< bool >;                       // vector of booleans
%template(IntVector)	            std::vector< int >;                        // vector of integers 
%template(UIntVector)		      std::vector< size_t >;                     // vector of size_t
%template(UnsignedIntVector)	   std::vector< unsigned int >;               // vector of unsigned integers 
%template(DoubleVector)		      std::vector< double >;                     // vector of doubles 
%template(StringVector)		      std::vector< std::string >;                // vector of strings 

%template(PairOfDoubles)		   std::pair< double, double >;               // pair of doubles
%template(StringPair)            std::pair<std::string, std::string>;       // pair of strings

%template(VectorDoubleVector)		std::vector< std::vector< double > >;      // vector of vectors of doubles
%template(VectorStringVector)		std::vector< std::vector< std::string > >; // vector of vector of strings

%template(PairOfDoublesVector)	std::vector< std::pair< double, double > >;         // vector of pair of doubles
%template(StringPairVector)      std::vector< std::pair<std::string, std::string> >; // vector of pair of strings                                                                          
%template(StringUIntPairVector)  std::vector< std::pair<std::string, size_t> >;      // vector of pair string and size_t

%rename(Equals)              operator==;
%rename(NotEquals)           operator!=;
%rename(LessThan)            operator<;
%rename(LessThanOrEquals)    operator<=;
%rename(GreaterThan)         operator>;
%rename(GreaterThanOrEquals) operator>=;
%rename(Plus)                operator+;
%rename(PlusEquals)          operator+=;
%rename(Minus)               operator-;
%rename(MinusEquals)         operator-=;
%rename(LogicalAnd)          operator&;
%rename(LogicalAndEquals)    operator&=;
%rename(LogicalOr)           operator|;
%rename(LogicalOrEquals)     operator|=;
%rename(LeftShift)           operator<<;
%rename(RightShift)          operator>>;
%rename(Assign)              operator=;
%rename(MoveNext)            operator++();
%ignore                      operator++(int);
%rename(PseudoFunction)      operator();
%rename(Subscript)           operator[];
%rename(IsSet)               operator bool;
