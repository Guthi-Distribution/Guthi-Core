g++ -c ./daemon_lib.cpp ./linux_pipe.cpp
ar -rvs libdaemon.a ./daemon_lib.o ./linux_pipe.o
rm ./daemon_lib.o ./linux_pipe.o
