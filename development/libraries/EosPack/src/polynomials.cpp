#include "polynomials.h"

#include <limits>
#include <cmath>


using namespace std;

polynomials::Range::Range()
{
   init();
}
// class Range --------------------------------------------------
void polynomials::Range::init()
{
   m_lowerBound = -std::numeric_limits<double>::max();
   m_upperBound =  std::numeric_limits<double>::max();
}

double polynomials::Range::getLowerBound() const
{   return m_lowerBound;}

double polynomials::Range::getUpperBound() const
{   return m_upperBound;}

void   polynomials::Range::setLowerBound(double low)
{   m_lowerBound = low;}

void   polynomials::Range::setUpperBound(double up)
{   m_upperBound = up;}

// class Term --------------------------------------------------
polynomials::Term::Term()
{
   init();
}
void polynomials::Term::init()
{
   m_coefficient = 0.0;
   m_exponent    = 0.0;
}
double polynomials::Term::getCoefficient() const
{   return m_coefficient;}

double polynomials::Term::getExponent() const
{   return m_exponent;}

void   polynomials::Term::setCoefficient(double coeff)
{   m_coefficient = coeff;}

void   polynomials::Term::setExponent(double expnt)
{   m_exponent = expnt;}

double polynomials::Term::operator()(double x) const
{  return m_coefficient * pow(x, m_exponent);  }

// class Polynomial --------------------------------------------------

#ifdef USEOLDPOLYNOMIAL
polynomials::Polynomial::Polynomial()
{
   init();
}

void polynomials::Polynomial::init()
{
   m_terms.clear();
}

void polynomials::Polynomial::appendTerm(Term term)
{
   m_terms.push_back(term);
}
const polynomials::TermList& polynomials::Polynomial::getTerms() const
{
   return m_terms;
}

double polynomials::Polynomial::operator()(double x) const
{  
   // double sum;
   // int i;

   // sum = m_terms [ m_terms.size () - 1 ].getCoefficient ();

   // for ( i = m_terms.size () - 2; i >= 0; --i ) {
   //    sum = sum * x + iter->getCoefficient ();
   // // for (iter=m_terms.begin();iter!=m_terms.end();++iter) {
   //    sum+=(*iter)(x);  
   // }

   double sum=0.0;
   TermList::const_iterator iter;
   // list<Term>::const_iterator iter;
   for (iter=m_terms.begin();iter!=m_terms.end();++iter)
   {
      sum+=(*iter)(x);  
   }

   return sum;		
}

#else

// class Polynomial --------------------------------------------------
polynomials::Polynomial::Polynomial()
{
   init();
}

void polynomials::Polynomial::init()
{
   m_coeffs.clear();
   m_terms.clear ();
   m_allExponentsAreInteger = true;
   
}

void polynomials::Polynomial::appendTerm(Term term)
{

   int exponent = int ( term.getExponent ());

   if ( double ( exponent ) != term.getExponent ()) {
      m_allExponentsAreInteger = false;
   }

   m_terms.push_back ( term );

   if ( exponent >= m_coeffs.size ()) {
      m_coeffs.resize ( exponent + 1, 0 );
   }

   m_coeffs [ exponent ] = term.getCoefficient ();
}

const polynomials::TermList polynomials::Polynomial::getTerms() const
{
   return m_terms;
}

double polynomials::Polynomial::operator()(double x) const
{  
   double sum = 0.0;
   if ( m_allExponentsAreInteger )
   {         
      for ( CoeffList::const_reverse_iterator iter = m_coeffs.rbegin(); iter != m_coeffs.rend(); ++iter )
      {
         sum = sum * x + (*iter);
      }
      return sum;
   }
   else
   {
      for ( TermList::const_iterator iter = m_terms.begin(); iter != m_terms.end(); ++iter )
      {
         sum+=(*iter)(x);  
      }
   }
   return sum;
}

#endif

// class Polynomial --------------------------------------------------

polynomials::PiecewisePolynomial::PiecewisePolynomial()
{
   init();
}

void polynomials::PiecewisePolynomial::init()
{
   m_ranges.clear();
   m_polynomials.clear();
}

void polynomials::PiecewisePolynomial::appendPiece(Polynomial poly, Range range)
{
   m_ranges.push_back(range);
   m_polynomials.push_back(poly);
}

const polynomials::PiecewisePolynomial::RangeList& polynomials::PiecewisePolynomial::getRanges() const
{  return m_ranges;}

const polynomials::PiecewisePolynomial::PolynomialList& polynomials::PiecewisePolynomial::getPolynomials() const
{  return m_polynomials;}

