#ifndef SPEACH_H_
#define SPEACH_H_

#include <zephyr/device.h>

#define SPEACH_MAX_LENGHT 500

class Speach
{
typedef struct  __attribute__((packed)) {
    uint8_t  byte1;
    uint16_t lenght;
    uint8_t  bype4;
    uint8_t  byte5;
} speach_header_t;


public:
    static Speach& getInstance();

    void init();

    /**
     * @param message null terminated string
     */
    void speak(const char *message);

private:
    Speach() = default;
    Speach (const Speach&) = delete;
    Speach& operator=(const Speach&) = delete;

    static void uart_tx_cb(const struct device *dev, void *this_ptr);

    static const struct device *const _uart_dev2;

    uint8_t _uart_data[SPEACH_MAX_LENGHT];
    uint8_t _uart_send_idx = 0;
    uint8_t _uart_data_size = 0;

    bool _is_init = false;
    speach_header_t _header = {0xFD, 0, 0x01, 0x00};
};
#endif