#pragma once
#include "defines.h"
#include "receiver_utils.c"

int get_control_start(int fd){
    char* packet;
    
    int ret=llread(fd, &packet);
    return 0;
}