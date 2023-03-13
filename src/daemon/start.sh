rm ./GUTHI_PIPE
g++ runtime.cpp linux_pipe.cpp ../filesystem/file_track_linux.cpp -std=c++20 -o linux_runtime && ./linux_runtime
