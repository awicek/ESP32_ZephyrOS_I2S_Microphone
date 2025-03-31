#ifndef SOUND_RECORDING_H_
#define SOUND_RECORDING_H_

#include <zephyr/kernel.h>




class I2SWrapper
{    
public:
    I2SWrapper();
    bool isReady();
    bool startRecording();
    bool stopRecording();
private:
    static void rxLoop(void *I2SWrapper_ptr, void*, void*);
    void process

    const struct device *_i2s_device;
    struct k_thread _rx_thread = {0};
    bool _is_ready = false;
};



#endif // SOUND_RECORDING_H_