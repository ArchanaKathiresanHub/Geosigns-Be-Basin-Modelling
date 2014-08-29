#include "mangle.h"
#include "utils.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <assert.h>

#include "System.h"
#include "globaldefs.h"

PetscLogDouble StartTime, EndTime;
PetscLogDouble CalculationTime;

int X_loc = 1;

void stripEndQuotes(string& str) {

#if 0
  string::size_type pos = str.find ("\"",0);
  if (pos == 0) str.erase(0,1);
  pos = str.find ("\"",0);
  if (pos == str.length()-1) str.erase(pos,1);
#endif
}

void ReallyStripEndQuotes(string& str) {

  string::size_type pos = str.find ("\"",0);
  if (pos == 0) str.erase(0,1);
  pos = str.find ("\"",0);
  if (pos == str.length()-1) str.erase(pos,1);
}

void underscoreBlankSpace(string& str) {

  string::size_type pos;
  for (pos=0; pos<str.length(); pos++) {
    if (str[pos] == ' ') {
      str[pos] = '_';
    }
  }

}
 
void underscoreSlashCharacter(string& str) {
  
  string::size_type pos;
  for (pos=0; pos<str.length(); pos++) {
    if (str[pos] == '/') {
      str[pos] = '_';
    }
  }

}

void blankSpaceUnderscore(string& str) {

  string::size_type pos;
  for (pos=0; pos<str.length(); pos++) {
    if (str[pos] == '_') {
      str[pos] = ' ';
    }
  }

}


void removeNonUsableCharacters ( const string& nonUsableString,
                                       string& usableString ) {
  
  usableString = utilities::mangle ( nonUsableString );
}

string removeNonUsableCharacters ( const string& nonUsableString ) {

  return utilities::mangle ( nonUsableString );
}


string Extract_Property_Name( const string& str ) 
{

  string A_String = str;

  string::size_type pos = A_String.find ("@",0);
  return A_String.substr(0,pos);

}

string Extract_Owner_Name( const string& str ) 
{

  string A_String = str;

  string::size_type pos = A_String.find ("@",0);
  return A_String.substr(++pos,A_String.length());

}

string Extract_Filename_Extension( const string& str ) {
  
  string filename  = str;
  string extension = "";
  string::size_type loc = filename.find (".",0);
  if ( loc != string::npos )
  {
    extension = filename.substr( ++loc );
  }
  return extension;

}

void removeExtension(string& str) {
  
  string::size_type pos = str.find (".",0);
  str = str.substr(0,pos);

}

void Clear_To_End_Of_Line () {
  cout << Escape_Character << "[K" << flush;
} 

void Clear_Screen () {
  cout << Escape_Character << "[2J" << flush;
} 

void Move_To_Location ( const int X, const int Y ) {
  char X_Str [ 5 ];
  char Y_Str [ 5 ];

  sprintf ( X_Str, "%d", X );
  sprintf ( Y_Str, "%d", Y );

  cout << Escape_Character << '[' << X_Str << ';' << Y_Str << 'H' << flush;

} 

void displayTime(const bool debug, const char* str)
{
   if (PetscGlobalRank != 0)
      return;

  PetscTime(&EndTime);
  CalculationTime = EndTime - StartTime; 
   long remainder = (long) CalculationTime;

   long secs = remainder % 60;
   remainder -= secs;
   long mins = (remainder / 60) % 60;
   remainder -= mins * 60;
   long hrs = remainder / (60 * 60);

   if (0)
   {
      if (X_loc == 1)
         Clear_Screen ();
    Move_To_Location ( X_loc, 1 );
    Clear_To_End_Of_Line ();
    X_loc += 1;
    Move_To_Location ( X_loc, 1 );
    X_loc += 3;
  }
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
  cout << str << hrs << " Hrs. " << mins << " Mins. " << secs << " Sec." << endl;
  cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
}

void displayTime(const string & str, PetscLogDouble StartTime, double * timeToDisplay )
{
   PetscLogDouble EndTime;

   PetscTime(&EndTime);
   double CalculationTime = ( timeToDisplay ? * timeToDisplay : EndTime - StartTime ); 
   long remainder = (long) CalculationTime;
   
   long secs = remainder % 60;
   remainder -= secs;
   long mins = (remainder / 60) % 60;
   remainder -= mins * 60;
   long hrs = remainder / (60 * 60);

   std::stringstream sstart;
   std::stringstream send;

   if( timeToDisplay != 0 ) {
      sstart << " Time: " << * timeToDisplay << " (";
      send   << ")";
   } else {
      sstart << "";
      send   << "";
   } 
   cout << str << sstart.str() << hrs << " Hrs. " << mins << " Mins. " << secs << " Sec." << send.str() << endl;

}

