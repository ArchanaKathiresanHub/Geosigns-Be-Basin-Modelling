// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_NUMERICUTILS_H
#define SUMLIB_NUMERICUTILS_H

#include "BaseTypes.h"
#include "SUMlib.h"

#include <vector>
#include <cmath>
#include <cstddef>


namespace SUMlib {

static const double MinStdDevEpsilon = 1e-3;

INTERFACE_SUMLIB_DEBUG
double MachineEpsilon() ;

/**
 * Returns true iff the two doubles are (approximately) equal to each other.
 */
INTERFACE_SUMLIB_DEBUG
bool IsEqualTo( double d1, double d2 );

/**
 * Perform singular value decomposition for the given matrix.
 *
 * Decompose the matrix a[r,c] into u.w.tr(v) where
 * u[m,n]  ordered as a[i,j] = a[i*np+j]
 * w[n,n]  diagonal matrix as array w[n]
 * v[n,n]  square array with v.tr(v) = I
 *
 * assumes m >= n
 *
 * Based on "Numerical Recipes in C: the art of scientific computing",
 *         Second Edition
 *         by W.H. Press et al., Cambridge Univ. Press, 1992
 *
 * @param[in,out] a on input: matrix to decompose, on output: unitary matrix
 * @param[out]    w Array representing diagonal matrix
 * @param[out]    v unitary matrix
 *
 * @return zero on success, 1 on failure
 */
INTERFACE_SUMLIB_DEBUG
int svdcmp( std::vector<std::vector<double> > &a,
            std::vector<double> &w,
            std::vector<std::vector<double> > &v );

/**
 * Backsubstitution of a linear set of equations for which singular value
 * decomposition has been performed.
 *
 * @param[in]     u    The u==a matrix from the decomposition.
 * @param[in]     w    The diagonal matrix from the decomposition.
 * @param[in]     v    The v matrix from the decomposition.
 * @param[in]     b    The right hand side of the linear equation set.
 * @param[in,out] x    The rms solution of the equation set.
 * @param[in]     minw The minimum value allowed for the elements of w.
 */
INTERFACE_SUMLIB_DEBUG
void svbksb( const std::vector<std::vector<double> >& u,
             const std::vector<double>& w,
             const std::vector<std::vector<double> >& v,
             const std::vector<double>& b,
             std::vector<double>& x,
             const double minw );

/**
 * Calculates the variances matrix.
 *
 * @param w The diagonal matrix from the sv decomposition.
 * @param v The v matrix from the sv decomposition.
 * @param vcm The resulting variance matrix.
 */
INTERFACE_SUMLIB_DEBUG
void svcovar( const std::vector<double>& w,
              const std::vector<std::vector<double> >& v,
              std::vector<std::vector<double> >& vcm );

/// Calculates the (pseudo) inverse of matrix a.
/// Assumes a is square.
/// @param[in,out] a on input: matrix to invert, on output: inv(a)
INTERFACE_SUMLIB_DEBUG
void svinv( std::vector<std::vector<double> > &a );

/**
 *   Returns index such that
 *      xt[index] <= xs < xt[index+1] if xt[0] <= xs <= xt[xt.size()-1]
 *      or index = 0 if xs < xt[0]
 *      or index = xt.size()-2 if xs > xt[xt.size()-1]
 *
 *   Requires xt[] values are monotonically increasing.
 * Requires xt.size() >= 2
 *
 * @param xs Value whose index is to e determined.
 * @param xt @todo Document
 *
 * @return @todo Document
 */
INTERFACE_SUMLIB_DEBUG
size_t bisection( const double xs,
                  const std::vector<double> &xt );

/**
* Performs linear interpolation into a table {xt[],yt[]} for
* the value xs. For values outside the range [xt[0],xt[n]] either
* truncation to the bounding values or extrapolation can be used.
* @param xs The interpolation argument.
* @param xt An array of x values.
* @param yt An array of y values corresponding to xt.
* @param extrapolateMin True to extrapolate values less than xt[0].
* @param extrapolateMax True to extrapolate values greater than xt[n].
* @return the interpolated value from yt corresponding to the location
*    of xs in xt.
*/
INTERFACE_SUMLIB_DEBUG
double interpolate( const double xs,
                    const std::vector<double> &xt,
                    const std::vector<double> &yt,
                    bool extrapolateMin, bool extrapolateMax );

/**
* The logarithm of the normal Gamma function.
* @param x The function argument.
* @return The Logarithm of the Gamma function.
*/

INTERFACE_SUMLIB_DEBUG
double logGamma( const double x );
INTERFACE_SUMLIB_DEBUG
double logProbGamma( double x, double m, double var );

/**
* The incomplete gamma function. This function is useful to
* calculate chi-square cumulative distribution values, e.g. for
* the goodness-of-fit determination
* @param a The incomplete parameter value.
* @param x The argument value
* @param gamser Pointer to real receiving the incomplete gamma value.
*/
INTERFACE_SUMLIB_DEBUG
int gammp( double a, double x, double *gamser );

/**
* The standard error function.
* @param x The function argument.
* @return The standard error function.
*/
INTERFACE_SUMLIB_DEBUG
double erf( double x );
/**
* The cumulative normal distribution function.
* @param x The function argument.
* @return cumulative normal distribution function.
*/
INTERFACE_SUMLIB_DEBUG
double cnp( double x );

INTERFACE_SUMLIB_DEBUG
double logProbNormal( double x, double m, double var, double *mn, double *mx );

/**
* Calculate the mean and variance of a truncated normal distribution.
* @param mu The mean value if the distribution was not truncated.
* @param sd The standard deviation if the distribution was not truncated.
* @param mn The lower argument bound for the truncation.
* @param mx The upper argument bound for the truncation.
* @param trmu The mean value of the truncated distribution.
* @param trsd The standard deviation of the truncated distribution.
*/
INTERFACE_SUMLIB_DEBUG
void trNormalProp( double mu, double sd,
                   double mn, double mx,
                   double& trmu, double& trsd );

/**
* Performs LU decomposition of a square, invertable matrix.
* @param a On input the matrix to be decomposed. On output this
*    matrix contains the decomposition.
* @param indx The pivot indices belonging to the decomposition.
* @return Error indicator, 0 means no error.
*/
INTERFACE_SUMLIB_DEBUG
int ludcmp( std::vector<std::vector<double> >& a,
            std::vector<int>& indx );

/**
* Solve a linear sqt of equations using the LU decomposition.
* @param a Matrix containing the LU decomposition.
* @param indx The pivot indices of the decomposition.
* @param b On input the right hand side of the linear system, on
*   output the solution vector.
*/
INTERFACE_SUMLIB_DEBUG
void lubksb( const std::vector<std::vector<double> >& a,
             const std::vector<int>& indx,
             std::vector<double>& b );

/**
* Calculates the determinant of a square matrix.
* @param a The matrix of which the determinant will be calculated.
* @return The determinant of the matrix a.
*/
INTERFACE_SUMLIB_DEBUG
double det( const std::vector<std::vector<double> >& a );

/// Calculates the mean squared error, i.e. the normalized sum of
/// squared differences between two vectors.
/// Assumes v1 and v2 have equal size.
/// @param [in] v1 first vector of values
/// @param [in] v2 second vector of values
/// @returns the averaged sum of squared differences, or zero if v1 is empty
INTERFACE_SUMLIB_DEBUG
double MeanSquaredError( RealVector const& v1, RealVector const& v2 );

/// Calculates the distance between 2 vectors v1 and v2, i.e.
/// the length (L2 norm) of v2 - v1.
/// Assumes v1 and v2 have equal size.
/// @param [in] v1 first vector of values
/// @param [in] v2 second vector of values
/// @returns the length of v2 - v1, or zero if v1 is empty
INTERFACE_SUMLIB_DEBUG
double LengthOfDiffVector( RealVector const& v1, RealVector const& v2 );

/// Calculates a Kriging distance between 2 vectors v1 and v2,
/// where the first l "continuous" vector elements are treated different as
/// the last v1.size() - l binary values.
/// Assumes v1 and v2 have equal size.
/// @param [in] v1 first vector of values
/// @param [in] v2 second vector of values
/// @param [in] l  size <= v1.size()
/// @returns Kriging distance
INTERFACE_SUMLIB_DEBUG
double KrigingDistance( RealVector const& v1, RealVector const& v2, unsigned int l );

/// Calculate the mean over all values of a vector
/// @param [in] v the values to calculate the mean for
/// @returns the mean value
INTERFACE_SUMLIB_DEBUG
double VectorMean( RealVector const& v );

/// Calculate the standard deviation over all values of a vector
/// @param [in] v    the values to calculate the standard deviation for
/// @param [in] mean the mean value
/// @returns the mean value
INTERFACE_SUMLIB_DEBUG
double VectorStdDev( RealVector const& v, double mean );

/// Calculate the mean and standard deviation over all values of a vector
/// Returns whether the distribution of values is not degenerated
/// @param [in] v the values to calculate the mean and standard deviation for
/// @param [out] mean the mean value
/// @param [out] stddev the standard deviation
/// @returns true if stddev > tol * ( 1 + fabs(mean) )
INTERFACE_SUMLIB_DEBUG
bool VectorMeanAndStdDev( RealVector const& v, double& mean, double& stddev );

/// Scale vector v to v[i] = ( v[i] - mean )/stddev
/// In the degenerate case, use stddev == 1
/// Returns whether the distribution of values in v is not degenerate
/// @param [in,out] v the vector to scale
/// @param [out] mean the mean value
/// @param [out] stddev the standard deviation
/// @returns true if stddev > tol * ( 1 +fabs(mean) )
INTERFACE_SUMLIB_DEBUG
bool VectorScaleRobust( RealVector & v, double & mean, double & stddev );

/// Scale vector v to v[i] = v[i] - mean
/// @param [in,out] v the vector to scale
/// @param [out] mean the mean value
INTERFACE_SUMLIB_DEBUG
void VectorScaleToMean( RealVector & v, double & mean );

/// Scale each column v_j of matrix m to v_j[i] = ( v[i] - mean_j )/stddev_j
/// In the degenerate case, use stddev == 1
/// mean and stddev are cleared and resized to the number of columns of m
/// Returns whether the distribution of values in v is not degenerate
/// @param [in,out] m the matrix to scale
/// @param [out] mean the mean value for each column of m
/// @param [out] stddev the standard deviation for each column of m
/// @returns true if stddev > tol * ( 1 +fabs(mean) ) for all columns of m
INTERFACE_SUMLIB_DEBUG
bool MatrixScaleRobust( RealMatrix & m, RealVector & mean, RealVector & stddev );

/// Scale each column v_j of matrix m to v_j[i] = v[i] - mean_j
/// mean is cleared and resized to the number of columns of m
/// @param [in,out] m the matrix to scale
/// @param [out] mean the mean value for each column of m
INTERFACE_SUMLIB_DEBUG
void MatrixScaleToMean( RealMatrix & m, RealVector & mean );

/// Calculate the mean value for each column of matrix m
/// mean is cleared and resized
/// @param [in] m matrix
/// @param [out] mean vector of mean values for the columns of m
INTERFACE_SUMLIB_DEBUG
void MatrixColumnMean( RealMatrix const& m, RealVector &mean );

/// Calculate the mean and standard deviation for each column of matrix m
/// mean is cleared and resized
/// @param [in]  m matrix
/// @param [out] mean vector of mean values for the columns of m
/// @param [out] stddev vector of standard deviation values for the columns of m
INTERFACE_SUMLIB_DEBUG
void MatrixColumnMeanAndStdDev( RealMatrix const& m, RealVector & mean, RealVector & stddev );

/// multiply a matrix with a vector w = m * v
/// w is cleared and resized to the number of columns of m.
/// @param [in] m the matrix to multiply
/// @param [in] v the vector to multiply
/// @param [out] w the result vector
INTERFACE_SUMLIB_DEBUG
void MatrixVectorProduct( RealMatrix const& m, RealVector const& v, RealVector & w );

/// calculates the L2 norm of a vector
/// i.e. the square root of sum of squares of all elements
/// @param [in] v vector to calculate the L2 norm for
/// @returns the calculated value (zero for an empty vector)
INTERFACE_SUMLIB_DEBUG
double VectorL2Norm ( RealVector const& v );

/// calculates range[k] := max[k] - min[k]
/// @param [in] min minimum values
/// @param [in] max maximum values (max[k] >= min[k])
/// @param [out] range difference between max and min for all elements
INTERFACE_SUMLIB_DEBUG
void CalcRange( RealVector const& min, RealVector const& max, RealVector &range );

/// calculates minimum standard deviation values
/// @param [in] min minimum values
/// @param [in] max maximum values (max[k] >= min[k])
/// @param [out] minimum std. dev. values
/// @param [in] fraction of max-min range to use for minStdDev
INTERFACE_SUMLIB_DEBUG
void CalcMinStdDev(
      RealVector const&       min,
      RealVector const&       max,
      RealVector              &minStdDev,
      double minStdDevEps =   MinStdDevEpsilon );

/// Calculates the average for each column. The result vector is resized to the number of entries in the first row.
/// @param [in]  m      matrix to calculate column averages for
/// @param [out] avg    vector of column averages
INTERFACE_SUMLIB_DEBUG
void CalcAverages( RealMatrix const& m, std::vector<double>& avg );

/// Calculates the covariances matrix. The result matrix is resized to the number of entries in the first row.
/// @param [in]  m      matrix to calculate covariances for
/// @param [in]  avg    vector of averages
/// @param [out] covmat matrix of covariances
INTERFACE_SUMLIB_DEBUG
void CalcCovariances( std::vector<RealVector> const& m, RealVector const& avg, RealMatrix& covmat );

/// Calculates a roughly approximated critical value of a Student T distribution (two-tailed) for df
/// degrees of freedom at a confidence level of p % (i.e. at a significance level of 100 - p %)
/// In this function p is a percentage, 50 <= p <= 99.9.
/// @param [in]  df   degrees of freedom
/// @param [in]  p    percentage
/// @returns the critical value
INTERFACE_SUMLIB_DEBUG
double CriticalValue( unsigned int const& df, double const& p );

/// Calculate singular value decomposition (SVD) of original matrix a
/// The SVD calculation changes matrix a but not its dimensions: m rows and n columns
/// Requirement: m >= n
/// @param [in,out] a   matrix of proxy model monomials (in) -> orthonormal matrix (out)
/// @param [out] w      n x 1 vector of singular values
/// @param [out] v      n x n orthonormal matrix
/// @returns the status of the SVD operation (0 = success)
int calculateSVD( std::vector<std::vector<double> >& a,
                  std::vector<double>& w,
                  std::vector<std::vector<double> >& v );

/// Calculates a t-statistic: t-statistic = ( estimator - hypothesis ) / standardError
INTERFACE_SUMLIB_DEBUG
double calculateTStatistic( double estimator, double hypothesis, double standardError );

/// Calculates a t-statistic where the hypothesis is set to zero.
INTERFACE_SUMLIB_DEBUG
double calculateTStatistic( double estimator, double standardError );

} // namespace SUMlib

#endif // SUMLIB_NUMERICUTILS_H
