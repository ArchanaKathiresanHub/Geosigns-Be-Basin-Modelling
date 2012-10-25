
%to add datamining io table to project3d file
function insertdataminingiotable(fp_ref, fp_out, Input, Calibrationdata)

if(fp_ref == -1)
    fprintf('insertdataminingiotable fp_ref == -1 \n');
end

if(fp_out == -1)
    fprintf('insertdataminingiotable: fp_out == -1 \n');
end
keyword = '[DataMiningIoTbl]';

fcopylinesuntilkeyword(keyword, fp_ref, fp_out );

%when the keyword is found,  read two more lines before writing into
% read line with ;
 activeline = fgetl(fp_ref); 
 fprintf(fp_out,'%s \n',activeline);
% skip line with property names such as BPAKey Time Xcoord etc.
 activeline = fgetl(fp_ref); 
 fprintf(fp_out,'%s \n',activeline);
% skip line with property units () () (Ma) etc.
 activeline = fgetl(fp_ref); 
 fprintf(fp_out,'%s \n',activeline);
 
 for(param=1:Input.NCalibParam)
    for(d=1:Input.CalibrationParameters(param).Points)
        x = Calibrationdata(param).Values(d).X ;
        y = Calibrationdata(param).Values(d).Y  ;
        z = Calibrationdata(param).Values(d).Z  ;
        datatype = Calibrationdata(param).Values(d).PropertyName ;
        %value = Calibrationdata(param).Values(d).Observed;
        calibrationline = [' ""   0 ' num2str(x) ' '  num2str(y) ' ' num2str(z)  ' ""   ""   "" "' datatype '" "" -9999'];
         fprintf(fp_out,'%s \n', calibrationline); 
    end
 end

fcopylinesuntilkeyword('TheUltimateEnd', fp_ref, fp_out);