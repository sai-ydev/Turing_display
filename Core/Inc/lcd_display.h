/*
 * lcd_display.h
 *
 *  Created on: Feb 19, 2022
 *      Author: yaman
 */

#ifndef INC_LCD_DISPLAY_H_
#define INC_LCD_DISPLAY_H_

#include "stm32l4xx_hal.h"

#define T_1 0b01000000
#define T_2 0b00100000
#define T_3 0b00010000
#define T_4 0b00001000
#define T_5 0b00000100
#define T_6 0b00000010
#define T_7 0b00000001


void lcd_init(I2C_HandleTypeDef* hi2c1);
HAL_StatusTypeDef lcd_send_cmd(uint8_t command, uint8_t val);
HAL_StatusTypeDef lcd_send_buffer(uint8_t *data, uint8_t length);
HAL_StatusTypeDef lcd_reset();
HAL_StatusTypeDef lcd_clear();
HAL_StatusTypeDef lcd_fill_digits(uint8_t n);
HAL_StatusTypeDef lcd_set_point(uint8_t pos);
HAL_StatusTypeDef lcd_set_digit(uint8_t pos, int8_t value);
void lcd_set_thousands(uint8_t n);

#endif /* INC_LCD_DISPLAY_H_ */