double polynomials::PiecewisePolynomial::operator()(double x) const
{  
   //find range containing x

   RangeList::const_iterator iterRange;
   PolynomialList::const_iterator iterPoly;

   // list<Range>::const_iterator iterRange;
   // list<Polynomial>::const_iterator iterPoly;
   bool foundRange=false;
   double result = 0.0;
   
   for (iterRange = m_ranges.begin(), iterPoly = m_polynomials.begin();iterRange!=m_ranges.end();++iterRange,++iterPoly)
   {
      if ( ( (*iterRange).getLowerBound() <= x ) && ( x <= (*iterRange).getUpperBound()  ) )
      {	
         foundRange = true;
         const Polynomial& polynomial = (*iterPoly);
         result = polynomial(x); 
         break; 
      }
   }
   
   if (! foundRange) {
      throw error::NumericError("x not in admissible interval.");	
   }
   
   return result;
}


// class Token --------------------------------------------------
polynomials::parse::Token::Token(polynomials::parse::TokenType type)
{ m_type= type; m_realNum=0.0 ; m_intNum=0;}

polynomials::parse::Token::Token()
{ m_type=polynomials::parse::NO_TYPE; m_realNum=0; m_intNum=0;}

polynomials::parse::Token::Token(int intNum)
{ m_type=polynomials::parse::INT_NUM; m_realNum=0.0 ; m_intNum=intNum;}

polynomials::parse::Token::Token(double realNum)
{ m_type=polynomials::parse::REAL_NUM; m_realNum=realNum; m_intNum=0;}

bool polynomials::parse::Token::operator==(char tokChar)
{
   return (  m_type==TokenType(tokChar)  );
}


bool polynomials::parse::Token::operator==(TokenType type)
{
   return (m_type==type);
}

polynomials::parse::TokenType polynomials::parse::Token::getType() const
{return m_type;}

int polynomials::parse::Token::getIntNum() const
{return m_intNum;}

double polynomials::parse::Token::getRealNum() const
{return m_realNum;}



//parsing functions
void polynomials::parse::ParseRange(vector<Token> theTokens, size_t& start, Range& range)
{
   range.init();
   
   if ( start+4 >= theTokens.size() )
      throw error::SyntaxError("reading range failed.");
   
   if (!(theTokens[start]=='['))
      throw error::SyntaxError("reading range failed.");
   
   Token tokLow=theTokens[start+1];
   if (tokLow==REAL_NUM)
      range.setLowerBound(tokLow.getRealNum());
   else if (! (tokLow=='*') )
      throw error::SyntaxError("reading range failed.");
   
   if (!(theTokens[start+2]==':'))
      throw error::SyntaxError("reading range failed.");
   
   Token tokUp =theTokens[start+3];
   if (tokUp==REAL_NUM)
      range.setUpperBound(tokUp.getRealNum());
   else if (! (tokUp=='*') )
      throw error::SyntaxError("reading range failed.");
   
   if (!(theTokens[start+4]==']'))
      throw error::SyntaxError("reading range failed.");
   
   start += 5;
}

void polynomials::parse::ParseTerm(vector<Token> theTokens, size_t& start, Term& term)
{
   int signCoeff=1;
   Token tok; 
   
   if (start < theTokens.size()) 
      tok = theTokens[start]; 
   else 
      throw error::SyntaxError("empty term.");

   //sign before coefficient?
   if (tok=='+') {++start;							}	
   if (tok=='-') {++start; signCoeff = -1;	}
   
   if (start < theTokens.size()) 
      tok = theTokens[start]; 
   else 
      throw error::SyntaxError("empty term.");
   
   //coefficient
   if (tok==REAL_NUM) 
      term.setCoefficient(signCoeff * tok.getRealNum());
   else 
      throw error::SyntaxError("wrong syntax when reading coefficient.");		
   
   //variable x ?
   if (++start < theTokens.size()) 
      tok = theTokens[start]; 
   else 
   {
      term.setExponent(0.0);
      return;
   }
   
   if (tok=='x')
   {	
      term.setExponent(1.0);
   }
   else
   {
      term.setExponent(0.0);
      return;
   }
   
   //exponent?
   if (++start < theTokens.size()) 
      tok = theTokens[start]; 
   else 
   {
      term.setExponent(1.0);
      return;
   }
   
   if (tok=='^')
   {
      if (++start < theTokens.size()) 
         tok = theTokens[start]; 
      else 
         throw error::SyntaxError("empty exponent.");
   }
   else 
      return;
   
   //sign before exponent?
   int signExp=1;
   if (tok=='+')  ++start;
   if (tok=='-') {++start; signExp =- 1;}
   
   if (start < theTokens.size()) 
      tok = theTokens[start]; 
   else 
      throw error::SyntaxError("empty exponent.");
   
   //exponent
   if (tok==REAL_NUM) 
      term.setExponent(signExp * tok.getRealNum());
   else 
      throw	error::SyntaxError("wrong syntax when reading exponent.");		
   
   ++start;
   return;
        
}

void polynomials::parse::ParsePolynomial (std::vector<Token> theTokens, size_t& start, Polynomial& polynomial)
{
   Token tok;
   
   if (start >= theTokens.size()) 
      throw error::SyntaxError("empty polynomial.");
   
   for(;;)
   {
      if (start >= theTokens.size()) 
         return;			
      
      tok = theTokens[start];
      
      if ( (tok=='+') || (tok=='-') ||  (tok==REAL_NUM) )
      {
         Term term;
         ParseTerm(theTokens, start, term);
         polynomial.appendTerm(term);
         
         if (start < theTokens.size())
            tok = theTokens[start];
         else
            tok = Token(END);
         
      }
      
      // check for subsequent terms
      if (! (  (tok=='+') || (tok=='-') ) )
         break; 
      
   }		
}

