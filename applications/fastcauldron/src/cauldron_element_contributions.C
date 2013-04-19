
void Basin_Modelling::Cauldron_Element_Contributions
   ( const double                   Time_Step,
     const Element_Geometry_Matrix& Previous_Geometry_Matrix,
     const Element_Geometry_Matrix& Geometry_Matrix,
     const Boundary_Conditions*     BCs,
     const int*                     DOF_Numbers,
     const Element_Vector&          Previous_Bulk_Fluid_Density,
     const Element_Vector&          Bulk_Fluid_Density,
     const Element_Vector&          Fluid_Mobility_Normal,
     const Element_Vector&          Fluid_Mobility_Parallel,
     const Element_Vector&          Source_Terms,
     const Element_Vector&          Overpressure,
           Element_Matrix&          Element_Jacobian,
           Element_Vector&          Element_Residual ) {

  // ======================================================================
  // ===
  // === The differential equation that FESOLV is trying to solve is:
  // ===
  // ===       (A(opn,hpn,xn,yn,zn,tn)-A(opo,hpo,xo,yo,zo,to))/delt =
  // ===       div(Btens.(grad(p0n+hpn)+grav)) + C
  // ===
  // === where op : overpressure, (the unknown)
  // ===       hp : hydrostatic pressure (known)
  // ===       x,y,z : the space coordinates
  // ===       t : time
  // ===       suffix o(ld)/n(ew) indicate at start/end of timestep 
  // ===       delt : timestep = tn -to, 
  // ===       A : fluid density times porosity, 
  // ===       Btens : fluid mobility tensor,
  // ===       grav : fluid density*gravity acceleration,
  // ===       C : fluid source term.
  // ===  
  // === This routine is just used to perform a single Newton iteration step.
  // === An estimate opne of the unknown opn is known.
  // === The linearized equation for opn around opne for a single Newton step is:
  // ===
  // ===      (dA/dop)*(opn-opne)+A(opne,hpn,xn,yn,zn,tn)-
  // ===             A(opo,hpo,xo,yo,zo,to))/delt = 
  // ===       div(Btens.(grad(p0n+hpn)+grav)) + C
  // ===
  // === where dA/dop is calculated at opne,hpn,xn,yn,zn,tn 
  // ===
  // ===
  // === All units are assumed to be SI.
  // === A and C are scalar functions of t,x,y,z and p.
  // === Btens should be a symmetric positive 3x3 tensor which is also
  // === a function of t,x,y,z and p. However we will assume that everywhere
  // === in the brick
  // === - the first principle axis of Btens is the normal to the 
  // ===   brick-bottom-quad in its midpoint,
  // === - the permeability in this direction is given 
  // ===   by the scalar Bn, which is a function of t,x,y,z and p.
  // === - the permeability in the other two directions is equal and
  // ===   given by Bp, which is also a function of t,x,y,z and p.
  // ===
  // === Bilinear element are used with Gauss-Lobatto integration.
  // ===
  // === To improve efficiency it is further assumed that the four vertical
  // === segments, SW_, NW_, NE_ and SE_ are purely vertical.
  // === 
  // === A, Bn, Bp and C are calculated with the pressures VALD1
  // === while in all other places VALD2 is used. VALD1 and VALD2 are always
  // === equal, except in the case that this routine is called to calculate
  // === cornerfluxes. In that case VALD1 is the pressure that was used
  // === to calculate A,B, and C previously and VALD2 is the newly calculated
  // === pressure.
  // ===
  // ======================================================================

  const double Surface_Overpressure = 0.0;

  int I, J, K;


//     max_nd is the maximum possible number of element nodes
//     nd is the actual number of nodes of this element
//     ------------------------------------------------
//
//     ======================================================================
//

  bool Element_Is_Anisotropic;

  Element_Boolean_Array STELM1;
  Element_Boolean_Array STELM2;
  Element_Boolean_Array STELM3;
  Element_Boolean_Array STELM4;

  Element_Boolean_Array STELM11;
  Element_Boolean_Array STELM12;
  Element_Boolean_Array STELM13;
  Element_Boolean_Array STELM14;


  Element_Vector A, AOld, Bn, Bp, C, D, DADOP, DADOPpm4, DBnDOP, DPpDOP, DCDOP, Xs, Ys, Zs, ZOld;
  Element_Vector DXI, DYI, DZzI, DXI2, DYI2, DX, DY, DZx, DZy, DZz, DZOldz;

  Element_Vector Stf11;
  Element_Vector Stf21;
  Element_Vector Stf31;
  Element_Vector Stf41;
  Element_Vector Stf22;
  Element_Vector Stf32;
  Element_Vector Stf33;
  Element_Vector Stf42;
  Element_Vector Stf43;
  Element_Vector Stf44;


  Element_Vector SPF21;
  Element_Vector SPF22;

  Element_Vector SPF31;
  Element_Vector SPF32;
  Element_Vector SPF33;

  Element_Vector SPF41;
  Element_Vector SPF42;
  Element_Vector SPF43;
  Element_Vector SPF44;


  Element_Vector SQF11;

  Element_Vector SQF21;
  Element_Vector SQF22;

  Element_Vector SQF31;
  Element_Vector SQF32;
  Element_Vector SQF33;

  Element_Vector SQF41;
  Element_Vector SQF42;
  Element_Vector SQF43;
  Element_Vector SQF44;


  Element_Vector SUF21;
  Element_Vector SUF22;

  Element_Vector SUF31;
  Element_Vector SUF32;
  Element_Vector SUF33;

  Element_Vector SUF41;
  Element_Vector SUF42;
  Element_Vector SUF43;
  Element_Vector SUF44;

  Element_Vector DETJ, DETJold;


  double DXDYo8,       DXDYo8Bp,     DXYo8Bp1,
         DXYo8Bp2,     DXYo8Bp3,     Permeability,        
         n1,           n2,           n3,
         LBten11,      LBten21,      LBten31,
         LBten22,      LBten32,      LBten33,
         Jt2I11,                     Jt2I22,
         Jt231,        Jt232,        Jt2I33, 
         JILP11,       JILP21,       JILP22,
         JILP31,       JILP32,       JILP33, 
         NB,
         NBDI11,       NBDI22,       NBDI33, 
         tz1, tz2,
         DY2, DXDYo82, DYI22, DX2, DXI22, DXYo8Bp4;

//        call get_iprint_info ( Output )
//        if(ldebug1) then
//           write(*,*)'in elbstfxyz'
//           write(*,*)'vald1'
//           write(*,*)(vald1(iND),iND=0,ND-1)
//           write(*,*)'vald2'
//           write(*,*)(vald2(iND),iND=0,ND-1)
//        endif

  //
  //     Argument ENERG is set to 0 just to give it a value.
  //     It is not used at all by FESOLV, because (TEMP/p)SOLVER does not use 
  //     this option of FESOLV.
  //
//    ENERG = 0.0;
  //      
  //     Check whether ND = parameter NDxyz+1 =8
  //   
//        If(ldebug1) then
//           IF( ND.NE.NDxyz+1 ) then
//              write(6,800)ND
//              stop
//           ENDIF
//        endif
  //
  //     ITYPE is not required anymore by this routine, but it still has to be set
  //     for routine FACEFLUX
  //
//    ITYPE = 2

  //     element type 
  Element_Type = 0;//ELEMENT_TYPE(JELM)

//      if (ldebug3)then
//         IF(ELTYPE .EQ. -1) WRITE(6, 920)
//      endif
 
//      call Get_Properties(JELM,X,Y,Z,Zold,A,
//     #                    DADOP,Aold,Bn,Bp,D)


  //
  //
  // This is REQUIRED for faulted elements
  //
//    for ( J = 1; J <= 8; J++ ) {

//      if ( Element_Type != 0 ) {
//        stelm1 ( J ) = true;
//        stelm2 ( J ) = true;
//        stelm3 ( J ) = true;
//        stelm4 ( J ) = true;
//        stelm11 ( J ) = false;
//        stelm12 ( J ) = false;
//        stelm13 ( J ) = false;
//        stelm14 ( J ) = false;

//        if ((J == 0) || (J == 4)) stelm11(J) = true;
//        if ((J == 1) || (J == 5)) stelm12(J) = true;
//        if ((J == 2) || (J == 6)) stelm13(J) = true;
//        if ((J == 3) || (J == 7)) stelm14(J) = true;

//        if ( DOF_Numbers [ J - 1 ] == DOF_Numbers [( J + 4 ) % 8 )] {
//          if ((J == 0) || (J == 4)) stelm1(J) = false;
//          if ((J == 1) || (J == 5)) stelm2(J) = false;
//          if ((J == 2) || (J == 6)) stelm3(J) = false;
//          if ((J == 3) || (J == 7)) stelm4(J) = false;
//        } // end if

//      } // end if

//    } // end loop


  Element_Is_Anisotropic = true; //BN ( 1 ) != BP ( 1 );

  for ( I = 1; I <= 8; I++ ) {
    Xs     ( I ) = Geometry_Matrix ( 1, I );
    Ys     ( I ) = Geometry_Matrix ( 2, I );
    Zs     ( I ) = Geometry_Matrix ( 3, I );
    Zs_Old ( I ) = Previous_Geometry_Matrix ( 3, I );
  } // end loop

  if ( Element_Type == 0 ) {

    DXI(1) = 1.0/(X(2)-X(1));
    DXI(2) = -DXI(1);
    DXI(3) = -DXI(1);
    DXI(4) =  DXI(1);
    DXI(5) =  DXI(1);
    DXI(6) = -DXI(1);
    DXI(7) = -DXI(1);
    DXI(8) =  DXI(1);

    DYI(1) = 1.0/(Y(4)-Y(1));
    DYI(2) =  DYI(1);
    DYI(3) = -DYI(1);
    DYI(4) = -DYI(1);
    DYI(5) =  DYI(1);
    DYI(6) =  DYI(1);
    DYI(7) = -DYI(1);
    DYI(8) = -DYI(1);

    DZx(1) = Z(2)-Z(1);
    DZx(2) = -DZx(1);
    DZx(3) = Z(4)-Z(3);
    DZx(4) = -DZx(3);
    DZx(5) = Z(6)-Z(5);
    DZx(6) = -DZx(5);
    DZx(7) = Z(8)-Z(7);
    DZx(8) = -DZx(7);
    
    DZy(1) = Z(4)-Z(1);
    DZy(2) = Z(3)-Z(2);
    DZy(3) = -DZy(2);
    DZy(4) = -DZy(1);
    DZy(5) = Z(8)-Z(5);
    DZy(6) = Z(7)-Z(6);
    DZy(7) = -DZy(6);
    DZy(8) = -DZy(5);

    DZz(1) = (Z(5)-Z(1));
    DZz(2) = (Z(6)-Z(2));
    DZz(3) = (Z(7)-Z(3));
    DZz(4) = (Z(8)-Z(4));
    DZz(5) = -DZz(1);
    DZz(6) = -DZz(2);
    DZz(7) = -DZz(3);
    DZz(8) = -DZz(4);

    DZzI(1) = 1.0/(Z(5)-Z(1));
    DZzI(2) = 1.0/(Z(6)-Z(2));
    DZzI(3) = 1.0/(Z(7)-Z(3));
    DZzI(4) = 1.0/(Z(8)-Z(4));
    DZzI(5) = -DZzI(1);
    DZzI(6) = -DZzI(2);
    DZzI(7) = -DZzI(3);
    DZzI(8) = -DZzI(4);

    DZoldz(1) = Zold(5)-Zold(1);
    DZoldz(2) = Zold(6)-Zold(2);
    DZoldz(3) = Zold(7)-Zold(3);
    DZoldz(4) = Zold(8)-Zold(4);
    DZoldz(5) = -DZoldz(1);
    DZoldz(6) = -DZoldz(2);
    DZoldz(7) = -DZoldz(3);
    DZoldz(8) = -DZoldz(4);

    N1 = -DXI(1)*( DZx(5) + DZx(8));
    N2 = -DYI(1)*( DZy(5) + DZy(6));
    N3 =  2.0;

    for ( IJK = 1; IJK <= 8; IJK++ ){
      DETJ(ijk)   = fabs ( 0.125 * DZz(ijk)   /(DXI(ijk)*DYI(ijk)));
      DETJold(ijk)= fabs ( 0.125 * DZoldz(ijk)/(DXI(ijk)*DYI(ijk)));
    } // end loop

    if (namd(ijk) == namd(mod(ijk+4,8))) {

      STF11(ijk) = 0.0;
      STF21(ijk) = 0.0;
      STF22(ijk) = 0.0;
      STF31(ijk) = 0.0;
      STF32(ijk) = 0.0;
      STF33(ijk) = 0.0;
      STF41(ijk) = 0.0;
      STF42(ijk) = 0.0;
      STF43(ijk) = 0.0;
      STF44(ijk) = 0.0;
      Element_Residual(ijk)  = 0.0;
      Element_Jacobian(ijk,ijk) = 0.0;

    } else {

      Permeability  = (Bn(ijk)/Bp(ijk)-1.0)/(n1 * n1 + n2 * n2 + n3 * n3 );
      LBten11 = 1.0 + Permeability * n1 * n1;
      LBten21 =       Permeability * n2 * n1;
      LBten31 =       Permeability * n3 * n1;
      LBten22 = 1.0 + Permeability * n2 * n2 - LBten21 * LBten21 / LBten11;
      LBten32 =       Permeability * n3 * n2 - LBten31 * LBten21 / LBten11;
      LBten33 = 1.0 + Permeability * n3 * n3 - LBten31 * LBten31 / LBten11 - Lbten32 * LBten32 / LBten22;

      Jt2I11 = DXI(ijk);
      Jt2I22 = DYI(ijk);
      Jt231  = DZx(ijk);
      Jt232  = DZy(ijk);
      Jt2I33 = DZzI(ijk);
      //
      //              Calculate the (L+D)(D**-1)(LT+D) factorisation of the 
      //              "scaled" B tensor = B/Bp. Store L+D in LBten.. .  
      //
      //     
      //              Calculate ((J*2)**-1).(L+D) store in JILPD
      //     
      JILP11 = LBten11*Jt2I11;
      JILP21 = LBten21*Jt2I22;
      JILP22 = LBten22*Jt2I22;
      JILP31 =(LBten31-Jt232*JILP21-Jt231*JILP11)*Jt2I33;
      JILP32 =(LBten32-Jt232*JILP22)             *Jt2I33;
      JILP33 = LBten33                           *Jt2I33;
      //     
      //              Calculate B*DetJ*(D**-1)*4/4 
      //              Multiply by 4 because we multiplied twice with (J*2)**-1
      //              Divide by 4 because we will Multiply twice with 2*F
      //     
      NB     = DETJ(ijk)*Bp(ijk);
      NBDI11 = NB/LBten11;
      NBDI22 = NB/LBten22;
      NBDI33 = NB/LBten33;
      //     
      //              Calculate contribution to "stiffnessmatrix":
      //              F.JILPD.D.(JILPD**T).FT
      //     
      STF11(ijk) = JILP11 * JILP11 *NBDI11;
      STF21(ijk) = JILP21*JILP11*NBDI11;
      STF22(ijk) = JILP22 * JILP22 * NBDI22 + JILP21 * JILP21 * NBDI11;
      STF31(ijk) = JILP31*JILP11*NBDI11;
      STF32(ijk) = JILP32*JILP22*NBDI22 + JILP31*JILP21*NBDI11;
      STF33(ijk) = JILP33 * JILP33 * NBDI33 +  JILP32 * JILP32 * NBDI22 + JILP31 * JILP31 * NBDI11;
     
      STF41(ijk) = -STF11(ijk)-STF21(ijk)-STF31(ijk);
      STF42(ijk) = -STF21(ijk)-STF22(ijk)-STF32(ijk);
      STF43(ijk) = -STF31(ijk)-STF32(ijk)-STF33(ijk);

      STF44(ijk) = -STF41(ijk)-STF42(ijk)-STF43(ijk);
    } // end if

    Element_Jacobian(1,1) = STF11(2)+STF22(4)+STF33(5)+STF44(1);

    Element_Jacobian(2,1) = STF41(1)+STF41(2);
    Element_Jacobian(2,2) = STF11(1)+STF22(3)+STF33(6)+STF44(2);
         
    Element_Jacobian(3,1) = STF21(2)+STF21(4);
    Element_Jacobian(3,2) = STF42(2)+STF42(3);
    Element_Jacobian(3,3) = STF11(4)+STF22(2)+STF33(7)+STF44(3);
         
    Element_Jacobian(4,1) = STF42(1)+STF42(4);
    Element_Jacobian(4,2) = STF21(1)+STF21(3);
    Element_Jacobian(4,3) = STF41(4)+STF41(3);
    Element_Jacobian(4,4) = STF11(3)+STF22(1)+STF33(8)+STF44(4);
         
    Element_Jacobian(5,1) = STF43(1)+STF43(5);
    Element_Jacobian(5,2) = STF31(1)+STF31(6);
    Element_Jacobian(5,3) = 0.0;
    Element_Jacobian(5,4) = STF32(1)+STF32(8);
    Element_Jacobian(5,5) = STF11(6)+STF22(8)+STF33(1)+STF44(5);
         
    Element_Jacobian(6,1) = STF31(2)+STF31(5);
    Element_Jacobian(6,2) = STF43(2)+STF43(6);
    Element_Jacobian(6,3) = STF32(2)+STF32(7);
    Element_Jacobian(6,4) = 0.0;
    Element_Jacobian(6,5) = STF41(5)+STF41(6);
    Element_Jacobian(6,6) = STF11(5)+STF22(7)+STF33(2)+STF44(6);

    Element_Jacobian(7,1) = 0.0;
    Element_Jacobian(7,2) = STF32(3)+STF32(6);
    Element_Jacobian(7,3) = STF43(3)+STF43(7);
    Element_Jacobian(7,4) = STF31(3)+STF31(8);
    Element_Jacobian(7,5) = STF21(6)+STF21(8);
    Element_Jacobian(7,6) = STF42(6)+STF42(7);
    Element_Jacobian(7,7) = STF11(8)+STF22(6)+STF33(3)+STF44(7);
         
    Element_Jacobian(8,1) = STF32(4)+STF32(5);
    Element_Jacobian(8,2) = 0.0;
    Element_Jacobian(8,3) = STF31(4)+STF31(7);
    Element_Jacobian(8,4) = STF43(4)+STF43(8);
    Element_Jacobian(8,5) = STF42(5)+STF42(8);
    Element_Jacobian(8,6) = STF21(5)+STF21(7);
    Element_Jacobian(8,7) = STF41(8)+STF41(7);
    Element_Jacobian(8,8) = STF11(7)+STF22(5)+STF33(4)+STF44(8);

//      Element_Jacobian(1,1) = +STF11(1)+STF22(3)+STF33(4)+STF44(0)

//      Element_Jacobian(2,1) = STF41(0)+STF41(1)
//      Element_Jacobian(2,2) = +STF11(0)+STF22(2)+STF33(5)+STF44(1)
         
//      Element_Jacobian(3,1) = STF21(1)+STF21(3)
//      Element_Jacobian(3,2) = STF42(1)+STF42(2)
//      Element_Jacobian(3,3) = +STF11(3)+STF22(1)+STF33(6)+STF44(2)
         
//      Element_Jacobian(4,1) = STF42(0)+STF42(3)
//      Element_Jacobian(4,2) = STF21(0)+STF21(2)
//      Element_Jacobian(4,3) = STF41(3)+STF41(2)
//      Element_Jacobian(4,4) = +STF11(2)+STF22(0)+STF33(7)+STF44(3)
         
//      Element_Jacobian(5,1) = STF43(0)+STF43(4)
//      Element_Jacobian(5,2) = STF31(0)+STF31(5)
//      Element_Jacobian(5,3) = 0.0
//      Element_Jacobian(5,4) = STF32(0)+STF32(7)
//      Element_Jacobian(5,5) = +STF11(5)+STF22(7)+STF33(0)+STF44(4)
         
//      Element_Jacobian(6,1) = STF31(1)+STF31(4)
//      Element_Jacobian(6,2) = STF43(1)+STF43(5)
//      Element_Jacobian(6,3) = STF32(1)+STF32(6)
//      Element_Jacobian(6,4) = 0.0
//      Element_Jacobian(6,5) = STF41(4)+STF41(5)
//      Element_Jacobian(6,6) = +STF11(4)+STF22(6)+STF33(1)+STF44(5)
         
//      Element_Jacobian(7,1) = 0.0
//      Element_Jacobian(7,2) = STF32(2)+STF32(5)
//      Element_Jacobian(7,3) = STF43(2)+STF43(6)
//      Element_Jacobian(7,4) = STF31(2)+STF31(7)
//      Element_Jacobian(7,5) = STF21(5)+STF21(7)
//      Element_Jacobian(7,6) = STF42(5)+STF42(6)
//      Element_Jacobian(7,7) = +STF11(7)+STF22(5)+STF33(2)+STF44(6)
         
//      Element_Jacobian(8,1) = STF32(3)+STF32(4)
//      Element_Jacobian(8,2) = 0.0
//      Element_Jacobian(8,3) = STF31(3)+STF31(6)
//      Element_Jacobian(8,4) = STF43(3)+STF43(7)
//      Element_Jacobian(8,5) = STF42(4)+STF42(7)
//      Element_Jacobian(8,6) = STF21(4)+STF21(6)
//      Element_Jacobian(8,7) = STF41(7)+STF41(6)
//      Element_Jacobian(8,8) = +STF11(6)+STF22(4)+STF33(3)+STF44(7)

  } // end if


  //
  //     CALCULATE THE STRICTLY UPPER TRIANGULAR PART OF THE "STIFFNESS" MATRIX
  //
  for ( I = 1; I <= 8; I++ ) {

    for ( J = 1; J <= I - 1; J++ ) {
      Element_Jacobian ( J, I ) = Element_Jacobian ( I, J );
    } // end loop

  } // end loop
  //
  //     CALCULATE THE CONDUCTIVITY CONTRIBUTION TO THE RIGHT HAND SIDE.
  //
  for ( I = 1; I <= 8; I++ ) {
    Element_Residual ( I ) = 0.0;

    for ( J = 1; J <= 8; J++ ) {
      Element_Residual ( I ) = Element_Residual ( I ) - Element_Jacobian ( J, I ) * Overpressure ( J );
    } // end loop

  } // end loop
  //
  //
  //     CONTRIBUTION OF A,C AND D TO RHS.
  //

//       const Element_Vector&          Previous_Bulk_Fluid_Density,
//       const Element_Vector&          Bulk_Fluid_Density,
//       const Element_Vector&          Fluid_Mobility_Normal,
//       const Element_Vector&          Fluid_Mobility_Parallel,
//       const Element_Vector&          Source_Terms,

  for ( I = 1; I <= 8; I++ ) {

    Element_Residual( I ) = Element_Residual( I ) + ( Source_Terms ( I )+Previous_Bulk_Fluid_Density ( I )*DETJold( I ) - Bulk_Fluid_Density  ( I )*DETJ( I ))*DELTI;
    //
    //     Contribution of dAdOP to Matrix
    //     Note that only diagonal of AMAT is changed.
    // 
//      Element_Jacobian( I, I ) = Element_Jacobian( I, I )+DADOP( I )*DELTI*DETJ( I );

  } // end loop
  //
  //     Element_Residual contains now the corner point heatflows so if only these
  //     are required return here
  //      IF(IFLAG.eq.2) return
  //  
  //     For a Dirichlet face (i.e., a face at which the temperature
  //     is prescribed) we modify all the element equations for the
  //     temperatures: we eliminate all the prescibed temperatures from
  //     all equations and replace the equations corresponnding to the prescribed
  //     nodes by the prescribed boundary equation. 
  //     Note that symmetry of the matrix is preserved.
  //
  //     loop over all nodes and find out if they have a fixed temperature
  //
  //     if one node does eliminate it from all equations, and replace equation
  //     for this node by 1*dp = p(fixed)-p(actual).
  //     --------------------------
  //
  for ( J = 1; J <= 8; J++ ){

    if ( Element_BCs [ J - 1 ] == Surface_Boundary ) {
//        FQUERY=NODE_GETFIXEDoverpressure_FWD (JELM, Is_Dirichlet_Node,J+1,FIXED_OP);

//      if ( Is_Dirichlet_Node ) {

//        for ( I = 1; I <= 8; I++ ){
//          Element_Jacobian (J,I) = 0.0;
//          Element_Residual(I) = Element_Residual(I) - Element_Jacobian (I,J)*(FIXED_OP - VALD2(J));
//          Element_Jacobian (I,J) = 0.0
//        } // end loop

      Element_Residual ( J )    = Dirichlet_Number * ( Surface_Overpressure - Overpressure ( J ));
      Element_Jacobian ( J, J ) = Dirichlet_Number;
    } // end if

  } // end loop

} // end Basin_Modelling::Cauldron_Element_Contributions


