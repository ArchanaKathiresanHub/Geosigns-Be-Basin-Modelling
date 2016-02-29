// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.
#ifndef PT_DIAGRAM_CALCULATOR_H
#define PT_DIAGRAM_CALCULATOR_H


#include <vector>
#include <utility>
#include <memory>

#include "EosPack.h"

///////////////////////////////////////////////////////////////////////////////
/// \brief Class for calculating P/T phase diagram using PVT flasher
///
/// All units in SI:
/// Component masses [kg]
/// Pressure [Pa]
/// Temperature [K]
///
// the typical usage should looks like this
//
//  
//  PTDiagramCalculator::DiagramType typeOfDiag = PTDiagramCalculator::MoleMassFractionDiagram;
//  // or PTDiagramCalculator::VolumeFractionDiagram
//  // or PTDiagramCalculator::MassFractionDiagram
//
//  PTDiagramCalculator diagramCalculator( typeOfDiagram, masses );
//
//  diagramCalculator.findBubleDewLines( CompTemperature, CompPressure, std::vector<double>() );
//  const std::pair< double, double> & critPt = diagramCalculator.getCriticalPoint();
//  double critT = critPt.first;
//  double critP = critPt.second;
// 
//  if ( critT > 0.0 && crtiP > 0.0 ) // found criticla P & T
//  {
//     ...
//  }
//
//  double bubbleP;
//  if ( diagBuilder->getBubblePressure( CompTemperature, bubbleP ) ) found bubble point for given T
//  {
//     ...
//  }
//
//  for ( double v = 0.0; v <= 1.0; v+= 0.1 ) // collect isolines, 0.0 - dew line / 1.0 - bubble line
//  {
//     const std::vector< std::pair<double,double> > & contLine = diagramCalculator.calcIsoline( v );
//     ...
//  } 
///////////////////////////////////////////////////////////////////////////////
class PTDiagramCalculator
{
public:
   typedef std::pair< double, double>  TPPoint;    // point on PT phase diagram (T,P)
   typedef std::vector< TPPoint >      TPLine;     // array of TPPoints as std::vector

      /// Define type of PT phase diagram
   typedef enum
   {
      MassFractionDiagram,         ///< mass fraction
      MoleMassFractionDiagram,     ///< mole mass fraction
      VolumeFractionDiagram        ///< volume fraction
   } DiagramType;

   /// \brief Constructor. It also creates default grid for T from g_MinimalTemperature:g_MinimalTemperature
   /// \param typeOfDiagram which type of diagram should be built - Mass/Mole/Volume fraction
   /// \param massFraction array  of component masses
   PTDiagramCalculator( DiagramType typeOfDiagram, const std::vector<double> & massFraction );

   /// \brief Destructor, nothing to delete yet
   ~PTDiagramCalculator() {;}

   /// \brief Calculate bubble/dew point lines, critical P and T and contour line for 0.5 fraction. 
   ///        Only after this call it is possible to calculate any other contour line.
   /// \param compT trap temperature for composition
   /// \param compP trap pressure for composition
   /// \param gridT 1D grid for temperature. If array is empty - the default grid is being used
   void findBubbleDewLines( double compT, double compP, const std::vector<double> & gridT );

   /// \brief Calculate contour line for given value. Should be called after findBubleDewLines().
   /// \param val Liquid fraction value for contour line. Must be between 0 and 1
   /// \returns array of (T,P) pair for each contour line point
   TPLine calcContourLine( double val );
   
   /// \brief Calculate set of contour lines for the given values set. Should be called after findBubleDewLines().
   /// \param vals the set of liquid fraction values for contour line. Must be between 0 and 1
   /// \returns array of (T,P) pair for each contour line point. Countour lines are divided by (-1,-1) value
   std::vector<double> calcContourLines( std::vector<double> & vals );

   /// \brief Get single phase separation line for 1 phase region as EosPack calculates it
   /// \return separation line as set of points
   TPLine getSinglePhaseSeparationLine();

   /// \brief Get critical point for defined in constructor composition. Must be called after bubble/dew lines calculation
   /// \return critical composition temperature,pressure
   TPPoint getCriticalPoint() const { return TPPoint( m_critT, m_critP ); }