void getElapsedTime(char* str)
{
   if (PetscGlobalRank != 0)
      return;

   PetscTime(&EndTime);
   CalculationTime = EndTime - StartTime; 
   long remainder = (long) CalculationTime;

   long secs = remainder % 60;
   remainder -= secs;
   long mins = (remainder / 60) % 60;
   remainder -= mins * 60;
   long hrs = remainder / (60 * 60);

   sprintf (str, "%2.2ld:%2.2ld:%2.2ld", hrs, mins, secs);
}

void displayProgress(const bool debug, 
		     const char* str,
                     const double& Age) {

  if ( PetscGlobalRank != 0 ) return;

  if ( 0 ) {
    if (X_loc == 1) Clear_Screen();
    Move_To_Location ( X_loc, 1 );
    Clear_To_End_Of_Line ();
  }

  #if defined (sgi)
     unsigned long Old_Flags = cout.setf ( ios::fixed );
  #else
     ios::fmtflags Old_Flags = cout.setf ( ios::fixed );
  #endif

  int Old_Precision = cout.precision ( 6 );
  cout << str << "Age: " << setw( 7 ) << Age << " (Ma) " << endl;
  cout.setf ( Old_Flags );
  cout.precision ( Old_Precision );

}

char* currentTimeStr () {

   time_t timer;

   time ( &timer );

   return ctime ( & timer );
}

std::string currentTimeStr2 () {

   std::stringstream buffer;

   time_t timer;
   tm* separatedTime;

   time ( &timer );

   separatedTime = localtime ( &timer );

   switch ( separatedTime->tm_wday ) {

     case 0:
        buffer << "Sun";
        break;

     case 1:
        buffer << "Mon";
        break;

     case 2:
        buffer << "Tue";
        break;

     case 3:
        buffer << "Wed";
        break;

     case 4:
        buffer << "Thu";
        break;

     case 5:
        buffer << "Fri";
        break;

     case 6:
        buffer << "Sat";
        break;

   }

   buffer << ", ";

   if ( separatedTime->tm_mday < 10 ) {
      buffer << '0' << setw ( 1 ) << separatedTime->tm_mday;
   } else {
      buffer << setw ( 2 ) << separatedTime->tm_mday;
   }

   buffer << ' ';

   switch ( separatedTime->tm_mon ) {

     case 0:
        buffer << "Jan";
        break;

     case 1:
        buffer << "Feb";
        break;

     case 2:
        buffer << "Mar";
        break;

     case 3:
        buffer << "Apr";
        break;

     case 4:
        buffer << "May";
        break;

     case 5:
        buffer << "Jun";
        break;

     case 6:
        buffer << "Jul";
        break;

     case 7:
        buffer << "Aug";
        break;

     case 8:
        buffer << "Sep";
        break;

     case 9:
        buffer << "Oct";
        break;

     case 10:
        buffer << "Nov";
        break;

     case 11:
        buffer << "Dec";
        break;

   }

   buffer << ' ';

   if ( separatedTime->tm_hour < 10 ) {
      buffer << '0' << setw ( 1 ) << separatedTime->tm_hour;
   } else {
      buffer << setw ( 2 ) << separatedTime->tm_hour;
   }

   buffer << ':';

   if ( separatedTime->tm_min < 10 ) {
      buffer << '0' << setw ( 1 ) << separatedTime->tm_min;
   } else {
      buffer << setw ( 2 ) << separatedTime->tm_min;
   }

   buffer << ':';

   if ( separatedTime->tm_sec < 10 ) {
      buffer << '0' << setw ( 1 ) << separatedTime->tm_sec;
   } else {
      buffer << setw ( 2 ) << separatedTime->tm_sec;
   }

   return buffer.str ();
}



