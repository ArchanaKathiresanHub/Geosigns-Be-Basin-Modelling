#ifdef sgi
#include <iostream.h>
#else
#include <iostream>
#endif

#include <math.h>

#include "globaldefs.h"
#include "lithoprops.h"

#include "utils.h"


LithoProps::~LithoProps() {
  m_lithoComponents.clear();
}

LithoComposition LithoProps::getComposition () const {

  double fraction1;
  double fraction2;
  double fraction3;

  std::string simpleName1;
  std::string simpleName2;
  std::string simpleName3;
  std::string mixingModel;

  if ( m_mixmodeltype == HOMOGENEOUS ) {
    mixingModel = "Homogeneous";
  } else if ( m_mixmodeltype == LAYERED ) {
    mixingModel = "Layered";
  } else {
    mixingModel = "Undefined";
  }

  if ( m_lithoComponents.size () == 1 ) {
    fraction1 = m_componentPercentage [ 0 ];
    fraction2 = 0.0;
    fraction3 = 0.0;
    simpleName1 = m_lithoComponents [ 0 ]->lithoname();
    simpleName2 = "";
    simpleName3 = "";
  } else if ( m_lithoComponents.size () == 2 ) {
    fraction1 = m_componentPercentage [ 0 ];
    fraction2 = m_componentPercentage [ 1 ];
    fraction3 = 0.0;
    simpleName1 = m_lithoComponents [ 0 ]->lithoname();
    simpleName2 = m_lithoComponents [ 1 ]->lithoname();
    simpleName3 = "";
  } else if ( m_lithoComponents.size () == 3 ) {
    fraction1 = m_componentPercentage [ 0 ];
    fraction2 = m_componentPercentage [ 1 ];
    fraction3 = m_componentPercentage [ 2 ];
    simpleName1 = m_lithoComponents [ 0 ]->lithoname();
    simpleName2 = m_lithoComponents [ 1 ]->lithoname();
    simpleName3 = m_lithoComponents [ 2 ]->lithoname();
  } else {
    fraction1 = 0.0;
    fraction2 = 0.0;
    fraction3 = 0.0;
    simpleName1 = "";
    simpleName2 = "";
    simpleName3 = "";
  }

  return LithoComposition ( simpleName1, simpleName2, simpleName3, fraction1, fraction2, fraction3, mixingModel );
}


void LithoProps::addLithology(SimpleLitho* a_lithology, const double a_percentage) {
  m_lithoComponents.push_back(a_lithology);
  m_componentPercentage.push_back(a_percentage);
}

void LithoProps::createThCondTbl() {


  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  int Table_Size;
  int Array_Size = 99999;

  double anisotropy;
  anisotropy = (*componentIter)->getThCondAn();
  
  double ThermalCondN,ThermalCondP;

  vector<SimpleLitho::XY> mixThermCondNTbl , mixThermCondPTbl;
  vector<SimpleLitho::XY> thermCondTbl = (*componentIter)->getThermCondTbl();
  vector<SimpleLitho::XY>::iterator XYIter = thermCondTbl.begin();

  Table_Size = thermCondTbl.size();
  Array_Size = min( Array_Size, Table_Size );

  while (thermCondTbl.end() != XYIter) {

    if (m_mixmodeltype == HOMOGENEOUS) {
      ThermalCondN = ThermalCondP = 1.0;
      ThermalCondN *= pow((*XYIter).getY(),(*percentIter)*0.01);
      ThermalCondP *= pow((*XYIter).getY()*anisotropy,(*percentIter)*0.01);
    } else if (m_mixmodeltype == LAYERED) {
      ThermalCondN = ThermalCondP = 0.0;
      ThermalCondN += (*percentIter)*0.01 / (*XYIter).getY();
      ThermalCondP += (*percentIter)*0.01 * (*XYIter).getY() * anisotropy;
    }

    SimpleLitho::XY xyPoint((*XYIter).getX(),ThermalCondN);
    mixThermCondNTbl.push_back(xyPoint);
    xyPoint.set((*XYIter).getX(),ThermalCondP);
    mixThermCondPTbl.push_back(xyPoint);
    ++XYIter;
  }

  ++componentIter;
  ++percentIter;

  while (m_lithoComponents.end() != componentIter) {
    //The following line should be uncommented in FastCauldron Release - OFM
    anisotropy = (*componentIter)->getThCondAn();
    thermCondTbl = (*componentIter)->getThermCondTbl();
    Table_Size = thermCondTbl.size();
    Array_Size = min( Array_Size, Table_Size );
    XYIter = thermCondTbl.begin();
    vector<SimpleLitho::XY>::iterator mixXYnIter = mixThermCondNTbl.begin();
    vector<SimpleLitho::XY>::iterator mixXYpIter = mixThermCondPTbl.begin();

    while ( (thermCondTbl.end() != XYIter) && (mixThermCondNTbl.end() != mixXYnIter) ) {
      ThermalCondN = (*mixXYnIter).getY();
      ThermalCondP = (*mixXYpIter).getY();

      if (m_mixmodeltype == HOMOGENEOUS) {
	ThermalCondN *= pow((*XYIter).getY(),(*percentIter)*0.01);
	ThermalCondP *= pow((*XYIter).getY()*anisotropy,(*percentIter)*0.01);
      } else if (m_mixmodeltype == LAYERED) {
	ThermalCondN += (*percentIter)*0.01 / (*XYIter).getY();
	ThermalCondP += (*percentIter)*0.01 * (*XYIter).getY() * anisotropy;
      }
      (*mixXYnIter).setY(ThermalCondN);
      (*mixXYpIter).setY(ThermalCondP);
      ++XYIter;
      ++mixXYnIter;
      ++mixXYpIter;
    }
    ++componentIter;
    ++percentIter;
  }

  // write into the interpolator
  int Counter = 0;
  vector<SimpleLitho::XY>::iterator mixXYnIter = mixThermCondNTbl.begin();
  for ( mixXYnIter = mixThermCondNTbl.begin(); mixXYnIter != mixThermCondNTbl.end();
	mixXYnIter++ ) {
    if ( ++Counter > Array_Size ) continue;
    if (m_mixmodeltype == LAYERED && (*mixXYnIter).getY() != 0.0) {
      addThermCondPointN((*mixXYnIter).getX(),1 / (*mixXYnIter).getY());
    } else {
      addThermCondPointN((*mixXYnIter).getX(),(*mixXYnIter).getY());      
    }
  }

  Counter = 0;
  vector<SimpleLitho::XY>::iterator mixXYpIter = mixThermCondPTbl.begin();
  for ( mixXYpIter = mixThermCondPTbl.begin(); mixXYpIter != mixThermCondPTbl.end();
	mixXYpIter++ ) {
    if ( ++Counter > Array_Size ) continue;
    addThermCondPointP((*mixXYpIter).getX(),(*mixXYpIter).getY());
  }

  while (mixThermCondNTbl.size() != 0) {
    vector<SimpleLitho::XY>::iterator mixXYnIter = mixThermCondNTbl.begin();
    mixThermCondNTbl.erase(mixXYnIter);
  }
  while (mixThermCondPTbl.size() != 0) {
    vector<SimpleLitho::XY>::iterator mixXYpIter = mixThermCondPTbl.begin();
    mixThermCondPTbl.erase(mixXYpIter);
  }

//   while (mixThermCondNTbl.size() != 0) {
//     vector<SimpleLitho::XY>::iterator mixXYnIter = mixThermCondNTbl.begin();
//     if (m_mixmodeltype == LAYERED && (*mixXYnIter).getY() != 0.0) {
//       addThermCondPointN((*mixXYnIter).getX(),1 / (*mixXYnIter).getY());
//     } else {
//       addThermCondPointN((*mixXYnIter).getX(),(*mixXYnIter).getY());      
//     }
//     mixThermCondNTbl.erase(mixXYnIter);
//   }
//   while (mixThermCondPTbl.size() != 0) {
//     vector<SimpleLitho::XY>::iterator mixXYpIter = mixThermCondPTbl.begin();
//     addThermCondPointP((*mixXYpIter).getX(),(*mixXYpIter).getY());
//     mixThermCondPTbl.erase(mixXYpIter);
//   }

}

