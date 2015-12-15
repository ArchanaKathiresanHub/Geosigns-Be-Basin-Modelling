function [x y z temperature heat_min heat_max heat_init] = readwelllocation(welllocation)
            fid = fopen(welllocation);
            C = textscan(fid, '%s %f %f %f %f %f %f %f ', 'Headerlines', 1); % name x y z temp heat_min max ini
            fclose(fid);
%             x = roundn(C{2},3);
%             y = roundn(C{3},3);
%             z = roundn(C{4},3);
            
            %without rounding
             x = C{2};
             y = C{3};
             z = C{4}; 
             temperature = C{5};
             heat_min = C{6};
             heat_max = C{7}; 
              heat_init = C{8};

              
              
             
%             heatDataGrid=textscan(fh, '%d %d %f');
%             xx = heatDataGrid{1};
%             yy = heatDataGrid{2};
%             heat = heatDataGrid{3};
            
            
%             fout = fopen('well_heat.dat','w');
%             for i=1:length(z)
%                 for j=1:length(heatDataGrid{1})
%                   if x(i)==xx(j) && y(i)==yy(j)
%                       fprintf(fout,'%d %d %f \n',xx(j),yy(j),heat(j));
%                       %fprintf('%d %d %f \n',xx(j),yy(j),heat(j));
%                     break;  
%                   end                    
%                 end               
%             end
end
