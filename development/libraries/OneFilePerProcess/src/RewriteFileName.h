#ifndef ONEFILEPERPROCESS_REWRITEFILENAME_H
#define ONEFILEPERPROCESS_REWRITEFILENAME_H

#include <stddef.h>

#ifdef __cplusplus
namespace OneFilePerProcess
{
extern "C" {
#endif


// Reformats a file name on the basis of a pattern and MPI rank and size. The
// resulting file name is equal to the given pattern with certain placeholders
// replaced by specific values. Therefore the pattern string may only contain
// characters that are legal in file names and placeholders of the form
// '{PLACEHOLDER}'. There are only three placeholders available:
// - '{NAME}'      The string given by fileName
// - '{MPI_SIZE}'  A string representation of the number of MPI processes
// - '{MPI_RANK}'  A string representation of the rank number of this MPI process.
// The generated file name is returned via generateFileName in a memory buffer
// that is 'malloc'ed. Therefore the result should be 'free'd again.
size_t rewriteFileName( const char * pattern, const char *  fileName, int mpiRank, int mpiSize, char * buffer, size_t bufferSize );


#ifdef __cplusplus
} // extern "C"
} // namespace OneFilePerProcess
#endif


#endif