double LithoProps::Exponential_Decompaction_Function( const double VES ) {

  if ( m_compactionincr <= 0.0 ) return 0.0;

  double  r1 = 1.0 - m_depoporosity * exp ( -VES * m_compactionincr );
  double  r2 = log ( r1 );
  double  r3 = VES + r2 / m_compactionincr;

  return r3;

}

LithoProps::LithoProps () {

 m_mixmodeltype = HOMOGENEOUS;
 Lithology_Fracture_Gradient = 1.0;
 m_isFaultLithology = false;

}

void LithoProps::makeFault ( const bool isFault ) {
 m_isFaultLithology = isFault;
}


double LithoProps::heatcapacity(double t) {

  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  double heatcapacity = 0;
  while (m_lithoComponents.end() != componentIter) {
    heatcapacity += (*componentIter)->heatcapacity(t) * (*percentIter)/100;
    ++componentIter;
    ++percentIter;
  }

  return heatcapacity;
}

double LithoProps::densityXheatcapacity(double t) {

  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  double heatcapacity = 0;
  while (m_lithoComponents.end() != componentIter) {
    heatcapacity += (*componentIter)->heatcapacity(t) * (*componentIter)->getDensity()  * (*percentIter)/100;
    ++componentIter;
    ++percentIter;
  }

  return heatcapacity; 
}


double LithoProps::hydrostatFullCompThickness(double maxVes, 
					      double thickness, 
					      double densitydiff,
					      bool   Overpressured_Compaction ) {

  /*This module is based on Lithology::CalcFullCompactedThickness method of Cauldron */

  /* if the compaction coefficient is nil then the full compacted thickness = thickness */

  ///
  /// If running an overpressure calculation then make an estimate how much 
  /// overpressure there might be. More overpressure implies less VES. The scaling
  /// factor is based on the amount of VES there would be if the basin was hydrostatically
  /// pressured.
  ///
  const double VESScaleFactor = 0.5;

  double c1;
  double c2;
  double Porosity;

  double MaxVesValue;
  double Solid_Thickness;

  if ( Overpressured_Compaction ) {
    MaxVesValue = maxVes * VESScaleFactor;
  } else {
    MaxVesValue = maxVes;
  }

  if ( m_Porosity_Model == EXPONENTIAL ) {

    if ( m_compactionincr == 0 ) {
      Solid_Thickness = thickness;
    } else {
      c1 = GRAVITY * densitydiff * m_compactionincr;

      c2 = ( 1.0 - exp( -c1 * thickness ) ) * m_depoporosity 
        * exp( -m_compactionincr * MaxVesValue );

      Solid_Thickness = log( 1.0 - c2 ) / c1 + thickness;
    } // end if

  } else {

    if ( m_Compaction_Coefficient_SM == 0.0 ) {
      Solid_Thickness = thickness;
    } else {

      const bool Loading_Phase               = true;
      const bool Include_Chemical_Compaction = false;

      //
      // Integrate the ODE using a 4th Order RK method, with N=2 intervals.
      //
      const int N = 2;
      int I;
      double K1, K2, K3, K4;
      double Phi;
      double VES;
      double VES_Top;
      double FCT;
      double VES_Scaling;

      const double H = thickness / double ( N );

      ///
      /// If we are initialising the model for an Overpressure run
      /// then we assume some overpressure. An amount that equates to VES = 0.5 * ( Pl - Ph )
      ///
      if ( Overpressured_Compaction ){
        VES_Scaling = VESScaleFactor;
      } else {
        VES_Scaling = 1.0;
      } // 

      Solid_Thickness = 0.0;
      VES_Top = MaxVesValue;
      //
      //
      // Porosity at top of the segment.
      //
      Porosity = Soil_Mechanics_Porosity_Function ( MaxVesValue, MaxVesValue, Loading_Phase, Include_Chemical_Compaction );

      for ( I = 1; I <= N; I++ ) {
        Phi = Porosity;

        K1 = ( 1.0 - Phi ) * H;
        VES = VES_Top + 0.5 * VES_Scaling * GRAVITY * densitydiff * K1;

        Phi = Soil_Mechanics_Porosity_Function ( VES, VES, Loading_Phase, Include_Chemical_Compaction );
//          Phi = porosity ( VES, VES, Include_Chemical_Compaction, chemicalCompactionDummy );

        K2 = ( 1.0 - Phi ) * H;
        VES = VES_Top + 0.5 * VES_Scaling * GRAVITY * densitydiff * K2;
        Phi = Soil_Mechanics_Porosity_Function ( VES, VES, Loading_Phase, Include_Chemical_Compaction );
//          Phi = porosity ( VES, VES, Include_Chemical_Compaction, chemicalCompactionDummy );

        K3 = ( 1.0 - Phi ) * H;
        VES = VES_Top + VES_Scaling * GRAVITY * densitydiff * K3;
        Phi = Soil_Mechanics_Porosity_Function ( VES, VES, Loading_Phase, Include_Chemical_Compaction );
//          Phi = porosity ( VES, VES, Include_Chemical_Compaction, chemicalCompactionDummy );

        K4 = ( 1.0 - Phi ) * H;
        FCT = ( K1 + 2.0 * ( K2 + K3 ) + K4 ) / 6.0;
  
        Solid_Thickness = Solid_Thickness + FCT;

        if ( I != N ) {
          VES_Top = VES_Top + VES_Scaling * GRAVITY * densitydiff * FCT;
          Phi = Soil_Mechanics_Porosity_Function ( VES_Top, VES_Top, Loading_Phase, Include_Chemical_Compaction );
//          Phi = porosity ( VES, VES, Include_Chemical_Compaction, chemicalCompactionDummy );
        }

      }

    }

  }

  return Solid_Thickness;

}


bool   LithoProps::isincompressible() { 

  compContainer::iterator componentIter = m_lithoComponents.begin();
  bool isincompressible = true;
  while (m_lithoComponents.end() != componentIter) {
    isincompressible &= (*componentIter)->isincompressible();
    ++componentIter;
  }

  return isincompressible; 
}


double LithoProps::porosity ( const double sigma, 
                              const double sigma_max, 
                              const bool   Include_Chemical_Compaction, 
                              const double Chemical_Compaction_Term ) const {

  double Porosity;
  bool   Loading_Phase = ( sigma >= sigma_max );

  if ( m_Porosity_Model == EXPONENTIAL ) {
    Porosity = Exponential_Porosity_Function ( sigma, sigma_max, Loading_Phase, Include_Chemical_Compaction );
  } else {
    Porosity = Soil_Mechanics_Porosity_Function ( sigma, sigma_max, Loading_Phase, Include_Chemical_Compaction );
  }

  if ( Include_Chemical_Compaction ) {
    Porosity = Porosity + Chemical_Compaction_Term;
    Porosity = Double_Max ( Porosity, Minimum_Porosity );
  }

  return Porosity;
}



