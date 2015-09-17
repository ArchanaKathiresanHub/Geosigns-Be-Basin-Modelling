#include <gtest/gtest.h>

#include "../src/LookUpDirectory.h"

TEST( TSRlookupTest, GetValue )
{
   
   TSR_Tables::LookUpDirectory tables;

   const TSR_Tables::LookUp * oneTable = tables.getTable( TSR_Tables::H2S_H2O_NaCl2m );

   //   EXPECT_GE( oneTable, 0 );
   TSR_Tables::dataValue oneValue;

   bool result = oneTable->get( 600, 20, oneValue );
   EXPECT_DOUBLE_EQ( 0.029635110999999999, oneValue.first );
   EXPECT_FLOAT_EQ( 1, oneValue.second );
 
   result = oneTable->get( 600, 24.5, oneValue );
   EXPECT_DOUBLE_EQ( 0.029694186550000001, oneValue.first );
   EXPECT_FLOAT_EQ(  1, oneValue.second  );

   result = oneTable->get( 555, 20, oneValue );
   EXPECT_DOUBLE_EQ(  0.029308884400000002, oneValue.first );
   EXPECT_FLOAT_EQ(  1, oneValue.second );

   result = oneTable->get( 501, 42, oneValue );
   EXPECT_DOUBLE_EQ(  0.029457453596000001, oneValue.first );
   EXPECT_FLOAT_EQ(  1, oneValue.second  );

   result = oneTable->get( 1, 0, oneValue );
   EXPECT_DOUBLE_EQ( 0.0024186780000000001, oneValue.first );
   EXPECT_FLOAT_EQ(  0, oneValue.second  );
   EXPECT_TRUE( result );

   result = oneTable->get( 1000, 300, oneValue );
   EXPECT_DOUBLE_EQ(  -1, oneValue.first );
   EXPECT_FLOAT_EQ(  -1, oneValue.second  );
   EXPECT_FALSE( result );

   result = oneTable->get( 1000, 275, oneValue );
   EXPECT_DOUBLE_EQ(  0.016581459999999999, oneValue.first );
   EXPECT_FLOAT_EQ(  0, oneValue.second  );
   EXPECT_TRUE( result );

   result = oneTable->get( 1000, 276, oneValue );
   EXPECT_DOUBLE_EQ( -1, oneValue.first );
   EXPECT_FLOAT_EQ( -1, oneValue.second  );
   EXPECT_FALSE( result );

   result = oneTable->get( 95, 110, oneValue );
   EXPECT_DOUBLE_EQ(  0.032146769999999998, oneValue.first );
   EXPECT_FLOAT_EQ(  0.5, oneValue.second  );
   EXPECT_TRUE( result );

   result = oneTable->get( 96, 110, oneValue );
   EXPECT_DOUBLE_EQ(  0.032405372799999999, oneValue.first );
   EXPECT_FLOAT_EQ(  0.6, oneValue.second  );
   EXPECT_TRUE( result );

   result = oneTable->get( 95, 105, oneValue );
   EXPECT_DOUBLE_EQ( 0.031945420249999995, oneValue.first );
   EXPECT_FLOAT_EQ(  0.75, oneValue.second  );
   EXPECT_TRUE( result );

   result = oneTable->get( 96, 105, oneValue );
   EXPECT_DOUBLE_EQ(  0.032079509899999997, oneValue.first );
   EXPECT_FLOAT_EQ(  0.8, oneValue.second );
   EXPECT_TRUE( result );


}

TEST( TSRlookupTest, TablesCheck )
{
   TSR_Tables::LookUpDirectory tables;

   for ( int i = 0; i < TSR_Tables::NumberOfTables; ++ i ) {
      TSR_Tables::TSR_Table oneTableName = (TSR_Tables::TSR_Table ) i;
      const TSR_Tables::LookUp * oneTable = tables.getTable( oneTableName );

      bool result = oneTable->checkValidity();

      if( not result )
          std::cout << oneTableName << std::endl;

      EXPECT_TRUE (  result );
   }
}

TEST( TSRlookupTest, GetTableName )
{
   TSR_Tables::LookUpDirectory tables;
  
   double cauldronSalinity = 0.01;
   TSR_Tables::TSR_Table oneTable = tables.getTableName( cauldronSalinity, pvtFlash::H2S );
   EXPECT_EQ ( TSR_Tables::H2S_H2Om, oneTable );

   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::COX );
   EXPECT_EQ ( TSR_Tables::CO2_H2Om, oneTable );

   cauldronSalinity = 0.1;
   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::H2S );
   EXPECT_EQ ( TSR_Tables::H2S_H2O_NaCl1m, oneTable );

   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::COX );
   EXPECT_EQ ( TSR_Tables::CO2_H2O_NaCl1m, oneTable );
 
   cauldronSalinity = 0.15;
   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::H2S );
   EXPECT_EQ ( TSR_Tables::H2S_H2O_NaCl2m, oneTable );

   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::COX );
   EXPECT_EQ ( TSR_Tables::CO2_H2O_NaCl2m, oneTable );
 
   cauldronSalinity = 0.25;
   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::H2S );
   EXPECT_EQ ( TSR_Tables::H2S_H2O_NaCl4m, oneTable );

   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::COX );
   EXPECT_EQ ( TSR_Tables::CO2_H2O_NaCl4m, oneTable );

   cauldronSalinity = 0.4;
   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::H2S );
   EXPECT_EQ ( TSR_Tables::H2S_H2O_NaCl6m, oneTable );
  
   oneTable = tables.getTableName( cauldronSalinity, pvtFlash::COX );
   EXPECT_EQ ( TSR_Tables::CO2_H2O_NaCl6m, oneTable );
}

TEST( TSRlookupTest, GetValueForSalinity )
{
   TSR_Tables::LookUpDirectory tables;
  
   TSR_Tables::dataValue oneValue;
   double cauldronSalinity = 0.01;

   bool result = tables.getValueForSalinity ( cauldronSalinity, pvtFlash::H2S, 1, 0, oneValue );
   EXPECT_DOUBLE_EQ ( 0.0031675776551219623, oneValue.first );
   EXPECT_DOUBLE_EQ ( 0.0, oneValue.second );
   EXPECT_TRUE (  result );

   result = tables.getValueForSalinity ( cauldronSalinity, pvtFlash::COX, 1, 0, oneValue );
   EXPECT_DOUBLE_EQ ( 0.0013333701143596641, oneValue.first );
   EXPECT_DOUBLE_EQ ( 0.0, oneValue.second );
   EXPECT_TRUE (  result );
}
