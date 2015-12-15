function  modifygridwindow(project3dfile,outputfilename,WindowXMin, WindowXMax, WindowYMin, WindowYMax)

    finput = fopen(project3dfile,'r');
    fout=fopen(outputfilename,'w');
    if(finput == -1)
        fprintf('cannot read .project3dfile \n');
    end
    keywordHeader  = '[ProjectIoTbl]';

    FoundHeader = 0;
    
   % fields to read
   
    
    %------------------------------------------
    % Find the header of the datamining io tbl
    %------------------------------------------
    while ( feof(finput) == 0 && FoundHeader == 0)
        line = fgetl(finput);
         fprintf(fout,'%s\n',line);
        if( strfind( line, keywordHeader) )
            FoundHeader = 1;
        end
    end
    if(FoundHeader == 1)
        line = fgetl(finput);% skip ;
        fprintf(fout,'%s\n',line);
        line = fgetl(finput);% read field names
        position=tokenposition(line,'WindowXMin',' ');%space as delimter
        fprintf(fout,'%s\n',line);      
       
       line = fgetl(finput);% skip unit row
       fprintf(fout,'%s\n',line);
       line = fgetl(finput);
       lineArray=tokenize(line, ' ');      
       lineArray{position} = num2str(WindowXMin);
       lineArray{position+1} = num2str(WindowXMax);
       lineArray{position+2} = num2str(WindowYMin);
       lineArray{position+3} = num2str(WindowYMax);       
        newline = lineArray{1};
                for i=2:numel(lineArray)
                    newline = horzcat(newline,' ',lineArray{i});
                end
       fprintf(fout,'%s\n',newline);
    end
     while ~feof(finput)
         line = fgetl(finput); 
         fprintf(fout,'%s\n',line);
     end
    fclose(finput);
    fclose(fout);
end