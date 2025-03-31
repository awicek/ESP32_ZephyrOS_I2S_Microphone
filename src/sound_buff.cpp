#include "sound_buff.h"

bool SoundQueue::pop()
{
    if (_capacity == 0)
        return;
    _idx_rear = (_idx_rear + 1) % NOF_BUF;
    _capacity -= 1;

}

bool SoundQueue::push()
{
    _idx_front = (_idx_front + 1) % NOF_BUF;
    _capacity += 1;
}


bool SoundQueue::getFrontContainer(uint8_t* container)
{
    if (_capacity == NOF_BUF)
        return false;
    container = _buffers[_idx_front];
    return true;
}


bool SoundQueue::getRearContainer(uint8_t* containter)
{
    if (_capacity == 0)
        return false;
    containter = _buffers[_idx_front];
}
