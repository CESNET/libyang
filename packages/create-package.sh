#!/usr/bin/env bash

if [ "$TRAVIS_PULL_REQUEST" == "true" ] ; then
    exit 0
fi
echo -e "[general]\napiurl = https://api.opensuse.org\n\n[https://api.opensuse.org]\nuser = $user\npass = $password" >~/.oscrc
cd $HOME/build/$TRAVIS_REPO_SLUG/build
osc checkout home:xvican01
if [ $TRAVIS_BRANCH == "devel" ]; then
	package="home:xvican01/libyang-experimental"
	name="libyang-experimental"
else
	package="home:xvican01/libyang"
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
