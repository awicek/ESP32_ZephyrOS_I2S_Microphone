#include "sound_recording.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2s.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(sound_recording, LOG_LEVEL_DBG);

#define i2S_RX_NODE DT_NODELABEL(i2s_rx)

// sample frequency is 32000, but its two channels so 
#define SAMPLE_FREQUENCY    32000 
#define SAMPLE_BIT_WIDTH    32
#define SAMPLE_SIZE         8
// only 2 channels are supported
#define NUMBER_OF_CHANNELS  2   
#define INITIAL_BLOCKS      4
#define TIMEOUT             50

#define BLOCK_SIZE  SOUND_Q_SIZE_OF_CONTAINER * SAMPLE_SIZE
#define BLOCK_COUNT (INITIAL_BLOCKS + 2)
K_MEM_SLAB_DEFINE_STATIC(mem_slab, BLOCK_SIZE, BLOCK_COUNT, 4);


#define STACK_SIZE_I2S 2048
#define PRIORITY_I2S 1
K_THREAD_STACK_DEFINE(i2s_record_stack, STACK_SIZE_I2S);


I2SWrapper::I2SWrapper(SoundQueue* queue):
    _data_queue(queue)
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


    rc = k_sem_init(&_start_recording_sem, 0, 1);
    if (rc < 0)
    {
        LOG_ERR("k_sem_init failed rc: %d", rc);
        return;
    }

    k_tid_t thread_id = k_thread_create(&_rx_thread, i2s_record_stack, STACK_SIZE_I2S,
                                        I2SWrapper::rxLoop, this, NULL, NULL,
                                        PRIORITY_I2S, 0, K_NO_WAIT);
    if (thread_id == NULL)
    {
        LOG_ERR("can not create a thread ");
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
    k_sem_give(&_start_recording_sem);
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
        k_sem_take(&(i2s->_start_recording_sem), K_FOREVER);
        while (1)
        {
            rc = i2s_read(i2s->_i2s_device, &mem_block, &mem_size);
            if (rc == 0)
            {
                i2s->processI2SMemBlock(mem_block, mem_size);
            }
            else if (rc == -EAGAIN)
            {
                break;
            }
            else 
            {
                LOG_ERR("error reading i2s rc: %d", rc);
            }
        }
    }
}


void I2SWrapper::processI2SMemBlock(void *mem_block, size_t size)
{
    static const size_t container_size = SOUND_Q_SIZE_OF_CONTAINER;
    static uint16_t *data_container = NULL;

    uint16_t *raw_data = (uint16_t*)mem_block;
    if (size != (SAMPLE_SIZE * container_size))
    {
        LOG_ERR("mem_block has wrong size");
	    goto free;
    }
    
    if (!_data_queue->getFrontContainer(data_container))
    {
        LOG_ERR("queue is full");
        goto free;
    }

    for (size_t i = 0; i < container_size; ++i)
    {
        data_container[i] = raw_data[i*4+3];
    }

    _data_queue->push();

free:
	k_mem_slab_free(&mem_slab, mem_block);
}



// void I2SWrapper::processI2SMemBlock(void *mem_block, size_t size)
// {
//     static const size_t container_size = SOUND_Q_SIZE_OF_CONTAINER;
//     static const size_t loop_size = SOUND_Q_SIZE_OF_CONTAINER / AVRG;
//     static uint16_t *data_container = NULL;
//     static int8_t loop_idx = 0; // when loop_idx == AVRG the data are queued


//     uint16_t *raw_data = (uint16_t*)mem_block;
//     if (size != (SAMPLE_SIZE * container_size))
//     {
//         LOG_ERR("mem_block has wrong size");
// 	    goto free;
//     }
    
//     if (loop_idx == 0)
//     {
//         if (!_data_queue->getFrontContainer(data_container))
//         {
//             LOG_ERR("queue is full");
//             goto free;
//         }
//     }

//     for (size_t i = 0; i < container_size; i += AVRG)
//     {
//         data_container[i/AVRG + AVRG * loop_size] = raw_data[i*4+3];
//     }

//     loop_idx +=1;
//     if (loop_idx == AVRG)
//     {
//         _data_queue->push();
//         loop_idx = 0;
//     }

// free:
// 	k_mem_slab_free(&mem_slab, mem_block);
// }

