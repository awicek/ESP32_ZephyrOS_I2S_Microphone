#include "user_buttons.h"

#include "sound_recording.h"
#include "network_com.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(user_buttons, LOG_LEVEL_DBG);


struct gpio_callback UserButtons::_buttons_cb_data = {};
struct gpio_dt_spec UserButtons::_l_button = {};
struct gpio_dt_spec UserButtons::_r_button = {};
bool UserButtons::_buttons_enabled = false;
bool UserButtons::_recording = false;
NetworkCom* UserButtons::_network_com = NULL;
I2SWrapper* UserButtons::_sound_rec = NULL;

UserButtons::UserButtons()
{
    _l_button = GPIO_DT_SPEC_GET(DT_NODELABEL(L_BUTTON_DT_LABEL), GPIO_PROPERTY);
    _r_button = GPIO_DT_SPEC_GET(DT_NODELABEL(R_BUTTON_DT_LABEL), GPIO_PROPERTY);

    if (!device_is_ready(_l_button.port) || !device_is_ready(_r_button.port))
    {
        LOG_ERR("error in buttos devicetree or gpio driver is not initialized");
        return;
    }
    
    int rc;
    rc = gpio_pin_configure_dt(&_l_button, GPIO_INPUT | GPIO_PULL_UP);
    if (rc < 0)
    {
        LOG_ERR("Error in configuraiont left button. rc: %d", rc);
        return;
    }

    rc = gpio_pin_configure_dt(&_r_button, GPIO_INPUT | GPIO_PULL_UP);
    if (rc < 0)
    {
        LOG_ERR("Error in configuraiont right button. rc: %d", rc);
        return;
    }

    rc = gpio_pin_interrupt_configure_dt(&_l_button, GPIO_INT_EDGE_RISING);
    if (rc < 0)
    {
        LOG_ERR("config interupt on left button failed rc: %d", rc);
        return;
    }

    rc = gpio_pin_interrupt_configure_dt(&_r_button, GPIO_INT_EDGE_RISING);
    if (rc < 0)
    {
        LOG_ERR("config interupt on right button failed rc: %d", rc);
        return;
    }

    gpio_init_callback(&_buttons_cb_data, buttonCb, BIT(_l_button.pin) | BIT(_r_button.pin));
    
    rc = gpio_add_callback_dt(&_l_button, &_buttons_cb_data);
    if (rc < 0)
    {
        LOG_ERR("gpio_add_callback_dt left button failed rc: %d", rc);
        return;
    }

    rc = gpio_add_callback_dt(&_r_button, &_buttons_cb_data);
    if (rc < 0)
    {
        LOG_ERR("gpio_add_callback_dt right button failed rc: %d", rc);
        return;
    }

    _is_initialized = true;
}


UserButtons& UserButtons::getInstance()
{
    static UserButtons instance;
    return instance;
}


bool UserButtons::isReady()
{
    return _is_initialized;
}


void UserButtons::buttonCb(const struct device *port,
                           struct gpio_callback *cb_data,
                           gpio_port_pins_t pins)
{    
    static uint64_t last_left_btn_pressed = 0;
    static uint64_t last_right_btn_pressed = 0;
    
    int64_t elapsed_l_ms;
    int64_t elapsed_r_ms;
    uint64_t curr_call_tick_count = k_uptime_ticks();
    
    if (pins != BIT(_l_button.pin) && pins != BIT(_r_button.pin))
        return;
    
    elapsed_l_ms = k_ticks_to_ms_floor64(curr_call_tick_count - last_left_btn_pressed);
    elapsed_r_ms = k_ticks_to_ms_floor64(curr_call_tick_count - last_right_btn_pressed); 
    
    if (pins == BIT(_l_button.pin))
    {
        last_left_btn_pressed = curr_call_tick_count;
        if (elapsed_l_ms < DEBOUNCE_PERIOD_MS)
            return;
        lPress();
    }
    else 
    {
        last_right_btn_pressed = curr_call_tick_count;
        if (elapsed_r_ms < DEBOUNCE_PERIOD_MS)
            return;
        rPress();
    }
}


void UserButtons::rPress()
{
    if (!_buttons_enabled)
        return;
    LOG_DBG("R button pressed");
    if (_recording)
    {
        if (_network_com == NULL || _sound_rec == NULL)
        {
            LOG_ERR("network_com or sound_rec is not initialized");
            return;
        }
        _sound_rec->stopRecording();

        if (0 != _network_com->stopRecording())
        {
            LOG_ERR("stop recording failed");
            return;
        }
        
        _recording = false;
        led_set_mode(LED_WAITING_FOR_RESPONSE);
    }
}


void UserButtons::lPress()
{
    if (!_buttons_enabled)
        return;
    LOG_DBG("L button pressed");
    if (!_recording)
    {
        if (_network_com == NULL || _sound_rec == NULL)
        {
            LOG_ERR("network_com or sound_rec is not initialized");
            return;
        }
        if (0 != _network_com->startRecording())
        {
            LOG_ERR("start recording failed");
            return;
        }
        _sound_rec->startRecording();
        _recording = true;
        led_set_mode(LED_RECORDING);
    }

}

void UserButtons::enableButtons(bool enable)
{
    _buttons_enabled = enable;
}

void UserButtons::setNetworkCom(NetworkCom *network_com)
{
    _network_com = network_com;
}

void UserButtons::setSoundRecording(I2SWrapper *sound_rec)
{
    _sound_rec = sound_rec;
}