function [mapProperties] =  readcomputedwelldataforinversion(file)
%function [n_entries ComputedWellData mapProperties] =  readcomputedwelldataforinversion(file)

% Open the file
fprintf('Opening file = %s \n',file);
fid22 = fopen(file,'r');
if(fid22 == -1) 
    fprintf('Unable to open file with ID %d \n', fid22 );    
end


% 2 headerlines (1 folder/file name,  2 properties name)
[X_m Y_m Depth_m  Hydrostatic_MPa Overpressure_MPa Pressure_MPa Lithostatic_MPa FracPres_MPa VES_MPa MaxVES_MPa Porosity Kv_log10_mD Kh_log10_mD ThCondv ThCondh Temperature_C  VRe  BulkDensity Viscosity  Water_Density] = textread(file, '%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f','headerlines',2);

 mapProperties = containers.Map;
 
 mapProperties('Depth') = Depth_m;
 mapProperties('Temperature') = Temperature_C;
 mapProperties('BulkDensity') = BulkDensity;
 mapProperties('Pressure') = Pressure_MPa;
 mapProperties('Porosity') = Porosity;
 mapProperties('Kv') = Kv_log10_mD;
 mapProperties('Kh') = Kh_log10_mD;
 mapProperties('Permeability') = Kh_log10_mD;
 mapProperties('VrVec2') = VRe;




% % Read the header information
% header1 = fgetl(fid22);
% header2  = fgetl(fid22);
% %fprintf('header1 = %s \n',header1);
% %fprintf('header2 = %s \n',header2);
% Ncells = 500;
% firstpass = 0;
% eofstat = 0;
% for(c = 1:Ncells)
%    %if( mod(c,100)==0)
%       %fprintf('reading computed well data entry %d \n', c);  
%    %end
%    eofstat = feof(fid22);
%    if(eofstat == 1 && firstpass ==0)
%        n_entries = c-1;
%        fprintf('end of computed well data file, n_entries = %d \n', n_entries);
%        firstpass = 1;
%    end
%    dummy          = fscanf(fid22,'%f',1);  % 1
%    test1          = isempty(dummy);
%    if(eofstat == 0 && test1 == 0 )
%      X_m          = dummy;
%      Y_m          = fscanf(fid22,'%f',1);  % 2
%      Depth_m      = fscanf(fid22,'%f',1);  % 3
%      Hydrostatic  = fscanf(fid22,'%f',1);  % 4
%      Overpressure = fscanf(fid22,'%f',1);  % 5
%      Pressure     = fscanf(fid22,'%f',1);  % 6 
%      Lithostatic  = fscanf(fid22,'%f',1);  % 7
%      FracPres     = fscanf(fid22,'%f',1);
%      Ves          = fscanf(fid22,'%f',1);  % 8 
%      Maxves       = fscanf(fid22,'%f',1);  % 9
%      porosity     = fscanf(fid22,'%f',1);  % 10
%      Kv           = fscanf(fid22,'%f',1);  % 11
%      Kh           = fscanf(fid22,'%f',1);  % 12
%      ThCondv      = fscanf(fid22,'%f',1);  % 13
%      ThCondh      = fscanf(fid22,'%f',1);  % 14
%      Temperature  = fscanf(fid22,'%f',1);  % 15
%      Vre          = fscanf(fid22,'%f',1);  % 16
%      BulkDensity      = fscanf(fid22,'%f',1);
%      viscosity    = fscanf(fid22,'%f',1);  % 17
%      water_density= fscanf(fid22,'%f',1);  % 18
%      
%      ComputedWellData(c).Wellname     = file;
%      ComputedWellData(c).Depth        = Depth_m;
%      ComputedWellData(c).Temperature  = Temperature;
%      ComputedWellData(c).Vr           = Vre;
%      ComputedWellData(c).Pressure     = Pressure;
%      ComputedWellData(c).Porosity     = porosity;
%      ComputedWellData(c).Ves          = Ves;
%      ComputedWellData(c).Overpressure = Overpressure;
%      ComputedWellData(c).Lithostatic  = Lithostatic;
%      ComputedWellData(c).Hydrostatic  = Hydrostatic;
%      ComputedWellData(c).BulkDensity      = BulkDensity;
%      ComputedWellData(c).Kv = Kv;
%      ComputedWellData(c).Kh = Kh;
%      
% %      
% %    WellDataWellname(c)    = file;
% %    WellDataDepth(c)        = Depth_m;
% %    WellDataTemperature(c)  = Temperature;
% %    WellDataVr(c)           = Vre;
% %    WellDataPressure(c)     = Pressure;
% %    WellDataPorosity(c)     = porosity;
% %    WellDataVes(c)          = Ves;
% %    WellDataOverpressure(c) = Overpressure;
% %    WellDataLithostatic(c)  = Lithostatic;
% %    WellDataHydrostatic(c)  = Hydrostatic;
% %    WellDataBulkDensity(c)      = BulkDensity;
% %    WellDataKv(c) = Kv;
% %    WellDataKh(c) = Kh;
%      
%      %fprintf('c= %d, depth = %f, pressure = %f \n', c,
%      %ComputedWellData(c).Depth, ComputedWellData(c).Pressure); 
%      n_entries = c;
%    end
% end
% 
% % mapProperties = containers.Map;
% % mapProperties('Depth') = WellDataDepth;
% % mapProperties('Temperature') = WellDataTemperature;
% % mapProperties('BulkDensity') = WellDataBulkDensity;
% % mapProperties('Pressure') = WellDataPressure;
% % mapProperties('Porosity') = WellDataPorosity;
% % mapProperties('Kv') = WellDataKv;
% % mapProperties('Kh') = WellDataKh;
% % mapProperties('Permeability') = WellDataKh;
% % mapProperties('Vr') = WellDataVr;



fclose(fid22);

%maxdepth = -10000;
%n_entries = n_entries - 1;
