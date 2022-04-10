#!/bin/bash
DATE=$(date +%Y%m%d_%H)

NUM=5
LOG=log_

for ((i=0;i<${NUM};i++))
do
    LOG_FILE=${LOG}${i}
    tar -zcvf ./data/${LOG_FILE}_${DATE}.tar.gz ./data/${LOG_FILE}
done

RMDATE=$(date "+%Y%m%d_%H" -d "-3 days")

TAR_BAK="./data/log_*_$RMDATE.tar.gz"

rm -rf $TAR_BAK
