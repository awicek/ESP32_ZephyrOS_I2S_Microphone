#ifndef SOUND_QUEUE_H_
#define SOUND_QUEUE_H_


#include <stdint.h>

class SoundQueue
{
static constexpr int NOF_BUF = 32;
static constexpr int SIZE_OF_CONTAINER = 512;
public:

private:
    uint8_t buffers[NOF_BUF][SIZE_OF_CONTAINER];
    

};


#endif // SOUND_QUEUE_H_