double LithoProps::Void_Ratio ( const double VES,
                                const double Max_VES,
                                const bool   Include_Chemical_Compaction,
                                const double Chemical_Compaction_Term ) {

  double Computed_Porosity = porosity ( VES, Max_VES, Include_Chemical_Compaction, Chemical_Compaction_Term );

  return Computed_Porosity / ( 1.0 - Computed_Porosity );

}


bool LithoProps::Allowable_Mixing () const {

  bool Mixing_Okay = true;

  compContainer::const_iterator    Simple_Lithologies = m_lithoComponents.begin();
  percentContainer::const_iterator Percentages        = m_componentPercentage.begin();

  if ( m_lithoComponents.size () > 1 ) {
    PorosityModelType First_Porosity_Model;
    bool              First_Active_Lithology = true;

    while (m_lithoComponents.end() != Simple_Lithologies ) {

      if ( *Percentages > 0.0 ) {

        if ( First_Active_Lithology ) {
          First_Porosity_Model = (*Simple_Lithologies) -> getPorosityModel ();
          First_Active_Lithology = false;
        } else if ( First_Porosity_Model != (*Simple_Lithologies) -> getPorosityModel ()) {
          Mixing_Okay = false;
        }

      }

      ++Simple_Lithologies;
      ++Percentages;
    }

  }

  if ( ! Mixing_Okay ) {
    Simple_Lithologies = m_lithoComponents.begin();
    Percentages        = m_componentPercentage.begin();

    cout << "****************  ERROR  Incorrect mixing  ****************" << endl;
    cout << "   Lithotype           Porosity model   percentage" << endl;

    while (m_lithoComponents.end() != Simple_Lithologies ) {
      cout << setw ( 20 ) << setiosflags(ios::left) <<  (*Simple_Lithologies)->lithoname () << resetiosflags(ios::left) 
           << setw ( 18 ) << PorosityModelTypeImage [(*Simple_Lithologies)->getPorosityModel ()]
           << setw ( 10 ) << (*Percentages) << endl;
      
      ++Simple_Lithologies;
      ++Percentages;
    }



  }


  return Mixing_Okay;
}


bool  LithoProps::reCalcProperties(){

  if ( ! Allowable_Mixing ()) {
    return false;
  }

  m_density = 0.0;
//    m_permeabilityaniso = 0.0;
  m_seismicvelocity = 0.0;

//    m_depoporosity = 0.0;
//    m_compactionincr = 0.0;
//    m_compactiondecr = 0.0;
//    m_compactionfact = 0.0;

  m_depopermeability = 0.0;
  m_heatproduction = 0.0; 
  m_permeabilityincr = 0.0;
  m_permeabilitydecr = 0.0;
  m_thermalcondaniso = 1.0;
  m_thermalconductivityval = 0.0;

  // loop through all the simple lithologies and calculate the
  // properties for this lithology
  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  Reference_Solid_Viscosity   = 0.0;
  Lithology_Activation_Energy = 0.0;
  Lithology_Fracture_Gradient = 0.0;
  Minimum_Mechanical_Porosity = 0.0;

  while (m_lithoComponents.end() != componentIter) {
    double pcMult = (double)(*percentIter)/100;

    // Matrix Property calculated using the arithmetic mean
    m_density                += (*componentIter)->getDensity()  * pcMult;

//      m_depoporosity           += (*componentIter)->getDepoPoro() * pcMult;

//      m_compactionincr         += (*componentIter)->getCompIncr() * pcMult; 
//      m_compactiondecr         += (*componentIter)->getCompDecr() * pcMult;
//      m_compactionfact         += (*componentIter)->getCompFact() * pcMult;

    m_depopermeability       += (*componentIter)->getDepoPerm() * pcMult;
    m_thermalconductivityval += (*componentIter)->getThCondVal() * pcMult;
    m_heatproduction         += (*componentIter)->getHeatProduction() * pcMult;
    m_seismicvelocity        += (*componentIter)->getSeismicVelocity() * pcMult;

    // Matrix Property calculated using the geometric mean
    m_thermalcondaniso       *= pow ((*componentIter)->getThCondAn(),pcMult);

    Reference_Solid_Viscosity   = Reference_Solid_Viscosity   + pcMult * (*componentIter)->Get_Reference_Solid_Viscosity ();
    Lithology_Activation_Energy = Lithology_Activation_Energy + pcMult * (*componentIter)->Get_Lithology_Activation_Energy ();
    Minimum_Mechanical_Porosity = Minimum_Mechanical_Porosity + pcMult * (*componentIter)->Get_Minimum_Mechanical_Porosity ();
    Lithology_Fracture_Gradient = Lithology_Fracture_Gradient + pcMult * (*componentIter)->Get_Lithology_Fracture_Gradient ();

    ++componentIter;
    ++percentIter;
  }

  Mix_Porosity_Model();
  Mix_Surface_Porosity ();
  Mix_Compaction_Coefficients ();
  reSetBehaviorForHysteresis();
  createThCondTbl();

//    Reference_Solid_Viscosity = 25.0;//1.0e17; // GPa . Ma
//    Lithology_Activation_Energy = 4.0e4; //    J / mol

//    cout << " Lithology_Activation_Energy " << m_lithology_id << "  " << Lithology_Activation_Energy << endl;

  if ( m_depoporosity < 0.0299 ) { 
    // Really less than 0.03 but some rounding may occur if user set a litho with 0.03 surface porosity
    Fractured_Permeability_Scaling_Value = 100.0;
  } else {
    Fractured_Permeability_Scaling_Value = 5.0;
  }

  return true;
}


//------------------------------------------------------------//


void LithoProps::Mix_Surface_Porosity() {

  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  m_depoporosity = 0.0;

  if ( m_Porosity_Model == SOIL_MECHANICS ) {
    double Simple_Surface_Porosity;
    double Simple_Void_Ratio;
    double Mixed_Void_Ratio = 0.0;

    while (m_lithoComponents.end() != componentIter) {

      Simple_Surface_Porosity =  (*componentIter)->getDepoPoro();
      Simple_Void_Ratio = Simple_Surface_Porosity / ( 1.0 - Simple_Surface_Porosity );
      Mixed_Void_Ratio = Mixed_Void_Ratio + Simple_Void_Ratio * (*percentIter) * 0.01;

      ++componentIter;
      ++percentIter;
    }

    m_depoporosity = Mixed_Void_Ratio / ( 1.0 + Mixed_Void_Ratio );
  } else {

    /* Dominant lithotype defines Porosity Model and Compaction Coefficient value */
    while (m_lithoComponents.end() != componentIter) {

      m_depoporosity = m_depoporosity  + (*componentIter)->getDepoPoro() * (*percentIter) * 0.01;

      ++componentIter;
      ++percentIter;
    }

  }


}


//------------------------------------------------------------//


void LithoProps::Mix_Compaction_Coefficients () {

  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  double Fraction;

  m_compactionincr = 0.0;
  m_compactiondecr = 0.0;
//    m_compactionfact = 0.0;
  m_Compaction_Coefficient_SM = 0.0;
  //
  //
  // As soon as we can be sure that the different lithology types DO NOT use 
  // the compaction coefficient of the other (in computing FCT, ...) then
  // we can uncomment the code that is commented out here.
  //
//    if ( m_Porosity_Model == SOIL_MECHANICS ) {

    while (m_lithoComponents.end() != componentIter) {
      Fraction = 0.01 * (*percentIter);

      m_Compaction_Coefficient_SM = m_Compaction_Coefficient_SM + (*componentIter)->getCompaction_Coefficient_SM() * Fraction;
      m_compactionincr = m_compactionincr + (*componentIter)->getCompIncr() * Fraction; 
      m_compactiondecr = m_compactiondecr + (*componentIter)->getCompDecr() * Fraction;
//        m_compactionfact = m_compactionfact + (*componentIter)->getCompFact() * Fraction;

      ++componentIter;
      ++percentIter;
    }

//    } else {

//      while (m_lithoComponents.end() != componentIter) {

//        Fraction = 0.01 * (*percentIter);

//        m_Compaction_Coefficient_SM = m_Compaction_Coefficient_SM + (*componentIter)->getCompaction_Coefficient_SM() * Fraction;
//        m_compactionincr = m_compactionincr + (*componentIter)->getCompIncr() * Fraction; 
//        m_compactiondecr = m_compactiondecr + (*componentIter)->getCompDecr() * Fraction;
//        m_compactionfact = m_compactionfact + (*componentIter)->getCompFact() * Fraction;

//        ++componentIter;
//        ++percentIter;
//      }

//    } 

}


