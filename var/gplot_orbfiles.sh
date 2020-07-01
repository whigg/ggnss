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
  title=$(cat nav.g${i} | head -1)
  gnuplot<<EOF
set xdata time
set timefmt '"%Y-%m-%d %H:%M:%S"'
# set terminal postscript portrait enhanced color dashed lw 1 "DejaVuSans" 12
set terminal pngcairo size 1400,600 enhanced font 'Helvetica,12'
set output "g${i}.png"

if (!exists("MP_LEFT"))   MP_LEFT = .1
if (!exists("MP_RIGHT"))  MP_RIGHT = .95
if (!exists("MP_BOTTOM")) MP_BOTTOM = .1
if (!exists("MP_TOP"))    MP_TOP = .9
if (!exists("MP_GAP"))    MP_GAP = 0.05

set multiplot layout 2,1 columnsfirst title "{/:Bold=15 ${title}}" \
      margins screen MP_LEFT, MP_RIGHT, MP_BOTTOM, MP_TOP spacing screen MP_GAP

#plot "foo" u 1:(\$2-\$7) title "Nav Vs Sp3 X (m)" w lp
#plot "foo" u 1:(\$3-\$8) title "Nav Vs Sp3 Y (m)"w lp
#plot "foo" u 1:(\$4-\$9) title "Nav Vs Sp3 Z (m)" w lp
plot "foo" u 1:5 title "Sp3 Clock(sec)" w lp, "foo" u 1:10 title "Nav Clock(sec)" w lp
plot "foo" u 1:(\$5-\$10)*299.792458 title "Nav Vs Sp3 Clock(m)" w lp
quit
EOF
done
