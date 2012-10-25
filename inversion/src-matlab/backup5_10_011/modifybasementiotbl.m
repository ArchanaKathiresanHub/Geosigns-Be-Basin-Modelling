function  modifybasementiotbl(inputfilename,outputfilename, mainVarType,cauldronName,  bottomboundarymodel, delimiter, modifiedVarValue)
   
    fout = fopen(outputfilename,'w');
    finput = fopen(inputfilename,'r');
    if(fout == -1)
        fprintf('creating .project3dfile: fout == -1 \n');
    end
    if(finput == -1)
        fprintf('creating .project3dfile: inputfilename file finput == -1 \n');
    end
    keywordHeader  = '';
    if strcmp(mainVarType,'Basement') % as in .project3d
        keywordHeader  = '[BasementIoTbl]';
    end
    if (isempty(keywordHeader))
        error('Variable type defined is not correct');
    end
    FoundHeader = 0;
    FoundVariableLine=0;
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
        line = fgetl(finput);
        %fprintf(fout,'%s \n',activeline);
        %%now read the cauldron variable header name line
        %find the position of the particular variable column
        position=tokenposition(line,cauldronName,delimiter);  %space as delimter

        while(isempty(position))
            fprintf(fout,'%s\n',line);
            line = fgetl(finput);   
            position=tokenposition(line,cauldronName,delimiter);%space as delimter
        end
        while( feof(finput) == 0 && FoundVariableLine == 0) %% look for variable line
             fprintf(fout,'%s\n',line);
            line = fgetl(finput);
            lineArray=tokenize(line, delimiter);
            %nameInQuotes=['"' bottomboundarymodel '"'];
            nameInQuotes=bottomboundarymodel; % its in quotes
            if(strcmp(lineArray{1},nameInQuotes))
                FoundVariableLine =1;
                lineArray{position} = modifiedVarValue;          
                newline = lineArray{1};
                for i=2:numel(lineArray)
                    newline = horzcat(newline,delimiter,lineArray{i});
                end
                %newlineArray=tokenize(newline, delimiter);
            end
            if(FoundVariableLine ==1)
                fprintf(fout,'%s\n',newline);
            end
        end
    end
    % write the rest of the lines to out
    while ~feof(finput)
         line = fgetl(finput); 
         fprintf(fout,'%s\n',line);
    end

    fclose(finput);
    fclose(fout);

end

    