   /// \brief Get cricondentherm point for defined in constructor composition. Must be called after bubble/dew lines calculation
   /// \return point on bubble/dew curve with maximum temperature
   TPPoint getCricondenthermPoint() const;

   /// \brief Get cricondenbar point for defined in constructor composition. Must be called after bubble/dew lines calculation
   /// \return point on bubble/dew curve with maximum pressure
   TPPoint getCricondenbarPoint() const;

   /// \brief calculate bubble pressure for given Temperature
   /// \param T temperature value 
   /// \param[out] bubbleP on success it contains bubble pressure for given temperature
   /// \return true on success, false otherwise
   bool getBubblePressure( double T, double * bubbleP ) const;

   /// \brief calculate (if it wasn't done before) liquid fraction value for given P & T points on the grid
   /// \param p pressure grid position 
   /// \param t temperature grid position
   /// \return liquid fraction value
   double getLiquidFraction( int p, int t );

   // some statistic info
   /// \brief get size of 1D grid along T axis
   /// \return grid T size
   int getSizeGridP() const { return static_cast<int>(m_gridP.size()); }

   /// \brief get size of 1D grid along T axis
   /// \return grid T size
   int getSizeGridT() const { return static_cast<int>(m_gridT.size()); }

   /// \brief Get the total number of PVT flasher calls during bubbl/dew lines search
   /// \return total number of flasher calls for bubble/dew lines search
   int getBubbleDewSearchIterationsNumber() const { return m_bdBisecIters; }

   /// \brief Get the total number of PVT flasher calls during contour lines search
   /// \return total number of flasher calls for contour lines search
   int getContourLinesSearchIterationsNumber() const { return m_isoBisecIters; }

   /// \brief Get the total number of PVT flasher calls during search correct A/B parameter 
   /// \return total number of flasher calls for A/B search
    int getTuneABIterationsNumber() const { return m_abTuneBisecIters; }

   /// \brief Returns 1D grid values for Temperature (X) axis
   /// \return grid along T
   std::vector<double> getGridT() const { return m_gridT; }

   /// \brief Returns 1D grid value for Pressure (Y) axis
   /// \return grid along P
   std::vector<double> getGridP() const { return m_gridP; }

   /// \brief Set tolerance value which used for bisection iterations and for cutting very small phase fractions (<eps^2)
   /// \param tol new tolerance value
   void setTolValue( double tol );   

   /// \brief Change the default value for A over B term in EosPack. Should be called before bubble/dew line search
   /// \param val new value for A over B term. Shold be more then 0 (at least)
   void setAoverBTerm( double val ) { m_AoverB = val; m_ChangeAoverB = true; }

   /// \brief Find A/B term value in such way that liquid/vapour division line will come through critical point
   /// \return found value of A/B term if it was found or the default value if not found (0.5 countour line doesn't cross bubble/dew line)
   double searchAoverBTerm();

   /// \brief Find critical point Tc and Pc in fastest way is possible
   /// \return found critical point (Tc,Pc) values if point was found or zero values if wasn't found
   TPPoint searchCriticalPoint();

   /// \brief Change the default value for nonlinear solver in PVT library
   /// \param stopTol stop tolerance for convergence of iterations, the default value for PTDiagramCalculator/EosPack 1e-6/1e-4
   /// \param maxItNum maximum iterations number, the default value for PTDiagramCalculator/EosPack 400/50
   /// \param newtonRelCoef relaxation coefficient for nonlinear Newton solver, the default value for PTDiagramCalculator/EosPack 0.2/1.0 
   void setNonLinSolverConvPrms( double stopTol, int maxItNum, double newtonRelCoef ) { m_stopTol = stopTol; m_maxIters = maxItNum; m_newtonRelCoeff = newtonRelCoef; }

private:
   std::unique_ptr<pvtFlash::EosPack> m_flasher;   ///< for multithreading purpouse keep own flasher

