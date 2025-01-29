#ifndef TIMER_H
#define TIMER_H

#define PIT_CMD 0x43
#define PIT_REPEAT 0x36
#define DATA_CH0 0x40

void init_timer();
void timer_handler();

#endif // !TIMER_H
