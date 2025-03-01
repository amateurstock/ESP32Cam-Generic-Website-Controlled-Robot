#include "main.hh"

extern int flash_gpio;
extern int splash_gpio;

void update_motors(motor_t *left,
                        motor_t *right,
                        int left_val,
                        int right_val) {
    left->magnitude = abs(left_val);
    right->magnitude = abs(right_val);

    digitalWrite(left->LOG_PIN_1,  (left_val  <  0));
    digitalWrite(left->LOG_PIN_2,  (left_val  >= 0));
    digitalWrite(right->LOG_PIN_1, (right_val >= 0));
    digitalWrite(right->LOG_PIN_2, (right_val <  0));

    analogWrite(left->PWM_PIN, left->magnitude);
    analogWrite(right->PWM_PIN, right->magnitude);
}
