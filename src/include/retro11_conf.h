#ifndef RETRO11_CONF_H
#define RETRO11_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Common Configuration Motor
 * @{
 */

#define CONF_MOTOR_PWM          (PWM_DEV(0))
#define CONF_MOTOR_FREQ           (1000U)
#define CONF_MOTOR_RES            (0x0064)

/**
 * @brief  Configuration for Motor A
 * @{
 */
#define CONF_MOTOR_A_PWM_CHAN     (0U)
#define CONF_MOTOR_A_DIRA         GPIO_PIN(PA,13)
#define CONF_MOTOR_A_DIRB         GPIO_PIN(PA,18)
/** @} */

/**
 * @brief  Configuration for Motor B
 * @{
 */
#define CONF_MOTOR_B_PWM_CHAN     (1U)
#define CONF_MOTOR_B_DIRA         GPIO_PIN(PA,28)
#define CONF_MOTOR_B_DIRB         GPIO_PIN(PA,19)
/** @} */

/**
 * @brief  Configuration for Multiplexer
 * @{
 */
#define CONF_MULTIPLEXER_RECV     GPIO_PIN(PA,22)
#define CONF_MULTIPLEXER_ADR_A    GPIO_PIN(PA,17)
#define CONF_MULTIPLEXER_ADR_B    GPIO_PIN(PA,16)
#define CONF_MULTIPLEXER_ADR_C    GPIO_PIN(PA,23)
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* RETRO11_CONF_H */