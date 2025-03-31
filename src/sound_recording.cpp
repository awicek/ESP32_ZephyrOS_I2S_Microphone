#include "sound_recording.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sound_recording, LOG_LEVEL_DBG);

#define i2S_RX_NODE DT_NODELABEL(i2s_rx)

// sample frequency is 32000, but its two channels so 
// sample frequency for 1 ch is 16000
#define SAMPLE_FREQUENCY    32000 
#define SAMPLE_BIT_WIDTH    32
#define BYTES_PER_SAMPLE    sizeof(uint32_t)
// only 2 channels are supported
#define NUMBER_OF_CHANNELS  2   
#define SAMPLES_PER_BLOCK   512
#define INITIAL_BLOCKS      4
#define TIMEOUT             50

// block size is 0.5kB * 4 = 2kB
#define BLOCK_SIZE  (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK)
#define BLOCK_COUNT (INITIAL_BLOCKS + 2)
// size of the mem_slab is 2kB * 6 = 12KB
K_MEM_SLAB_DEFINE_STATIC(mem_slab, BLOCK_SIZE, BLOCK_COUNT, 4);


#define STACK_SIZE_I2S 2048
#define PRIORITY_I2S 1


I2SWrapper::I2SWrapper()
{
    int rc;
    
    _i2s_device = DEVICE_DT_GET(i2S_RX_NODE);
    if (!device_is_ready(_i2s_device))
    {
        LOG_ERR("is2 dirver not ready");
        return;
    }
    
    struct i2s_config config;
    config.word_size = SAMPLE_BIT_WIDTH;
    config.channels = NUMBER_OF_CHANNELS;
    config.format = I2S_FMT_DATA_FORMAT_LEFT_JUSTIFIED;
    config.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
    config.frame_clk_freq = SAMPLE_FREQUENCY;
    config.mem_slab = &mem_slab;
    config.block_size = BLOCK_SIZE;
    config.timeout = TIMEOUT;
    rc = i2s_configure(_i2s_device, I2S_DIR_RX, &config);
    if (rc < 0)
    {
        LOG_ERR("i2s configuration failed rc: %d", rc);
        return;
    }

    _is_ready = true;
}


bool I2SWrapper::isReady()
{
    return _is_ready;
}

bool I2SWrapper::startRecording()
{
    if (!_is_ready)
        return false;

    int rc;
    rc = i2s_trigger(_i2s_device, I2S_DIR_RX, I2S_TRIGGER_START);
    if (rc < 0)
    {
        LOG_ERR("i2s_trigger START rc: %d", rc);
        return false;
    }

    return true;
}

bool I2SWrapper::stopRecording()
{
    if (!_is_ready)
        return false;

    int rc;
    rc = i2s_trigger(_i2s_device, I2S_DIR_RX, I2S_TRIGGER_DROP);
    if (rc < 0)
    {
        LOG_ERR("i2s_trigger DROP rc: %d", rc);
        return false;
    }

    return true;
}


void I2SWrapper::rxLoop(void *I2SWrapper_ptr, void*, void*)
{
    I2SWrapper *i2s = static_cast<I2SWrapper*>(I2SWrapper_ptr);
    void *mem_block;
    size_t mem_size;
    int rc;
    
    while(1)
    {
        rc = i2s_buf_read(i2s->_i2s_device, &mem_block, &mem_size);
        
        if (rc < 0)
        {
            
        }
        else 
        {
            LOG_ERR("error reading i2s rc: ", rc);
        }
    }
}