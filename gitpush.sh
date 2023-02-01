#!/bin/bash

[ $# -gt 0 ] && {
  commitInfo=$1
}||{
    commitInfo="update"
}

git pull
git add .
git commit -m "`date '+%Y/%m/%d %H:%M:%S'` `whoami` $commitInfo"
git push origin xb
git log|head -n 20
echo "https://github.com/ssbandjl/daos/tree/xb"

