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

#include "NumericFunctions.h"

const char Escape_Character = 27; 

void stripEndQuotes(std::string& str);

void ReallyStripEndQuotes(std::string& str);

void underscoreBlankSpace(std::string& str);
void underscoreSlashCharacter(std::string& str);
void blankSpaceUnderscore(std::string& str);

bool isUsableCharacter ( const char c );

void removeNonUsableCharacters ( const std::string& nonUsableString,
    std::string& usableString );

std::string removeNonUsableCharacters ( const std::string& nonUsableString );


void removeExtension(std::string& str);
std::string Extract_Property_Name( const std::string& str );
std::string Extract_Owner_Name( const std::string& str );
std::string Extract_Filename_Extension( const std::string& str );
void getDateAndTime(char* str);
void displayTime(const bool debug, const char* str);
void getElapsedTime(char* str);
void monitorProcessMemorySize();
void displayProgress(const bool debug, 
		     const char* str,
		     const double& Age);

void displayTime( const std::string & str, PetscLogDouble StartTime);
void Display_Merging_Progress( const std::string & fileName, PetscLogDouble StartTime);
void Display_Merging_Progress( const std::string & fileName, PetscLogDouble StartTime, const std::string & message );

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

std::string DoubleToString(const double D);
std::string IntegerToString(const int I);
void Copy_String_To_Char( const std::string& str, char* char_array, const int char_length );

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
/// \f$ r = \lor_i v_i \f$
bool broadcastAnyTrueBooleanValue ( const bool localValue );

/// For all processes: returns false if any process passes a false value, otherwise returns true.
///
/// \f$ r = \land_i v_i \f$
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

template<typename Scalar>
inline bool inRange ( const Scalar& value, 
                      const Scalar& lowerLimit,
                      const Scalar& upperLimit ) {

  return (( lowerLimit <= value ) && ( value <= upperLimit ));
  
}



#endif /* _UTILS_H_ */
