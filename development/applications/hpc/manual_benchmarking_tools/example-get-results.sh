#!/bin/bash

echo -e "DESCRIPTION\tNUMBER\tCORES\tCORES_PER_NODE\tSIM_MODE\tDATA\tTS\tMODEL\tVOLUME\tVOLUME_OPTION\tGROSS\tNETT\tASSEM\tSOLVE\tPROP"
for job 
do
  echo -n `basename $job` | tr '-' '\t'
  echo -ne '\t'
  
  if [ -e $job/output.log ]; then
    gross=`sed -n -e '/real/s/real\s*\([[:digit:]]*\)m\([[:digit:].]*\)s/\1 \2/p' $job/output.log | awk '{ time = $1 * 60 + $2; print time }'`
    echo -ne "$gross\t"
  else
    echo -ne "#N/A\t"
  fi

  if [ -e $job/output-rank-0.log ]; then
    nett=`sed -n -e '/End of simulation/s/End of simulation: \([[:digit:]]*\) Hrs. \([[:digit:]]*\) Mins. \([[:digit:]]*\) Sec./\1 \2 \3/p' $job/output-rank-0.log | awk '{ time = 3600 * $1 + 60 * $2 + $3 ; print time}'`

    assem=`sed -n -e '/^ total System_Assembly_Time/{s/^ total System_Assembly_Time\s*//p;q}' $job/output-rank-0.log`
    solve=`sed -n -e '/^ total System_Solve_Time\s*/{s/^ total System_Solve_Time\s*//p;q}' $job/output-rank-0.log`
    prop=`sed -n -e '/^ total Property_Calculation_Time\s*/{s/^ total Property_Calculation_Time\s*//p;q}' $job/output-rank-0.log` 
 
    echo -e "$nett\t$assem\t$solve\t$prop"

  else
    echo -e "#N/A\t#N/A\t#N/A"
  fi

  
done
