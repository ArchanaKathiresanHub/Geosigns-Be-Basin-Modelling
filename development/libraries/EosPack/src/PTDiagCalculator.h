// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.
#ifndef PT_DIAGRAM_CALCULATOR_H
#define PT_DIAGRAM_CALCULATOR_H

#include <vector>

///////////////////////////////////////////////////////////////////////////////
/// \brief Class for calculating P/T phase diagram using PVT flasher
///
/// All units in SI:
/// Component masses [kg]
/// Pressure [Pa]
/// Temperature [K]
///
/// the typical usage should look like this
///
//  PTDiagramCalculator diagramCalculator( typeOfDiagram, masses );
//
//  diagramCalculator.findBubleDewLines( CompTemperature, CompPressure, std::vector<double>() );
//  double critT = diagramCalculator.getCriticalP();
//  double critP = diagramCalculator.getCriticalT();
//
//  for ( double v = 0.0; v <= 1.0; v+= 0.1 )
//  {
//     const std::vector< std::pair<double,double> > & contLine = diagramCalculator.calcIsoline( v );
//     ...
//  } 
///////////////////////////////////////////////////////////////////////////////
class PTDiagramCalculator
{
public:
   /// Define type of PT phase diagram
   enum DiagramType
   {
      MassFractionDiagram,         ///< mass fraction
      MoleMassFractionDiagram,     ///< mole mass fraction
      VolumeFractionDiagram        ///< volume fraction
   };

   /// \brief Constructor. It also creates default grid for T from g_MinimalTemperature:g_MinimalTemperature
   /// \param typeOfDiagram which type of diagram should be built - Mass/Mole/Volume fraction
   /// \param massFraction array of component masses
   PTDiagramCalculator( DiagramType & typeOfDiagram, const std::vector<double> & massFraction );

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
   std::vector< std::pair<double, double> > calcContourLine( double val );

   /// \brief Get critical temperature for defined in constructor composition. Must be called after bubble/dew lines calculation
   /// \return critical composition temperature
   double getCriticalT() const { return m_critT; }

   /// \brief Get critical pressure for defined in constructor composition. Must be called after bubble/dew lines calculation
   /// \return critical composition pressure
   double getCriticalP() const { return m_critP; }

   /// \brief calculate bubble pressure for given Temperature
   /// \param T temperature value 
   /// \param[out] bubbleP on success it contains bubble pressure for given temperature
   /// \return true on success, false otherwise
   bool getBubblePressure( double T, double & bubbleP ) const;
   
   // some statistic info
   /// \brief get size of 1D grid along T axis
   /// \return grid T size
   int getSizeGridP() const { return m_gridP.size(); }

   /// \brief get size of 1D grid along T axis
   /// \return grid T size
   int getSizeGridT() const { return m_gridT.size(); }

   /// \brief Get the total number of PVT flasher calls during bubbl/dew lines search
   /// \return total number of flasher calls for bubble/dew lines search
   int getBubbleDewSearchIterationsNumber() const { return m_bdBisecIters; }

   /// \brief Get the total number of PVT flasher calls during contour lines search
   /// \return total number of flasher calls for contour lines search
   int getContourLinesSearchIterationsNumber() const { return m_isoBisecIters; }

   std::vector<double> getGridT() const { return m_gridT; }
   std::vector<double> getGridP() const { return m_gridP; }

private:
   DiagramType m_diagType;           ///< which type of diagram are calculating - Mass/Mole/Volume fraction

   std::vector<double> m_gridT;                  ///< 1D grid for Temperature values
   std::vector<double> m_gridP;                  ///< 1D grid for Pressure values
   
   std::vector< std::vector<double> > m_liqFrac; ///< 2D grid of liquid fraction for P/T grid, -1 - not initialised yet value

   std::vector<double> m_masses;      ///< components mass fraction

   std::vector< std::pair< double,double > > m_bubbleDewLine; ///< Bubble/Dew line
   std::vector< std::pair< double,double > > m_c0p5Line;      ///< 0.5 contour line

   std::vector< std::pair<double,double> >::iterator  m_critPointPos; // position of critical point on bubble/dew line

   double m_eps;                     ///< some small number used in bisection iterations convergence and for mass fraction comparison

   double m_critP;                   ///< critical point P
   double m_critT;                   ///< critical point T

   int    m_bdBisecIters;            ///< total number of PVT flasher calls for bubble dew points search
   int    m_isoBisecIters;           ///< total number of PVT flasher calls contour line points search

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
   /// \param guessP initial P guess for iterations
   /// \param guessT initial T guess for iterations
   /// \param foundP[out] on return, if iterations were successful, it contains bubble or dew point pressure value
   /// \param foundT[out] on return, if iterations were successful, it contains bubble or dew point temperature value
   /// \return true if value was found, false otherwise
   bool doBisectionForContourLineSearch( size_t p1, size_t t1, size_t p2, size_t t2, double frac, double & foundP, double & foundT );

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
   /// \brief p pressure
   /// \brief t temperature
   /// \return phase ID
   int getPhase( size_t p, size_t t );
};

#endif
