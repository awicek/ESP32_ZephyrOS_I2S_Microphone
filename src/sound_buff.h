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
class SoundQueue
{

static constexpr size_t NOF_BUF = 64;
static constexpr size_t SIZE_OF_CONTAINER = 512;

public:
    bool pop();
    bool push();
    bool getFrontContainer(uint8_t* container);
    bool getRearContainer(uint8_t* containter);
    inline size_t getContainerSize() const;

private:
    uint8_t _buffers[NOF_BUF][SIZE_OF_CONTAINER];
    size_t _idx_front = 0;
    size_t _idx_rear = 0;
    size_t _capacity = 0;
};


inline size_t SoundQueue::getContainerSize() const
{
    return SIZE_OF_CONTAINER;
}

#endif // SOUND_QUEUE_H_