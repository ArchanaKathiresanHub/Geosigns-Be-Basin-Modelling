#ifndef _NUMERIC_FUNCTIONS__QUADRATURE_H_
#define _NUMERIC_FUNCTIONS__QUADRATURE_H_

namespace NumericFunctions {

   /// Enables retrieval of Gauss-Legendre and Gauss-Lobatto quadrature points and weights
   /// for degrees upto and including 20, and computation until numerical precision 
   /// prevents the data from being useful.
   class Quadrature {

   public :

      typedef const double* QuadratureArray;

      /// Number of faces in the element.
      static const int NumberOfFaces = 6;

      /// The maximum quadrature degree.
      enum { MaximumQuadratureDegree = 20 };


      /// \brief Get the instance of the singlton.
      static Quadrature& getInstance ();

      static void finaliseQuadrature ();

      /// Return the Gauss-Legendre quadrature points and weights for the specified degree.
      ///
      /// Degree should be in the range 1 ..  MaximumQuadratureDegree
      void getGaussLegendreQuadrature ( const int              Degree,
                                              QuadratureArray& Points,
                                              QuadratureArray& Weights ) const;

      /// Return the Gauss-Lobatto quadrature points and weights for the specified degree.
      ///
      /// Degree should be in the range 2 .. MaximumQuadratureDegree, 
      /// if Degree = 1, then the Degree = 2 quadrature set are 
      /// returned, as there is no 1 point Lobatto quadrature rule!
      void getGaussLobattoQuadrature  ( const int              Degree,
                                              QuadratureArray& Points,
                                              QuadratureArray& Weights ) const;

      /// Compute the Gauss-Legendre quadrature.
      void computeGaussLegendreQuadrature ( const int      Degree,
                                                  double*& Points,
                                                  double*& Weights );

      /// Compute the Gauss-Lobatto quadrature.
      void computeGaussLobattoQuadrature  ( const int      Degree,
                                                  double*& Points,
                                                  double*& Weights );

      /// \brief Get the quadrature points for the dimensio that is fixed on the boundary.
      void getFixedQuadrature ( const int              face,
                                      QuadratureArray& points,
                                      QuadratureArray& weights ) const;

   private :

      static Quadrature* s_quadratureInstance;

      /// Initialise the qarrays containing the quadrature points and weights.
      void initialiseQuadrature ();

      /// Compute the Legendre polynomial and derivatives for the degree and point specified.
      void setLegendrePolynomials ( const int     Degree,
                                    const double  Evaluation_Point,
                                          double& Phi,
                                          double& Phi_Dash,
                                          double& Phi_Dash_Dash );


      /// Arrays containing the quadrature points and weights.
      double* m_gaussLegendreQuadraturePoints  [ MaximumQuadratureDegree ];
      double* m_gaussLegendreQuadratureWeights [ MaximumQuadratureDegree ];

      double* m_gaussLobattoQuadraturePoints   [ MaximumQuadratureDegree ];
      double* m_gaussLobattoQuadratureWeights  [ MaximumQuadratureDegree ];

      double* m_fixedFaceQuadraturePoints  [ NumberOfFaces ];
      double* m_fixedFaceQuadratureWeights [ NumberOfFaces ];

   };

}

#endif // _NUMERIC_FUNCTIONS__QUADRATURE_H_
