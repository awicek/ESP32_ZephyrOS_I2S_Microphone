#ifndef SOUND_RECORDING_H_
#define SOUND_RECORDING_H_

#include <zephyr/kernel.h>

#include "sound_queue.h"


class I2SWrapper
{    
public:
    I2SWrapper(SoundQueue *queue);
    bool isReady();
    bool startRecording();
    bool stopRecording();
private:
    static void rxLoop(void *I2SWrapper_ptr, void*, void*);
    void processI2SMemBlock(void *mem_block, size_t size);

    const struct device *_i2s_device;
    struct k_thread _rx_thread = {0};
    struct k_sem _start_recording_sem = {0}; // signals _rx_thread to start recording
    SoundQueue *_data_queue = NULL;

    bool _is_ready = false;
    
};



#endif // SOUND_RECORDING_H_