//------------------------------------------------------------//


void   LithoProps::Mix_Porosity_Model() {

  double Largest_Volume_Fraction = -100.0;

  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  /* Dominant lithotype defines Porosity Model and Compaction Coefficient value */
  while (m_lithoComponents.end() != componentIter) {

    if ( (double)(*percentIter) > Largest_Volume_Fraction ) {

      Largest_Volume_Fraction = (double)(*percentIter);

      m_Porosity_Model = (*componentIter)->getPorosityModel();

    }

    ++componentIter;
    ++percentIter;
    
  }

 
}

void   LithoProps::reSetBehaviorForHysteresis() {

  double VolFrac = 0.0, SandVolFrac = 0.0, ShaleVolFrac = 0.0;
  bool   NoPerm = false;

  // loop through all the simple lithologies and calculate the
  // properties for this lithology
  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();
  while (m_lithoComponents.end() != componentIter) {
    VolFrac = (double)(*percentIter)/100;

  switch ((*componentIter)->getPermModel())
    {
    case SAND: {
      SandVolFrac += VolFrac;
      break;
    }
    case SHALE: {
      ShaleVolFrac += VolFrac;
      break;
    }
    case MULTI_POINT: {
      ShaleVolFrac += VolFrac;
      break;
    }
    case IMPERMEABLE: {
      NoPerm = true;
      break;
    }
    case NONE: {
      NoPerm = true;
      break;
    }
    default: {
      IBSASSERT(false);
    }
    }    
    ++componentIter;
    ++percentIter;
  }

  if (m_mixmodeltype == HOMOGENEOUS) {
    if (!NoPerm)
      {
	if (SandVolFrac > 50.0)
	  {
            m_hysteresisbehaviour = SAND_BEHAVIOR;
	  }
	else
	  {
            m_hysteresisbehaviour = SHALE_BEHAVIOR;
	  }
      }
    else
      {
	m_hysteresisbehaviour = NO_PERM_BEHAVIOR;
      }
  } else if (m_mixmodeltype == LAYERED) {
    if (!NoPerm)
      {
	if (ShaleVolFrac > 0.0)
	  {
            m_hysteresisbehaviour = SHALE_BEHAVIOR;
	  }
	else
	  {
            m_hysteresisbehaviour = SAND_BEHAVIOR;
	  }
      }
    else
      {
	m_hysteresisbehaviour = NO_PERM_BEHAVIOR;
      }
  }  
}

void LithoProps::setMixModel(string mixmodel) {
  if (mixmodel == "Homogeneous") {
    m_mixmodeltype = HOMOGENEOUS;
  } else if (mixmodel == "Layered") {
    m_mixmodeltype = LAYERED;
  } else {
    m_mixmodeltype = UNDEFINED;
    cout << "mixmodel not defined: " << mixmodel << endl;
  }
}

double LithoProps::Compute_Segment_Thickness ( const double Top_Max_VES, 
					       const double Bot_Max_VES,
					       const double Density_Difference ) {

  if ( m_Porosity_Model == EXPONENTIAL ) {

    double d1 = Exponential_Decompaction_Function( Top_Max_VES );
    
    double d2 = Exponential_Decompaction_Function( Bot_Max_VES );
    
    return ( d2 - d1 ) / ( GRAVITY * Density_Difference );

  } else {

    return Numerical_Decompaction ( Top_Max_VES, Bot_Max_VES, Density_Difference )
      / ( GRAVITY * Density_Difference );

  }

}

double LithoProps::Compute_Segment_Thickness ( const double Top_Max_VES, 
					       const double Bot_Max_VES,
					       const double Top_VES, 
					       const double Bot_VES,
					       const double Density_Difference ) {

  double Psi_Top    = Void_Ratio ( Top_VES, Top_Max_VES, false, 0.0 );
  double Psi_Bottom = Void_Ratio ( Bot_VES, Bot_Max_VES, false, 0.0 );


//    double Psi_Top = Compute_Void_Ratio ( Top_VES, Top_Max_VES, m_depoporosity, 
//  					m_compactionincr, m_compactiondecr, 
//  					( Top_VES >= Top_Max_VES ) );
//    double Psi_Bottom = Compute_Void_Ratio ( Bot_VES, Bot_Max_VES, m_depoporosity, 
//  					m_compactionincr, m_compactiondecr, 
//  					( Bot_VES >= Bot_Max_VES ) );


  return ( 1 + ( Psi_Bottom + Psi_Top ) / 2.0 );

}

double LithoProps::Numerical_Decompaction ( const double Top_Max_VES, 
					    const double Bot_Max_VES, 
					    const double Density_Difference ) {

  const bool Loading_Phase               = true;
  const bool Include_Chemical_Compaction = false;

  double Result = 0.0;

  /* computes the integral using an N points ( N = 3 or 5 ) Gauss-Legendre quadrature */

  /* 5 points Gauss-Legendre quadrature */

  const int    Number_Of_Sections = 3;

  const double Points   [ Number_Of_Sections ] = { -0.7745966692, 
						    0.0000000000, 
						    0.7745966692 };

  const double Weights  [ Number_Of_Sections ] = { 0.555555555555, 
						   0.888888888888, 
						   0.555555555555 };

  /* 5 points Gauss-Legendre quadrature

  const int    Number_Of_Sections = 5;

  const double Points   [ Number_Of_Sections ] = { -0.9061798459, 
        					   -0.5384693101, 
						    0.0000000000,   
						    0.5384693101,  
						    0.9061798459 }; 
  
  const double Weights  [ Number_Of_Sections ] = { 0.2369268850,  
						   0.4786286705,  
						   0.5688888888888888,  
						   0.4786286705 , 
						   0.2369268850  };

  */

  const double BmT = Bot_Max_VES - Top_Max_VES; // BmT = Bottom minus Top
  const double BpT = Bot_Max_VES + Top_Max_VES; // BpT = Bottom plus Top

  int    I;
  double VES;
  double Porosity;

  for ( I = 0; I < Number_Of_Sections; I++ ) {

    VES = 0.5 * ( BmT * Points [ I ] + BpT );

    Porosity = Soil_Mechanics_Porosity_Function ( VES, VES, Loading_Phase, Include_Chemical_Compaction );

//      Porosity = Compute_Porosity ( VES, VES, m_depoporosity, 
//  				  m_compactionincr, m_compactionincr, true );

    Result = Result + Weights [ I ] / ( 1.0 - Porosity ); 

  }
  
  Result = 0.5 * BmT * Result;

  return Result;

}

