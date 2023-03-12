g++ -c ./daemon_lib.cpp ./win32_daemon_impl.cpp 
ar -rvs libdaemon.a ./daemon_lib.o ./win32_daemon_impl.o 
rm ./daemon_lib.o ./win32_daemon_impl.o
