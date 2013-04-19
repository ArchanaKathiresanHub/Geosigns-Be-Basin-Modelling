#ifndef __global_strings__
#define __global_strings__

namespace Graph_Properties 
{
   static const char* Pressure = "Pressure";
   static const char* Temperature = "Temperature";
   static const char* VolumeOil = "VolumeOil";
   static const char* VolumeGas = "VolumeGas";
   static const char* TrapCapacity = "TrapCapacity";
   static const char* TrapIoTbl = "TrapIoTbl";
   static const char* MigrationIoTbl = "MigrationIoTbl";
   static const char* TotalGas = "TotalGas";
   static const char* TotalOil = "TotalOil";
   static const char* WetGas = "WetGas";
   static const char* StockTank = "Stock Tank Conditions";
   static const char* Masses = "Masses";
   static const char* OilPhase = "Oil Phase";
   static const char* GasPhase = "Gas Phase";
}

namespace UserSettings_NS
{
   static const char* OilGasVolumeRangeMin = "/OilGasVolumeRange/Min";
   static const char* OilGasVolumeRangeMax = "/OilGasVolumeRange/Max";
   static const char* OilGasVolumeRangeEnabled = "/OilGasVolumeRange/Enabled";
   
   static const char* MassRangeMin = "/MassRange/Min";
   static const char* MassRangeMax = "/MassRange/Max";
   static const char* MassRangeEnabled = "/MassRange/Enabled";
   
   static const char* SnapshotRangeMin = "/SnapshotRange/Min";
   static const char* SnapshotRangeMax = "/SnapshotRange/Max";
   static const char* SnapshotRangeEnabled = "/SnapshotRange/Enabled";
   
   static const char* TempPressRangeMin = "/TempPressRange/Min";
   static const char* TempPressRangeMax = "/TempPressRange/Max";
   static const char* TempPressRangeEnabled = "/TempPressRange/Enabled";
}

#endif