void LithoProps::calcBulkDensXHeatCapacity ( FluidProps *Fluid, 
                                             const double Depth, 
                                             const double Porosity,
                                             const double Temperature, 
                                                   double& BulkDensXHeatCapacity ) {

  bool LithoHasFluid = false;
  if (Fluid != (FluidProps*)0) LithoHasFluid = true;

  double MatrixDensXHeatCap = densityXheatcapacity(Temperature);
  
  if (LithoHasFluid) {
    
    double Pressure = Depth * GRAVITY * Fluid->getSimpleDensity() * Pa_To_MPa;
    
    double FluidDensXHeatCap = Fluid->densXheatcapacity(Temperature,Pressure);
    
    BulkDensXHeatCapacity = (MatrixDensXHeatCap * (1.0 - Porosity))
      + (FluidDensXHeatCap * Porosity);

  } 
  else {

    BulkDensXHeatCapacity = MatrixDensXHeatCap;

  }
}


void LithoProps::calcBulkDensXHeatCapacity_Coupled (       FluidProps *Fluid,
                                                     const double  Porosity, 
                                                     const double  Pressure, 
                                                     const double  Temperature, 
                                                           double& BulkDensXHeatCapacity ) {

  bool LithoHasFluid = (Fluid != (FluidProps*)0);

  double MatrixDensXHeatCap = densityXheatcapacity(Temperature);
  
  if (LithoHasFluid) {
    
    double FluidDensXHeatCap = Fluid->densXheatcapacity(Temperature,Pressure);
    
    BulkDensXHeatCapacity = MatrixDensXHeatCap * (1.0 - Porosity) + FluidDensXHeatCap * Porosity;

  } else {
    //
    //
    // Should this be scaled by ( 1.0 - Porosity ) ?
    //
    BulkDensXHeatCapacity = MatrixDensXHeatCap;

  }
}


void LithoProps::calcBulkDensity ( FluidProps *Fluid,
                                   const double Porosity,
                                   const double porePressure,
                                   const double temperature,
                                         double &BulkDensity ) {

  bool LithoHasFluid = false;
  if (Fluid != (FluidProps*)0) LithoHasFluid = true;

  double MatrixDensity = m_density;
  
  if (LithoHasFluid) {
    
    double FluidDensity = Fluid->density ( temperature, porePressure );
    
    BulkDensity = (MatrixDensity * (1.0 - Porosity))
      + (FluidDensity * Porosity);

  } 
  else {

    BulkDensity = MatrixDensity;

  }
}

void LithoProps::calcVelocity ( FluidProps *Fluid, 
                                const string& algorithmName, 
                                const double  Porosity, 
                                const double  BulkDensity, 
                                const double  porePressure, 
                                const double  temperature, 
                                      double& Velocity ) {

   if ( algorithmName == GardnersVelocityDensityAlgorithm ) {
   
     Velocity = pow ( BulkDensity / GardnerVelocityConstant, 4 );

   } else if ( algorithmName == WylliesTimeAverageAlgorithm ) {

     if ( Fluid != 0 ) {
       double FluidVelocity;

       FluidVelocity = Fluid->seismicvelocity ( temperature, porePressure );
       Velocity = ( FluidVelocity * m_seismicvelocity ) / ( Porosity * m_seismicvelocity + ( 1.0 - Porosity ) * FluidVelocity );
     } else {
       Velocity = m_seismicvelocity;
     }
     
   }

}

void LithoProps::calcBulkPermeabilityNP( const double  Ves, 
					 const double  MaxVes, 
					 const double  Porosity, 
					 double&       BulkPermeabilityN, 
					 double&       BulkPermeabilityP ) 
{

  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();
  
  double PreviousPermN = 1.0E+08;
  
  if (m_mixmodeltype == HOMOGENEOUS || m_isFaultLithology ) {
    BulkPermeabilityN = 1.0;
    BulkPermeabilityP = 1.0;
  } else if (m_mixmodeltype == LAYERED) {
    BulkPermeabilityN = 0.0;
    BulkPermeabilityP = 0.0;
  }
  
  while (m_lithoComponents.end() != componentIter) {
    double VolFrac = (double)(*percentIter)*0.01;
    double PermVal = (*componentIter)->permeability(Ves,MaxVes,Porosity);
    double PermAniso = (*componentIter)->getPermAniso();
    
    if (m_mixmodeltype == HOMOGENEOUS || m_isFaultLithology ) {
      BulkPermeabilityN *= pow(PermVal,VolFrac);
      BulkPermeabilityP *= pow(PermVal*PermAniso,VolFrac);
    } else if (m_mixmodeltype == LAYERED) {

      if (PreviousPermN < PermVal) {
	BulkPermeabilityN = PreviousPermN;
	BulkPermeabilityP = PreviousPermN * PermAniso;
      } else {
	BulkPermeabilityN = PermVal;
	BulkPermeabilityP = PermVal * PermAniso;
      }

    }

    PreviousPermN = BulkPermeabilityN;
    ++componentIter;
    ++percentIter;
  }

  
  BulkPermeabilityN = Double_Min ( BulkPermeabilityN, 1000.0 );
  BulkPermeabilityP = Double_Min ( BulkPermeabilityP, 1000.0 );

  BulkPermeabilityN *= MILLIDARCYTOM2;
  BulkPermeabilityP *= MILLIDARCYTOM2;
  
}

void LithoProps::calcBulkThermCondNP(FluidProps *Fluid, 
				      double Porosity, double Temperature, 
				      double &BulkTHCondN, double &BulkTHCondP) {

  bool LithoHasFluid = false;
  if (Fluid != (FluidProps*)0) LithoHasFluid = true;

  double MatrixTHCondN = thermalconductivityN(Temperature);
  double MatrixTHCondP = thermalconductivityP(Temperature);

  double FluidThCond = 1.0;
  if (LithoHasFluid) {
    FluidThCond = Fluid->thermalconductivity(Temperature);
  }

  BulkTHCondN = pow(MatrixTHCondN, 1.0 - Porosity) * pow(FluidThCond,Porosity);
  BulkTHCondP = pow(MatrixTHCondP, 1.0 - Porosity) * pow(FluidThCond,Porosity);

}

void LithoProps::calcBulkHeatProd(double Porosity, double &BulkHeatProd) {

  BulkHeatProd = (1.0 - Porosity) * m_heatproduction * W_IN_mW;

}

double LithoProps::Reference_Effective_Stress () const {

  /* This may be some input value in the future */
  /* OR  Computed from the lithomatrix components? */

  return 1.0E+05;

}

double LithoProps::Reference_Void_Ratio () const {

  return m_depoporosity / ( 1.0 - m_depoporosity );

}

double LithoProps::Compute_D_Void_Ratio_DP ( const double VES, 
					     const double Max_VES,
                                             const bool   Include_Chemical_Compaction,
                                             const double Chemical_Compaction_Term ) {


  const bool Loading_Phase = VES >= Max_VES;

  if ( m_Porosity_Model == EXPONENTIAL ) {
    return Exponential_D_Void_Ratio_DP    ( VES, Max_VES, Loading_Phase, Include_Chemical_Compaction, Chemical_Compaction_Term );
  } else {
    return Soil_Mechanics_D_Void_Ratio_DP ( VES, Max_VES, Loading_Phase, Include_Chemical_Compaction, Chemical_Compaction_Term );
  } 

} 

//------------------------------------------------------------//

LithoComposition::LithoComposition ( const string& lithoName1,
                                     const string& lithoName2,
                                     const string& lithoName3, 
                                     const double  p1,
                                     const double  p2,
                                     const double  p3, 
                                     const string& lithoMixModel ) {

  setComposition ( lithoName1, lithoName2, lithoName3, p1, p2, p3, lithoMixModel );

}

//------------------------------------------------------------//

