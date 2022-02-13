set   autoscale                        # scale axes automatically
unset log                              # remove any log-scaling
unset label                            # remove any previous labels
set xtic auto                          # set xtics automatically
set ytic auto                          # set ytics automatically
set title "Transmission Rate"
set xlabel "Time"
set ylabel "BPS"

set style line 1 linecolor rgb '#0060ad' linetype 1 linewidth 2 pointtype 7 pointsize 1.5
plot    "stats.dat" with linespoints linestyle 1
pause -1
