/*
 * main.c
 *
 *  Created on: Jul 3, 2025
 *      Author: Lenovo Final Project
 */
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
void LCD_sendCommand(unsigned char command) {
    PORTA &= ~(1<<1); // RS = 0 for command mode
    _delay_ms(1);
    PORTA |= (1<<2);  // Enable pulse start
    _delay_ms(1);
    PORTA = (PORTA & 0x87) | ((command & 0xF0) >> 1);
    _delay_ms(1);
    PORTA &= ~(1<<2);
    _delay_ms(1);
    PORTA |= (1<<2);
    _delay_ms(1);
    PORTA = (PORTA & 0x87) | ((command & 0x0F) << 3);
    _delay_ms(1);
    PORTA &= ~(1<<2);
    _delay_ms(1);
}

void LCD_displayCharacter(unsigned char data) {
    PORTA |= (1<<1);  // RS = 1 for data mode
    _delay_ms(1);
    PORTA |= (1<<2);
    _delay_ms(1);
    PORTA = (PORTA & 0x87) | ((data & 0xF0) >> 1);
    _delay_ms(1);
    PORTA &= ~(1<<2);
    _delay_ms(1);
    PORTA |= (1<<2);
    _delay_ms(1);
    PORTA = (PORTA & 0x87) | ((data & 0x0F) << 3);
    _delay_ms(1);
    PORTA &= ~(1<<2);
    _delay_ms(1);
}

void LCD_init() {
    DDRA = 0xFE;
    _delay_ms(20);
    LCD_sendCommand(0x33); // Initialization sequence
    LCD_sendCommand(0x32);
    LCD_sendCommand(0x28); // 4-bit mode
    LCD_sendCommand(0x0C); // Display ON, cursor OFF
    LCD_sendCommand(0x01); // Clear display
    LCD_sendCommand(0x06); // Entry mode
}

void LCD_displayString(char *str) {
    while(*str) LCD_displayCharacter(*str++);
}

void LCD_clearScreen() {
    LCD_sendCommand(0x01);
}

void LCD_moveCursor(unsigned char row, unsigned char col) {
    LCD_sendCommand(0x80 + (row == 0 ? col : col + 0x40));
}

// ........... KEYPAD FUNCTIONS .........
unsigned char KEYPAD_getPressedKey() {
    DDRB = 0xF0;
    PORTB = 0x0F;
    PORTD |= 0x3C;

    while(1) {
        for (int row = 4; row < 8; row++) {
            PORTB = ~(1 << row);
            _delay_ms(5);
            for (int col = 2; col <= 5; col++) {
                if (!(PIND & (1 << col))) { // Check for button press
                    while (!(PIND & (1 << col))); // Wait for release
                    return (row - 4) * 4 + (col - 2) + 1;
                }
            }
        }
    }
}

unsigned char KEYPAD_4x4_adjustKeyNumber(unsigned char button_number) {
    unsigned char keypad_button = 0;
    switch(button_number) {
        case 1: keypad_button = 7; break;
        case 2: keypad_button = 8; break;
        case 3: keypad_button = 9; break;
        case 4: keypad_button = '%'; break;
        case 5: keypad_button = 4; break;
        case 6: keypad_button = 5; break;
        case 7: keypad_button = 6; break;
        case 8: keypad_button = '*'; break;
        case 9: keypad_button = 1; break;
        case 10: keypad_button = 2; break;
        case 11: keypad_button = 3; break;
        case 12: keypad_button = '-'; break;
        case 13: keypad_button = 13; break;
        case 14: keypad_button = 0; break;
        case 15: keypad_button = '='; break;
        case 16: keypad_button = '+'; break;
        default: keypad_button = button_number; break;
    }
    return keypad_button;
}

// .......... SYSTEM FUNCTIONS ...........
void displayMainMenu() {
    LCD_clearScreen();
    LCD_displayString("2-L 3-R 4-Wait");
    LCD_moveCursor(1,0);
    LCD_displayString("5-AC");
}

void signalBlink(uint8_t led1, uint8_t led2) {
    for (int i = 0; i < 10; i++) {
        PORTC ^= (1 << led1);
        if (led2 != 255) PORTC ^= (1 << led2);
        _delay_ms(800);
    }
    PORTC &= ~(1 << led1);
    if (led2 != 255) PORTC &= ~(1 << led2);
}

void acMode() {
    uint8_t inAC = 1;
    while (inAC) {
        LCD_clearScreen();
        LCD_displayString("AC MODE");
        LCD_moveCursor(1,0);
        LCD_displayString("1:ON 2:OFF");

        uint8_t raw = KEYPAD_getPressedKey();
        uint8_t input = KEYPAD_4x4_adjustKeyNumber(raw);
        _delay_ms(300);

        if (input == 1) {
            PORTC |= (1 << PC1); // Turn AC ON (Green LED)
        } else if (input == 2) {
            PORTC &= ~(1 << PC1); // Turn AC OFF
        } else if (input == 6) {
            inAC = 0; // Exit AC mode
        } else {
            LCD_clearScreen();
            LCD_displayString("Invalid Input");
            _delay_ms(1000);
        }
    }
}

int main() {
    DDRA = 0xFE; // PA1–PA6 as output
    DDRC |= (1 << PC0) | (1 << PC1) | (1 << PC2); // Output LEDs
    DDRD &= ~(0x3C); PORTD |= 0x3C;
    DDRB |= 0xF0; // Rows output

    LCD_init();
    LCD_displayString("Press 1 to Start");

    uint8_t state = 0;

    while (1) {
        uint8_t raw = KEYPAD_getPressedKey();
        uint8_t key = KEYPAD_4x4_adjustKeyNumber(raw);
        _delay_ms(300);

        switch (state) {
            case 0:
                if (key == 1) {
                    state = 1;
                    displayMainMenu();
                }
                break;

            case 1:
                switch (key) {
                    case 2:
                        LCD_clearScreen();
                        LCD_displayString("---Left Signal---");
                        signalBlink(PC2, 255);
                        displayMainMenu();
                        break;
                    case 3:
                        LCD_clearScreen();
                        LCD_displayString("--Right Signal--");
                        signalBlink(PC0, 255);
                        displayMainMenu();
                        break;
                    case 4:
                        LCD_clearScreen();
                        LCD_displayString("--- Wait Signal---");
                        signalBlink(PC2, PC0);
                        displayMainMenu();
                        break;
                    case 5:
                        acMode();
                        displayMainMenu();
                        break;
                    case 6:
                        displayMainMenu();
                        break;

                }
                break;
        }
    }
}

