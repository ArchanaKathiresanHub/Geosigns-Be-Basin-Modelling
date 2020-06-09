#include "SseElementAssembly.h"

#include <xmmintrin.h>
#include <mm_malloc.h>
#include <cassert>

using namespace std;

// res = a * c
// where c is a diagonal matrix
inline void scalematSSE ( int numberOfQuadraturePoints, double* a, double* c, double* r )
{
  __m128d* as  = (__m128d*)(a);
  __m128d* res = (__m128d*)(r);

  int start = 0;

  if ( numberOfQuadraturePoints % 2 == 1 )
  {
    __m128d c1 = _mm_set1_pd ( c [ 0 ]);

    res [ 0 ] = _mm_mul_pd ( c1, as [ 0 ]);
    res [ 1 ] = _mm_mul_pd ( c1, as [ 1 ]);
    res [ 2 ] = _mm_mul_pd ( c1, as [ 2 ]);
    res [ 3 ] = _mm_mul_pd ( c1, as [ 3 ]);

    as   += 4;
    res  += 4;
    start = 1;
  } 

  for ( int i = start; i < numberOfQuadraturePoints; i += 2, as += 8, res+= 8 )
  {
    __m128d c1 = _mm_set1_pd ( c [ i ]);
    __m128d c2 = _mm_set1_pd ( c [ i + 1 ]);

    res [ 0 ] = _mm_mul_pd ( c1, as [ 0 ]);
    res [ 1 ] = _mm_mul_pd ( c1, as [ 1 ]);
    res [ 2 ] = _mm_mul_pd ( c1, as [ 2 ]);
    res [ 3 ] = _mm_mul_pd ( c1, as [ 3 ]);

    res [ 4 ] = _mm_mul_pd ( c2, as [ 4 ]);
    res [ 5 ] = _mm_mul_pd ( c2, as [ 5 ]);
    res [ 6 ] = _mm_mul_pd ( c2, as [ 6 ]);
    res [ 7 ] = _mm_mul_pd ( c2, as [ 7 ]);
  }
}

// r = a * b
// where b is a block diagon al matrix, each diagonal block is a 3x1 vector.
inline void mul3x1matSSE ( int numberOfQuadraturePoints, double* a, double* b, double* r )
{
   __m128d* am = (__m128d*)(a);
   __m128d* res = (__m128d*)(r);

   double* bv = b;

   for ( int i = 1; i <= numberOfQuadraturePoints; i += 1, am += 12, res+= 4, bv += 3 )
   {
      __m128d c1 = _mm_set1_pd ( bv [ 0 ]);
      __m128d c2 = _mm_set1_pd ( bv [ 1 ]);
      __m128d c3 = _mm_set1_pd ( bv [ 2 ]);

      res [ 0 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( c1, am [ 0 ]), _mm_mul_pd ( c2, am [ 4 ])), _mm_mul_pd ( c3, am [  8 ]));
      res [ 1 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( c1, am [ 1 ]), _mm_mul_pd ( c2, am [ 5 ])), _mm_mul_pd ( c3, am [  9 ]));
      res [ 2 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( c1, am [ 2 ]), _mm_mul_pd ( c2, am [ 6 ])), _mm_mul_pd ( c3, am [ 10 ]));
      res [ 3 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( c1, am [ 3 ]), _mm_mul_pd ( c2, am [ 7 ])), _mm_mul_pd ( c3, am [ 11 ]));
   }  
}


