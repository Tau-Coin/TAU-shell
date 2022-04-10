#!/bin/bash
DATE=$(date +%Y%m%d_%H)

LOG0=log_0
LOG1=log_1
LOG2=log_2
LOG3=log_3
LOG4=log_4

tar -zcvf ./data/${LOG0}_${DATE}.tar.gz ./data/${LOG0}

RMDATE=$(date "+%Y%m%d_%H" -d "-3 days")

TAR_BAK="./data/log_*_$RMDATE.tar.gz"

rm -rf $TAR_BAK
