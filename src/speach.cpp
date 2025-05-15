#include "speach.h"

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/sys/byteorder.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(speach, LOG_LEVEL_DBG);

#define UART_DEVICE_NODE_2 DT_NODELABEL(uart2)
const struct device *const Speach::_uart_dev2 =  DEVICE_DT_GET(UART_DEVICE_NODE_2);
static char MSG_SET_ENG[] = "[g2]";
static char MSG_SET_VOLUME[] = "[v1]";
static char MSG_SET_SPEED[] = "[s3]";
static char MSG_HELLO[] = "Hello. What are we cooking today!";


Speach& Speach::getInstance()
{
    static Speach instance;
    return instance;
}
    
void Speach::init()
{
    int rc;
    if (!device_is_ready(_uart_dev2)) {
		LOG_ERR("UART2 device not found!");
		return;
	}

    rc = uart_irq_callback_user_data_set(_uart_dev2, uart_tx_cb, this);
	if (rc < 0) {
		if (rc == -ENOTSUP) {
			LOG_ERR("Interrupt-driven UART API support not enabled");
		} else if (rc == -ENOSYS) {
			LOG_ERR("UART device does not support interrupt-driven API");
		} else {
			LOG_ERR("Error setting UART callback: %d", rc);
		}
		return;
	}

    _is_init = true;
    speak(MSG_SET_ENG);
    k_msleep(100);
    speak(MSG_SET_VOLUME);
    k_msleep(100);
    speak(MSG_SET_SPEED);
    k_msleep(100);
    speak(MSG_HELLO);
}

void Speach::speak(const char *message)
{
    if (! _is_init)
        return;

    sys_put_be16(strlen(message) + 2 , (uint8_t*)&_header.lenght);
    memcpy(_uart_data, &_header, sizeof(speach_header_t));
    memcpy(_uart_data + sizeof(speach_header_t), message, strlen(message));

    _uart_send_idx = 0;
    _uart_data_size = strlen(message) + sizeof(speach_header_t);

    LOG_HEXDUMP_DBG(_uart_data, _uart_data_size, "speach:");
    uart_irq_tx_enable(_uart_dev2); 
}


void Speach::uart_tx_cb(const struct device *dev, void *speach_ptr)
{
    static int rc;
    static Speach *this_ptr = (Speach*)speach_ptr;

    if (!uart_irq_update(dev))
	{
		LOG_ERR("uart_irq_update does not retur 1");
		return;
	}

    if (uart_irq_tx_ready(dev) && this_ptr->_uart_send_idx < this_ptr->_uart_data_size)
	{
		rc = uart_fifo_fill(dev, this_ptr->_uart_data + this_ptr->_uart_send_idx,
							 this_ptr->_uart_data_size - this_ptr->_uart_send_idx);
		if (rc > 0)
			this_ptr->_uart_send_idx += rc;
	}
	if (this_ptr->_uart_send_idx >= this_ptr->_uart_data_size)
		uart_irq_tx_disable(dev);
}