// r = a * b
// where b is a block diagonal matrix, each diagonal block is a 3x3 matrix.
inline void mul3x3matSSE ( int numberOfQuadraturePoints, double* a, double* c, double* r )
{
   __m128d* bv = (__m128d*)(a);
   __m128d* res = (__m128d*)(r);

   double* jac = c;

   for ( int i = 1; i <= numberOfQuadraturePoints; i += 1, bv += 12, res+= 12, jac += 9 )
   {
      __m128d j11 = _mm_set1_pd ( jac [ 0 ]);
      __m128d j21 = _mm_set1_pd ( jac [ 1 ]);
      __m128d j31 = _mm_set1_pd ( jac [ 2 ]);

      __m128d j12 = _mm_set1_pd ( jac [ 3 ]);
      __m128d j22 = _mm_set1_pd ( jac [ 4 ]);
      __m128d j32 = _mm_set1_pd ( jac [ 5 ]);

      __m128d j13 = _mm_set1_pd ( jac [ 6 ]);
      __m128d j23 = _mm_set1_pd ( jac [ 7 ]);
      __m128d j33 = _mm_set1_pd ( jac [ 8 ]);

      res [ 0 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j11, bv [ 0 ]), _mm_mul_pd ( j21, bv [ 4 ])), _mm_mul_pd ( j31, bv [  8 ]));
      res [ 1 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j11, bv [ 1 ]), _mm_mul_pd ( j21, bv [ 5 ])), _mm_mul_pd ( j31, bv [  9 ]));
      res [ 2 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j11, bv [ 2 ]), _mm_mul_pd ( j21, bv [ 6 ])), _mm_mul_pd ( j31, bv [ 10 ]));
      res [ 3 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j11, bv [ 3 ]), _mm_mul_pd ( j21, bv [ 7 ])), _mm_mul_pd ( j31, bv [ 11 ]));

      res [ 4 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j12, bv [ 0 ]), _mm_mul_pd ( j22, bv [ 4 ])), _mm_mul_pd ( j32, bv [  8 ]));
      res [ 5 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j12, bv [ 1 ]), _mm_mul_pd ( j22, bv [ 5 ])), _mm_mul_pd ( j32, bv [  9 ]));
      res [ 6 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j12, bv [ 2 ]), _mm_mul_pd ( j22, bv [ 6 ])), _mm_mul_pd ( j32, bv [ 10 ]));
      res [ 7 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j12, bv [ 3 ]), _mm_mul_pd ( j22, bv [ 7 ])), _mm_mul_pd ( j32, bv [ 11 ]));

      res [  8 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j13, bv [ 0 ]), _mm_mul_pd ( j23, bv [ 4 ])), _mm_mul_pd ( j33, bv [  8 ]));
      res [  9 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j13, bv [ 1 ]), _mm_mul_pd ( j23, bv [ 5 ])), _mm_mul_pd ( j33, bv [  9 ]));
      res [ 10 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j13, bv [ 2 ]), _mm_mul_pd ( j23, bv [ 6 ])), _mm_mul_pd ( j33, bv [ 10 ]));
      res [ 11 ] = _mm_add_pd ( _mm_add_pd ( _mm_mul_pd ( j13, bv [ 3 ]), _mm_mul_pd ( j23, bv [ 7 ])), _mm_mul_pd ( j33, bv [ 11 ]));
   }  
}


// private 
// Multiplies 8xn by 8xn^t using sse instructions.
// c = a * b^t + \beta c
inline void matmulSSEadd( int numberOfQuadraturePoints, double* a, double* b, double  alpha, double* c )
{
   __m128d alpha2 = _mm_set1_pd ( alpha );

   int start;
   int bPos;

   __m128d* as = (__m128d*)(a);
   __m128d* cs = (__m128d*)(c);

   if ( (numberOfQuadraturePoints % 2) == 0 )
   {
      cs = (__m128d*)(c);

      for ( int j = 0; j < 8; ++j, cs += 4 )
      {
         as = (__m128d*)(a);

         __m128d bj1 = _mm_set1_pd ( b [ j ]);
         __m128d bj2 = _mm_set1_pd ( b [ j +  8 ]);

         cs [ 0 ] = _mm_add_pd ( _mm_mul_pd ( alpha2, cs [ 0 ]), _mm_add_pd ( _mm_mul_pd ( as [ 0 ], bj1 ), _mm_mul_pd ( as [ 4 ], bj2 )));
         cs [ 1 ] = _mm_add_pd ( _mm_mul_pd ( alpha2, cs [ 1 ]), _mm_add_pd ( _mm_mul_pd ( as [ 1 ], bj1 ), _mm_mul_pd ( as [ 5 ], bj2 )));
         cs [ 2 ] = _mm_add_pd ( _mm_mul_pd ( alpha2, cs [ 2 ]), _mm_add_pd ( _mm_mul_pd ( as [ 2 ], bj1 ), _mm_mul_pd ( as [ 6 ], bj2 )));
         cs [ 3 ] = _mm_add_pd ( _mm_mul_pd ( alpha2, cs [ 3 ]), _mm_add_pd ( _mm_mul_pd ( as [ 3 ], bj1 ), _mm_mul_pd ( as [ 7 ], bj2 )));
      }
      start = 2;
   }
   else
   {
      for ( int j = 0; j < 8; ++j, cs += 4 )
      {
         __m128d bj1 = _mm_set1_pd ( b [ j ]);

         cs [ 0 ] = _mm_add_pd ( _mm_mul_pd ( alpha2, cs [ 0 ]), _mm_mul_pd ( as [ 0 ], bj1 ));
         cs [ 1 ] = _mm_add_pd ( _mm_mul_pd ( alpha2, cs [ 1 ]), _mm_mul_pd ( as [ 1 ], bj1 ));
         cs [ 2 ] = _mm_add_pd ( _mm_mul_pd ( alpha2, cs [ 2 ]), _mm_mul_pd ( as [ 2 ], bj1 ));
         cs [ 3 ] = _mm_add_pd ( _mm_mul_pd ( alpha2, cs [ 3 ]), _mm_mul_pd ( as [ 3 ], bj1 ));
      }
      start = 1;
   }

   cs = (__m128d*)(c);

   for ( int j = 0; j < 8; ++j, cs += 4 )
   {
      bPos = j + 8 * start;
      as = (__m128d*)(&a [ start * 8 ]);

      for ( int k = start; k < numberOfQuadraturePoints; k += 2, as += 8, bPos += 8 )
      {
         __m128d bj1 = _mm_set1_pd ( b [ bPos ]);
         __m128d bj2 = _mm_set1_pd ( b [ bPos +  8 ]);

         cs [ 0 ] = _mm_add_pd ( cs [ 0 ], _mm_add_pd ( _mm_mul_pd ( as [ 0 ], bj1 ), _mm_mul_pd ( as [ 4 ], bj2 )));
         cs [ 1 ] = _mm_add_pd ( cs [ 1 ], _mm_add_pd ( _mm_mul_pd ( as [ 1 ], bj1 ), _mm_mul_pd ( as [ 5 ], bj2 )));
         cs [ 2 ] = _mm_add_pd ( cs [ 2 ], _mm_add_pd ( _mm_mul_pd ( as [ 2 ], bj1 ), _mm_mul_pd ( as [ 6 ], bj2 )));
         cs [ 3 ] = _mm_add_pd ( cs [ 3 ], _mm_add_pd ( _mm_mul_pd ( as [ 3 ], bj1 ), _mm_mul_pd ( as [ 7 ], bj2 )));
      }
   }
}

