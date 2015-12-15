
funList = {@fun1,@fun2,@fun3};
dataList = {100,200,300}; %# or pass file names 

parfor i=1:length(funList)
    %# call the function
    funList{i}(dataList{i});
end