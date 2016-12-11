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
- Menu: File -> Open, choose "date"-mysqladmin file or "date"-vmstat or "date"-netstat_s file.

You can also enable groups:
- Enable any graph group: File -> Choose Curves, select any group.
Or add additional files:
- File -> Add File it could be same or different date.

Or provide file as a command line argument
```bash
./mysqlpat 2016_10_27_16_13_13-mysqladmin
```



Advanced usage
----
You can aggregate many mysqladmin ext samples with:

```bash
grep -m 1 Uptime *mysqladmin|awk '{print $4, $1}'|sort -n|awk '{print $2}'|perl -pe 's/:\|//g'|xargs cat | cat > ~/test-mysqladmin
```


Or create 10 minutes graph without pt-stalk:

```mysqladmin ext -i1 -c600 > ~/test-mysql```


Installation
----

This program uses Qt5 and Qwt.

In order to use built-in Qwt:
```bash
# install Qt 5 development tools:
apt-get install cmake g++ qt5-default libqt5svg5-dev libqt5opengl5-dev
git clone https://github.com/ihanick/mysqlpat.git
cd mysqlpat
git submodule update --init --recursive
mkdir bld
cd bld
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILTIN_QWT=ON ../
# ubuntu 14.04 or other hosts with cmake < 3.1 require 
cmake -DCMAKE_CXX_FLAGS="-std=c++11" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBUILTIN_QWT=ON ../
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

Windows build:
- install cmake from http://cmake.org/ (add it to path to simplify things during install)
- install git from https://git-scm.com/download/win
In git shell open some directory and fetch sources:
```bash
git clone https://github.com/ihanick/mysqlpat.git
cd mysqlpat
git submodule update --init --recursive

```
- Download and install Community OpenSource version of Qt for gpl aplications: https://www.qt.io/download-open-source/
(windows xp requires QT_OPENGL=software environment variable to be configured). You need only a single version of qt, 5.7 + mingw32 works fine (tested on Windows 10 and windows xp)
- Launch Qt creator and open CMakeLists.txt project file
- If needed specify path to cmake with Tools -> Options -> Build & Run -> CMake
- Press Run


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