void Display_Temperature_Solver_Progress( const double Age, 
					  const double Time_Step,
                                          const bool   newLine )
{

  if ( PetscGlobalRank != 0 ) return;

  char time[124];
  getElapsedTime (time);

  ostringstream buf;
  buf.precision(4);
  buf.setf(ios::fixed);

  buf << "o Solving Temperature... Age: " << setw( 8 ) << Age << " (Ma) - TimeStep: " 
      << setw( 8 ) << Time_Step << " (Ma) " << "Elapsed: " << time;

  cout << buf.str();
  if ( newLine ) {
     cout << endl;
  }
  cout << flush;
}

void Display_Pressure_Solver_Progress( const int    Iteration_Number,
				       const int    Maximum_Number_Iterations,
				       const double Age, 
				       const double Time_Step,
                                       const bool   newLine )
{

  if ( PetscGlobalRank != 0 ) return;

  char time[124];
  getElapsedTime (time);

  ostringstream buf;
  buf.precision(4);
  buf.setf(ios::fixed);

  buf << "o Solving Pressure [" << setw( 2 ) << Iteration_Number << " / " << setw( 2 ) 
      << Maximum_Number_Iterations << "]... Age: " << setw( 8 ) << Age << " (Ma) - TimeStep: " 
      << setw( 8 ) << Time_Step << " (Ma) " << "Elapsed: " << time;

  cout << buf.str();
  if ( newLine ) {
     cout << endl;
  }
  cout << flush;

}

void Display_Coupled_Solver_Progress ( const int    Iteration_Number,
				       const int    Maximum_Number_Iterations,
				       const double Age, 
				       const double Time_Step,
                                       const bool   newLine ) {

  if ( PetscGlobalRank != 0 ) return;

  char time[124];
  getElapsedTime (time);

  ostringstream buf;
  buf.precision(4);
  buf.setf(ios::fixed);

  buf << "o Solving Coupled [" << setw( 2 ) << Iteration_Number << " / " << setw( 2 ) 
       << Maximum_Number_Iterations << "]... Age: " << setw( 8 ) << Age << " (Ma) - TimeStep: " 
       << setw( 8 ) << Time_Step << " (Ma) " << "Elapsed: " << time;

  cout << buf.str();
  if ( newLine ) {
     cout << endl;
  }
  cout << flush;


}

void displayComparisonProgress(int FileNumber, int TotalNumberOfFiles) {

  if (X_loc == 1) Clear_Screen();
  Move_To_Location ( X_loc, 1 );
  Clear_To_End_Of_Line ();
  cout << setw(7) << FileNumber << " of " << TotalNumberOfFiles 
       << " datafiles compared" << endl;
}


void setSolverTolerance (       KSP&  linearSolver,
			  const double newTolerance ) {

  PetscReal relativeTolerance;
  PetscReal absoluteTolerance;
  PetscReal divergenceTolerance;
  int       maximumIterations;
//   KSP       solverKSP;

//   SLESGetKSP ( linearSolver, &solverKSP );

  //
  // First get current default tolerances
  //
  KSPGetTolerances ( linearSolver, 
                     &relativeTolerance, 
                     &absoluteTolerance, 
                     &divergenceTolerance, 
                     &maximumIterations );

  //
  // Now set the Relative Tolerance to the new tolerance
  //
  KSPSetTolerances ( linearSolver,
                     newTolerance,
                     absoluteTolerance,
                     divergenceTolerance,
                     maximumIterations );

}

void setSolverMaxIterations (       KSP&  linearSolver,
                              const int    newMaxIts ) {

  PetscReal relativeTolerance;
  PetscReal absoluteTolerance;
  PetscReal divergenceTolerance;
  int       maximumIterations;
//   KSP       solverKSP;

//   SLESGetKSP ( linearSolver, &solverKSP );

  //
  // First get current default tolerances
  //
  KSPGetTolerances ( linearSolver, 
                     &relativeTolerance, 
                     &absoluteTolerance, 
                     &divergenceTolerance, 
                     &maximumIterations );

  //
  // Now set the Relative Tolerance to the new tolerance
  //
  KSPSetTolerances ( linearSolver,
                     relativeTolerance,
                     absoluteTolerance,
                     divergenceTolerance,
                     newMaxIts );

}

void setLinearSolverType (       KSP&        linearSolver,
                           const std::string& solverName ) {

   KSPSetType ( linearSolver, const_cast<char*>(solverName.c_str ()));

   // check the command line for any additional linear solver parameters.
   // This may also disregard the linear solver type that is indicated in the parameter list.
   KSPSetFromOptions ( linearSolver );
}


