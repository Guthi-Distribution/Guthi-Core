#pragma once
#include <sys/inotify.h>


struct Event {
    int inotify_fd;


    Event() {
        inotify_fd = inotify_init();    

        //TODO: handle error for init
    }


    void remove_event() {
        inotify_rm_watch(inotify_fd, 0);
    }
};