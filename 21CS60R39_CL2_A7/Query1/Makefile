run_query:
	(cat ../event1.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event2.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event3.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event4.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event5.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event6.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event7.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event8.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event9.txt | python3 mapper.py | sort -n| python3 combiner.py \
	& cat ../event10.txt | python3 mapper.py | sort -n| python3 combiner.py) \
	|  sort -n| python3 reducer.py > result_event.txt
