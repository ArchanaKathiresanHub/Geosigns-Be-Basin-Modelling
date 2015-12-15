// Copyright 2011, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.
#ifndef EOS_WATER_H
#define EOS_WATER_H

/// \brief  Methods for the water properties
class EosWater
{
public:
   // Constructor
   EosWater( void );

   //Copy constructor; shut off
   EosWater( const EosWater &self );

   // Destructor
   virtual ~EosWater( void ){ }

   // DOCUMENT ME!
   virtual void Water( int iM, int iNc, int iDrv, int iVolume, int iGetV, int iGetT, int iGetH, int iTherm, 
      double *pP, double *pT, double *pS, double *pDSda, double *pRo, double *pDRodp, double *pDRodt,
      double *pDRods, double *pDRoda, double *pMu, double *pDMudp, double *pDMudt, double *pDMuds,
      double *pDMuda, double *pIft, double *pDIftdp, double *pDIftdt, double *pDIftda, double *pHeat,
      double *pDHeatdp, double *pDHeatdt, double *pDHeatds, double *pDHeatda, double *pWork1 );

   // Return the water vapor pressure
   double WaterVaporPressure( double dT );

protected:
   // Get the water density
   void WaterDensity( int iM, int iNc, int iDrv, int iVolume, double *pP, double *pT, double *pS, double *pDSda,
      double *pRo, double *pDRodp, double *pDRodt, double *pDRods, double *pDRoda );

   // Compute water enthalpy
   void   WaterHeat( int iM, int iNc, int iDrv, int iTherm, double *pP, double *pT, 
      double *pS, double *pDSda, double *pHeat, double *pDHeatdp, double *pDHeatdt, 
      double *pDHeatds, double *pDHeatda, double *pWork1 );

   // Set the water/reference phase ift
   void WaterIft( int iM, int iNc, int iDrv, double *pIft, double *pDIftdp, double *pDIftdt, double *pDIftda );

   // Set the water viscosity
   void WaterViscosity( int iM, int iNc, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pDSda,
      double *pRo, double *pDRodp, double *pDRodt, double *pDRods, 
      double *pMu, double *pDMudp, double *pDMudt, double *pDMuds, 
      double *pDMuda, double *pWork1 );

   // Get the water volume
   void WaterVolume( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pRo, double *pDRodp,
      double *pDRodt, double *pDRods, double *pWork1 );

   // Set the salt partial volume
   void SaltVolume( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pVol, double *pDVoldp,
      double *pDVoldt, double *pDVolds );

   // Set the salt partial heat
   void SaltHeat( int iM, int iDrv, int iTherm, double *pP, double *pT, double *pS, double *pHeat, double *pDHeatdp,
      double *pDHeatdt, double *pDHeatds );

   // Get temperature terms for water enthalpy
   void WaterHeatTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms );

   // Get temperature dependent terms for water viscosity
   void WaterViscosityTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms );

   // Get temperature terms for water volume
   void WaterVolumeTerms( int iM, int iDrv, double *pT, double *pWTerms, double *pDTerms );

protected:

   // Pressure conversion constant from MKS units to flasher internal units.  
   // For PSI set to 1/6894.7. Controlled by pdTables[EOS_METHOD_PRESCONV] 
   double m_dConvPressure;

   // Temperature conversion constant from MKS units to flasher internal units.  
   // For R set to 1.8. Controlled by pdTables[EOS_METHOD_TEMPCONV] 
   double m_dConvTemperature;

   // Volume conversion constant from MKS units to flasher internal units.
   // For FT3 set to 35.314667. Controlled by pdTables[EOS_METHOD_VOLCONV]   
   double m_dConvVolume;

   // Viscosity conversion constant from MKS units to flasher internal units.
   // For CP set to 1000. Controlled by pdTables[EOS_METHOD_VISCCONV]              
   double m_dConvViscosity;

   // Density conversion constant from MKS units to flasher internal units.  
   // For LB set to 2.2046226. Controlled by pdTables[EOS_METHOD_DENSCONV]  
   double m_dConvDensity;

   // Heat conversion constant from MKS units to flasher internal units.  
   // For BTU set to 2.2046226. Controlled by pdTables[EOS_METHOD_HEATCONV] 
   double m_dConvHeat;

   // Water/reference phase ift pdTables[EOS_METHOD_WATERIFT] 
   double m_dWaterIft;

   // Water model constant.
   // Controlled by piTables[EOS_METHOD_WATERMODEL]
   // Options are:
   //  EOS_WATER_CORRELATIONS: use correlations
   //  EOS_WATER_LINEAR: use linear model
   int m_iWaterModel;

   //--------------------------------------------------------------------// 
   // Terms associated with the calculation of water DENSITY, viz.:
   // DENS = DENS0 * ( 1 + DENSDP * P ) * ( 1 + DENSDT * T ) + S *
   //      ( DENS0DS * ( 1 + DENSDPDS * P ) * ( 1 + DENSDTDS * T ) )
   //--------------------------------------------------------------------// 

   // Water base density constant.  
   // First element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDens;

   // Water density pressure constant term. 
   // Second element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDp;

   // Water density temperature constant term.
   // Third element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDt;

   // Water base density salinity derivative. 
   // Eigth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDs;

   // Water density pressure term salinity derivative.
   // Ninth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDpDs;

   // Water density temperature term salinity derivative.
   // Tenth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterDensDtDs;

   //--------------------------------------------------------------------// 
   // Terms associated with the calculation of water VISCOSITY, viz.:
   // VISC = VISC0 * ( 1 + VISCDP * P ) * ( 1 + VISCDT * T ) + S *
   //      ( VISCDS * ( 1 + VISCDPDS * P ) * ( 1 + VISCDTDS * T ) )
   //--------------------------------------------------------------------// 

   // Water base viscosity.
   // Fourth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterVisc;

   // Water viscosity pressure term. 
   // Fifth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDp;

   // Water viscosity temperature term.
   // Sixth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDt;

   // Water base viscosity salinity derivative. 
   // Eleventh element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDs;

   // Water viscosity pressure term salinity derivative.
   // Twelfth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDpDs;

   // Water viscosity temperature term salinity derivative.
   // Thirteenth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterViscDtDs;

   //--------------------------------------------------------------------// 
   // Terms associated with the calculation of water ENTHALPY, viz.:
   // ENTH = ENTHDT * T + S * ENTHDTDS * T 
   //--------------------------------------------------------------------// 

   // Water enthalpy term. 
   // Seventh element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterHeat;

   // Water enthalpy term salinity derivative.
   // Fourteenth element of array controlled by pRArray[EOS_METHOD_WATERTERMS]
   double m_dWaterHeatDs;
};

#endif
