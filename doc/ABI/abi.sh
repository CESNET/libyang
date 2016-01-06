#!/bin/sh
ABI_MON=abi-monitor
ABI_TRK=abi-tracker

which $ABI_MON > /dev/null || exit 1
which $ABI_TRK > /dev/null || exit 1

if test $1; then
	LIMIT="-limit $1"
	l=$1
fi

test -e gitrepo || git clone https://github.com/CESNET/libyang.git gitrepo
test -e packages || mkdir packages
test -e src/libyang || mkdir -p src/libyang
rm -rf src/libyang/*

cd gitrepo
git checkout devel
for i in `git log --grep=VERSION --oneline | sed 's/ .* /_/'`; do
	if test "$l" -eq 0; then
		rm -rf ../abi_dump/libyang/${i//*_}
		rm -rf ../installed/libyang/${i//*_}
	else
		l=$(($l-1))
		if test ! -f ../packages/libyang-${i//*_}.tgz; then
			git checkout ${i//_*}
			tar -czf ../packages/libyang-${i//*_}.tgz gitrepo
		fi
		cp ../packages/libyang-${i//*_}.tgz ../src/libyang/
	fi
done
cd ..

cp libyang.json libyang.aux
abi-monitor -get -build $LIMIT libyang.aux
abi-tracker -build libyang.aux
#rm -rf libyang.aux

