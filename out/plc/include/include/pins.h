#pragma once

// //UART -> BB
// #ifndef DEV_BOARD
#define UART_BB UART_NUM_0
#define TXD_PIN (GPIO_NUM_21)
#define RXD_PIN (GPIO_NUM_20)
// #else
// //old
// #define UART_BB UART_NUM_1
// #define TXD_PIN (GPIO_NUM_5)
// #define RXD_PIN (GPIO_NUM_4)
// #endif

#define FPGA_SPI_CS (GPIO_NUM_8)
#define FPGA_SPI_CLK (GPIO_NUM_2)
#define FPGA_SPI_MISO -1
#define FPGA_SPI_MOSI (GPIO_NUM_10)

#define MULTI_IO (GPIO_NUM_9)

// 
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

// #define INPUT6_PIN (GPIO_NUM_10)
#define OUTPUT3_PIN (GPIO_NUM_8)

#define OUTPUT1_PIN (GPIO_NUM_6)
#define OUTPUT2_PIN (GPIO_NUM_7)
#define OUTPUT4_PIN (GPIO_NUM_2)
// #define OUTPUT5_PIN (GPIO_NUM_)
// #define OUTPUT6_PIN