// Multiplies 8xn by 8xn^t using sse instructions.
// c = a * b^t
inline void matmulSSEassign( int numberOfQuadraturePoints, double* a, double* b, double* c )
{
   int start;
   int bPos;

   __m128d* as = (__m128d*)(a);
   __m128d* bs = (__m128d*)(b);
   __m128d* cs = (__m128d*)(c);

   if ( (numberOfQuadraturePoints % 2) == 0 )
   {
      for ( int j = 0; j < 8; ++j, cs += 4 )
      {
         bPos = j;

         __m128d bj1 = _mm_set1_pd ( b[ bPos ]);
         __m128d bj2 = _mm_set1_pd ( b[ bPos +  8 ]);

         cs [ 0 ] = _mm_add_pd ( _mm_mul_pd ( as [ 0 ], bj1 ), _mm_mul_pd ( as [ 4 ], bj2 ));
         cs [ 1 ] = _mm_add_pd ( _mm_mul_pd ( as [ 1 ], bj1 ), _mm_mul_pd ( as [ 5 ], bj2 ));
         cs [ 2 ] = _mm_add_pd ( _mm_mul_pd ( as [ 2 ], bj1 ), _mm_mul_pd ( as [ 6 ], bj2 ));
         cs [ 3 ] = _mm_add_pd ( _mm_mul_pd ( as [ 3 ], bj1 ), _mm_mul_pd ( as [ 7 ], bj2 ));
      }
      start = 2;
   }
   else
   {
      for ( int j = 0; j < 8; ++j, cs += 4 )
      {
         __m128d bj1 = _mm_set1_pd( b[j] );

         cs [ 0 ] = _mm_mul_pd ( as [ 0 ], bj1 );
         cs [ 1 ] = _mm_mul_pd ( as [ 1 ], bj1 );
         cs [ 2 ] = _mm_mul_pd ( as [ 2 ], bj1 );
         cs [ 3 ] = _mm_mul_pd ( as [ 3 ], bj1 );
      }
      start = 1;
   }
   cs = (__m128d*)(c);

   for ( int j = 0; j < 8; ++j, cs += 4 )
   {
      bPos = j;
      as = (__m128d*)(&a [ start * 8 ]);

      for ( int k = start; k < numberOfQuadraturePoints; k += 2, as += 8, bPos += 16 )
      {
         bPos = j + k * 8;

         __m128d bj1 = _mm_set1_pd ( b [ bPos ]);
         __m128d bj2 = _mm_set1_pd ( b [ bPos +  8 ]);

         cs [ 0 ] = _mm_add_pd ( cs [ 0 ], _mm_add_pd ( _mm_mul_pd ( as [ 0 ], bj1 ), _mm_mul_pd ( as [ 4 ], bj2 )));
         cs [ 1 ] = _mm_add_pd ( cs [ 1 ], _mm_add_pd ( _mm_mul_pd ( as [ 1 ], bj1 ), _mm_mul_pd ( as [ 5 ], bj2 )));
         cs [ 2 ] = _mm_add_pd ( cs [ 2 ], _mm_add_pd ( _mm_mul_pd ( as [ 2 ], bj1 ), _mm_mul_pd ( as [ 6 ], bj2 )));
         cs [ 3 ] = _mm_add_pd ( cs [ 3 ], _mm_add_pd ( _mm_mul_pd ( as [ 3 ], bj1 ), _mm_mul_pd ( as [ 7 ], bj2 )));
      }
   }
}