void setPreconditionerFillLevels (      KSP&  linearSolver,
                                  const int    fillLevel ) {


  PC pc;
  PC subpc;
  const char* pcType;

  KSPGetPC ( linearSolver, &pc );
  PCGetType ( pc, &pcType );

  if ( string ( pcType ) == PCBJACOBI ) {
     KSP* allSubSles;
     int numberOfBlocks;
     int whichBlock;

     PCBJacobiGetSubKSP (pc, &numberOfBlocks, &whichBlock, &allSubSles );

     KSPGetPC ( allSubSles[0], &subpc );

     PCFactorSetLevels  ( subpc, fillLevel );
//      PCILUSetLevels  ( subpc, fillLevel );
     PCGetType ( subpc, &pcType );
  }


//    PC preconditioner;
//    PC subPc;

//    char* pcType;

//    SLESGetPC ( linearSolver, &preconditioner );
//    PCCompositeGetPC (preconditioner,0,&subPc);

//    PCGetType ( subPc, &pcType );

//    PetscPrintf ( PETSC_COMM_WORLD, " sub-pc type %s\n", pcType );

//    PCILUSetFill ( subPc, fillLevel );



   // check the command line for any additional linear solver parameters.
   // This may also disregard the linear solver type that is indicated in the parameter list.
//    SLESSetFromOptions ( linearSolver );
}


int getSolverMaxIterations ( KSP& linearSolver ) {

  PetscReal relativeTolerance;
  PetscReal absoluteTolerance;
  PetscReal divergenceTolerance;
  int       maximumIterations;

  KSPGetTolerances ( linearSolver, 
                     &relativeTolerance, 
                     &absoluteTolerance, 
                     &divergenceTolerance, 
                     &maximumIterations );

  return maximumIterations;
}

const std::string& getKspConvergedReasonImage ( const KSPConvergedReason reason ) {

   static std::map<KSPConvergedReason, std::string> stringReasons;
   static std::string unknownReason = "Unknown Reason";
   static bool stringsInitialised = false;

   if ( not stringsInitialised ) {
      stringReasons [ KSP_CONVERGED_RTOL_NORMAL ] = "KSP_CONVERGED_RTOL_NORMAL";
      stringReasons [ KSP_CONVERGED_ATOL_NORMAL ] = "KSP_CONVERGED_ATOL_NORMAL";
      stringReasons [ KSP_CONVERGED_RTOL ] = "KSP_CONVERGED_RTOL";
      stringReasons [ KSP_CONVERGED_ATOL ] = "KSP_CONVERGED_ATOL";
      stringReasons [ KSP_CONVERGED_ITS ] = "KSP_CONVERGED_ITS";
      stringReasons [ KSP_CONVERGED_CG_NEG_CURVE ] = "KSP_CONVERGED_CG_NEG_CURVE";
      stringReasons [ KSP_CONVERGED_CG_CONSTRAINED ] = "KSP_CONVERGED_CG_CONSTRAINED";
      stringReasons [ KSP_CONVERGED_STEP_LENGTH ] = "KSP_CONVERGED_STEP_LENGTH";
      stringReasons [ KSP_CONVERGED_HAPPY_BREAKDOWN ] = "KSP_CONVERGED_HAPPY_BREAKDOWN";
      stringReasons [ KSP_DIVERGED_NULL ] = "KSP_DIVERGED_NULL";
      stringReasons [ KSP_DIVERGED_ITS ] = "KSP_DIVERGED_ITS";
      stringReasons [ KSP_DIVERGED_DTOL ] = "KSP_DIVERGED_DTOL";
      stringReasons [ KSP_DIVERGED_BREAKDOWN ] = "KSP_DIVERGED_BREAKDOWN";
      stringReasons [ KSP_DIVERGED_BREAKDOWN_BICG ] = "KSP_DIVERGED_BREAKDOWN_BICG";
      stringReasons [ KSP_DIVERGED_NONSYMMETRIC ] = "KSP_DIVERGED_NONSYMMETRIC";
      stringReasons [ KSP_DIVERGED_INDEFINITE_PC ] = "KSP_DIVERGED_INDEFINITE_PC";
      stringReasons [ KSP_DIVERGED_NANORINF ] = "KSP_DIVERGED_NANORINF";
      stringReasons [ KSP_DIVERGED_INDEFINITE_MAT ] = "KSP_DIVERGED_INDEFINITE_MAT";
      stringReasons [ KSP_CONVERGED_ITERATING ] = "KSP_CONVERGED_ITERATING";
      stringsInitialised = true;
   }

   std::map<KSPConvergedReason, std::string>::const_iterator imageItem = stringReasons.find ( reason );

   if ( imageItem != stringReasons.end ()) {
      return imageItem->second;
   } else {
      return unknownReason;
   }

}

