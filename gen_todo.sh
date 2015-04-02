#!/bin/sh

echo "# TODOリスト"
grep -rn TODO $1 | sed -e "s/^.\//* /"
echo ""
echo "## BUGSリスト"
grep -rn 'BUGS:' $1 | sed -e "s/^.\//* /"
