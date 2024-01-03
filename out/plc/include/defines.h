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

/*
Command: <USB_COMMAND_WRITE_LD>
Response: 0xff <USB_ESP_OK / USB_ESP_ERROR> 0xff
See sequence diagram for detailed work
*/
#define USB_COMMAND_WRITE_LD 0x11

/*
Command: <USB_COMMAND_READ_LD>
Response: 0xff <USB_ESP_OK / USB_ESP_ERROR> 0xff
See sequence diagram for detailed work
*/
#define USB_COMMAND_READ_LD 0x12

/*
Command: <USB_COMMAND_WRITE_LD_END>
Response: 0xff <USB_ESP_OK / USB_ESP_ERROR> 0xff
See sequence diagram for detailed work
*/
#define USB_COMMAND_WRITE_LD_END 0xff

/*
Command: <USB_COMMAND_GET_EXTENSION_MODULES_COUNT>
Response: <extensionModulesCount 1> 
*/
#define USB_COMMAND_GET_EXTENSION_MODULES_COUNT 0x1f

/*
Command: <USB_COMMAND_GET_DEVICE_INFO> <deviceIndeks>
Response: <deviceType 1> <firmwareVersion 1> <numberOfAnalogInputs 1> <deviceInitTime 4>
*/
#define USB_COMMAND_GET_DEVICE_INFO 0x20 

/*
NOT IMPLEMENTED
Command: <USB_COMMAND_GET_DEVICE_STATUS> <deviceIndeks>
Response: <?> <timeError> <errorCode> ...
*/
#define USB_COMMAND_GET_DEVICE_STATUS 0x21 

/*
Command: <USB_COMMAND_GET_DIGITAL_OUTPUTS> <deviceIndeks>
Response: <outputsLowByte 1> <outputsHighByte 1>
*/
#define USB_COMMAND_GET_DIGITAL_OUTPUTS 0x27

/*
Command: <USB_COMMAND_GET_DIGITAL_INPUTS> <deviceIndeks>
Response: <inputsLowByte 1> <inputsHighByte 1>
*/
#define USB_COMMAND_GET_DIGITAL_INPUTS 0x28

/*
NOT TESTED
Command: <USB_COMMAND_GET_DIGITAL_INPUTS> <deviceIndeks>
Response: analog data
*/
#define USB_COMMAND_GET_ANALOG_INPUTS 0x29 

/*
Command: <USB_COMMAND_GET_DIGITAL_INPUTS_ALL>
Response: <inputs[0] 2> <inputs[1] 2> ...
Send data for all (controller + extension modules) devices
*/
#define USB_COMMAND_GET_DIGITAL_INPUTS_ALL 0x2a

/*
Command: <USB_COMMAND_GET_DIGITAL_OUTPUTS_ALL>
Response: <outputs[0] 2> <outputs[1] 2> ...
Send data for all (controller + extension modules) devices
*/
#define USB_COMMAND_GET_DIGITAL_OUTPUTS_ALL 0x2b

/*
Command: <USB_COMMAND_ECHO> <data <=63>
Response: <data <=63>
*/
#define USB_COMMAND_ECHO 0x30 

/*
Command: <USB_COMMAND_GET_DEVICE_TEMPERATURE>
Response: <rawTemperature 1>
To calculate use formula: 0.4386 * rawTemperature - 27.88 [*C]
*/
#define USB_COMMAND_GET_DEVICE_TEMPERATURE 0x31

/*
Command: <USB_COMMAND_GET_DEVICE_TIME> 
Response: <deviceTime 8>
First low byte
*/
#define USB_COMMAND_GET_DEVICE_TIME 0x32

/*
Command: <USB_COMMAND_SET_NEUTRAL_MODE>
Response: -
Stop executing LD and sets outputs to low
*/
#define USB_COMMAND_SET_NEUTRAL_MODE 0x40 

/*
Command: <USB_COMMAND_UNSET_NEUTRAL_MODE>
Response: -
Resmue executing LD
*/
#define USB_COMMAND_UNSET_NEUTRAL_MODE 0x41 

/*
Command: <USB_COMMAND_GET_ERRORS>
Response: <errorCounter 4> <sendErrors 1> <errorTime 4> <errorCode 4> ...
Send last 5 errors
*/
#define USB_COMMAND_GET_ERRORS 0xf0

//ESP responses to PC
#define USB_ESP_READY_TO_RECEIVE_PACKET 0x15
#define USB_ESP_ERROR 0x17
#define USB_ESP_OK 0x16

#define USB_MODE_IDLE 1
#define USB_MODE_READ 2
#define USB_MODE_WRITE 3
#define USB_MODE_BEFORE_RECEIVE_LD_SAVE 10
#define USB_MODE_RECEIVE_LD_SAVE 11
#define USB_MODE_SEND_LD_SAVE 12
#define USB_MODE_BEFORE_SEND_LD_SAVE 14
#define USB_MODE_TEST 20


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

