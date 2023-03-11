rm ./GUTHI_PIPE
g++ runtime.cpp linux_pipe.cpp -std=c++14 -o linux_runtime && ./linux_runtime
