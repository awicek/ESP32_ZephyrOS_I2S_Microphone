#ifndef SOUND_QUEUE_H_
#define SOUND_QUEUE_H_

#include <stddef.h>
#include <stdint.h>
#include <zephyr/kernel.h>

/**
 * Queue capacity is 64 * 0.5 kB = 32 kB
 * I2S sampling rate is 16 kSamples/s 
 * One sample has 2Bytes 
 * This queue can store up to 1s of recording
 */ 
#define SOUND_Q_NOF_BUF 64
#define SOUND_Q_SIZE_OF_CONTAINER 256

class SoundQueue
{
public:
    SoundQueue();
    void pop();
    void push();
    bool getFrontContainer(uint16_t*& container);
    bool getRearContainer(uint16_t*& containter);
    void waitForContainer();
    inline size_t getContainerSize() const;

private:
    uint16_t (*_buffers)[SOUND_Q_SIZE_OF_CONTAINER];
    size_t _idx_front = 0;
    size_t _idx_rear = 0;
    size_t _capacity = 0;
    struct k_mutex _mtx; // gurards _capacity
    
    struct k_sem _capacity_sem; 
};

#endif // SOUND_QUEUE_H_