/*BHEADER**********************************************************************
 * Copyright (c) 2008,  Lawrence Livermore National Security, LLC.
 * Produced at the Lawrence Livermore National Laboratory.
 * This file is part of HYPRE.  See file COPYRIGHT for details.
 *
 * HYPRE is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License (as published by the Free
 * Software Foundation) version 2.1 dated February 1999.
 *
 * $Revision: 1.1 $
 ***********************************************************************EHEADER*/

#define HYPRE_RELEASE_NAME hypre
#define HYPRE_RELEASE_VERSION 2.9.0b
#define HYPRE_RELEASE_DATE 2012/10/30
#define HYPRE_RELEASE_TIME 00:00:00
#define HYPRE_RELEASE_BUGS hypre-support@llnl.gov

/* Use long long int for HYPRE_Int */
/* #undef HYPRE_BIGINT */

/* Compile without MPI */
/* #undef HYPRE_SEQUENTIAL */

/* Use HYPRE timing routines */
/* #undef HYPRE_TIMING */

/* Use internal BLAS library */
/* #undef HYPRE_USING_HYPRE_BLAS */

/* Use internal LAPACK library */
/* #undef HYPRE_USING_HYPRE_LAPACK */

/* Use assumed partition */
/* #undef HYPRE_NO_GLOBAL_PARTITION */

/* Print HYPRE errors */
/* #undef HYPRE_PRINT_ERRORS */

/* Use OpenMP */
#define HYPRE_USING_OPENMP

/* #undef HYPRE_HAVE_MPI */
/* #undef HYPRE_HAVE_MPI_COMM_F2C */

/* Define as follows to set the Fortran name mangling scheme:
 * 0 = unspecified
 * 1 = no underscores
 * 2 = one underscore
 * 3 = two underscores
 * 4 = caps, no underscores */
#define HYPRE_FMANGLE 0

/* Define as in HYPRE_FMANGLE to set the BLAS name mangling scheme */
#define HYPRE_FMANGLE_BLAS 0

/* Define as in HYPRE_FMANGLE to set the LAPACK name mangling scheme */
#define HYPRE_FMANGLE_LAPACK 0

/* Define to a macro mangling the given C identifier (in lower and upper
 * case), which must not contain underscores, for linking with Fortran. */
#define HYPRE_F77_FUNC(name,NAME) name ## _

/* As F77_FUNC, but for C identifiers containing underscores. */
#define HYPRE_F77_FUNC_(name,NAME) name ## __
