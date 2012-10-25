
function fcopylinesuntilkeyword(keyword, fp_ref, fp_out)


if(fp_ref == -1)
    fprintf('FCopyLinesUntilKeyword: fp_ref == -1 \n');
end

if(fp_out == -1)
    fprintf('FCopyLinesUntilKeyword: fp_out == -1 \n');
end



FoundIt = 0;
while ( feof(fp_ref) == 0 && FoundIt == 0) 
    %reading line by line
    activeline = fgetl(fp_ref); 
    %copying line to a new file  >>>>>>>>>>>>>change this to SPEED UP
    fprintf(fp_out,'%s \n',activeline);%>>>>>>>>>>>>>change this to SPEED UP
    if( strfind( activeline, keyword) ) % find keyword string in activeline
       FoundIt = 1; 
       fprintf('Found keyword \n');
    end
    
end



%void FCopyLinesUntilKeyWord(char *keyWord, FILE *flot, FILE *fout) 
%* Read a file until a keyword, and translate the line after the keyword into a float value */
%	char comment[1030];
%	int getIt;
%	float value = 0;

%	getIt=0;
	
%	do {
%		if (feof(flot)==0) {
%			fgets(comment,1024,flot);			
%			fputs(comment,fout);
%			if (strstr(comment,keyWord)!=0) getIt=1;
%		}
%		
%	} while (getIt==0 && feof(flot)==0);
%}
% search 