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
  gnuplot<<EOF
set xdata time
set timefmt '"%Y-%m-%d %H:%M:%S"'
set terminal postscript portrait enhanced color dashed lw 1 "DejaVuSans" 12
set output "g${i}.ps"
set key title "GPS SV: G${i}"
plot "foo" u 1:(\$5-\$10)*299.792458 w lp
quit
EOF
done
