#ifndef COOP
#define COOP

void coopmult_add_task(void (*entry)(void*), void* args);

void coopmult_run();

void coopmult_sleep();

#endif