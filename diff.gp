set title "fibdrv kernel 計算時間"
set xlabel "n"
set ylabel "time(ns)"
set xtics 0, 10
set key left

plot \
"fast.txt" using 1:3 with linespoints linewidth 2 title "快速費氏優化前",\
"new_fast.txt" using 1:3 with linespoints linewidth 2 title "快速費氏優化後",\
"client.txt" using 1:3 with linespoints linewidth 2 title "原本版本"
pause -1
