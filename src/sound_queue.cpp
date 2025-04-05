#include "sound_queue.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sound_queue, LOG_LEVEL_ERR);

uint16_t BUFFERS[SOUND_Q_NOF_BUF][SOUND_Q_SIZE_OF_CONTAINER] = {0};

SoundQueue::SoundQueue()
{
    _buffers = BUFFERS;
    
    int rc = k_mutex_init(&_mtx);
    if (rc < 0 )
    {
        LOG_ERR("k_mutex_init failed rc: %d", rc);
    }

    rc = k_sem_init(&_capacity_sem, 0, SOUND_Q_NOF_BUF);
    if (rc < 0)
    {
        LOG_ERR("k_sem_init failed rc: %d", rc);
    }
}

void SoundQueue::pop()
{
    k_mutex_lock(&_mtx, K_FOREVER);

    if (_capacity == 0)
    {
        k_mutex_unlock(&_mtx);
        return;
    }
    _idx_rear = (_idx_rear + 1) % SOUND_Q_NOF_BUF;
    _capacity -= 1;
    k_mutex_unlock(&_mtx);
    return;

}

void SoundQueue::push()
{
    k_mutex_lock(&_mtx, K_FOREVER);
    if (_capacity == SOUND_Q_NOF_BUF)
    {
        k_mutex_unlock(&_mtx);
        return;
    }
    _idx_front = (_idx_front + 1) % SOUND_Q_NOF_BUF;
    _capacity += 1;
    k_sem_give(&_capacity_sem);
    k_mutex_unlock(&_mtx);
    return;
}


bool SoundQueue::getFrontContainer(uint16_t*& container)
{
    k_mutex_lock(&_mtx, K_FOREVER);
    if (_capacity == SOUND_Q_NOF_BUF)
    {
        k_mutex_unlock(&_mtx);
        return false;
    }
    container = _buffers[_idx_front];
    k_mutex_unlock(&_mtx);
    return true;
}


bool SoundQueue::getRearContainer(uint16_t*& containter)
{
    k_mutex_lock(&_mtx, K_FOREVER);
    if (_capacity == 0)
    {
        k_mutex_unlock(&_mtx);
        return false;
    }
    containter = _buffers[_idx_rear];
    k_mutex_unlock(&_mtx);
    return true;
}


void SoundQueue::waitForContainer()
{
    k_sem_take(&_capacity_sem, K_FOREVER);
}