void LithoComposition::setComposition ( const string& lithoName1,
                                        const string& lithoName2,
                                        const string& lithoName3, 
                                        const double  p1,
                                        const double  p2,
                                        const double  p3, 
                                        const string& lithoMixModel ) {


  lythoType1 = lithoName1;
  lythoType2 = lithoName2;
  lythoType3 = lithoName3;
  percent1 = p1;
  percent2 = p2;
  percent3 = p3;
  mixModel = lithoMixModel;

  if ( lythoType1 > lythoType2 ){
    std::swap ( lythoType1, lythoType2 );
    std::swap ( percent1, percent2 );
  }

  if ( lythoType2 > lythoType3 ){
    std::swap ( lythoType2, lythoType3 );
    std::swap ( percent2, percent3 );
  }

  if ( lythoType1 > lythoType2 ){
    std::swap ( lythoType1, lythoType2 );
    std::swap ( percent1, percent2 );
  }

}

//------------------------------------------------------------//

const string& LithoComposition::lithologyName ( const int whichSimpleLithology ) const {

  if ( whichSimpleLithology == 1 ) {
    return lythoType1;
  } else if ( whichSimpleLithology == 2 ) {
    return lythoType2;
  } else { // whichSimpleLithology == 3
    return lythoType3;
  } 

}

double LithoComposition::lithologyFraction ( const int whichSimpleLithology ) const {

  if ( whichSimpleLithology == 1 ) {
    return percent1;
  } else if ( whichSimpleLithology == 2 ) {
    return percent2;
  } else { // whichSimpleLithology == 3
    return percent3;
  } 

}

const string&LithoComposition::mixingModel () const {
  return mixModel;
}


string LithoComposition::Return_Key_String() const{

  string Key_String = "";

  Key_String += lythoType1;
  Key_String += "_";
  Key_String += lythoType2;
  Key_String += "_";
  Key_String += lythoType3;
  Key_String += "_";
  char char_percent1[8]; sprintf(char_percent1,"%7.3f\0",percent1);
  char char_percent2[8]; sprintf(char_percent2,"%7.3f\0",percent2);
  char char_percent3[8]; sprintf(char_percent3,"%7.3f\0",percent3);
  Key_String += char_percent1;
  Key_String += "_";
  Key_String += char_percent2;
  Key_String += "_";
  Key_String += char_percent3;
  Key_String += "_";
  Key_String += mixModel;  

  return Key_String;

}

bool operator< (const LithoComposition& lhs, const LithoComposition& rhs) {
  
  string rhs_string = rhs.Return_Key_String();
  string lhs_string = lhs.Return_Key_String();

  return ( (lhs_string < rhs_string) );
}

//------------------------------------------------------------//


void LithoProps::Get_Porosity ( const double             VES, 
                                const double             Max_VES,
                                const bool               Include_Chemical_Compaction,
                                const double             Chemical_Compaction_Term,
                                      Compound_Porosity& Porosity ) {

  int Lithology_Index = 0;
  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  double Component_Porosity;

  while (m_lithoComponents.end() != componentIter) {

    Component_Porosity = (*componentIter)->Porosity ( VES, Max_VES, Include_Chemical_Compaction, Chemical_Compaction_Term );
    Porosity ( Lithology_Index ) = Component_Porosity;

    ++componentIter;
    ++percentIter;
    Lithology_Index = Lithology_Index + 1;
  }

  Porosity.Set_Mixed_Porosity ( this -> porosity ( VES, Max_VES, Include_Chemical_Compaction, Chemical_Compaction_Term ));
}


//------------------------------------------------------------//


//  void LithoProps::Compute_Porosities ( const Basin_Modelling::Evaluations_Vector& VES,
//                                        const Basin_Modelling::Evaluations_Vector& Max_VES,
//                                        const Basin_Modelling::Evaluations_Vector& Chemical_Compaction,
//                                              Basin_Modelling::Evaluations_Vector& Porosities ) {

//    int I;

//    for ( I = 0; I < Porosities.Dimension (); I++ ) {
//      Porosities ( I ) = this -> porosity ( VES ( I ), Max_VES ( I ), Chemical_Compaction ( I ));
//    }

//  }


//  //------------------------------------------------------------//


//  void LithoProps::Compute_Porosities ( const Basin_Modelling::Evaluations_Vector& VES,
//                                        const Basin_Modelling::Evaluations_Vector& Max_VES,
//                                        const Basin_Modelling::Evaluations_Vector& Chemical_Compaction,
//                                              Compound_Porosity_Array&             Porosities ) {

//    int I;


//    for ( I = 0; I < Porosities.Dimension (); I++ ) {
//      this -> Get_Porosity ( VES ( I ), Max_VES ( I ), Chemical_Compaction ( I ), Porosities ( I ));
//    }

//  }


//------------------------------------------------------------//


void LithoProps::calcBulkPermeabilityNP ( const double             VES, 
                                          const double             Max_VES, 
			                  const Compound_Porosity& Porosity, 
                                                double&            Permeability_Normal, 
                                                double&            Permeability_Plane ) {

  int Component_Index = 0;

  compContainer::iterator componentIter = m_lithoComponents.begin();
  percentContainer::iterator percentIter = m_componentPercentage.begin();

  
  double PreviousPermN = 1.0E+08;
  
  if (m_mixmodeltype == HOMOGENEOUS || m_isFaultLithology ) {
    Permeability_Normal = 1.0;
    Permeability_Plane  = 1.0;
  } else if (m_mixmodeltype == LAYERED) {
    Permeability_Normal = 0.0;
    Permeability_Plane  = 0.0;
  }
  
  while (m_lithoComponents.end() != componentIter) {
    double VolFrac = (double)(*percentIter)*0.01;
    double PermVal = (*componentIter)->permeability( VES, Max_VES, Porosity ( Component_Index ));
    double PermAniso = (*componentIter)->getPermAniso();

    if (m_mixmodeltype == HOMOGENEOUS || m_isFaultLithology ) {
      Permeability_Normal *= pow(PermVal,VolFrac);
      Permeability_Plane  *= pow(PermVal*PermAniso,VolFrac);
    } else if (m_mixmodeltype == LAYERED) {

      if (PreviousPermN < PermVal) {
	Permeability_Normal = PreviousPermN;
	Permeability_Plane = PreviousPermN * PermAniso;
      } else {
	Permeability_Normal = PermVal;
	Permeability_Plane = PermVal * PermAniso;
      }

    }

    PreviousPermN = Permeability_Normal;
    Component_Index = Component_Index + 1;
    ++componentIter;
    ++percentIter;
  }
  
  Permeability_Normal *= MILLIDARCYTOM2;
  Permeability_Plane  *= MILLIDARCYTOM2;
 

}


//------------------------------------------------------------//


