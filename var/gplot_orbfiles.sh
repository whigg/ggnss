#! /bin/bash

#for i in {01..32} ; do
#  gnuplot<<EOF
#set xdata time
#set timefmt '"%Y-%m-%d %H:%M:%S"'
#set terminal postscript portrait enhanced color dashed lw 1 "DejaVuSans" 12
#set output "g${i}.ps"
#set key title "GPS SV: G${i}"
#plot "nav.g${i}" u 1:5 w lp, "igs.g${i}" u 1:5 w lp
#quit
#EOF

#for i in `seq 1 32` ; do
#  cat warn.log | grep "##SV G${i} " | sed 's/##//g' > foo
#  j=$i
#  if test $i -lt 10 ; then j="0${i}" ; fi
#  gnuplot<<EOF
#set xdata time
#set timefmt '"%Y-%m-%d %H:%M:%S"'
#set terminal postscript portrait enhanced color dashed lw 1 "DejaVuSans" 12
#set output "g${j}.ps"
#set key title "GPS SV: G${j} (from PPP)"
#plot "igs.g${j}" u 1:(\$5*1e0) w lp, "nav.g${j}" u 1:5 w lp, "foo" u 14:12 w lp,
#quit
#EOF
#done

for i in {01..32} ; do
  var/pasteoncol.py sp3.g${i} nav.g${i} > foo
  title=$(cat nav.g${i} | grep "## SV:" | sed 's/##//g')
  gnuplot<<EOF
# set terminal postscript portrait enhanced color dashed lw 1 "DejaVuSans" 12
set terminal pngcairo dashed enhanced
# set xdata time
# set timefmt '"%Y-%m-%d %H:%M:%S"'
set output "g${i}.png"
set multiplot layout 2,1 title "${title}"
#set key title "${title}"
set xlabel "sec in day"
set ylabel "micro-seconds"
plot "foo" u 2:6 w lp title "Nav Clock", "foo" u 2:11 w lp title "Sp3 Clock"
set ylabel "meters"
plot "foo" u 2:(\$6-\$11)*299.792458 w lp title "nav-sp3 clock diff in meters"
quit
EOF
done
