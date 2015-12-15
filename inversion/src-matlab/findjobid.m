function [Job_Id] = findjobid(string)

%string = ['Job <570686> is submitted to default queue <untouchable>.']

bracket1 = findstr(string, '<');
%fprintf('First bracket is at %d \n', bracket1(1) );

bracket2 = findstr(string, '>');
%fprintf('Second bracket is at %d \n', bracket2(1) );

min = bracket1+1;
max = bracket2-1;

Job_Id = '';
for (j=min:max)
    Job_Id = strcat(Job_Id,string(j));    
end

Job_Id = str2num(Job_Id);

%fprintf('string is %s \n', num2str(Job_Id));








