#!/bin/bash
ssh phast -q 'scp $(ls -t /tmp/*.dq4 | head -1) office:/tmp'
