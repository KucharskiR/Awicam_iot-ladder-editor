#pragma once

//fpga size: 104161 bytes + RAM => 256K
#define FIRMWARE_VERSION 1

//Select only one!
#if !(defined(W1VC64R_BOARD) || defined(W1VC128R_BOARD) || defined(W1VC1616R_BOARD))
// #define DEV_BOARD
// #define W1VC64R_BOARD
#define W1VC128R_BOARD
// #define W1VC1616R_BOARD
#endif


#define DEBUG
#define NO_DEBUG_UART
#define NO_DEBUG_SPI

//Disable this to revers CS polarity 
#define TEST_SPI

/*---------------------------------------------*/
/*--------------SPI CONFIGURATION--------------*/
/*---------------------------------------------*/
// SPI -> FPGA
#define FPGA_SPI_CS (GPIO_NUM_8)
#define FPGA_SPI_CLK (GPIO_NUM_2)
#define FPGA_SPI_MISO -1
// #define FPGA_SPI_MISO (GPIO_NUM_7)
#define FPGA_SPI_MOSI (GPIO_NUM_10)
#define FPGA_CRESET_B (GPIO_NUM_9)
#define FPGA_SPI_SPEED_HZ 1000000 //1MHz => 20MHz



/*---------------------------------------------*/
/*-------------UART CONFIGURATION--------------*/
/*---------------------------------------------*/
// UART -> BB
//Póki co uart1 zamiast uart0 bo na arduino cli nie dziala (coś jeszcze jakieś smieci wysyła)
#define UART_BB UART_NUM_1
#define TXD_PIN (GPIO_NUM_21)
#define RXD_PIN (GPIO_NUM_20)
#define MULTI_IO (GPIO_NUM_9)

#define SEND_RESPONSE_WAIT 200

// Masks
#define INIT_ADDRESS                0x3E       //0b00111110
#define ADDRESS_MASK                0x3E       //0b00111110
#define RESPONSE_FLAG               0x80       //0b10000000
#define REQUEST_RESPONSE_FLAG_MASK  0x80       //0b10000000
#define DATA_COMMAND_FLAG_MASK      0x40       //0b01000000
#define PARITY_BIT_MASK             0x01       //0b00000001
#define NO_PARITY_MASK              0xFE       //0b11111110
#define PARITY_RESPONSE_FLAG        0x81       //0b10000001

// Comannds
#define INTRODUCE 0x01
#define SETUP_ANALOG_INPUT 0x02
#define ANALOG_READ_1 0x0B
#define ANALOG_READ_2 0x0C
#define ANALOG_READ_3 0x0D
#define ANALOG_READ_4 0x0E


/*---------------------------------------------*/
/*---------------PIN DEFINITIONS---------------*/
/*---------------------------------------------*/

#define INPUT1_PIN (GPIO_NUM_0)
#define INPUT2_PIN (GPIO_NUM_1)
#define INPUT3_PIN (GPIO_NUM_3)

#ifndef TEST_UART_BB
#define INPUT4_PIN (GPIO_NUM_4)
#define INPUT5_PIN (GPIO_NUM_5)
#else
#define INPUT4_PIN (GPIO_NUM_0)
#define INPUT5_PIN (GPIO_NUM_0)
#endif

#ifdef W1VC64R_BOARD
#define INPUT6_PIN (GPIO_NUM_10)
#endif

#define OUTPUT1_PIN (GPIO_NUM_6)
#define OUTPUT2_PIN (GPIO_NUM_7)
// Trzeba zrobić by pin GPIO 2 i 8 działało jako output i w spi
#define OUTPUT3_PIN (GPIO_NUM_8)
#define OUTPUT4_PIN (GPIO_NUM_2)

/*---------------------------------------------*/
/*----------------ACCESS POINT-----------------*/
/*---------------------------------------------*/

#define AP_SSID "ESP-Controller"
#define AP_PASS "password"