   DiagramType         m_diagType;               ///< which type of diagram are calculating - Mass/Mole/Volume fraction
   std::vector<double> m_gridT;                  ///< 1D grid for Temperature values
   std::vector<double> m_gridP;                  ///< 1D grid for Pressure values
   
   std::vector< std::vector<double> > m_liqFrac; ///< 2D grid of liquid fraction for P/T grid, -1 - not initialised yet value

   std::vector<double> m_masses;      ///< components mass fraction

   TPLine m_bubbleDewLine; ///< Bubble/Dew line
   TPLine m_c0p5Line;      ///< 0.5 contour line
   TPLine m_spsLine;       ///< single phase separation line outside of the 2 phase region

   TPLine::iterator  m_critPointPos; // position of critical point on bubble/dew line

   double m_eps;                     ///< some small number used in bisection iterations convergence and for mass fraction comparison
   double m_epsT;                    ///< some small number used in bisection iterations convergence for Temperature 
   double m_epsP;                    ///< some small number used in bisection iterations convergence for Pressure

   // PVT library parameters
   double m_AoverB;                  ///< A over B term EosPack parameter
   bool   m_ChangeAoverB;            ///< Should we use A/B term as EosPack parameter
   double m_stopTol;                 ///< Stop tolerance for nonlinear solver of EosPack
   int    m_maxIters;                ///< Max. iterations number for nonlinear solver of EosPack
   double m_newtonRelCoeff;          ///< Rel. coefficient for Newton nonlinear solver of EosPack

   double m_critP;                   ///< critical point P
   double m_critT;                   ///< critical point T

   int    m_bdBisecIters;            ///< total number of PVT flasher calls for bubble dew points search
   int    m_isoBisecIters;           ///< total number of PVT flasher calls contour line points search
   int    m_abTuneBisecIters;        ///< total number of PVT flasher calls for tuning A/B


   /// \brief Call PVT library and calculate mole/volume or mass fraction for Liquid/Vapor phases for given composition for single set of P, T values
   /// \param p pressure
   /// \param t temperature
   /// \param composition  array of mass values for each component
   /// \param[out] massFraction on return contains accumulated fractions for vapor/liquid phases
   /// \param massFracType which fraction should be calculated - mass, mole or volume fraction
   /// \return which phases exist in given composition for given P and T. "unknown" if call for flashing was failed
   int getMassFractions( double p, double t, const std::vector<double> & composition, double massFraction[2] );

   /// \brief Search by doing bisections bubble or dew point value, phases should be different for minP and maxP
   /// \param p1 lower P border for bisections
   /// \param t1 lower T border for bisections
   /// \param p2 upper P border for bisections
   /// \param t2 upper T border for bisections
   /// \param foundP[out] on return, if iterations were successful, it contains bubble or dew point pressure value
   /// \param foundT[out] on return, if iterations were successful, it contains bubble or dew point temperature value
   /// \return true if value was found, false otherwise
   bool doBisectionForBubbleDewSearch( size_t p1, size_t t1, size_t p2, size_t t2, double & foundP, double & foundT );

   /// \brief Search by doing bisection iterations for contour line values. Bisections could be done by the Pressure or by the Temperature
   /// \param p1 lower P border for bisections
   /// \param t1 lower T border for bisections
   /// \param p2 upper P border for bisections
   /// \param t2 upper T border for bisections
   /// \param frac liquid fraction value for contour line
   /// \param foundP[out] on return, if iterations were successful, it contains bubble or dew point pressure value
   /// \param foundT[out] on return, if iterations were successful, it contains bubble or dew point temperature value
   /// \return true if value was found, false otherwise
   bool doBisectionForContourLineSearch( int    p1, int    t1, int    p2, int    t2, double frac, double & foundP, double & foundT );
   bool doBisectionForContourLineSearch( double p1, double t1, double p2, double t2, double frac, double & foundP, double & foundT );
   bool doBisectionForContourLineSearch( double p1, double t1, double p2, double t2, int ph1, int ph2, double f1, double f2, double frac, double & foundP, double & foundT );

