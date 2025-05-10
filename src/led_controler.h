#ifndef LED_CONTROLLER_H_
#define LED_CONTROLLER_H_


typedef enum {
    LED_WAITING_FOR_CONNECTION = 0, // red toggles
    LED_WAITING_FOR_SERVER,         // both toggles
    LED_WAITING_FOR_RESPONSE,       // both toggles in oposite phase
    LED_RECORDING,                  // green toggles
    LED_READY,                      // green on
} led_mode_t;


void led_set_mode(led_mode_t mode);

#endif // LED_CONTROLLER_HPP
