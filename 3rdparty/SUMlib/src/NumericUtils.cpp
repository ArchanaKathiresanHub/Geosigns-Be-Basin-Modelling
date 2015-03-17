// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

#include "Exception.h"
#include "NumericUtils.h"

using std::vector;

namespace SUMlib {

double MachineEpsilon()
{
   return std::numeric_limits<double>::epsilon();
}

bool IsEqualTo( double d1, double d2 )
{
   const double tolerance = 1e-14;
   return ( fabs( d1 - d2 ) <= tolerance * ( fabs( d1 ) + fabs( d2 ) ) );
}

int svdcmp( vector<vector<double> > &a,
      vector<double> &w,
      vector<vector<double> > &v )
{
   try
   {
      const int maxit = 30;
      const size_t mp = a.size();
      const size_t np = a[0].size();

      int flag;
      double h;
      double value;
      double y;
      double z;

      int l = 0;
      double g = 0;
      double scale = 0;
      double anorm = 0;

      // Resize the output matrices
      w.resize( np );
      v.resize( np );
      for ( size_t i = 0; i < np; ++i )
         v[i].resize( np );

      vector<double> rv1( np );

      for ( size_t i=0; i < np; i++ )
      {
         l = i + 1;
         rv1[i] = scale * g;
         g=0.0;
         double s = 0.0;
         scale=0.0;
         if ( i < mp )
         {
            for ( size_t k=i; k < mp; k++)
               scale += fabs( a[k][i] );

            if ( scale )
            {
               for ( size_t k=i; k < mp; k++ )
               {
                  a[k][i]/=scale;
                  s+=a[k][i]*a[k][i];
               }
               double f=a[i][i];
               if (f<0)
                  g = sqrt(s);
               else
                  g = -sqrt(s);
               h=f*g-s;
               a[i][i]=f-g;
               if ( i != np-1 )
               {
                  for ( size_t j=l; j<np; j++ )
                  {
                     s=0.0;
                     for ( size_t k=i; k < mp; k++)
                     {
                        s+=a[k][i]*a[k][j];
                     }
                     f=s/h;
                     for ( size_t k=i; k < mp; k++)
                     {
                        a[k][j] += f*a[k][i];
                     }
                  }
               }
               for ( size_t k=i; k < mp; k++)
                  a[k][i] *= scale;
            }
         }
         w[i]=scale*g;
         g = 0.0;
         s = 0.0;
         scale = 0.0;
         if ( i < mp && (i != np-1) )
         {
            for ( size_t k=l; k < np; k++ )
               scale += fabs(a[i][k]);

            if ( scale )
            {
               for ( size_t k=l; k < np; k++ )
               {
                  a[i][k] /= scale;
                  s += a[i][k] * a[i][k];
               }
               double f=a[i][l];
               if (f < 0)
                  g = sqrt(s);
               else
                  g = -sqrt(s);
               h = f*g-s;
               a[i][l] = f-g;
               for ( size_t k=l; k<np; k++)
                  rv1[k] = a[i][k]/h;

               if ( i != mp-1 )
               {
                  for ( size_t j=l; j < mp; j++ )
                  {
                     s=0.0;
                     for ( size_t k=l; k < np; k++ )
                        s += a[j][k]*a[i][k];
                     for ( size_t k=l; k < np; k++ )
                        a[j][k] += s*rv1[k];
                  }
               }

               for ( size_t k=l; k < np; k++ )
                  a[i][k] *= scale;
            }
         }

         value = fabs(w[i]) + fabs(rv1[i]);
         anorm = ( anorm > value ) ? anorm : value;
      }

      for ( int i = np-1; i >= 0; i-- )
      {
         if ( i < static_cast<int>(np)-1 )
         {
            if ( g )
            {
               for ( size_t j=l; j < np; j++)
                  v[j][i]=(a[i][j]/a[i][l])/g;

               for ( size_t j=l; j < np; j++)
               {
                  double s=0.0;
                  for ( size_t k=l; k < np; k++ )
                     s+=a[i][k]*v[k][j];

                  for ( size_t k=l; k < np; k++ )
                     v[k][j]+=s*v[k][i];
               }
            }
            for ( size_t j=l; j < np; j++ )
            {
               v[i][j]=0.0;
               v[j][i]=0.0;
            }
         }
         v[i][i] = 1.0;
         g=rv1[i];
         l=i;
      }

      // Accumulation of left hand transformations
      int nm = ( np < mp ) ? np : mp;
      for ( int i = np-1; i >= 0; i-- )
      {
         l=i+1;
         g=w[i];
         if ( i < static_cast<int>(np)-1 )
         {
            for ( size_t j=l; j<np; j++ )
               a[i][j]=0.0;
         }
         if ( g )
         {
            g = 1/g;
            if ( i != static_cast<int>(np)-1 )
            {
               for ( size_t j=l; j < np; j++ )
               {
                  double s=0.0;
                  for ( size_t k=l; k < mp; k++ )
                     s+=a[k][i]*a[k][j];

                  double f=(s/a[i][i])*g;

                  for ( size_t k=i; k < mp; k++ )
                     a[k][j]+=f*a[k][i];
               }
            }
            for ( size_t j=i; j < mp; j++ )
               a[j][i]*=g;
         }
         else
         {
            for ( size_t j=i; j < mp; j++ )
               a[j][i]=0.0;
         }
         a[i][i]+=1.0;
      }

      // Diagonalization of bi-diagonal form

      // Loop over singular values
      for ( int k=np-1; k >= 0; k-- )
      {
         // Iterations
         for ( int its=1; its <= maxit; its++ )
         {
            flag = 1; // For emulating goto
            for ( l=k; l>=0; l-- )
            {
               nm=l-1;
               if ((fabs(rv1[l])+anorm)==anorm)
               {
                  flag   = 0;
                  break;
               }
               if ((fabs(w[nm])+anorm)==anorm)
               {
                  break;
               }
            }
            if ( flag )
            {
               double c=0.0;
               double s=1.0;
               for ( int i=l; i<=k; i++ )
               {
                  double f=s*rv1[i];
                  rv1[i]=c*rv1[i];
                  if ((fabs(f)+anorm)==anorm) break;
                  g=w[i];
                  if (fabs(f)>fabs(g))
                  {
                     c=g/f;
                     h=fabs(f)*sqrt(1.0+c*c);
                  }
                  else
                  {
                     if( fabs(g) >  0.0 )
                     {
                        c=f/g;
                        h=fabs(g)*sqrt(1.0+c*c);
                     }
                     else
                     {
                        h=0.0;
                     }
                  }
                  w[i]=h;
                  h=1/h;
                  c= (g*h);
                  s=-(f*h);
                  for ( size_t j=0;  j<mp;   j++ )
                  {
                     y=a[j][nm];
                     z=a[j][i];
                     a[j][nm]=(y*c)+(z*s);
                     a[j][i]=-(y*s)+(z*c);
                  }
               }
            }
            z=w[k];
            if ( l == k )
            {
               // convergence
               if ( z < 0.0 )
               {
                  w[k]=-z;
                  for ( size_t j=0;  j < np;   j++ )
                     v[j][k] = -v[j][k];
               }
               break;
            }
            if ( its == maxit )
               return 1;

            double x = w[l];
            nm = k-1;
            y = w[nm];
            g = rv1[nm];
            h = rv1[k];
            double f = ((y-z)*(1.0+z/y)/h+((g-h)/y)*(g/h+1.0))/2;

            g=sqrt(f*f+1.0);

            f = f ? f+g*f/fabs(f) : g;
            f = (1.0-z/x)*(x+z)+(h/x)*(y/f-h);

            double c=1.0;
            double s=1.0;
            for ( int j=l;  j <= nm; j++ )
            {
               int i = j+1;
               g = rv1[i];
               y = w[i];
               h = s*g;
               g = c*g;
               if ( fabs(f) > fabs(h) )
               {
                  c=h/f;
                  z=fabs(f)*sqrt(1.0+c*c);
               }
               else
               {
                  if( fabs(h) > 0.0 )
                  {
                     c=f/h;
                     z=fabs(h)*sqrt(1.0+c*c);
                  }
                  else
                  {
                     z=0.0;
                  }
               }
               rv1[j]=z;
               c=f/z;
               s=h/z;
               f= (x*c)+(g*s);
               g=-(x*s)+(g*c);
               h=y*s;
               y*=c;
               for ( size_t jj=0; jj<np; jj++ )
               {
                  x=v[jj][j];
                  z=v[jj][i];
                  v[jj][j]= (x*c)+(z*s);
                  v[jj][i]=-(x*s)+(z*c);
               }
               if (fabs(f)>fabs(h))
               {
                  x=h/f;
                  z=fabs(f)*sqrt(1.0+x*x);
               }
               else
               {
                  if( fabs(h) > 0.0 )
                  {
                     x=f/h;
                     z=fabs(h)*sqrt(1.0+x*x);
                  }
                  else
                  {
                     z=0.0;
                  }
               }
               w[j]=z;
               if ( z )
               {
                  z=1/z;
                  c=f*z;
                  s=h*z;
               }
               f= (c*g)+(s*y);
               x=-(s*g)+(c*y);
               for ( size_t jj=0; jj<mp; jj++ )
               {
                  y=a[jj][j];
                  z=a[jj][i];
                  a[jj][j]= (y*c)+(z*s);
                  a[jj][i]=-(y*s)+(z*c);
               }
            }
            rv1[l]=0.0;
            rv1[k]=f;
            w[k]=x;
         }
      }
   }

   catch (...)
   {
      throw "svdcmp(): Matrix is singular";
   }

   return 0;
}



/**
 *  Singular values back substitution function.
 *
 */

void svbksb( const vector<vector<double> >& u,
      const vector<double>& w,
      const vector<vector<double> >& v,
      const vector<double>& b,
      vector<double>& x,
      const double minw )
{
   const size_t m = u.size();
   const size_t n = u[0].size();

   vector<double> tmp(n);

   for ( size_t j=0; j < n; ++j )
   {
      double s = 0;
      if ( fabs(w[j]) > minw )
      {
         for ( size_t i = 0; i < m; ++i )
            s += u[i][j] * b[i];
         s /= w[j];
      }
      tmp[j] = s;
   }

   for ( size_t j = 0; j < n; ++j )
   {
      double s = 0;
      for ( size_t jj = 0; jj < n; ++jj )
         s += v[j][jj] * tmp[jj];
      x[j] = s;
   }
}

/**
 *  Covariance calculation for singular values 'match'.
 *
 */

void svcovar( const vector<double>& w,
      const vector<vector<double> >& v,
      vector<vector<double> >& vcm )
{
   const size_t m = w.size();
   vector<double> wti(m);

   for ( size_t i = 0; i < m; ++i )
      wti[i] = (w[i] == 0) ? 0 : 1.0 / (w[i]*w[i]);

   for ( size_t i = 0; i < m; ++i )
   {
      for ( size_t j=0; j < i+1; ++j )
      {
         double sum = 0;
         for ( size_t k = 0; k < m; ++k )
            sum += v[k][i] * v[k][j] * wti[k];
         vcm[i][j] = sum;
         vcm[j][i] = sum;
      }
   }
}

// Calculates the (pseudo) inverse of a square matrix a.
// A warning message may be displayed inside this function. That warning message assumes that this function is called in
// the context of Kriging.
void svinv( vector<vector<double> > &a )
{
   const unsigned int aSize = a.size();
   assert( a.front().size() == aSize );
   RealMatrix v;
   RealVector w;

   // Do the singular value decomposition a = u*w*v'
   int stat = svdcmp( a, w, v ); //Now a = u from SVD
   if ( stat != 0 )
   {
      std::cout << "Warning: SVD has not converged yet. Kriging results may be biased." << std::endl;
   }
   assert( w.size() == aSize );
   assert( a.size() == aSize );
   assert( a.front().size() == aSize );
   assert( v.size() == aSize );
   assert( v.front().size() == aSize );

   // If stat != 0, SVD has not yet been converged. In that exceptional case,
   // however, we assume that the results are still usable.
   // So far (d.d. 15-01-2014), this function is only called in the context of Kriging.
   // The worst that can happen therefore is that the Kriging weights are perturbed,
   // probably such that the Kriging effect is underestimated.

   // Get maximum of w
   double wmax = *(std::max_element( w.begin(), w.end() ) );
   // Singular values w smaller than eps are discarded
   const double eps = w.size() * MachineEpsilon() * wmax;
   // Calculate (pseudo) inverse of diagonal matrix w
   for ( size_t i = 0; i < w.size(); ++i )
   {
      if ( w[i] < eps)
      {
         w[i] = 0.0;
      }
      else
      {
         w[i] = 1.0/w[i];
      }
   }

   // Calculate v = v*inv(w)
   for ( size_t i = 0; i < w.size(); ++i )
   {
      for ( size_t j = 0; j < w.size(); ++j )
      {
         v[i][j] *= w[j];
      }
   }

   // Calculate the inverse a = v*u' = (u*v')'
   RealMatrix u(a);
   for ( size_t i = 0; i < w.size(); ++i )
   {
      MatrixVectorProduct( u, v[i], a[i] );
   }
}

size_t bisection( const double xs,
      const vector<double> &xt )
{
   size_t xtsize = xt.size();
   assert( xtsize > 1 );

   if ( xs <= xt[0] )
      return 0;

   if ( xs >= xt[xtsize-2] )
      return xtsize-2;

   // Get the index 'it' such that xs[indx] is
   // between xt[it] and xt[it+1] using bisection.
   // Assumes increasing values for xt
   size_t it = 0;
   size_t high = xtsize-1;
   while ( high > it+1 )
   {
      size_t mid = (it+high)/2;
      if ( xs < xt[mid] )
      {
         high = mid;
      }
      else // if ( xs >= mt[mid] )
      {
         it = mid;
      }
   }

   return it;
}

double interpolate( const double xs,
      const vector<double> &xt,
      const vector<double> &yt,
      bool extrapolateMin, bool extrapolateMax )
{
   size_t xtsize = xt.size();
   assert( xtsize > 1 );

   // Left truncation
   if ( xs <= xt[0] && !extrapolateMin )
      return yt[0];

   // Right truncation
   if ( xs >= xt[xtsize-1] && !extrapolateMax )
      return yt[xtsize-1];

   // Get the index 'it' such that xs[indx] is
   // between xt[it] and xt[it+1] using bisection.
   // Assumes increasing values for xt
   size_t it = bisection( xs, xt );

   // Calculate the linear interpolation factor fac
   double fac = (xs - xt[it]) / (xt[it+1] - xt[it]);

   // Do the linear interpolation
   return yt[it] + fac * ( yt[ it+1 ] - yt[it] );
}

/**
 *
 *   Log of Gamma function
 *   Based on "Numerical Recipes in C: the art of scientific computing",
 *          Second Edition
 *          by W.H. Press et al., Cambridge Univ. Press, 1992
 *
 */

double logGamma( const double xx )
{
   const double cof[6] = {
      76.18009172947146, -86.50532032941677, 24.01409824083091,
      -1.231739572450155, 0.1208650973866179e-2, -0.5395239384953e-5 };
   const double stp = 2.5066282746310005;

   double x = xx-1.0;
   double tmp = x+5.5;
   tmp = (x+0.5) * log(tmp) - tmp;

   double ser = 1.000000000190015;
   for ( int j = 0; j < 6; j++ )
   {
      x=x+1.0;
      ser=ser+cof[j]/x;
   }

   return tmp+log(stp*ser);
}

/**
 *  Logarithm of the Gamma probability density function of x
 *  with mean m and variance var.
 *
 */

double logProbGamma( double x, double m, double var )
{
   assert( m > 0 );
   assert( var > 0 );
   assert( x > 0 );

   double lamb = m / var;
   double r = m * lamb;
   double tmp = lamb * x;

   return log(lamb) - logGamma(r) + (r-1) * log(tmp) - tmp;
}

/***********************************************************************
 *                                                                      *
 *  Incomplete gamma function                                           *
 * Based on "Numerical Recipes in C: the art of scientific computing",  *
 *          Second Edition                                              *
 *          by W.H. Press et al., Cambridge Univ. Press, 1992           *
 *                                                                      *
 ***********************************************************************/

static int gser( double *gamser, double a, double x, double *gln )
{
   int n;
   double sum, del, ap;

   assert( x >= 0.0 );

   *gln = logGamma( a );

   if ( x == 0.0 )
   {
      *gamser = 0.0;
   }
   else
   {
      ap = a;
      del = sum = 1.0/a;
      for ( n=1; n < 100; n++ )
      {
         ++ap;
         del *= x/ap;
         sum += del;
         if ( fabs(del) < fabs(sum)*3e-7 )
         {
            *gamser = sum * exp(-x+a*log(x)-(*gln));
            return 0;
         }
      }
      /* No convergence */
      return 1;
   }

   return 0;
}

static int gcf( double *gammcf, double a, double x, double *gln )
{
   *gln = logGamma( a );
   double b=x+1.0-a;
   double c=1.0/1e-30;
   double d=1.0/b;
   double h=d;

   int i = 1;
   for ( ;i<101;i++)
   {
      double an = -i * (i-a);
      b += 2.0;
      d = an*d+b;
      if ( fabs(d) < 1e-30 )
         d = 1e-30;
      c=b+an/c;
      if ( fabs(c) < 1e-30 )
         c = 1e-30;
      d=1.0/d;
      double del=d*c;
      h *= del;
      if ( fabs( del - 1.0 ) < 3e-7 )
         break;
   }
   if ( i > 100 )
   {
      // No convergence
      return 1;
   }
   *gammcf = exp(-x+a*log(x)-(*gln))*h;

   return 0;
}

int gammp( double a, double x, double *gamser )
{
   double gln;
   int ret = 0;

   if ( x < 0.0 || a <= 0.0 )
   {
      // Arguments not correct
      ret = 2;
   }
   else if ( x < (a+1.0) )
   {
      ret = gser( gamser, a, x, &gln );
   }
   else
   {
      ret = gcf( gamser, a, x, &gln );
      *gamser = 1.0 - *gamser;
   }

   return ret;
}

/**
 *   erf(x) = 2/sqrt(pi) integrate[0,x] exp(-t*t) dt
 *
 *   erf(0) = 0
 *   erf(infinite) = 1
 *  erf(-x) == -erf(x)
 *
 */

double erf( double x )
{
   double gamser = 0;
   int stat = gammp( 0.5, x*x, &gamser );
   assert( stat == 0 );
   return x < 0 ? -gamser : gamser;
}

/**
 *  Cumulative normal distribution
 *   = integral[-infinity,x](exp(-t*t/2)dt) / sqrt(2 pi)
 *
 *  cnp(x) = 1 - cnp(-x)
 *  cnp(infinite) == 1
 *  cnp(0) == 0.5
 *  cnp(1) == 0.8413
 */

double cnp( double x )
{
   return ( erf( x/sqrt(2.0) ) + 1 ) / 2;
}

/**
 *  p(x;m,var) = C(min,max) (2.pi.var)^(-0.5) exp[ -0.5 (x-m)^2 / var ]
 *
 *  log : -0.5 * (x-m)^2 / var  + -0.5 * log(2.pi.var) - log(C)
 *
 */

double logProbNormal( double x, double m, double var, double *min, double *max )
{
   const double pi = 4 * atan(1.0);

   assert( var > 0 );
   double d = sqrt(var);
   double tmp = -0.5 *(x-m)*(x-m) / var;
   double tmpn = -0.5 * log( 2 * pi * var );

   // Adapt normalization constant when applying bounds
   double tpr = 1;

   if ( min )
   {
      // Minimum bound given
      assert( x >= *min );

      double lpr = cnp( (*min - m) / d );
      tpr -= lpr;
   }

   if ( max )
   {
      // Minimum bound given
      assert( x <= *max );

      double rpr = 1 - cnp( (*max - m) / d );
      tpr -= rpr;
   }

   assert( tpr > 0 );

   return tmp + tmpn - log(tpr);
}

void trNormalProp( double mu, double sd, double mn, double mx,
                   double& trmu, double& trsd )
{
   double pi = 4*atan(1.0);
   double xmn = (mn-mu)/sd;
   double xmx = (mx-mu)/sd;
   double phimn = exp(-xmn*xmn/2.0)/(sqrt(2*pi));
   double phimx = exp(-xmx*xmx/2.0)/(sqrt(2*pi));

   double n = cnp(xmx)-cnp(xmn);
   trmu = mu + sd*(phimn-phimx)/n;
   trsd = sd*sd*(1+(xmn*phimn-xmx*phimx)/n) - (trmu-mu)*(trmu-mu);
   trsd = sqrt(trsd);
}

/***********************************************************************
 * Based on "Numerical Recipes in C: the art of scientific computing",  *
 *          Second Edition                                              *
 *          by W.H. Press et al., Cambridge Univ. Press, 1992           *
 ***********************************************************************/

#define tiny 1.0e-6

int ludcmp( vector<vector<double> >& a, vector<int>& indx )
{
   size_t ne = a.size();
   size_t imax = 0;
   double aamax = 0;
   double dum;
   vector<double> vv(100);

   for ( size_t i = 0; i < ne; i++ )
   {
      aamax= 0;
      for ( size_t j = 0; j < ne; j++ )
      {
         if ( fabs( a[i][j] ) > aamax )
         {
            aamax = fabs( a[i][j] );
         }
      }
      if ( aamax == 0.0 )
      {
         /* Singular */
         return 1;
      }
      vv[i] = 1.0 / aamax;
   }

   for ( size_t j = 0; j < ne; j++ )
   {
      for ( size_t i = 0; i < j; i++ )
      {
         for ( size_t k = 0; k < i; k++ )
         {
            a[i][j] -= a[i][k] * a[k][j];
         }
      }
      aamax = 0.0;
      for ( size_t i = j; i < ne; i++ )
      {
         for ( size_t k = 0; k < j; k++ )
         {
            a[i][j] -= a[i][k] * a[k][j];
         }
         dum = vv[i] * fabs(a[i][j]);
         if ( dum >= aamax )
         {
            imax = i;
            aamax = dum;
         }
      }
      if ( j != imax )
      {
         for ( size_t k = 0; k < ne; k++ )
         {
            dum = a[imax][k];
            a[imax ][k] = a[j][k];
            a[j][ k] = dum;
         }
         vv[imax] = vv[j];
      }
      indx[j] = imax;
      if (j != ne-1 )
      {
         if ( a[j][j] == 0.0 )
         {
            a[j][j] = tiny;
         }
         dum = 1.0 / a[j][j];
         for ( size_t i = j+1; i < ne; i++ )
         {
            a[i][j] *= dum;
         }
      }
   }

   if ( a[ne-1][ne-1] == 0.0 )
   {
      a[ne-1][ne-1] = tiny;
   }

   return 0;
}

/***********************************************************************
 * Based on "Numerical Recipes in C: the art of scientific computing",  *
 *          Second Edition                                              *
 *          by W.H. Press et al., Cambridge Univ. Press, 1992           *
 ***********************************************************************/

void lubksb( const vector<vector<double> >& a,
      const vector<int>& indx,
      vector<double>& b )
{
   size_t ne = a.size();
   int ii = -1;

   for ( size_t i = 0; i < ne; i++ )
   {
      int ll = indx[i];

      double sum = b[ll];
      b[ll] = b[i];
      if ( ii != -1 )
      {
         for ( size_t j = ii; j < i; j++ )
         {
            sum -= a[i][j] * b[j];
         }
      }
      else if ( sum != 0.0 )
      {
         ii=i;
      }
      b[i] = sum;
   }

   for ( size_t i = ne; i > 0; i-- )
   {
      double sum = b[i-1];
      for ( size_t j = i; j < ne; j++ )
      {
         sum -= a[i-1][j] * b[j];
      }
      b[i-1] = sum / a[i-1][i-1];
   }
}

/***********************************************************************
 * Based on "Numerical Recipes in C: the art of scientific computing",  *
 *          Second Edition                                              *
 *          by W.H. Press et al., Cambridge Univ. Press, 1992           *
 ***********************************************************************/

double det( const vector<vector<double> >& a )
{
   double d = 1;
   vector<vector<double> > aa = a;

   vector<int> indx;
   indx.resize( a.size() );
   int stat = ludcmp( aa, indx );
   assert( stat == 0 );

   for ( size_t i = 0; i < a.size(); ++i )
   {
      d *= aa[i][i];
   }

   return d;
}

namespace{
// Calculates the squared difference of the two arguments
double squared_difference( double x, double y )
{
   const double diff( x - y );
   return diff*diff;
}
// Function object for calculating the squared error wrt. the mean
struct squared_error
{
   squared_error(double mean) : m_mean(mean) {}
   // the squared error of val wrt. mean
   double operator()( double const& val ) const { return squared_difference( val, m_mean ); }
   // for std::accumulate
   double operator()( double result, double const& val ) const { return result + this->operator()(val); }
   private:
      double m_mean;
};

} // anon. namespace

double MeanSquaredError( RealVector const& v1, RealVector const& v2 )
{
   if ( v1.empty() )
   {
      return 0;
   }

   // Calculate the squared difference
   return std::inner_product( v1.begin(), v1.end(), v2.begin(), 0.0, std::plus<double>(), &squared_difference ) / v1.size();
}

double LengthOfDiffVector( RealVector const& v1, RealVector const& v2 )
{
   if ( v1.empty() )
   {
      return 0;
   }

   // Calculate the length of v2 - v1
   return sqrt( std::inner_product( v1.begin(), v1.end(), v2.begin(), 0.0, std::plus<double>(), &squared_difference ) );
}

double KrigingDistance( RealVector const& v1, RealVector const& v2, unsigned int l )
{
   if ( v1.empty() )
   {
      return 0;
   }

   // Check whether the 2 vectors share the same binary values. If they don't, a value larger than
   // the maximum correlation length is returned just to enforce zero covariance between v1 and v2.
   for ( unsigned int i = l; i < v1.size(); ++i )
   {
      if ( fabs( v2[i] - v1[i] ) > 0.5 )
      {
         return ( 1.0 + 2*sqrt( double( l ) ) );
      }
   }

   // Now the 2 vectors share the same binary values. As a result,
   // a realistic correlation length can be calculated.
   double l2 = 0.0;
   for ( unsigned int i = 0; i < l; ++i )
   {
      double diff = v2[i] - v1[i];
      l2 += ( diff * diff );
   }
   return sqrt( l2 );
}

/// Implementation for AutoScaleRobust
double VectorMean( RealVector const& v )
{
   if ( v.empty() )
   {
      return 0;
   }
   // calculate sum over all elements, divide by number of elements
   return ( std::accumulate( v.begin(), v.end(), 0.0 ) ) / v.size();
}

double VectorStdDev( RealVector const& v, double mean )
{
   if (v.size() < 2 )
   {
      return 0;
   }
   const double mse = std::accumulate( v.begin(), v.end(), 0.0, squared_error(mean) );
   return sqrt( mse /( v.size() - 1 ) );

}

bool VectorMeanAndStdDev( RealVector const& v, double& mean, double& stddev )
{
   if ( v.empty() )
   {
      mean = 0;
      stddev = 0;
      return false;
   }
   static const double tolerance = sqrt( MachineEpsilon() );

   mean=VectorMean( v );
   stddev=VectorStdDev( v, mean );
   return ( stddev >= tolerance * ( 1 + fabs(mean) ) );
}

bool VectorScaleRobust( RealVector & v, double & mean, double & stddev )
{
   const bool ok = VectorMeanAndStdDev( v, mean, stddev );

   // prevent dividing by zero in case of gegenerate variance
   if ( ! ok )
   {
      stddev=1.0;
   }

   // v[i] := v[i] - mean
   transform( v.begin(), v.end(), v.begin(), bind2nd( std::minus<double>(), mean ) );

   // v[i] := v[i] / stddev
   transform( v.begin(), v.end(), v.begin(), bind2nd( std::divides<double>(), stddev ) );

   return ok;
}

void VectorScaleToMean( RealVector & v, double & mean )
{
   mean = VectorMean( v );

   // v[i] := v[i] - mean
   transform( v.begin(), v.end(), v.begin(), bind2nd( std::minus<double>(), mean ) );
}


bool MatrixScaleRobust( RealMatrix & m, RealVector & mean, RealVector & stddev )
{
   static const double tolerance = sqrt( MachineEpsilon() );
   bool allOk = true;

   const size_t nRows( m.size() );
   const size_t nCols( nRows ? m.front().size() : 0 );

   // Calculate column-wise mean and standard deviation
   MatrixColumnMeanAndStdDev( m, mean, stddev );

   // Scale
   for ( size_t i = 0; i < nCols; ++i )
   {
      const bool ok = ( stddev[i] >= tolerance * ( 1 + fabs(mean[i]) ) );
      const double stddev_ok = ok ? stddev[i] : 1.0;
      allOk = allOk ? ok : false;
      const double m0=mean[i];

      for ( size_t j = 0; j < nRows; ++j )
      {
         m[j][i] = ( m[j][i] - m0 )/ stddev_ok;
      }
   }

   return allOk;
}

void MatrixScaleToMean( RealMatrix & m, RealVector & mean )
{
   // Calculate column-wise mean
   MatrixColumnMean( m, mean );

   // Scale
   for ( RealMatrix::iterator it = m.begin(); it != m.end(); ++it )
   {
      RealVector & row( *it );
      std::transform( row.begin(), row.end(), mean.begin(), row.begin(), std::minus<double>() );
   }
}

void MatrixColumnMean( RealMatrix const& m, RealVector &mean )
{
   const size_t nRows( m.size() );
   const size_t nCols( nRows ? m.front().size() : 0 );

   mean.assign( nCols, 0 );
   for ( RealMatrix::const_iterator it = m.begin(); it != m.end(); ++it )
   {
      RealVector const& row( *it );
      std::transform( row.begin(), row.end(), mean.begin(), mean.begin(), std::plus<double>() );
   }
   std::transform( mean.begin(), mean.end(), mean.begin(), std::bind2nd ( std::divides<double>(), static_cast<double>(nRows) ) );
}

void MatrixColumnMeanAndStdDev( RealMatrix const& m, RealVector & mean, RealVector & stddev )
{
   const size_t nRows( m.size() );
   const size_t nCols( nRows ? m.front().size() : 0 );

   MatrixColumnMean( m, mean );

   // no need to clear stddev because all elements are set
   if (nRows < 2 )
   {
      stddev.assign(nCols,0);
      return;
   }
   stddev.resize( nCols );
   for ( size_t i = 0; i < nCols; ++i )
   {
      double sum=0;
      for ( size_t j = 0; j < nRows; ++j )
      {
         double err = m[j][i] - mean[i];
         sum += err * err;
      }
      stddev[i] = sqrt( sum / (nRows - 1 ) );
   }
}

// Function object for inner product with a vector
struct InnerProduct
{
   InnerProduct ( RealVector const& v, double offset = 0.0 ) :
      vec(v),
      init( offset )
   {
      // empty
   }
   inline double operator() ( RealVector const& v )
   {
      return std::inner_product( v.begin(), v.end(), vec.begin(), init );
   }

