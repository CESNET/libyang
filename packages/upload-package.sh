#!/usr/bin/env bash

GIT_PATH=`git rev-parse --show-cdup`
if [ "$GIT_PATH" != "" ]; then
    echo "Must be run from repository root"
    exit
fi

if [ $# -ne 2 ]; then
    echo "Usage $0 <osb-user> <osb-pass>"
    exit
fi

# for correct git log command output used in changelog
LC_TIME=en_US.UTF-8
# upload package even if the version is not newer
FORCEVERSION=1
# needed so that the package is even created
TRAVIS_EVENT_TYPE=cron
# what package to upload (devel = libyang-experimental, master = libyang)
TRAVIS_BRANCH=`git branch --show-current`

# OSB login
osb_user=$1
osb_pass=$2

# create and upload the package
. ./packages/create-package.sh