void getDateAndTime(char* str) {

  time_t rawtime;
  struct tm* timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  strftime (str, 512, "%d-%b-%Y %H:%M:%S", timeinfo);
  
}

string DoubleToString(const double D) {

  char D_char[32];

  sprintf(D_char,"%f",D);

  return string(D_char);

}

string IntegerToString(const int I) {

  char I_char[32];

  sprintf(I_char,"%i",I);

  return string(I_char);

}

void Copy_String_To_Char( const string& str, char* char_array, const int char_length )
{

  PETSC_ASSERT( str.length() < string::size_type ( char_length ));

  string::size_type pos;

  for ( pos = 0; pos < str.length(); pos++) {
      char_array[ pos ] = str[ pos ];
  }

  for ( pos = str.length(); pos < string::size_type ( char_length ); pos++) {
      char_array[ pos ] = '\0';
  }

}

int Convert_String_Argument_For_FORTRAN_Code ( int* Integer_Array, const char* Input_String )
{
  int I;
  int Result = 0;              
  int Length = strlen ( Input_String );

  /* Check the input string is not too long for Fortran format */
  if ( Length > 256 )
  {
    Result = 1;               
    Length = -1;
  }

  for ( I = 0; I < 256; I++ )
  {
    if ( I <= Length )
    {
      Integer_Array [ I ] = Input_String [ I ];
    }
    else
    {
      Integer_Array [ I ] = ' ';
    }
  }

  return Result;

}

void StartTiming() {

  StartTime = 0.0;
  PetscTime(&StartTime);

}

void monitorProcessMemorySize() {

  double LocalSize = GetResidentSetSize(); //Mbytes
  double TotalSize;
  // PetscGlobalSum(&LocalSize,&TotalSize,PETSC_COMM_WORLD);

  MPI_Allreduce( &LocalSize, &TotalSize, 1, MPIU_SCALAR, MPIU_SUM, PETSC_COMM_WORLD);

  PetscPrintf(PETSC_COMM_WORLD,"  ~~Process Memory~~\n");
  PetscSynchronizedPrintf(PETSC_COMM_WORLD,"    [%d] %f\n",PetscGlobalRank,LocalSize);
  PetscSynchronizedFlush(PETSC_COMM_WORLD);
  if ( PetscGlobalSize > 1 ) PetscPrintf(PETSC_COMM_WORLD,"        %f\n",TotalSize);
  PetscPrintf(PETSC_COMM_WORLD,"  ~~~~~~~~~~~~~~~~~~\n");

}

void Get_Coefficients_From_String ( const char*   Coefficient_String,
                                    const int     Number_Of_Coefficients,
				    double*&      Coefficients ) {

  if ( Number_Of_Coefficients == 0 ) {
  
    Coefficients = (double*)(0);
    return;

  }

  int I;
  int Coefficient_String_Start = 0;

  for ( I = 0; I < Number_Of_Coefficients; I++ ) {

    /* find the position of the first character in the Coefficient_String */

    while ( Coefficient_String [ Coefficient_String_Start ] == ' ' ) {
      Coefficient_String_Start = Coefficient_String_Start + 1;
    }

    /* convert string to double */
    //
    Coefficients [ I ] = atof ( &Coefficient_String [ Coefficient_String_Start ] );

    /* find the next non-blank position in the Coefficient_String */

    while (( Coefficient_String [ Coefficient_String_Start ] != 0 ) &&
           ( Coefficient_String [ Coefficient_String_Start ] != ' ' ) ) {
      Coefficient_String_Start = Coefficient_String_Start + 1;
    }

  }

}