void LithoProps::calcBulkPermeabilityNPDerivative ( const double             VES, 
                                                    const double             Max_VES,
                                                    const Compound_Porosity& Porosity, 
                                                          double&            Permeability_Derivative_Normal, 
                                                          double&            Permeability_Derivative_Plane ) {

  double Derivatives    [ 3 ];
  double Permeabilities [ 3 ];

  double fraction;
  double minimumPermeability = 1.0e10;
  double minimumPermeabilityAnisotropy;
  double derivativeTerm;

  int minimumPosition;
  int componentCount = 0;

  compContainer::iterator componentIter = m_lithoComponents.begin();

  while (m_lithoComponents.end() != componentIter) {
    (*componentIter)->permeabilityDerivative ( VES, Max_VES, Porosity ( componentCount ), Permeabilities [ componentCount ], Derivatives [ componentCount ]);

    if ( Permeabilities [ componentCount ] < minimumPermeability ) {
      minimumPermeability = Permeabilities [ componentCount ];
      minimumPermeabilityAnisotropy = (*componentIter)->getPermAniso();
      minimumPosition = componentCount;
    }

    ++componentCount;
    ++componentIter;
  }

  componentIter = m_lithoComponents.begin();

  if (m_mixmodeltype == LAYERED && ! m_isFaultLithology ) {
    Permeability_Derivative_Normal = Derivatives [ minimumPosition ];
    Permeability_Derivative_Plane  = Derivatives [ minimumPosition ] * minimumPermeabilityAnisotropy;
  } else {
    compContainer::iterator componentIter2;
    percentContainer::iterator percentIter = m_componentPercentage.begin();
    percentContainer::iterator percentIter2;

    int I;
    int J;

    Permeability_Derivative_Normal = 0.0;
    Permeability_Derivative_Plane  = 0.0;

    I = 0;

    while (m_lithoComponents.end() != componentIter) {
      percentIter2 = m_componentPercentage.begin();
      componentIter2 = m_lithoComponents.begin();
      J = 0;
      derivativeTerm = 1.0;

      while (m_lithoComponents.end() != componentIter2 ) {
        fraction = (double)(*percentIter2)*0.01;

        if ( componentIter != componentIter2 ) {
          derivativeTerm = derivativeTerm * fraction * pow ( Permeabilities [ J ], fraction );
        }

        ++componentIter2;
        ++percentIter2;
        ++J;
      }

      fraction = 0.01 * double(*percentIter);
      derivativeTerm = derivativeTerm * fraction * Derivatives [ I ] * pow ( Permeabilities [ I ], fraction - 1.0 );

      Permeability_Derivative_Normal = Permeability_Derivative_Normal + derivativeTerm;
      Permeability_Derivative_Plane  = Permeability_Derivative_Plane  + derivativeTerm * (*componentIter)->getPermAniso();
      ++componentIter;
      ++percentIter;
      ++I;
    }

  }

  Permeability_Derivative_Normal = Permeability_Derivative_Normal * MILLIDARCYTOM2;
  Permeability_Derivative_Plane  = Permeability_Derivative_Plane  * MILLIDARCYTOM2;

}


//------------------------------------------------------------//


void LithoProps::Set_Chemical_Compaction_Terms 
   ( const double Rock_Viscosity,
     const double Activation_Energy ) {

  Reference_Solid_Viscosity = Rock_Viscosity;
  Lithology_Activation_Energy = Activation_Energy;

}


//------------------------------------------------------------//


double LithoProps::Integrate_Chemical_Compaction 
   ( const double Time_Step,
     const double VES,
     const double Porosity,
     const double Temperature ) const {

  if ( Porosity > Minimum_Porosity ) {

    double Solid_Viscosity;
    double Temperature_Term = 1.0 / ( Temperature + 273.15 ) - 1.0 / ( Reference_Temperature + 273.15 );

    //
    // 1.0e9 = results should be in Giga Pascals . Million years.
    //
    Solid_Viscosity = 1.0e9 * Reference_Solid_Viscosity * Secs_IN_MA * exp ( Lithology_Activation_Energy * Temperature_Term / Gas_Constant );
//      Solid_Viscosity = 1.0e9 * Reference_Solid_Viscosity * Secs_IN_MA * exp ( 15.0 * Lithology_Activation_Energy * Temperature_Term / Gas_Constant );

    return -Time_Step * Secs_IN_MA * ( 1.0 - Porosity ) * VES / Solid_Viscosity;

  } else {
    return 0.0;
  }

}


//------------------------------------------------------------//


double LithoProps::Integrate_Chemical_Compaction 
   ( const double Time_Step,
     const double Previous_VES,
     const double Current_VES,
     const double Previous_Porosity,
     const double Current_Porosity,
     const double Previous_Temperature,
     const double Current_Temperature ) const {

  if ( Current_Porosity > Minimum_Porosity ) {

    double Previous_Solid_Viscosity;
    double Current_Solid_Viscosity;
    double Previous_Temperature_Term = 1.0 / Previous_Temperature - 1.0 / Reference_Temperature;
    double Current_Temperature_Term  = 1.0 / Current_Temperature  - 1.0 / Reference_Temperature;

    //
    //
    // 1.0e9 = results should be in Giga Pascals . Million years.
    //
    Previous_Solid_Viscosity = 1.0e9 * Reference_Solid_Viscosity * Secs_IN_MA * exp ( 15.0 * Lithology_Activation_Energy * Previous_Temperature_Term / Gas_Constant );
    Current_Solid_Viscosity  = 1.0e9 * Reference_Solid_Viscosity * Secs_IN_MA * exp ( 15.0 * Lithology_Activation_Energy * Current_Temperature_Term  / Gas_Constant );

    return -0.5 * Time_Step * Secs_IN_MA * (( 1.0 - Previous_Porosity ) * Previous_VES / Previous_Solid_Viscosity 
                                          + ( 1.0 - Current_Porosity  ) * Current_VES  / Current_Solid_Viscosity );

  } else {
    return 0.0;
  }

}


//------------------------------------------------------------//


bool LithoProps::Has_Hydraulically_Fractured ( const double Hydrostatic_Pressure,
                                               const double Pore_Pressure,
                                               const double Lithostatic_Pressure ) const {

  return Pore_Pressure > fracturePressure ( Hydrostatic_Pressure, Lithostatic_Pressure );

}


//------------------------------------------------------------//

double LithoProps::fracturePressure ( const double hydrostaticPressure,
                                      const double lithostaticPressure ) const {
  return  Lithology_Fracture_Gradient * ( lithostaticPressure - hydrostaticPressure ) + hydrostaticPressure;
}

//------------------------------------------------------------//


double LithoProps::Exponential_Porosity_Function ( const double VES, 
						   const double Max_VES, 
						   const bool   Loading_Phase, 
						   const bool   Include_Chemical_Compaction ) const {

  double Exponential_Porosity;

  if ( Include_Chemical_Compaction ) {

    if ( Loading_Phase ) {
      Exponential_Porosity = ( m_depoporosity - Minimum_Mechanical_Porosity ) * exp( -m_compactionincr * VES ) + Minimum_Mechanical_Porosity;
    } else {
      Exponential_Porosity = ( m_depoporosity - Minimum_Mechanical_Porosity ) * exp( m_compactiondecr * ( Max_VES - VES ) - m_compactionincr * Max_VES ) +
                               Minimum_Mechanical_Porosity;
    }

  } else {

     if ( Loading_Phase ) {
      Exponential_Porosity = m_depoporosity * exp( -m_compactionincr * VES );
    } else {
      Exponential_Porosity = m_depoporosity * exp( m_compactiondecr * ( Max_VES - VES ) - m_compactionincr * Max_VES );
    }

  }

  return Exponential_Porosity;

} 


//------------------------------------------------------------//