   /// \brief Search by doing bisection iterations for single phases separation line values. Bisections could be done by the Pressure or by the Temperature
   /// \param p1 lower P border for bisections
   /// \param t1 lower T border for bisections
   /// \param p2 upper P border for bisections
   /// \param t2 upper T border for bisections
   /// \param foundP[out] on return, if iterations were successful, it contains bubble or dew point pressure value
   /// \param foundT[out] on return, if iterations were successful, it contains bubble or dew point temperature value
   /// \return true if value was found, false otherwise
   bool doBisectionForSinglePhaseSeparationLineSearch( size_t p1, size_t t1, size_t p2, size_t t2, double & foundP, double & foundT );

   /// \brief Find critical point as intersection of 0.5 isoline with bubble/dew curve. If not found - go along bubble/dew curve
   ///        and look for phase changed from gas to liquid
   /// \return true on success, false otherwise
   bool findCriticalPointInContourBubbleDewLinesIntersection();

   /// \brief Find critical point as single phase change point along bubble/dew linei
   /// \brief addToBubblDewLine add found value to bubble/dew line
   /// return true on success, false otherwise
   bool findCriticalPointInChangePhaseAlongBubbleDewLine( bool addToBubblDewLine = true );

   /// \brief Find critical pont by calling findCriticalPointInContourBubbleDewLinesIntersection() first, if it wasn't found
   ///        then call findCriticalPointInChangePhaseAlongBubbleDewLine()
   /// \return true on success, false otherwise
   bool findCriticalPoint();

   /// \brief Extrapolate countour line last segment to one phase region and find phase change point on this extrapolated line (critical point)
   /// \param isoline array of points for countour line, used to estimate lenght of line for extrapolation 
   /// \param P1 first point of contour line interval pressure value
   /// \param P2 second point of contour line interval pressure value
   /// \param T1 first point of contour line interval temperature value
   /// \param T2 second point of contour line interval temperature value
   /// \return true if phase change point was found, false otherwise
   bool extrapolateContourLineToOnePhaseRegion( const TPLine & isoline, double P1, double T1, double P2, double T2, double & critP, double & critT );

   /// \brief Create 1D grids along P and T
   /// \param minP lower bound for P grid
   /// \param maxP upper bound for P grid
   /// \param minT lower bound for T grid
   /// \param maxT upper bound for T grid
   void generatePTGrid( double minP, double maxP, double minT, double maxT );

   /// \brief Extend P grid to a new maximal given value. Also resize 2D liquid fraction array
   /// \param newPMax new maximal value for Pressure
   void extendPGrid( double newPMax );

   /// \brief Extend T grid to a new size. Also resize 2D liquid fraction array
   /// \param steps number of points to be added
   void extendTGrid( int steps );

   /// \brief Return which phase exist for given (p,t)
   /// \param p pressure
   /// \param t temperature
   /// \return phase ID
   int getPhase( size_t p, size_t t );

   /// \brief Structure for describing rectangle area on P/T grid. Used for search starting point for tracing contour lines
   struct SuperCell { 
      size_t p1; size_t p2; size_t t1; size_t t2;
      SuperCell( size_t pp1, size_t pp2, size_t tt1, size_t tt2 )
      {
         t1 = tt1; t2 = tt2; p1 = pp1; p2 = pp2;
      }
   }; 
   /// \brief Trace countour line for given liquid fraction value. If fast is true, trace only till intersection with bubble/dew line
   /// \param val liquid fraction value for contour line
   /// \param isoloine[out] on output contour line as array of (T,P) points
   /// \param fast if true, trace isoline till intersection with bubble/dew line only (used for search of critical point)
   /// \return true on success, false otherwise
   bool traceContourLine( double val, TPLine & isoline, bool fast );

   /// \brief Search position of given rectangle area on P/T phase diagram (in which region - 1 or 2 phase or both)
   /// \param p1 node index on pressure grid for low left corner of rectangle
   /// \param p2 node index on pressure grid for upper right corner of rectangle
   /// \param t1 node index on temperature grid for low left corner of rectangle
   /// \param t2 node index on temperature grid for upper right corner of rectangle
   /// \return position of the rectangle on P/T phase diagram as enum value
   int checkCell( size_t p1, size_t p2, size_t t1, size_t t2, double val );
};

#endif