// end 


Sse::SseNewElementAssembly::SseNewElementAssembly()
{
   // allocate memory for maximum size - 4x4x4 - 64
   numberOfPoints = 4 * 4 * 4;

   // zero order term
   P  = static_cast<double*>( _mm_malloc( 8 * sizeof ( double ) * numberOfPoints, 32 ) );
   PC = static_cast<double*>( _mm_malloc( 8 * sizeof ( double ) * numberOfPoints, 32 ) );
   C  = static_cast<double*>( _mm_malloc(     sizeof ( double ) * numberOfPoints, 32 ) );

   // result matrix
   K  = static_cast<double*>( _mm_malloc ( 8 * 8 * sizeof ( double ), 32 ) );

   // first order term
   G  = static_cast<double*>(_mm_malloc ( 8 * sizeof ( double ) * 3 * numberOfPoints, 32 ));
   GB = static_cast<double*>(_mm_malloc ( 8 * sizeof ( double ) *     numberOfPoints, 32 ));
   B  = static_cast<double*>(_mm_malloc (     sizeof ( double ) * 3 * numberOfPoints, 32 ));

   // second order term
   GA = static_cast<double*>( _mm_malloc ( 8 * sizeof ( double ) * 3 *     numberOfPoints, 32 ));
   A  = static_cast<double*>( _mm_malloc (     sizeof ( double ) * 3 * 3 * numberOfPoints, 32 ));

   assert( P );
   assert( C );
   assert( PC );
   assert( K );
   assert( G );
   assert( B );
   assert( GB );
   assert( A );
   assert( GA );
}

void Sse::SseNewElementAssembly:: freeMatrices()
{
   if ( !P  ) { _mm_free( P  ); P  = 0; }

   if ( !C  ) { _mm_free( C  ); C  = 0; }
   if ( !PC ) { _mm_free( PC ); PC = 0; }  
                 
   if ( !K  ) { _mm_free( K  ); K  = 0; }
                 
   if ( !G  ) { _mm_free( G  ); G  = 0; }
   if ( !B  ) { _mm_free( B  ); B  = 0; }
   if ( !GB ) { _mm_free( GB ); GB = 0; }

   if ( !A  ) { _mm_free( A  ); A  = 0; }
   numberOfPoints = -1;
}

void Sse::SseNewElementAssembly::InitAssembly( int xyQuadPts, int zQuadPts, const std::vector<double> & randData )
{
   assert( xyQuadPts < 5 && zQuadPts < 5 );

   numberOfPoints = xyQuadPts * xyQuadPts * zQuadPts;

   size_t ri = 0;

   // column wise order!
   for (    int i = 0; i < numberOfPoints; ++i )
      C[i] = randData[ri++];

   for (    int i = 0; i < numberOfPoints; ++i )
      for ( int j = 0; j < 8; ++j ) 
         P[i + j * 8] = randData[ri++];

   for (    int i = 0; i < 8; ++i ) 
      for ( int j = 0; j < 3 * numberOfPoints; ++j )
         G[i * 3 * numberOfPoints + j] = randData[ri++]; // G row wise

   for ( int i = 0; i < 3 * numberOfPoints; ++i ) B[i] = randData[ri++];

   for (       int i = 0; i < 9 * numberOfPoints; i += 9 )
      for (    int k = 0; k < 3; ++k )
         for ( int l = 0; l < 3; ++l )
            A[i + k * 3 + l] = randData[ri++]; // vector of column wise 3x3 matrices
}


void Sse::SseNewElementAssembly::AssembleElement()
{
   scalematSSE(  numberOfPoints, P, C, PC ); // PC
   mul3x1matSSE( numberOfPoints, G, B, GB ); // GB
   mul3x3matSSE( numberOfPoints, G, A, GA ); // GA
      
   matmulSSEassign(  numberOfPoints, PC, P, K ); // K  = PC * P'
   matmulSSEadd(     numberOfPoints, GB, P, 1.0, K ); // K += GB * P'
   matmulSSEadd( 3 * numberOfPoints, GA, G, 1.0, K ); // K += GA * G'
}

