#!/bin/bash
DQHOST="piN8"
ssh $DQHOST -q '~/work/MPCEX/dqc.py -w1 -n10000 >>/tmp/dqc.log; scp $(ls -t /mnt/disk1/data/*.dq4 | head -1) office:/tmp/'

