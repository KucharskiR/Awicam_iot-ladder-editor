#pragma once

#define USE_LOGGER_USB
//fpga size: 104161 bytes + RAM => 256K
#define FIRMWARE_VERSION 1

//Select only one!
#if !(defined(W1VC64R_BOARD) || defined(W1VC128R_BOARD) || defined(W1VC1616R_BOARD))
// #define DEV_BOARD
// #define W1VC64R_BOARD
#define W1VC128R_BOARD
// #define W1VC1616R_BOARD
#endif

// Always send data to all 31 extension modules, only testing purpose
// #define FORCE_SEND_TO_MAX_EXTENSION_MODULES

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


#define FPGA_FLAG_I2C_MIDDLE_BYTE 0x00
#define FPGA_FLAG_I2C_FIRST_BYTE 0x01
#define FPGA_FLAG_I2C_LAST_BYTE 0x02
#define FPGA_FLAG_I2C_RECEIVE_BYTES 0x10

#define FPGA_FLAG_RECEIVE_BIT 0x10
#define FPGA_FLAG_RECEIVE_LAST_BIT 0x20
#define FPGA_FLAG_ACK_BIT 0x40
#define FPGA_FLAG_RECEIVE_FROM_I2C_DEVICE 0x01



#define FPGA_MAX_I2C_QUEUE_SIZE 8
#define FPGA_MAX_QUEUE_SIZE 64

/*---------------------------------------------*/
/*-----------MultiIO CONFIGURATION-------------*/
/*---------------------------------------------*/
#define MULTI_IO_PIN (GPIO_NUM_9)
#define MULTI_IO UART_NUM_1

//#define MULTI_IO_TIME_TO_WAIT 5

//Temporary address
#define MULTI_IO_CONTROLLER_ADDRESS 0x01
#define MULTI_IO_LOWER_BOARD_ADDRESS 0xaa

#define MULTI_IO_MY_ADDRESS MULTI_IO_CONTROLLER_ADDRESS


//Temporary commands
#define MULTI_IO_COMMAND_INTRODUCE 0x01
#define MULTI_IO_COMMAND_SET_DIGIT 0x02

//Temp lower board identification signatures
#define LOWER_BOARD_TEST 0xcc
#define LOWER_BOARD_UNDEFINED 0x00
#define LOWER_BOARD_64R 0x01
#define LOWER_BOARD_128R 0x02
#define LOWER_BOARD_1616R 0x03

/*---------------------------------------------*/
/*-------------UART CONFIGURATION--------------*/
/*---------------------------------------------*/
// UART -> Extensions(BB)

// Use loggerUSB to prevent sending log data via UART0
#define UART_BB UART_NUM_0
#define TXD_PIN (GPIO_NUM_21)
#define RXD_PIN (GPIO_NUM_20)


#define SEND_RESPONSE_WAIT 200
#define INIT_ADDRESS                0x1F       //0b00011111

// Masks
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
/*--------------USB CONFIGURATION--------------*/
/*---------------------------------------------*/
//Commands from PC

// /* Idea

// command:
// <~0x80 & cmd> <len> <some> <crc>

// response:
// <~0x80 & cmd> <len> <some> <crc>

// error:
// <0x80 | cmd> <len> <err_code> <crc>
// */

/*---------------------------------------------*/
/*---------------PIN DEFINITIONS---------------*/
/*---------------------------------------------*/

#define INPUT1_PIN (GPIO_NUM_0)
#define INPUT2_PIN (GPIO_NUM_1)
#define INPUT3_PIN (GPIO_NUM_3)

// #ifndef TEST_UART_BB
// #define INPUT4_PIN (GPIO_NUM_0)
// #define INPUT5_PIN (GPIO_NUM_0)
// #else
#define INPUT4_PIN (GPIO_NUM_4)
#define INPUT5_PIN (GPIO_NUM_5)
// #endif

#ifdef W1VC64R_BOARD
#define INPUT6_PIN (GPIO_NUM_10)
#endif

#define OUTPUT1_PIN (GPIO_NUM_6)
#define OUTPUT2_PIN (GPIO_NUM_7)

// Also using in SPI
#define OUTPUT3_PIN (GPIO_NUM_8)
// Also using in SPI
#define OUTPUT4_PIN (GPIO_NUM_2)

/*---------------------------------------------*/
/*----------------ACCESS POINT-----------------*/
/*---------------------------------------------*/

#define AP_SSID "ESP-Controller"
#define AP_PASS "password"



/*---------------------------------------------*/
/*------------------UTILITIES------------------*/
/*---------------------------------------------*/
#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')


struct DigitalInputsStructure {
  uint32_t  deviceInitTime;
  uint8_t   deviceType;
  uint8_t   firmwareVersion;
  uint8_t   numberOfAnalogInputs;
  uint32_t  lastDigitalInputUpdateTime;
  uint16_t  digitalInputStates;
  uint32_t  lastDigitalOutputUpdateTime;
  uint16_t  digitalOutputStates;
  uint32_t  aIUpdateTime[4];
  uint16_t  aIValue[4];

  uint8_t lowerBoardId;
};

