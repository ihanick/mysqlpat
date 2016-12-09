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

Simple usage
----
- collect pt-stalk report: https://www.percona.com/blog/2013/01/03/percona-toolkit-by-example-pt-stalk/
- Menu: File -> Open, choose "date"-mysqladmin file or vmstat file.
- Enable any graph group: File -> Choose Curves, select any group.
- If needed add more files: File -> Add File it could be same or different date.



Advanced usage
----
You can aggregate many mysqladmin ext samples with:

```grep -m 1 Uptime *mysqladmin|awk '{print $4, $1}'|sort -n|awk '{print $2}'|perl -pe 's/:\|//g'|xargs cat | cat > ~/test-mysqladmin```


Or create 10 minutes graph without pt-stalk:

```mysqladmin ext -i1 -c600 > ~/test-mysql```


Installation
----

This program uses Qt5 and Qwt.

In order to use built-in Qwt:
```bash
# install Qt 5 development tools:
apt-get install qt5-default
git clone https://github.com/ihanick/mysqlpat.git
cd mysqlpat
git submodule update --init --recursive
mkdir bld
cd bld
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILTIN_QWT=ON ../
make -j4
```

OS X build:
```bash
brew install cmake qt5
git clone https://github.com/ihanick/mysqlpat.git
cd mysqlpat
git submodule update --init --recursive
mkdir bld
cd bld
CMAKE_PREFIX_PATH=`ls -d /usr/local/Cellar/qt5/*|tail -n 1` cmake -DBUILTIN_QWT=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j4
```

Alternative way, use system qwt:
```bash
# install Qt 5 development tools:
apt-get install qt5-default libqwt-qt5-dev
mkdir bld
cd mysqlpat
git submodule update --init --recursive
mkdir bld
cd bld
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ../
```


You can also install full qt development suite from https://www.qt.io/ and compile cmake project from QtCreator: http://doc.qt.io/qtcreator/creator-project-cmake.html
