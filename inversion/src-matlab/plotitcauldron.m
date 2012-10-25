
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Function:         PlotIt(FVr_temp,iter,S_struct)
% Author:           Rainer Storn
% Description:      PlotIt can be used for special purpose plots
%                   used in deopt.m.
% Parameters:       FVr_temp     (I)    Paramter vector
%                   iter         (I)    counter for optimization iterations
%                   S_Struct     (I)    Contains a variety of parameters.
%                                       For details see Rundeopt.m
% Return value:     -
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [S_struct] = plotitcauldron(Misfit,cme,FVr_temp,iter,S_struct, Calibrationdata, Input)

%where cme is  S_bestval.FVr_oa(1);

%-----------Pressure well plot----------------------------
  
  batch_id = S_struct.batch_winner;
  best_fit_k = S_struct.best_fit_k;
  NWells = Input.NWells;
  
  
  if(batch_id ==1)
       best_fit_k = S_struct.best_initialisation_run;
       fprintf('PlotIt-Cauldron: Best initialisation run = %d \n', S_struct.best_initialisation_run );
  end
  
  %=============================
  % PLOT 1 - All wells & trends
  %=============================
  for(w=1:NWells)
      % Get the well name
      wellfile = [Input.LaunchDir Input.BaseName '_' int2str(batch_id) '_' int2str(best_fit_k) '_CauldronOutputDir/' Input.Well(w).Name];
      outwellfile = [Input.OutputDir Input.BaseName 'Well_' Input.Well(w).Name '_Gen_' int2str(iter) '_results.well'];
      copywellfile=['cp' ' ' wellfile ' ' outwellfile];
      system(copywellfile);
      %fprintf('test opening %s \n', wellfile);
      fid22 = fopen(wellfile,'r');
      if(fid22 == -1)
          fprintf('Cannot find Cauldron well file %s \n',wellfile);
      end
      fclose(fid22);
      pause(5);
      % Get the well data
      %[n_entries ComputedWellData mapProperties] =  readcomputedwelldataforinversion(wellfile);
      [mapProperties] =  readcomputedwelldataforinversion(wellfile);
      for(param=1:Input.NCalibParam)
          if(mapProperties.isKey(Input.CalibrationParameters(param).PropertyName))
              %IMPORTANT nighlty version doesnt print vrvec2 values
              %correctly to .well file. so use alternate way
              %SimulatedWellDataProperty(param).Values =  mapProperties(Input.CalibrationParameters(param).PropertyName);
              project3dname = [Input.LaunchDir Input.BaseName '_' int2str(batch_id) '_' int2str(best_fit_k) '.project3d']
              [DataMiningIoTbl_run]   = readcauldrondataminingiotbl(project3dname);
              N_entries = length(DataMiningIoTbl_run); % total calib points     
              propCal = ['"' Input.CalibrationParameters(param).PropertyName '"'];
                for(d=1:N_entries)
                    if(strcmp(propCal,DataMiningIoTbl_run(d).PropertyName)==1)
                        SimulatedWellDataProperty(param).Values(d) = DataMiningIoTbl_run(d).Value;
                        SimulatedWellDataProperty(param).ZCoord(d) = DataMiningIoTbl_run(d).ZCoord;
                    end
                end
              
          else
              error('Error, well data file doesnt contain the Calibration Parameter');
          end
      end
      
      
      for(param=1:Input.NCalibParam)
          for(d=1:Input.CalibrationParameters(param).Points)
              if( strcmp(Calibrationdata(param).Values(d).PropertyName,Input.CalibrationParameters(param).CalibrationData(d).PropertyName) == 1   && w == Calibrationdata(param).Values(d).WellIndex )
                  CalibrationParameters(param).Depth(d) = -Calibrationdata(param).Values(d).Z;
                  CalibrationParameters(param).Data(d)  = Calibrationdata(param).Values(d).Observed;
              end
          end
      end
      figure(1);%% >>>> added to avoid overwriting the original
      for(param=1:Input.NCalibParam)
          %figure;%% >>>> added to avoid overwriting the original
          k_phi_plot = subplot(1,Input.NCalibParam,param);
          valueStr = mat2str(SimulatedWellDataProperty(param).Values);
          plotOper = Input.CalibrationParameters(param).PlotOperator;
          evalStr = [ plotOper '(' valueStr ')'];
          Values = eval(evalStr);
                    
          %plot(Values,-mapProperties('Depth'),'-ko');
          %results = [Values -mapProperties('Depth')];
          plot(Values,-SimulatedWellDataProperty(param).ZCoord,'-k*');
          results = [Values',SimulatedWellDataProperty(param).ZCoord'];
          %outputfilenm = [Input.OutputDir 'Well_' Input.Well(w).Name '_Gen_' int2str(iter) '_Param_' int2str(param) '_plot.dat'];
          outputfilenm = [Input.OutputDir Input.BaseName 'Well_' Input.Well(w).Name '_Gen_' int2str(iter) '_Param_' int2str(param) '_plot.dat'];
          dlmwrite(outputfilenm, results, 'delimiter', '\t','precision', 6);
          hold on
          scatter(CalibrationParameters(param).Data,CalibrationParameters(param).Depth,'g','filled');
          %scatter(CalibrationParameters(param).Data,CalibrationParameters(param).Depth,'g');
          title(sprintf(Input.CalibrationParameters(param).PropertyName), 'interpreter','none');
          grid on
          set(k_phi_plot,'YLim',[-Inf 0]);
      end
      % Save the screen as an image
      outputfilenm = [Input.OutputDir 'Well_' Input.Well(w).Name '_Gen_' int2str(iter) '_plot.png'];
      print(1,'-dpng','-r300',outputfilenm);
      

      pause(2)
      
      % clear variables
      %clear maxdepth;
      %clear n_entries;
      %clear ComputedWellData;
      clear CalibrationParameters(param).Depth;
      clear CalibrationParameters(param).Depth;
      close(1) % close the figure
  end % end of well loop
 
  %=============================
  % PLOT 2 Accummulate and plot the convergence data
  %=============================
  figure(2);%% >>>> added to avoid overwriting the original
  ConvergenceHistory = S_struct.ConvergenceHistory;
  ConvergenceHistory.Bestcost(iter) = cme;            % best cost per generation
  for(k=1:S_struct.I_NP)
      ConvergenceHistory.Allcost(k,iter) = Misfit(k);
  end
   
  for(n=1:Input.NVar)                                  % Corresponding variables of bes settings this generation
     ConvergenceHistory.VarBest(n,iter) = FVr_temp(n); % This generations best settings
  end
  
  % Variable settings for all draws!
  for(k=1:S_struct.I_NP)      % number of runs in one batch
      for(q=1:S_struct.I_D)   % number of variables to fit
          ConvergenceHistory.Allsettings(iter,k,q) = S_struct.FM_pop(k,q);
      end
  end  % iter = generation, k = Nruns (population), q = variables
  
  S_struct.ConvergenceHistory = ConvergenceHistory;
  
  for(i=1:iter)
    convergence_scores(i) = ConvergenceHistory.Bestcost(i);
    x_axisvalue(i) = i;
  end
  
  plotTitle = [Input.BaseName ' best cost ' int2str(cme) ];
  %plot(x_axisvalue,convergence_scores,'r');
  scatter(x_axisvalue,convergence_scores,'ro','filled');
  %scatter(x_axisvalue,convergence_scores,'ro');
  grid on
  title(plotTitle,  'interpreter','none');
  %axis([1 S_struct.I_itermax 0 1]);
  outputfilenm = [Input.OutputDir Input.BaseName '_BestCost_Gen_' int2str(iter) '_plot'];
  print(2,'-dpng','-r300',outputfilenm);
  pause(5);
  close(2);
  % delete plot for previous iteration, if iter>1
  if iter>1
      plot2remove=[Input.OutputDir Input.BaseName '_BestCost_Gen_' int2str(iter-1) '_plot.png'];
      plotremove=['rm -f  ' plot2remove];
      system(plotremove);
  end

  
   %=============================
  % PLOT 3 Make a scatter plot of all the cost values sofar
   %=============================
   figure(3);%% >>>> added to avoid overwriting the original
  for(i=1:iter)
      clear temp_error
      clear x_axisvalue
      for(k=1:S_struct.I_NP)
          temp_error(k) = ConvergenceHistory.Allcost(k,i);
          x_axisvalue(k) = i;
          scatter(x_axisvalue,temp_error,'ro','filled');
          %scatter(x_axisvalue,temp_error,'ro');
          hold on
      end
      grid on
      hold on
  end
  
  plotTitle = [Input.BaseName ' bulk cost ' int2str(cme) ];
  title(plotTitle, 'interpreter','none');
  %axis([1 S_struct.I_itermax 0 1]);
  outputfilenm = [Input.OutputDir Input.BaseName '_BulkCost_Gen_' int2str(iter) '_plot'];
  print(3,'-dpng','-r300',outputfilenm);
  pause(5);
  close(3);
    % delete plot for previous iteration, if iter>1
  if iter>1
      plot2remove=[Input.OutputDir Input.BaseName '_BulkCost_Gen_' int2str(iter-1) '_plot.png'];
      plotremove=['rm -f  ' plot2remove];
      system(plotremove);
  end
   %=============================
  % PLOT Variable history 
   %=============================
  %---Parameter settings plot for the three variables
  figure(4);%% >>>> added to avoid overwriting the original
  for(p=1:S_struct.I_D)       % number of variables to fit
      plot_index = p;
      subplot(ceil(Input.NVar/2),2,plot_index)
      hold on
      grid on
      axis([1 S_struct.I_itermax S_struct.FVr_minbound(1,p)  S_struct.FVr_maxbound(1,p)]);
      
      % Add all the values as a circle
      for(i=1:iter)                %iteration/generation
          for(k=1:S_struct.I_NP)    %run
              variable(k) = ConvergenceHistory.Allsettings(i,k,p);
              temp_iterator(k)= i;
          end
           scatter(temp_iterator,variable,'filled','b');
              %scatter(temp_iterator,variable,'b');
          axis([1 S_struct.I_itermax S_struct.FVr_minbound(1,p)  S_struct.FVr_maxbound(1,p)]);
      end
      
      clear temp_iterator
      clear variable     
      % Draw the best fitt values as a triangle
      for(i=1:iter)
          for(k=1:S_struct.I_NP)
              variablebest(k) = ConvergenceHistory.VarBest(p,i);
              temp_iterator(k)= i;   
          end
          scatter(temp_iterator,variablebest,'r^','filled');
              %scatter(temp_iterator,variablebest,'r^');
              axis([1 S_struct.I_itermax S_struct.FVr_minbound(1,p)  S_struct.FVr_maxbound(1,p)]);
      end
      
      clear temp_iterator
      clear variablebest
      
      title(Input.InversionParameters(p).Name,'interpreter','none' );
  end
  hold off
  outputfilenm = [Input.OutputDir Input.BaseName '_VariableHist_' int2str(iter) '_plot'];
  print(4,'-dpng','-r300',outputfilenm);

  close(4);
   % delete plot for previous iteration, if iter>1
  if iter>1
      plot2remove=[Input.OutputDir Input.BaseName '_VariableHist_' int2str(iter-1) '_plot.png'];
      plotremove=['rm -f  ' plot2remove];
      system(plotremove);
  end
 
  fprintf('Finished plotting ...\n');
  pause(1); %wait for one second to allow convenient viewing
  return
