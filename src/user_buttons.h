#ifndef USER_BUTTONS_H_
#define USER_BUTTONS_H_

#include "sound_recording.h"
#include "network_com.h"      

#include <zephyr/drivers/gpio.h>

#define L_BUTTON_DT_LABEL l_button
#define R_BUTTON_DT_LABEL r_button 
#define GPIO_PROPERTY gpios
constexpr int64_t DEBOUNCE_PERIOD_MS = 30;
constexpr int64_t DOUBLE_CLICK_PERIOD_MS = 200;

class UserButtons
{
public:
    
    static UserButtons& getInstance();

    void setNetworkCom(NetworkCom *network_com);
    void setSoundRecording(I2SWrapper *soudn_rec);

    bool isReady();

    void enableButtons(bool enable);

private:
    UserButtons();
    UserButtons(const UserButtons&) = delete;
    UserButtons& operator=(const UserButtons&) = delete;
    
    static void buttonCb(const struct device *port,
                         struct gpio_callback *cb_data,
                         gpio_port_pins_t pins);
    
    static void rPress();
    static void lPress();


    static struct gpio_callback _buttons_cb_data;
    static struct gpio_dt_spec _l_button;
    static struct gpio_dt_spec _r_button;
    static bool _buttons_enabled;
    static bool _recording;
    static NetworkCom *_network_com;
    static I2SWrapper *_sound_rec;
    
    bool _is_initialized = false;
};

#endif // USER_BUTTONS_H_