   RealVector vec;
   double init;
};

void MatrixVectorProduct( RealMatrix const& m, RealVector const& v, RealVector & w )
{
   // Matrix m stored as vector of rows
   const size_t nRows( m.size() );
   const size_t nCols( nRows ? m.front().size() : 0 );

   if ( nRows == 0 || nCols == 0 )
   {
      THROW2 (DimensionOutOfBounds, "Empty matrix" );
   }
   if ( nCols != v.size() )
   {
      THROW2 (DimensionMismatch, "matrix column count does not match size of vector" );
   }

   w.resize( nRows );
   std::transform( m.begin(), m.end(), w.begin(), InnerProduct( v ) );
}

double VectorL2Norm ( RealVector const& v )
{
   return sqrt( std::inner_product( v.begin(), v.end(), v.begin(), 0.0 ) );
}

void CalcRange( RealVector const& min, RealVector const& max, RealVector &range )
{
   range.resize( max.size() );
   std::transform( max.begin(), max.end(), min.begin(), range.begin(), std::minus<double>() );
}

void CalcMinStdDev(
      RealVector const& min,
      RealVector const& max,
      RealVector        &minStdDev,
      double            eps /* MinStdDevEpsilon */
      )
{
   const unsigned int size( min.size() );
   minStdDev.resize( size );

   for ( unsigned int i = 0; i < size; ++i)
   {
      minStdDev[i] = eps * ( max[i] - min[i] );
   }
}

void CalcAverages( RealMatrix const& m, vector<double>& avg )
{
   if ( m.empty() )
   {
      avg.clear();
      return;
   }
   avg.resize(m.front().size());

   // Copy the first sample
   avg = m[0];

   // Add the rest of the samples
   for ( size_t i = 1; i < m.size(); ++i )
      for ( size_t j = 0; j < m[i].size(); ++j )
         avg[j] += m[i][j];

   // Divide by the number of samples
   for ( size_t j = 0; j < avg.size(); ++j )
      avg[j] /= m.size();
}

void CalcCovariances( vector<RealVector> const& m, RealVector const& avg, RealMatrix& covmat )
{
   if ( m.empty() )
   {
      covmat.clear();
      return;
   }

   // The number of parameters in the sample m
   const unsigned int num( m.size() );

   // The number of elements in the parameters
   const unsigned int size( m.front().size() );

   assert( avg.size() == size );

   // Resize the covariances matrix
   covmat.resize( size );
   for ( size_t i = 0; i < size; ++i )
   {
      covmat[i].resize( size );
      // Initialise all values to zero
      std::fill( covmat[i].begin(), covmat[i].end(), 0.0 );
   }

   // For all samples add (valuej-avgj) * (valuek-avgk)
   for ( size_t i = 0; i < num; ++i )
   {
      for ( size_t k = 0; k < size; ++k )
      {
         for ( size_t j = k; j < size; ++j )
         {
            covmat[j][k] += (m[i][j]-avg[j])*(m[i][k]-avg[k]);
         }
      }
   }

   // @todo replace with MachineEpsilon()*MachineEpsilon() ???
   static const double minVariance = 1e-32;

   for ( size_t k = 0; k < size; ++k )
   {
      for ( size_t j = k; j < size; ++j )
      {
         // Divide by the sample size
         covmat[j][k] /= num;

         // Truncate values
         if ( j == k && fabs( covmat[j][j] ) < minVariance )
         {
            covmat[j][j] = minVariance;
         }

         // Complete the symmetry
         covmat[k][j] = covmat[j][k];
      }
   }
}

double CriticalValue( unsigned int const& df, double const& p )
{
   double f1 = -0.1 + 63.7 / ( 100.0 - p );
   double fInf = 2.59 - 0.0175 * ( 100.0 - p ) - 0.32 * log( 100.0 - p );
   double c1 = 1.0 + 2.0 * pow( p/100.0, 20.0 );
   double c2 = 0.25;
   if ( p > 99.0 )
   {
      c2 += 0.06 * ( p - 99.0 );
   }
   else if ( p < 90.0 )
   {
      c2 += 0.25;
   }
   else
   {
      c2 += 0.056 * pow( 99.0 - p, 0.68 );
   }
   double w = exp( -c1 * pow( double( df - 1 ), c2 ) );

   return fInf * ( 1.0 - w ) + f1 * w;
}

int calculateSVD( vector<vector<double> >& a, vector<double>& w, vector<vector<double> >& v )
{
   const size_t nRows( a.size() );
   const size_t nCols( nRows ? a.front().size() : 0 );
   if (  nRows == 0 || nCols == 0 )
   {
      return 0;
   }
   // Clearing not necessary
   w.resize( nCols );
   v.resize( nCols, RealVector( nCols ) );

   // Do the singular value decomposition by calling svdcmp of which the current implementation
   // does not fully support underdetermined (nRows < nCols) systems of equations.
   // Note: In the context of polynomial proxies, nRows = nCols also refers to an underdetermined
   // system as the column that corresponds to the intercept has been eliminated from matrix a.
   assert( nRows > nCols );
   int stat = svdcmp( a, w, v );
   assert( a.size() == nRows );
   assert( a.front().size() == nCols );
   return stat;
}

} // namespace SUMlib
