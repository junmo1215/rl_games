# python -V;

# time ./evaluate.sh

begin_path=10000000
end_path=12950000
max_episode=100

for i in $(seq $begin_path 50000 $end_path)
do
    echo "model_path: saved_networks/network-dqn-$i"
    python evaluate.py \
    saved_networks/network-dqn-$i --max_episode=$max_episode \
    --save 
done
