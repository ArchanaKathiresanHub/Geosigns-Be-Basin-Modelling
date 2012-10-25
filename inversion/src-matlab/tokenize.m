function [newtokens] = tokenize(string,delimiter)
tokens = {};
QuotationMark=0;
while ~(isempty(string))
    [ss,string] = strtok(string,delimiter);
    if ~isempty(ss) 
       tokens{end+1} = ss;
    end
    if(strfind(ss,'"'))
        QuotationMark=1;
    end
end
if(QuotationMark)
    %check for quotes in string line like "December 10 2010"
    newtokens = {};
    iloop = 1;
    while (iloop <= numel(tokens))
        if(isempty(strfind(tokens{iloop},'"')))
            [newtokens{end+1}] = tokens{iloop};
        elseif( strfind(tokens{iloop},'"')  == [1,length(tokens{iloop})])
            [newtokens{end+1}] = tokens{iloop};
%             str = tokens{iloop};
%             newstr=str(2);
%             for i=3:length(str)-1
%                 newstr=horzcat(newstr,str(i));
%             end
%             [newtokens{end+1}] = newstr;
        elseif(strfind(tokens{iloop},'"')==1)
            newstring = tokens{iloop};
            iloop=iloop+1;
            pos = strfind(tokens{iloop},'"');
            strlength = length(tokens{iloop});
            while(isempty(pos) || pos~=strlength)  %%(isempty(pos)) %% || pos~=strlength) %%(strfind(tokens{i},'"'))) %~=length(tokens{i}) )
                newstring = horzcat(newstring,delimiter,tokens{iloop});
                iloop=iloop+1;
                pos = strfind(tokens{iloop},'"');
                strlength = length(tokens{iloop});
            end
            newstring = horzcat(newstring,delimiter,tokens{iloop});
            [newtokens{end+1}] = newstring;
        end
        iloop=iloop+1;
    end
else
    newtokens = tokens;
end

%     for i = (1:numel(tokens)-1)
%         fprintf([tokens{i} '.'])
%     end
%
%     fprintf([tokens{end} '\n'])
end