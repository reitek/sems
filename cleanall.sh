#!/bin/sh

if [ $DEVELDIR == "" ]; then
	echo "DEVELDIR not defined."
	echo "Exiting"
	exit 1
fi

make clean
echo ""
echo ""
rm -fr $DEVELDIR/vendors/sems/dist
echo "cleaned dist folder"


