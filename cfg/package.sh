#!/bin/bash

files='/usr/local/sbin/sems 
    /usr/local/lib/sems/plug-in/*   
    /usr/local/lib/sems/audio/*          
    /usr/local/lib/sems/ivr/*           
    /usr/local/etc/sems/sems*.conf 
    /usr/local/etc/sems/etc/* /usr/local/share/doc/sems/README /etc/init.d/sems*'
tar cvzf sems-bin-1.2.1.tar.gz $files
