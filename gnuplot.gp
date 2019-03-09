set title "fibdrv"
set xlabel "n"
set ylabel "time(ns)"
set xtics 0, 10
set key left

plot \
"client.txt" using 1:2 with linespoints linewidth 2 title "user space時間差",\
"client.txt" using 1:3 with linespoints linewidth 2 title "kernel 計算時間",\
"client.txt" using 1:4 with linespoints linewidth 2 title "kernel傳遞至user space時間開銷"
pause -1
