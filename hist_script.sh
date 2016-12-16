#!/bin/sh

gnuplot &
child = $!
cat 'plot_template.txt' > '/proc/${child}/fd/0';
./ftw8 >> '/proc/${child}/fd/0';
echo 'e' >> '/proc/${child}/fd/0';