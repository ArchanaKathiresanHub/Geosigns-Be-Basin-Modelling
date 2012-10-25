#!/bin/bash
#
hosts=`bhosts -s cldrn | grep '^ '`

for host in $hosts; do
    echo $host
    ssh $host $HOME/dokill.ksh
done

