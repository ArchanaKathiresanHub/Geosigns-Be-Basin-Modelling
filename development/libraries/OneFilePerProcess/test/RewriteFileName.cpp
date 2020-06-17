#include "../src/RewriteFileName.h"


#include <gtest/gtest.h>


// rewriteFileName has 6 input parameters.
// pattern:
//   Typical equivalence classes: "Abc{NAME}def{MPI_RANK}ghi{MPI_SIZE}jkl"
//   Atpyical: NULL | "" | "Abc" | "{" | "Abc{def}g" | "{NAME}"
//
// fileName:
//   Typical: "./a path/Some_file.dat"
//   Atpyical: NULL | ""
//
// mpiRank:
//   Typical: Any non-negative integer
//   Atypical: A negative number
//
// mpiSize:
//   Typical: Any strict positive integer
//   Atypical: Any non-positive integer
//
// buffer:
//   Typical: Empty, Valid memory block of size bufferSize
//   Atypical: bufferSize > 0 && buffer == NULL

TEST( OneFilePerProcess, rewriteFileName)
{
   ::testing::FLAGS_gtest_death_test_style="threadsafe";

   char buffer[80];
   size_t retVal;
   // typical test cases
   // 1. output to empty buffer
   retVal = rewriteFileName( "Abc{NAME}def{MPI_RANK}ghi{MPI_SIZE}jkl", "./a path/Some_file.dat", 0, 1, NULL, 0);
   EXPECT_EQ( strlen( "Abc./a path/Some_file.datdef0ghi1jkl") + 1, retVal);

   // 2. output to non-empty buffer
   retVal = rewriteFileName( "Abc{MPI_RANK}def{MPI_RANK}ghi{MPI_SIZE}jkl{NAME}mno", "./other'spath/file.h5", 1, 2, buffer, sizeof(buffer));
   EXPECT_EQ( strlen( "Abc1def1ghi2jkl./other'spath/file.h5mno") + 1, retVal);
   EXPECT_STREQ( "Abc1def1ghi2jkl./other'spath/file.h5mno", buffer);

   // atypical test cases
   // 3. NULL pattern - empty buffer
   EXPECT_DEATH( rewriteFileName( NULL, "file.dat", 3, 4, NULL, 0), "Assertion.*failed" );

   // 4. NULL pattern - non-empty buffer
   EXPECT_DEATH( rewriteFileName( NULL, "file.dat", 3, 4, buffer, sizeof(buffer)), "Assertion.*failed" );

   // 5. Empty string pattern  - empty buffer
   retVal = rewriteFileName( "", "file.dat", 5, 6, NULL, 0);
   EXPECT_EQ( 1, retVal );

   // 6. Empty string pattern  - non-empty buffer
   retVal = rewriteFileName( "", "file.dat", 5, 6, buffer, sizeof(buffer));
   EXPECT_EQ( 1, retVal );
   EXPECT_STREQ( "", buffer);

   // 7. "abc" pattern  - empty buffer
   retVal = rewriteFileName( "abc", "file.dat", 12, 13, NULL, 0);
   EXPECT_EQ( 4, retVal );

   // 8. "abc" string pattern  - non-empty buffer
   retVal = rewriteFileName( "defg", "file.dat", 14, 15, buffer, sizeof(buffer));
   EXPECT_EQ( 5, retVal );
   EXPECT_STREQ( "defg", buffer);

   // 9. "{" string pattern  - empty buffer
   retVal = rewriteFileName( "{", "file.dat", 13, 16, NULL, 0);
   EXPECT_EQ( 2, retVal );

   // 10. "{" string pattern - non-empty buffer
   retVal = rewriteFileName( "{", "file.dat", 11, 16, buffer, sizeof(buffer));
   EXPECT_EQ( 2, retVal );
   EXPECT_STREQ( "{", buffer);

   // 11. "abc{def}g" string pattern  - empty buffer
   retVal = rewriteFileName( "abc{def}g", "file.dat", 10, 56, NULL, 0);
   EXPECT_EQ( 10, retVal );

   // 12. "abc{def}g" string pattern - non-empty buffer
   retVal = rewriteFileName( "abc{def}g", "file.dat", 8, 16, buffer, sizeof(buffer));
   EXPECT_EQ( 10, retVal );
   EXPECT_STREQ( "abc{def}g", buffer);

   // 12. "{NAME}" string pattern  - empty buffer
   retVal = rewriteFileName( "{NAME}", "file.dat", 10, 20, NULL, 0);
   EXPECT_EQ( 9, retVal );

   // 13. "{NAME}" string pattern - non-empty buffer
   retVal = rewriteFileName( "{NAME}", "file.dat", 11, 22, buffer, sizeof(buffer));
   EXPECT_EQ( 9, retVal );
   EXPECT_STREQ( "file.dat", buffer);

   // 14. NULL file name - empty buffer
   EXPECT_DEATH( rewriteFileName(  "Abc{NAME}def{MPI_RANK}ghi{MPI_SIZE}jkl", NULL, 7, 8, NULL, 0), "Assertion.*failed" );

   // 15. NULL file name - non-empty buffer
   EXPECT_DEATH( rewriteFileName(  "Abc{NAME}def{MPI_RANK}ghi{MPI_SIZE}jkl", NULL, 7, 8, buffer, sizeof(buffer)) , "Assertion.*failed");

   // 16. Empty file name - empty buffer
   retVal = rewriteFileName(  "{MPI_SIZE}abc{MPI_RANK}def{NAME}g", "", 9, 10, NULL, 0 );
   EXPECT_EQ( strlen("10abc9defg") + 1, retVal);

   // 17. Empty file fame - non-empty buffer
   retVal = rewriteFileName(  "{MPI_SIZE}abc{NAME}def{MPI_RANK}g", "", 11, 12, buffer, sizeof(buffer) );
   EXPECT_EQ( strlen("12abcdef11g" ) + 1, retVal);

   // 18. Negative MPI rank - empty buffer
   EXPECT_DEATH( rewriteFileName(  "123", "file.dat", -1, 12, NULL, 0 ), "Assertion.*failed");

   // 19. Negative MPI rank - non-empty buffer
   EXPECT_DEATH( rewriteFileName(  "123", "file.dat", -1, 12, buffer, sizeof(buffer) ), "Assertion.*failed");

   // 20. Negative MPI size - empty buffer
   EXPECT_DEATH( rewriteFileName(  "123", "file.dat", -1, 12, NULL, 0 ), "Assertion.*failed");

   // 21. Negative MPI size - non-empty buffer
   EXPECT_DEATH( rewriteFileName(  "123", "file.dat", 5, 0, buffer, sizeof(buffer) ), "Assertion.*failed");

   // 20. NULL pointer for buffer
   EXPECT_DEATH( rewriteFileName(  "123", "file.dat", 1, 12, NULL, 1 ), "Assertion.*failed");
}
