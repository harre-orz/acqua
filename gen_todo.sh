#!/bin/sh

echo '/*! \page TODOリスト'
grep -rn TODO ./include | sed -e 's/^.\// * /'
echo '*/'

echo '/*! \page BUGSリスト'
grep -rn 'BUGS:' $1
echo '*/'
