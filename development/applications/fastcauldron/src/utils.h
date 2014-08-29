#ifndef _UTILS_H_
#define _UTILS_H_

#include <petsc.h>
#include <petscksp.h>
#include <petscts.h>
#include <petscdm.h>
#include <functional>
#include <limits>

#include <math.h>
#include <string>
using namespace std;

#include "NumericFunctions.h"

const char Escape_Character = 27; 

void stripEndQuotes(string& str);

void ReallyStripEndQuotes(string& str);

void underscoreBlankSpace(string& str);
void underscoreSlashCharacter(string& str);
void blankSpaceUnderscore(string& str);

bool isUsableCharacter ( const char c );

void removeNonUsableCharacters ( const string& nonUsableString,
                                       string& usableString );

string removeNonUsableCharacters ( const string& nonUsableString );


void removeExtension(string& str);
string Extract_Property_Name( const string& str );
string Extract_Owner_Name( const string& str );
string Extract_Filename_Extension( const string& str );
void getDateAndTime(char* str);
void displayTime(const bool debug, const char* str);
void getElapsedTime(char* str);
void monitorProcessMemorySize();
void displayProgress(const bool debug, 
		     const char* str,
		     const double& Age);

void displayTime( const string & str, PetscLogDouble StartTime, double * timeToDisplay );

char* currentTimeStr ();


void Display_Temperature_Solver_Progress( const double Age, 
					  const double Time_Step,
                                          const bool   newLine );

void Display_Pressure_Solver_Progress( const int    Iteration_Number,
				       const int    Maximum_Number_Iterations,
				       const double Age, 
				       const double Time_Step,
                                       const bool   newLine );

void Display_Coupled_Solver_Progress ( const int    Iteration_Number,
				       const int    Maximum_Number_Iterations,
				       const double Age, 
				       const double Time_Step,
                                       const bool   newLine );

void displayComparisonProgress(int FileNumber, int TotalNumberOfFiles);

void Clear_To_End_Of_Line ();
void Clear_Screen ();
void Move_To_Location ( const int X, const int Y );

string DoubleToString(const double D);
string IntegerToString(const int I);
void Copy_String_To_Char( const string& str, char* char_array, const int char_length );

void Get_Coefficients_From_String ( const char*   Coefficient_String,
                                    const int     Number_Of_Coefficients,
				    double*&      Coefficients );

void Get_Coordinates_From_Filename_String ( const char*   Filename_String,
					    double &      X_Coordinate,
					    double &      Y_Coordinate );

void StartTiming();
bool File_Exists ( const char* File_Name );

bool File_Exists ( const std::string& fileName );

/// Removes file, returns true if successful.
bool removeFile ( const std::string& fileName );

void setSolverTolerance (       KSP&  linearSolver,
                          const double newTolerance );

void setSolverMaxIterations (       KSP&  linearSolver,
                              const int    newMaxIts );

void setPreconditionerFillLevels (      KSP&  linearSolver,
                                  const int    fillLevel );


void setLinearSolverType (       KSP&        linearSolver,
                           const std::string& solverName ) ;

int getSolverMaxIterations ( KSP& linearSolver );

int Convert_String_Argument_For_FORTRAN_Code ( int* Integer_Array, const char* Input_String );

std::string toLower ( const std::string& str );

double Double_Epsilon ();

double Float_Epsilon ();

#define PETSC_ASSERT(status){ if (!(status)) {/*PetscEnd();*/ assert(status);}}
//  #define PETSC_ASSERT(status){ if (!successfulExecution(status)) {PetscEnd(); assert(status);}}

/// For all processes: returns true if any process passes a true, otherwise returns false.
// Perhaps a name change here! 
/// \$f r = \lor_i v_i \$f
bool broadcastAnyTrueBooleanValue ( const bool localValue );

/// For all processes: returns false if any process passes a false value, otherwise returns true.
///
/// \$f r = \land_i v_i \$f
bool successfulExecution ( const bool localSuccessfulExecution );

/// Get the minimum value from all processes.
double globalMinimum ( const double value );

/// Get the maximum value from all processes.
double globalMaximum ( const double value );

const std::string& getKspConvergedReasonImage ( const KSPConvergedReason reason );

//------------------------------------------------------------//

inline bool streq ( const char* Left, const char* Right );

