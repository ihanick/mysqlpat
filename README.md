mysqlpat
========

Performance Analysis Tool for mysql.

The tool could be used to investigate:
- load spikes
- IO problems
- abnormal activity for mysql


How it works?
----
- Can create a graph for any SHOW GLOBAL STATUS parameter
- You can have gaps in input data
- Any value increasing with uptime will be represented by number per second (e.g. bytes_received)
- You can use zoom and drag graph to investigate any point of graph
- You can check values for all selected graphs under mouse cursor



If you are using pt-stalk:

```grep -m 1 Uptime *mysqladmin|awk '{print $4, $1}'|sort -n|awk '{print $2}'|perl -pe 's/:\|//g'|xargs cat | cat > ~/test-mysqladmin```


Creating 10 minutes graph:

```mysqladmin ext -i1 -c600 > ~/test-mysql```


Installation
----

You should have Qt5 & Qwt 6.1 installed.