double LithoProps::Soil_Mechanics_Porosity_Function ( const double VES, 
						      const double Max_VES, 
						      const bool   Loading_Phase, 
						      const bool   Include_Chemical_Compaction ) const {

  const double VES_0            = Reference_Effective_Stress ();

  const double Epsilon_100      = Reference_Void_Ratio ();

  const double Lithology_Compaction_Coefficient = m_Compaction_Coefficient_SM;

  double Porosity;
  double Void_Ratio;
  double M, C;
  double Phi_Max_VES;
  double Phi_Min_VES;
  double VES_Used;

  const double Percentage_Porosity_Rebound = 0.02; // => %age porosity regain


  if ( Loading_Phase ) {
    
    VES_Used = Double_Max ( VES, Max_VES );
    VES_Used = Double_Max ( VES_Used, VES_0 );

    Void_Ratio = Epsilon_100 - Lithology_Compaction_Coefficient * log ( VES_Used / VES_0 );

    Porosity = Void_Ratio / ( 1.0 + Void_Ratio );

  } else {

    Void_Ratio = Epsilon_100 - Lithology_Compaction_Coefficient 
      * log (  Double_Max ( VES_0, Max_VES ) / VES_0 ); 

    Phi_Max_VES = Void_Ratio / ( 1.0 + Void_Ratio );

    Void_Ratio = Epsilon_100;

    Phi_Min_VES = Void_Ratio / ( 1.0 + Void_Ratio );

    M = Percentage_Porosity_Rebound * ( Phi_Max_VES - Phi_Min_VES ) / ( Max_VES - VES_0 );

    C = ( ( 1.0 - Percentage_Porosity_Rebound ) * Phi_Max_VES * Max_VES - 
	  Phi_Max_VES * VES_0 + Percentage_Porosity_Rebound * Phi_Min_VES * Max_VES ) / 
          ( Max_VES - VES_0 );

    Porosity = M * VES + C;

  } 

  /* Force porosity to be in range 0.03 .. Surface_Porosity */

  if ( Include_Chemical_Compaction ) {
    Porosity = Double_Max ( Porosity, Minimum_Mechanical_Porosity );
  }

  Porosity = Double_Max ( Porosity, Minimum_Porosity );
  Porosity = Double_Min ( Porosity, m_depoporosity );

  return Porosity;

} 


//------------------------------------------------------------//


double LithoProps::Exponential_D_Void_Ratio_DP ( const double VES, 
						 const double Max_VES, 
						 const bool   Loading_Phase, 
						 const bool   Include_Chemical_Compaction, 
						 const double Chemical_Compaction_Term ) {

  const double Biot = 1.0;

  double psi, dpsi;

  /* This routine will return the derivative of the void-ratio (dpsi) */
  /* using a cut-off value of psi.  */

  double min_VES = 1.0e5; // 

//    double min_VES = 
//      Exponential_Minimum_Stress ( m_depoporosity, Max_VES, m_compactionincr, m_compactiondecr, Loading_Phase );

  double VES_used = Double_Max ( VES, min_VES );
  
  psi = Void_Ratio ( VES_used, Max_VES, Include_Chemical_Compaction, Chemical_Compaction_Term );

  dpsi = -Biot * Exponential_D_Void_Ratio_D_VES ( psi, VES_used, Max_VES, Loading_Phase );
  
  return dpsi;
       
}


//------------------------------------------------------------//


double LithoProps::Soil_Mechanics_D_Void_Ratio_DP ( const double VES, 
						    const double Max_VES, 
						    const bool   Loading_Phase, 
                                                    const bool   Include_Chemical_Compaction, 
                                                    const double Chemical_Compaction_Term ) {

  const double Biot = 1.0;

  double Psi;
  double D_Psi_D_P;

  /* This routine will return the derivative of the void-ratio (dpsi) */ 
  /* using a cut-off value of psi. */

  double Min_VES = Reference_Effective_Stress ();
  double VES_Used = Double_Max ( VES, Min_VES );
  
  Psi = Void_Ratio ( VES_Used, Max_VES, Include_Chemical_Compaction, Chemical_Compaction_Term );

  D_Psi_D_P = -Biot * 
    Soil_Mechanics_D_Void_Ratio_D_VES ( Psi, VES_Used, Max_VES, Loading_Phase );

  return D_Psi_D_P;

} 


//------------------------------------------------------------//


double LithoProps::Soil_Mechanics_D_Void_Ratio_D_VES ( const double Psi, 
						       const double VES, 
						       const double Max_VES, 
						       const bool   Loading_Phase ) {


  /* This is the derivative of the void-ratio w.r.t. sigma with singular point ! */
  /* If loading is TRUE then Max_VES = VES */

  double D_Psi_D_Sigma;

  //
  //  d psi       d psi   d phi
  //  -------  = ------   ------
  //  d sigma     d phi   d sigma
  //

  if ( VES > 0.0 ) {
    D_Psi_D_Sigma = -m_Compaction_Coefficient_SM / VES;
  } else {
    D_Psi_D_Sigma = -m_Compaction_Coefficient_SM / Double_Epsilon ();
  }

  return D_Psi_D_Sigma;

} 


//------------------------------------------------------------//


double LithoProps::Exponential_D_Void_Ratio_D_VES ( const double Void_Ratio, 
						    const double VES, 
						    const double Max_VES, 
						    const bool   Loading_Phase ) {

  /* This is the derivative of the void-ratio w.r.t. VES with singular point */
  /* If Loading_Phase is TRUE then Max_VES = VES */
  
  double dpsi, cC1, cC2, dpsidphi;

  //
  //  d psi       d psi   d phi
  //  -------  = ------   ------
  //  d sigma     d phi   d sigma
  //

  cC1 = Void_Ratio / ( 1.0 + Void_Ratio );
  cC2 = 1.0 - cC1;
  dpsidphi = 1.0 / ( cC2 * cC2 );

  if ( Loading_Phase ) {
    dpsi = -m_compactionincr * cC1 * dpsidphi;
  } else {
    dpsi = -m_compactiondecr * cC1 * dpsidphi;
  }

  return dpsi;

}


//------------------------------------------------------------//


double LithoProps::exponentialPorosityDerivative ( const double VES, 
                                                   const double maxVES,
                                                   const bool   includeChemicalCompaction,
                                                   const double chemicalCompactionTerm ) const {

  //
  //
  //   d Phi   d Phi   d ves       d Phi
  //   ----- = ----- x -----  =  - -----  = cc * Phi
  //     dp    d ves     dp        d ves
  //
  //
  //
  double porosityDerivative;
  double porosityValue;

  porosityValue = porosity ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm );

  if ( VES >= maxVES ) {
    porosityDerivative = m_compactionincr * porosityValue;
  } else {
    porosityDerivative = m_compactiondecr * porosityValue;
  }

  return porosityDerivative;
}


//------------------------------------------------------------//


double LithoProps::soilMechanicsPorosityDerivative ( const double VES, 
                                                     const double maxVES,
                                                     const bool   includeChemicalCompaction,
                                                     const double chemicalCompactionTerm ) const {

  //
  //
  //   d Phi   d Phi   d Psi   d ves           1          Beta
  //   ----- = ----- x ----- x -----  =  ------------- x  ----
  //     dp    d Psi   d ves     dp      ( 1 + Psi )^2     ves
  //
  //
  //
  double porosityDerivative;
  double porosityValue;
  double VESValue;

  //
  // What to do if the VES is zero, for now just take double model epsilon ( ~= O(10^-16))
  //
  VESValue = Double_Max ( VES, Double_Epsilon ());

  porosityValue = porosity ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm );
  porosityDerivative = pow ( 1.0 - porosityValue, 2 ) * m_Compaction_Coefficient_SM / VESValue;

  return porosityDerivative;
}


//------------------------------------------------------------//


double LithoProps::computePorosityDerivativeWRTPressure ( const double VES, 
                                                          const double maxVES,
                                                          const bool   includeChemicalCompaction,
                                                          const double chemicalCompactionTerm ) const {

  double porosityDerivative;

  if ( m_Porosity_Model == EXPONENTIAL ) {
    porosityDerivative = exponentialPorosityDerivative ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm );
  } else {
    porosityDerivative = soilMechanicsPorosityDerivative ( VES, maxVES, includeChemicalCompaction, chemicalCompactionTerm );
  }

  return porosityDerivative;

}


//------------------------------------------------------------//
