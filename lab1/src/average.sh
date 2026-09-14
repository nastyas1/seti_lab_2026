sum=0
count=$#
for arg in "$@"; do
    sum=$(echo "$sum + $arg" | bc)
done
average=$(echo "scale=2; $sum / $count" | bc)
echo "count arg: $count"
echo "average: $average"
