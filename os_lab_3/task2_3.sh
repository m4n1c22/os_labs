#!\bin\sh

sh pr_cs.sh p 2 funnymsgs_p1 producer_1 &
sh pr_cs.sh p 2 funnymsgs_p2 producer_2 &
sh pr_cs.sh c 3 consumer_1 &
sh pr_cs.sh c 3 consumer_1 &
sh pr_cs.sh stats &

