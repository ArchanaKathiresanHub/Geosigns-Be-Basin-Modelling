#ifndef POLYNOMIALS_H_
#define POLYNOMIALS_H_

#ifdef sgi
#ifdef _STANDARD_C_PLUS_PLUS
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;
#else // !_STANDARD_C_PLUS_PLUS
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include<strstream.h>
typedef strstream ostringstream;
typedef istrstream istringstream;
#endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;
#endif // sgi


#include <string>
#include <vector>
#include <list>

///provides functionality for parsing piecewise polynomials from a string and evaluation
///
///This namespace provides functionality for parsing piecewise polynomials from a string and evaluation.
///A PiecewisePolynomial is defined by a list of pairs consisting of a Range they are defined on
///and a Polynomial. A Polynomial consists of a list of Term-objects, where a Term is 
///has the form a*x^b (coefficient a, exponent b).
///
///A PiecewisePolynomial can be parsed from a string using the
///functions in the polynomials::parse-namespace (for format description see there).
namespace polynomials
{
  
  //-------------------------------------------------------------------------------	
  ///provides functionality to store and manage 1D-intervals functions are defined on
  class Range
    {
    private:
      double m_lowerBound;
      double m_upperBound;
      
    public:
      Range(); //initializes the Range-object with maximum range of double
      double getLowerBound() const;
      double getUpperBound() const;
      void   setLowerBound(double low);
      void   setUpperBound(double up);
      void init();
      
    };
  
  //-------------------------------------------------------------------------------	
  ///provides functionality to store and evaluationg terms of type a*x^b
  class Term
    {
    private:
      double m_coefficient;
      double m_exponent;
      
    public:
      Term(); 
      void init();
      double getCoefficient() const;
      double getExponent() const;
      void   setCoefficient(double coeff);
      void   setExponent(double expnt);
      double operator()(double x) const;
    };
  
  //-------------------------------------------------------------------------------	
  ///provides functionality to store and evaluate polynomials consisting of terms

   typedef std::vector<Term> TermList;

   //#define USEOLDPOLYNOMIAL 1

#ifdef USEOLDPOLYNOMIAL

  class Polynomial
    {
    public:

      Polynomial(); 
      void init();
      void appendTerm(Term term);
      const TermList& getTerms() const;
      double operator()(double x) const;
    private:
      TermList m_terms;
    };

#else

   class Polynomial
   {
   public:

      typedef std::vector<double> CoeffList;

      Polynomial(); 

      void init();

      void appendTerm(Term term);

      const TermList getTerms() const;

      double operator()(double x) const;

    private:

      CoeffList m_coeffs;
      TermList  m_terms;
      bool      m_allExponentsAreInteger;
      
    };

#endif
  
  //-------------------------------------------------------------------------------	
  
  
  ///provides functionality to store and evaluate a piecewise polynomial
  class PiecewisePolynomial
    {
    public:

       typedef std::vector<Range> RangeList;

       typedef std::vector<Polynomial> PolynomialList;

       // typedef std::list<Range> RangeList;

       // typedef std::list<Polynomial> PolynomialList;


       PiecewisePolynomial(); 
       void init();
       void appendPiece(Polynomial poly, Range range);

       const RangeList& getRanges() const;

       const PolynomialList& getPolynomials() const;

       // const std::list<Range>& getRanges() const;
       // const std::list<Polynomial>& getPolynomials() const;

       double operator()(double x) const;

    private:


       // std::list<Range> m_ranges;
       // std::list<Polynomial> m_polynomials;

       RangeList m_ranges;
       PolynomialList m_polynomials;
      
      
    };
  
  //-------------------------------------------------------------------------------------
  
  ///provides functionality to parse a PiecewisePolynomial, Polynomial, Term and/or Range from a string.
  
  /**First the string must be broken into a vector of Token-objects using the function
     GetTokens(). Then any of the functions ParsePiecewisePolynomial(), ParsePolynomial(), ParseTerm(), ParseRange()
     can be used to parse the corresponding object starting 
     from the position start (passed as argument, usually 0 pointing to first Token) in the Token-vector. 
     After parsing, start will point to the next 
     Token-vector position. The syntax is described in Extended Backus Naur Form (EBNF) for every Parse...() function. */
  namespace parse
    {
      
      enum TokenType 
	{
	  NO_TYPE=0, INT_NUM,	REAL_NUM, END,
	  PLUS='+', MINUS='-',
	  RANGE_START='[', RANGE_END=']',
	  RANGE_SEPARATOR=':',	PIECE_SEPARATOR=';', CARET='^', VARIABLE_NAME='x'
	};
      
      class Token
	{
	private:
	  TokenType m_type;
	  
	  double 		m_realNum;
	  int 			m_intNum;
	  
	public:				
	  Token();
	  Token(TokenType type);
	  Token(int intNum);
	  Token(double realNum);
	  
	  TokenType getType() const;
	  int getIntNum() const;
	  double getRealNum() const;
	  bool operator==(char charType);
	  bool operator==(TokenType type);
	};		
      
      ///(EBNF):   range:= '['  bound ',' bound ']';    bound:= REAL_NUM | '*'
      void ParseRange(std::vector<Token> theTokens, size_t& start, Range& range);
      
      ///(EBNF):   term:= [+|-] REAL_NUM ['x' ['^' REAL_NUM]] 
      void ParseTerm (std::vector<Token> theTokens, size_t& start, Term& term);
      
      ///(EBNF):   polynomial := [+|-] term {+|- term}
      void ParsePolynomial (std::vector<Token> theTokens, size_t& start, Polynomial& term);
      
      ///(EBNF):   piecewise_polynomial := polynomial | (range  polynomial {';' range  polynomial } )
      void ParsePiecewisePolynomial (std::vector<Token> theTokens, size_t& start, PiecewisePolynomial& piecewise);
      
      Token GetNextToken(istream& istr);
      
      ///breaks a string func into a Token-vector
      void GetTokens(const std::string& func, std::vector<Token> & theTokens );
      ostream& operator<<(ostream& os, const Token& tok);
    }
  
  ostream& operator<<(ostream& os, const Range& range);
  ostream& operator<<(ostream& os, const Term& range);
  ostream& operator<<(ostream& os, const Polynomial& poly);
  ostream& operator<<(ostream& os, const PiecewisePolynomial& piecewise);
  
  //-----------------------------------------------------------------------------------------
  namespace error 
    {
      struct SyntaxError 
      {
	std::string info;
	SyntaxError(std::string str) { info = str; }
      };
      
      struct NumericError 
      {
	std::string info;
	NumericError(std::string str) { info = str; }
      };
      
      
    }
}




#endif