inline int Integer_Max ( const int X, const int Y );
inline int Integer_Min ( const int X, const int Y );


template<typename Scalar>
bool inRange ( const Scalar& value, 
               const Scalar& lowerLimit,
               const Scalar& upperLimit );


//------------------------------------------------------------//
//  All inlined functions go here
//------------------------------------------------------------//


bool streq ( const char* Left, const char* Right ) {
  return strcmp ( Left, Right ) == 0;
}


//------------------------------------------------------------//


int Integer_Max ( const int X, const int Y ) {

  if ( X > Y ) {
    return X;
  } else {
    return Y;
  } // end if

} // end Integer_Max


//------------------------------------------------------------//


int Integer_Min ( const int X, const int Y ) {

  if ( X < Y ) {
    return X;
  } else {
    return Y;
  } // end if

} // end Integer_Min



template <class Number>
class EqualityTest :public std::binary_function<Number,Number,bool> {
public:
   bool operator ()( const Number &X, const Number &Y )
   {
      return NumericFunctions::isEqual ( X, Y, std::numeric_limits<Number>::epsilon() );
   }
};

template <class Number>
class LessThan:public std::binary_function<Number,Number,bool> {
public:
   bool operator ()( const Number &X, const Number &Y )
   {
      EqualityTest<Number> theTest;

      if( false == theTest(X,Y))
      {
         return X < Y;
      }

      return false;
   }
};

template<typename Scalar>
inline bool inRange ( const Scalar& value, 
                      const Scalar& lowerLimit,
                      const Scalar& upperLimit ) {

  return (( lowerLimit <= value ) && ( value <= upperLimit ));
  
}


//------------------------------------------------------------//

template<typename Scalar>
struct MpiType;

template<>
struct MpiType<char> {
   static const MPI_Datatype type = MPI_CHAR;
};

template<>
struct MpiType<const char> {
   static const MPI_Datatype type = MPI_CHAR;
};

template<>
struct MpiType<unsigned char> {
   static const MPI_Datatype type = MPI_UNSIGNED_CHAR;
};

template<>
struct MpiType<const unsigned char> {
   static const MPI_Datatype type = MPI_UNSIGNED_CHAR;
};

template<>
struct MpiType<int> {
   static const MPI_Datatype type = MPI_INT;
};

template<>
struct MpiType<const int> {
   static const MPI_Datatype type = MPI_INT;
};

template<>
struct MpiType<unsigned int> {
   static const MPI_Datatype type = MPI_UNSIGNED;
};

template<>
struct MpiType<const unsigned int> {
   static const MPI_Datatype type = MPI_UNSIGNED;
};

template<>
struct MpiType<float> {
   static const MPI_Datatype type = MPI_FLOAT;
};

template<>
struct MpiType<const float> {
   static const MPI_Datatype type = MPI_FLOAT;
};

template<>
struct MpiType<double> {
   static const MPI_Datatype type = MPI_DOUBLE;
};

template<>
struct MpiType<const double> {
   static const MPI_Datatype type = MPI_DOUBLE;
};




/// Get the maximum value from all processes.
template<typename Scalar>
Scalar MpiMaximum ( const Scalar& operand ) {

   Scalar localOp = operand;
   Scalar result;

   MPI_Allreduce ( &localOp, &result, 1, 
                   MpiType<Scalar>::type, MPI_MAX, PETSC_COMM_WORLD );

   return result;
}

// Specialisation of MpiMaximum for Boolean values.
template<>
bool MpiMaximum<bool> ( const bool& operand );

/// Get the minimum value from all processes.
template<typename Scalar>
Scalar MpiMinimum ( const Scalar& operand ) {

   Scalar localOp = operand;
   Scalar result;

   MPI_Allreduce ( &localOp, &result, 1, 
                   MpiType<Scalar>::type, MPI_MIN, PETSC_COMM_WORLD );

   return result;
}

// Specialisation of MpiMinimum for Boolean values.
template<>
bool MpiMinimum<bool> ( const bool& operand );


/// Get the sum of values from all processes.
template<typename Scalar>
Scalar MpiSum ( const Scalar& operand ) {

   Scalar localOp = operand;
   Scalar result;

   MPI_Allreduce ( &localOp, &result, 1, 
                   MpiType<Scalar>::type, MPI_SUM, PETSC_COMM_WORLD );

   return result;
}



#endif /* _UTILS_H_ */
