/*
 * lcd_display.c
 *
 *  Created on: Feb 19, 2022
 *      Author: yaman
 */

#include "lcd_display.h"


#define BUS_TIMEOUT 1000
#define T_1 0b01000000
#define T_2 0b00100000
#define T_3 0b00010000
#define T_4 0b00001000
#define T_5 0b00000100
#define T_6 0b00000010
#define T_7 0b00000001

#define PCF_CONTINUE (1 << 7)
//     AAA
//    D   C
//    D   C
//     FFF
//    E   B
//    E   B
//     GGG  PP

#define SEG_A 0b10000000
#define SEG_B 0b01000000
#define SEG_C 0b00100000
#define SEG_D 0b00001000
#define SEG_E 0b00000100
#define SEG_F 0b00000010
#define SEG_G 0b00000001

#define SEG_P 0b00010000

// The following segments are mapped to location 10 and 11
// following this format (* means unused bit):
//   T1 Min Mem Err T2 T4 T3 * T7 T5 T6 * * * * *
#define SEG_Err 0b00010000
#define SEG_Mem 0b00100000
#define SEG_Min 0b01000000

#define TS_SIZE 7
const uint16_t TS_SEGMENTS[TS_SIZE] = { 0b1000000000000000, 0b0000100000000000,
		0b0000001000000000, 0b0000010000000000, 0b0000000001000000,
		0b0000000000100000, 0b0000000010000000 };

const uint8_t TS_INDEX[TS_SIZE] = { T_1, T_2, T_3, T_4, T_5, T_6, T_7 };

#define BUFFER_SIZE 12
#define DIGITS 10
#define FLAGS 10

const uint8_t DIGIT_SEGMENTS[] = {
SEG_A | SEG_C | SEG_B | SEG_G | SEG_E | SEG_D,
SEG_C | SEG_B,
SEG_A | SEG_C | SEG_G | SEG_E | SEG_F,
SEG_A | SEG_C | SEG_B | SEG_G | SEG_F,
SEG_C | SEG_B | SEG_D | SEG_F,
SEG_A | SEG_B | SEG_G | SEG_D | SEG_F,
SEG_A | SEG_B | SEG_G | SEG_E | SEG_D | SEG_F,
SEG_A | SEG_C | SEG_B,
SEG_A | SEG_C | SEG_B | SEG_G | SEG_E | SEG_D | SEG_F,
SEG_A | SEG_C | SEG_B | SEG_G | SEG_D | SEG_F };

uint8_t _buffer[BUFFER_SIZE];
I2C_HandleTypeDef* hi2c;

void lcd_init(I2C_HandleTypeDef* i2c){
	hi2c = i2c;
	lcd_clear();
}

HAL_StatusTypeDef lcd_send_cmd(uint8_t command, uint8_t val) {
	uint16_t DevAddress = 0x38 << 1;
	uint8_t buf[2];
	buf[0] = command;
	buf[1] = val;

	// send register address
	HAL_StatusTypeDef rslt = HAL_I2C_Master_Transmit(hi2c, DevAddress, buf, 2,
	BUS_TIMEOUT);
	return rslt;
}

HAL_StatusTypeDef lcd_send_buffer(uint8_t *data, uint8_t length) {
	uint16_t DevAddress = 0x38 << 1;
	uint8_t buf[length + 2];
	buf[0] = 0xE0;
	buf[1] = 0;

	for (int i = 0; i < BUFFER_SIZE; i++) {
		buf[i + 2] = data[i];
	}

	// send register address
	HAL_StatusTypeDef rslt = HAL_I2C_Master_Transmit(hi2c, DevAddress, buf,
			(length + 2), BUS_TIMEOUT);
	return rslt;
}

HAL_StatusTypeDef lcd_reset() {
	HAL_StatusTypeDef rslt = lcd_send_cmd(0xE0, 0x48);
	HAL_Delay(3);
	rslt = lcd_send_cmd(0xE0, 0x70);
	return rslt;
}

HAL_StatusTypeDef lcd_clear() {

	uint8_t buffer[BUFFER_SIZE] = { 0 };

	HAL_StatusTypeDef rslt = lcd_send_buffer(buffer, BUFFER_SIZE);

	return rslt;
}

HAL_StatusTypeDef lcd_fill_digits(uint8_t n) {
	uint8_t buf[DIGITS];

	for (int i = 0; i < DIGITS; i++) {
		buf[i] = DIGIT_SEGMENTS[n];
	}

	HAL_StatusTypeDef result = lcd_send_buffer(buf, DIGITS);

	return result;
}

HAL_StatusTypeDef lcd_set_point(uint8_t pos) {

	uint8_t buf[DIGITS] = { 0 };
	uint8_t rslt;

	if (pos < DIGITS) {
		for (uint8_t dp = 0; dp < DIGITS; dp++) {
			// Clear all other decimal points.
			buf[dp] = buf[dp] & ~SEG_P;
		}
		buf[pos] |= SEG_P;
	}

	rslt = lcd_send_buffer(buf, DIGITS);

	return rslt;
}

HAL_StatusTypeDef lcd_set_digit(uint8_t pos, int8_t value) {

	uint8_t buf[DIGITS] = { 0 };
	uint8_t rslt;

	if (pos < DIGITS) {

		buf[pos] =
				value >= 0 && value <= 9 ?
						(buf[pos] & SEG_P) | DIGIT_SEGMENTS[value] : 0;
	}

	rslt = lcd_send_buffer(buf, DIGITS);

	return rslt;
}

void lcd_set_thousands(uint8_t n) {
	uint8_t buf[BUFFER_SIZE] = { 0 };
	uint16_t onBits = 0;
	uint16_t offBits = 0;

	for (int i = 0; i < TS_SIZE; i++) {
		if (TS_INDEX[i] & n) {
			onBits |= TS_SEGMENTS[i];
		} else {
			offBits |= TS_SEGMENTS[i];
		}
	}

	offBits = ~offBits;
	uint8_t oldValue = buf[FLAGS];
	buf[FLAGS] = (oldValue | (onBits >> 8)) & (offBits >> 8);
	oldValue = _buffer[FLAGS + 1];
	buf[FLAGS + 1] = (oldValue | (onBits & 0xff)) & (offBits & 0xff);
	lcd_send_buffer(buf, BUFFER_SIZE);
}
