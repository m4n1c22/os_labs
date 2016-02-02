#!\bin\sh

sh pr_cs.sh p 2 funnymsgs_p1 user_producer_1 &
sh pr_cs.sh p 2 funnymsgs_p2 user_producer_2 &
sh pr_cs.sh c 3 consumer_1 &
sh pr_cs.sh c 3 consumer_2 &
sh pr_cs.sh stats 1 &