void Get_Coordinates_From_Filename_String ( const char*   Filename_String,
					    double &      X_Coordinate,
					    double &      Y_Coordinate ) 
{

  int Filename_String_Start = 0;

  /* find the first blank space in the Filename_String */
  //
  while ( Filename_String [ Filename_String_Start ] != ' ' ) 
  {
    Filename_String_Start = Filename_String_Start + 1;
  }

  /* convert string to double */
  //
  X_Coordinate = atof ( &Filename_String [ Filename_String_Start ] );

  /* find the comma position in the Filename_String */
  //
  while ( Filename_String [ Filename_String_Start ] != ',' )  
  {
    Filename_String_Start = Filename_String_Start + 1;
  }

  /* convert string to double */
  //
  Filename_String_Start = Filename_String_Start + 1;
  Y_Coordinate = atof ( &Filename_String [ Filename_String_Start ] );
  
}

bool File_Exists ( const char* File_Name ) {

  struct stat File_Stats;
  int Error;

  Error = stat ( File_Name, &File_Stats );

  return Error == 0;

} 

bool File_Exists ( const std::string& fileName ) {
  return File_Exists ( fileName.c_str ());
}


bool removeFile ( const std::string& fileName ) {
  return remove ( fileName.c_str ()) == 0;
}


std::string toLower ( const std::string& str ) {

    std::string strCpy( str );

    std::transform ( strCpy.begin(), strCpy.end(), strCpy.begin(), ::tolower );
    return strCpy;
}

double Double_Epsilon () {

  static bool   First_Time = true;
  static double Model_Epsilon_Value;

  if ( First_Time ) {
    
    double T     = 1.0;
    double Delta = 0.5;
    double TT    = T + Delta;
    
    while ( T != TT ) {
      Model_Epsilon_Value = Delta;
      T = TT;
      Delta = 0.5 * Delta;
      TT = T + Delta;
    } 

    Model_Epsilon_Value = 4.0 * Model_Epsilon_Value;

    First_Time = false;
  } 

  return Model_Epsilon_Value;

}


double Float_Epsilon () {

  static bool  First_Time = true;
  static float Model_Epsilon_Value;

  if ( First_Time ) {
    
    double T     = 1.0;
    double Delta = 0.5;
    double TT    = T + Delta;
    
    while ( T != TT ) {
      Model_Epsilon_Value = Delta;
      T = TT;
      Delta = 0.5 * Delta;
      TT = T + Delta;
    } 

    Model_Epsilon_Value = 4.0 * Model_Epsilon_Value;

    First_Time = false;
  } 

  return Model_Epsilon_Value;

}

bool broadcastAnyTrueBooleanValue ( const bool localValue ) {

  int localValueInt;
  int globalValueInt;

  if ( localValue ) {
    localValueInt = 1;
  } else {
    localValueInt = 0;
  }

  MPI_Allreduce( &localValueInt, &globalValueInt, 1, 
                 MPI_INT, MPI_MAX, PETSC_COMM_WORLD );

  return ( globalValueInt > 0 );
}

bool successfulExecution ( const bool localSuccessfulExecution ) {

  int localSuccessfulExecutionInt;
  int globalSuccessfulExecutionInt;

  if ( localSuccessfulExecution ) {
    localSuccessfulExecutionInt = 1;
  } else {
    localSuccessfulExecutionInt = 0;
  }

  MPI_Allreduce( &localSuccessfulExecutionInt, &globalSuccessfulExecutionInt, 1, 
                 MPI_INT, MPI_MIN, PETSC_COMM_WORLD );

  return ( globalSuccessfulExecutionInt > 0 );
}


double globalMinimum ( const double value ) {

   double localValue = value;
   double globalValue;

   
   MPI_Allreduce ( &localValue, &globalValue, 1, 
                   MPI_DOUBLE, MPI_MIN, PETSC_COMM_WORLD );

   return globalValue;
}


double globalMaximum ( const double value ) {

   double localValue = value;
   double globalValue;

   
   MPI_Allreduce ( &localValue, &globalValue, 1, 
                   MPI_DOUBLE, MPI_MAX, PETSC_COMM_WORLD );

   return globalValue;
}



template<>
bool MpiMaximum<bool> ( const bool& operand ) {

   int localOperand = ( operand ? 1 : 0 );
   int result;

   MPI_Allreduce ( &localOperand, &result, 1, 
                   MpiType<int>::type, MPI_MAX, PETSC_COMM_WORLD );

   return result == 1;
}

template<>
bool MpiMinimum<bool> ( const bool& operand ) {

   int localOperand = ( operand ? 1 : 0 );
   int result;

   MPI_Allreduce ( &localOperand, &result, 1, 
                   MpiType<int>::type, MPI_MIN, PETSC_COMM_WORLD );

   return result == 1;
}
