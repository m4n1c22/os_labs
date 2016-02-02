#!\bin\sh

insmod fifo.ko fifo_size=43
./fifo_perm.sh
insmod producer_1.ko rate=2 msg='funny_by_prod1' instance='prod_1'
insmod producer_2.ko rate=2 msg='funny_by_prod2' instance='prod_2'
insmod consumer_1.ko rate=6 instance='cons_1'
sh pr_cs.sh p 3 funnymsgs_p1 user_producer_1 &
sh pr_cs.sh p 3 funnymsgs_p2 user_producer_2 &
sh pr_cs.sh p 5 funnymsgs_p3 user_producer_3 &
sh pr_cs.sh c 2 user_consumer_1 &
sh pr_cs.sh c 4 user_consumer_2 &
sh stats.sh &
