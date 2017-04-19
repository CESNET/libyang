#!/usr/bin/env bash

if [ "$TRAVIS_PULL_REQUEST" == "true" -o "$TRAVIS_EVENT_TYPE" != "cron"] ; then
    exit 0
fi
# check osb_user and osb_pass
if [ -z "${osb_user}" -o -z "${osb_pass}" ]; then
    exit 0
fi

echo -e "[general]\napiurl = https://api.opensuse.org\n\n[https://api.opensuse.org]\nuser = ${osb_user}\npass = ${osb_pass}" >~/.oscrc
cd $HOME/build/$TRAVIS_REPO_SLUG/build
osc checkout home:liberouter
if [ $TRAVIS_BRANCH == "devel" ]; then
	package="home:liberouter/libyang-experimental"
	name="libyang-experimental"
else
	package="home:liberouter/libyang"
	name="libyang"
fi
cp packages/* $package
VERSION=$(cat CMakeCache.txt | grep "LIBYANG_VERSION:STRING=" | sed 's/LIBYANG_VERSION:STRING=//')
cd $package
echo -e "$name ($VERSION) stable; urgency=low\n" >debian.changelog
HASHES=(`git log -2 --grep="VERSION .* [0-9]+\.[0-9]+\.[0-9]+" -i -E | grep "commit " | sed 's/commit //' | tr '\n' ' '`)
git log "${HASHES[1]}..${HASHES[0]}" --pretty=format:"  * %s%n" >>debian.changelog
git log -1  --pretty=format:"%n -- %aN <%aE>  %aD%n" >>debian.changelog
wget "https://github.com/CESNET/libyang/archive/$TRAVIS_BRANCH.tar.gz"
osc commit -m travis-update
