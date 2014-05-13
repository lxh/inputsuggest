#!/bin/sh

date >> date.log
echo "isformat" >> date.log
./isformat -c config.ini -m 900000 >> date.log
date >> date.log
echo "isdict" >> date.log
#./isdict -c config.ini >> date.log
date >> date.log
echo "isindex" >> date.log
./isindex -c config.ini >> date.log
date >> date.log
echo "over" >> date.log