void polynomials::parse::ParsePiecewisePolynomial (std::vector<Token> theTokens, size_t& start, PiecewisePolynomial& piecewise)
{
   Token tok;
   tok = theTokens[start];
   
   // no range is given -> only parse polynomial and use default (=maximun) range
   if ( (tok=='+') || (tok=='-') ||  (tok==REAL_NUM) )
   {
      Range defaultRange; 			
      Polynomial polynomial;	
      
      ParsePolynomial(theTokens, start, polynomial);
      
      piecewise.appendPiece(polynomial, defaultRange);
      
      return;
   }
   
   
   for(;;)
   {
      if ( ! ( theTokens[start] == '[' ) ) break;	
      
      Range range;
      Polynomial polynomial;	
      
      ParseRange(theTokens, start, range);
      
      ParsePolynomial(theTokens, start, polynomial);
      
      piecewise.appendPiece(polynomial, range);
      
      //		if ( ( theTokens.size() > start+1 ) && ( theTokens[start] == ';' ) )
      if ( ( theTokens.size() > start ) && ( theTokens[start] == ';' ) )
         
         ++start;
      else 
         break;
   }
   
}

void  polynomials::parse::GetTokens(const std::string& func, std::vector<Token> & theTokens )
{

   istringstream ins(func);
   
   list<Token> tokList;
   for(;;)
   {
      Token tok=GetNextToken(ins);
      if (tok==END)
         break;
      else
         tokList.push_back(tok);
      
   }	
   
   //save list in vector
   theTokens.clear();
   theTokens.resize(tokList.size());
   
   int i;
   list<Token>::const_iterator iter;
   for (iter=tokList.begin(),i=0;iter!=tokList.end();++iter,++i)
      theTokens[i]=(*iter);
   
}

polynomials::parse::Token  polynomials::parse::GetNextToken(istream& istr)
{
   char ch; double r;
   
   do 
   {	// skip whitespace except '\n'
      if(!istr.get(ch)) return Token(END);
   } while (ch!='\n' && isspace(ch));

   switch (ch) 
   {
   case 0:
      return Token(END);
   case '+':
   case '-':
   case '[':
   case ']':
   case ':':
   case ';':
   case 'x':
   case '*':
   case '^':
      return Token(TokenType(ch));
   case '0': case '1': case '2': case '3': case '4':
   case '5': case '6': case '7': case '8': case '9':
   case '.':
      istr.putback(ch);
      istr >> r;
      //test if r is an integer
      //int i = int(r);
      //if (double(i)==r)
      //	return Token(i);
      //else
      return Token(r);
   default:
      throw error::SyntaxError("Unknown token.");
      
   }
	return Token(END); //return anything
}

ostream&  polynomials::parse::operator<<(ostream& os, const Token& tok)
{
   if (tok.getType()==REAL_NUM)
      os << tok.getRealNum() << 'r';
   else if (tok.getType()==INT_NUM)  
      os << tok.getIntNum() << 'i';
   else 
      os << char(tok.getType());
   return os;
   
} 

ostream& polynomials::operator<<(ostream& os, const Term& term)
{
   os << term.getCoefficient() << "x^" << term.getExponent();
   return os;
   
} 

ostream& polynomials::operator<<(ostream& os, const Range& range)
{
   os << '[' << range.getLowerBound() << ':' << range.getUpperBound() << ']';
   return os;

} 

ostream& polynomials::operator<<(ostream& os, const Polynomial& poly)
{
   TermList::const_iterator iter;
   const TermList terms=poly.getTerms();   
   
   // list<Term>::const_iterator iter;
   // const list<Term>& terms=poly.getTerms();   
   
   for (iter=terms.begin();iter!=terms.end();++iter)
   {
      double coeff = (*iter).getCoefficient();
      if (coeff>=0.0) 
         os << '+'; 
      
      os << (*iter);  
   }
   return os;		
}

ostream& polynomials::operator<<(ostream& os, const PiecewisePolynomial& piecewise)
{
   polynomials::PiecewisePolynomial::RangeList::const_iterator iterRange;
   polynomials::PiecewisePolynomial::PolynomialList::const_iterator iterPoly;
   
   const polynomials::PiecewisePolynomial::RangeList& ranges = piecewise.getRanges();   
   const polynomials::PiecewisePolynomial::PolynomialList& polynomials = piecewise.getPolynomials();   
   
   for (iterRange=ranges.begin(), iterPoly=polynomials.begin() ;iterPoly!=polynomials.end();++iterPoly, ++iterRange)
   {
      os << (*iterRange) << " " << (*iterPoly) << " ; " ;
   }
   return os;		
}



