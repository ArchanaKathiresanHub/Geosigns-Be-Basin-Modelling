function [line] = readline(fid)
line = fgetl(fid);
line = skipemptyorcommentlines(fid,line);%skip if any empty lines
end