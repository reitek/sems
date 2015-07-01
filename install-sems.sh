#!/bin/sh

if [ $DEVELDIR == "" ]; then
	echo "DEVELDIR not defined."
	echo "Exiting"
	exit 1
fi

make install \
	DESTDIR=$ENVDIR/dist \
	basedir= \
	prefix=/opt/reitek/sems \
	cfg-prefix= \
	cfg-target=/etc/opt/reitek/sems/
