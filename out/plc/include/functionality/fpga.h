/*---------------------------------------------*/
/*--------------FPGA communication-------------*/
/*---------------------------------------------*/

/*
Etap 1:
Prosty expander IO, pracujący tylko z 14 liniami wyjściowymi. Protokół:
- Podczas nieaktywnego CS wszystkie pozostałe sygnały nie są w żaden sposób interpretowane, SOSI jest wejściem. Rosnący sygnał CS kasuje wewnętrzne liczniki odebranych bitów i przygotowuje układ do pracy. To obowiązuje także dla wszystkich innych komend.
- Przez pierwsze kilka etapów implementacji umówmy się, że transmisja ma zawsze 2 bajty, z których pierwszy zawsze jest w kierunku do naszego układu FPGA, drugi bajt jest do lub od FPGA, w zależności od treści 1. bajtu.
- Struktura danych: >0>0>AD>AC>AB>AA>A9>A8 >A7>A6>A5>A4>A3>A2>A1>A0 (znak > wskazuje kierunek transmisji do FPGA).
- Po opadającym zboczu CS bity AD..A0 pojawiają się na 14-bitowym porcie wyjściowym FPGA. Jeśli impulsów SCK łącznie będzie <>16 to te dane się nie pojawiają na porcie A, czyli tak jakby w ogóle nie było transmisji.
- Dekoder poleceń powinien zinterpretować pierwsze dwa zera kolejno jako:
-- >0 - kierunek transmisji drugiego bajtu w stronę do FPGA
-- >0 - urządzenie o adresie zero, co w praktyce oznacza zapis do 14b portu A.

Etap 2:
Dodanie odczytu 8-bitowego portu B.
- Struktura danych: >1>0>0>0>AB>AA>A9>A8 <B7<B6<B5<B4<B3<B2<B1<B0.
- Dekoder poleceń powinien zinterpretować kolejno:
-- >1 - kierunek transmisji drugiego bajtu - od FPGA do procka,
-- >0 - urządzenie o adresie zero,
-- >0>0 - adres rejestru wewnętrznego expandera, tutaj odczyt z portu B
-- >AB>AA>A9>A8 - to są dane do zapisania do portu A, czyli odczytując port B "przy okazji" zapisujemy 4 bity portu A
-- po 8. impulsie SCK, na podstawie powyższych pierwszych czterech bitów >1>0>0>0 wewnętrzny sygnał SOSI_DIR zmienia się na aktywny, dzięki czemu na SOSI będą się pojawiać kolejne dane z portu B.
-- <B7<B6<B5<B4<B3<B2<B1<B0 - kolejne odczytane bity z portu B.

Etap 3:
Dodanie odczytu 7-bitowego portu C i obsługa enkapsulowanej magistrali I2C:
- Struktura danych: >1>0>0>1>SD>SC>AD>AC <SA<C6<C5<C4<C3<C2<C1<C0.
- Dekoder poleceń powinien zinterpretować kolejno:
-- >1 - kierunek transmisji drugiego bajtu - od FPGA do procka,
-- >0 - urządzenie o adresie zero,
-- >0>1 - adres rejestru wewnętrznego expandera, tutaj odczyt z portu C i obsługa I2C
-- >SD>SC - dwa bity wyjściowe na zewnątrz tego etapu, do sterowania I2C, opis poniżej,
-- >AD>AC - to są dane do zapisania do portu A "przy okazji"
-- po 8. impulsie SCK zmiana kierunku SOSI na wyjście (po zdekodowaniu >1>0>0>1 )
-- <SA - bit wejściowy z I2C, to w praktyce może być zakodowane jakby 8. bit z portu C (C7),
-- <C6<C5<C4<C3<C2<C1<C0 - kolejne odczytane bity z portu C.
-- na zewnątrz tego etapu, bit SC staje się na nóżce FPGA sygnałem SCL, natomiast bity SD i SA tworzą dwukierunkowy bufor linii SDA typu otwarty kolektor - SD=0 to nadawanie na SDA silnego zera, SD=1 to stan wysokiej impedancji SDA; SA to odczyt aktualnego stanu linii SDA.


Kolejnych etapów prac na razie nie opisuję, bo będą potrzebne dużo później. Ale przesyłam notatki dotyczące protokołu 😄

Urządzenia:
ADR=0: RW 0 N N D D D D - expander na płycie głównej, D=dane, N=4 rejestry wewnętrzne do odczytu 00..11 (do zapisu jeden rejestr)
ADR=1: RW 1 0 N D D D D - sterownik klawiatury, LCD, D=dane, N=2 rejestry wewnętrzne 0..1
ADR=2: RW 1 1 N L L L L - Eth+SD card, L=długość pakietu, N=2 rejestry wewnętrzne 0..1

Expander płyty głównej - ADR=0 (0):
0  0  AD AC AB AA A9 A8        A7 A6 A5 A4 A3 A2 A1 A0    Zapis do portu wyjściowego AD..A0
1  0  0  0  AB AA A9 A8     B7 B6 B5 B4 B3 B2 B1 B0    Odczyt z portu wejściowego B7..B0
1  0  0  1  SD SC AD AC         SA C6 C5 C4 C3 C2 C1 C0    Odczyt z portu wejściowego C6..C0, obsługa I2C
1  0  1  0  x  x  x  x    Odczyt z portu wejściowego...
1  0  1  1  x  x  x  x    Odczyt z portu wejściowego...

Sterownik klawiatury, LCD - ADR=1 (10):
0  1  0  0  AB AA A9 A8        A7 A6 A5 A4 A3 A2 A1 A0    Zapis do portu wyjściowego AB..A0 (port pseudo-dwukierunkowy 8b)
0  1  0  1  FU x  x  x         B7 B6 B5 B4 B3 B2 B1 B0    Zapis do portu wyjściowego B7..B0 lub Firmware Update
1  1  0  0  AB AA A9 A8        A7 A6 A5 A4 A3 A2 A1 A0    Odczyt z portu wejściowego A7..A0 (port pseudo-dwukierunkowy 8b)
1  1  0  1  SD SC AD AC        SA C6 C5 C4 C3 C2 C1 C0    Odczyt z portu wejściowego C6..C0, obsługa I2C

Sterownik Ethernet MAC - ADR=2 (11):
0  1  1  N x x x x    Zapis
1  1  1  N x x x x    Odczyt
*/

#pragma once
#include "../includes.h"
#include "../defines.h"
#include "filesystem.h"
#include "esp32/rom/crc.h"

extern ControllerStatus controllerStatus;

spi_device_handle_t fpga;

spi_bus_config_t spi_conifg = {//Default MSb
    .mosi_io_num = FPGA_SPI_MOSI,
    .miso_io_num = FPGA_SPI_MISO,
    .sclk_io_num = FPGA_SPI_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .flags = SPICOMMON_BUSFLAG_GPIO_PINS | SPICOMMON_BUSFLAG_MASTER
  };

spi_device_interface_config_t fpga_conifg = {
    .command_bits = 8, // is first 8bits send to fpga
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,                  //SPI mode 0
    .clock_speed_hz = FPGA_SPI_SPEED_HZ, 
    .spics_io_num = FPGA_SPI_CS,         // CS Pin
  // #ifdef TEST_SPI
    .flags = SPI_DEVICE_HALFDUPLEX  | SPI_DEVICE_3WIRE | SPI_DEVICE_NO_DUMMY,
  // #else
  //   .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE | SPI_DEVICE_POSITIVE_CS,
  // #endif
    .queue_size = 1,
    .pre_cb = NULL,
    .post_cb = NULL,
  };

inline void programFPGA()
{
  gpio_set_level(FPGA_SPI_CS, 0);

  #if defined(DEBUG) && !defined(NO_DEBUG_SPI)
  const char* TASK_TAG = "PROGAM_FPGA";
  #endif

  //ESPIDF SPIFFS
  uint8_t* buffer = new uint8_t[104161];
  FILE* fileFPGAespidf;
  fileFPGAespidf = openFPGAFile();
  if(fileFPGAespidf == NULL) {
    ESP_LOGE("PROGRAM_FPGA", "FPGA config file NOT exists!");
    return;
  }
  size_t fileSize = readFPGAFile(fileFPGAespidf, buffer, 104161);
  // if(fileSize != 104161) {
  //   ESP_LOGW("PROGRAM_FPGA", "Read wrong amount of bytes! Read %d", fileSize);
  //   // return;
  // }
  closeFPGAFile(fileFPGAespidf);


  #if defined(DEBUG) && !defined(NO_DEBUG_SPI)
  ESP_LOGI(TASK_TAG, "File loaded to memory, fragment:");
  ESP_LOGI(TASK_TAG, "%#02x %#02x %#02x %#02x %#02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
  #endif

  // gpio_reset_pin(FPGA_SPI_CLK);
  // Programming needs other device interface
  spi_device_interface_config_t device_conifg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,                  //SPI mode 0
    .clock_speed_hz = FPGA_SPI_SPEED_HZ,   
    .spics_io_num = -1,       // CS Pin drive manually
    .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE,
    .queue_size = 1,
    .pre_cb = NULL,
    .post_cb = NULL,
  };
  
  spi_bus_add_device(SPI2_HOST, &device_conifg, &fpga);
  
  #if defined(DEBUG) && !defined(NO_DEBUG_SPI)
  ESP_LOGI(TASK_TAG, "Added device");
  #endif

  gpio_set_direction(FPGA_CRESET_B, GPIO_MODE_OUTPUT);
  gpio_set_direction(FPGA_SPI_CS, GPIO_MODE_OUTPUT);

  gpio_set_level(FPGA_CRESET_B, 0);
  vTaskDelay(20 / portTICK_PERIOD_MS); // delayed signal on pcb
  // #ifdef TEST_SPI
  // gpio_set_level(FPGA_SPI_CS, 0);
  // #else
  gpio_set_level(FPGA_SPI_CS, 1);
  // #endif
  gpio_set_level(FPGA_SPI_CLK, 1);
  //wait minimum 200ns
  // vTaskDelay(1 / portTICK_PERIOD_MS);
  vTaskDelay(0.3 / portTICK_PERIOD_MS);
  gpio_set_level(FPGA_CRESET_B, 1);
  //wait minimum 1200ns -> clear internal configuration
  vTaskDelay(1.3 / portTICK_PERIOD_MS);
  
  //send 8 dummy clocks
  // #ifdef TEST_SPI
  // gpio_set_level(FPGA_SPI_CS, 1);
  // #else
  gpio_set_level(FPGA_SPI_CS, 0);
  // #endif
  
  spi_transaction_t transaction ={
    .length = 8,
    .tx_buffer = buffer,
  };
  spi_device_transmit(fpga, &transaction);
  
  #if defined(DEBUG) && !defined(NO_DEBUG_SPI)
  ESP_LOGI(TASK_TAG, "Sent dummy block");
  #endif

  //send program
  size_t sent = 0;
  
  // #ifdef TEST_SPI
  // gpio_set_level(FPGA_SPI_CS, 0);
  // #else
  gpio_set_level(FPGA_SPI_CS, 1);
  // #endif
  
  while (sent < fileSize) {
    size_t length = (sent + 64 < fileSize) ? 64 : fileSize - sent;
    spi_transaction_t transactionProgram = {
      .length = length * 8, // in bits
      .tx_buffer = buffer + sent,
    };

    // spi_device_transmit(fpga, &transactionProgram);
    spi_device_polling_transmit(fpga, &transactionProgram);
    sent += 64;
    
    #if defined(DEBUG) && !defined(NO_DEBUG_SPI)
    ESP_LOGI("PROGRAM FPGA", "Send: %d / %d bytes ", sent, fileSize);
    #endif
  }
  // #ifdef TEST_SPI
  // gpio_set_level(FPGA_SPI_CS, 1);
  // #else
  gpio_set_level(FPGA_SPI_CS, 0);
  // #endif

  #if defined(DEBUG) && !defined(NO_DEBUG_SPI)
  ESP_LOGI("PROGRAM FPGA", "wait");
  #endif

  //wait
  spi_transaction_t transactionProgramwait ={
    .length = 100, // in bits
    .tx_buffer = buffer,
  };
  spi_device_transmit(fpga, &transactionProgramwait);
  
  spi_transaction_t transactionProgramEPSI ={
    .length = 50, // in bits
    .tx_buffer = buffer,
  };
  spi_device_transmit(fpga, &transactionProgramEPSI);

  // CDONE -> 1
  delete[] buffer;

  spi_bus_remove_device(fpga);
  spi_bus_free(SPI2_HOST);
  
  //After programming reinit spi device
  spi_bus_initialize(SPI2_HOST, &spi_conifg, SPI_DMA_DISABLED);
  spi_bus_add_device(SPI2_HOST, &fpga_conifg, &fpga);

  #ifdef DEBUG
  ESP_LOGI("PROGRAM_FPGA", "Program FPGA done!");
  #endif
}

// Read inputs states from fpga
// @param data drive 4 outputs on fpga 4 MSbs are reserved
// @return received byte (inputs from fpga)
inline uint8_t readInputsFPGA(uint8_t data)
{
  uint8_t toSend = 0x80 | (0x0f & data); 
  uint8_t received;

  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 1*8,
    .rxlength = 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &received,
  };

  //Set to spi
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);

  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  //Set to drive gpio 
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);

  // ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  received: " BYTE_TO_BINARY_PATTERN "",
  // BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(received));

  return received;
}

// Set outputs on FPGA
// @param data outputs to be set on fpga, 2 MSbs are reserved
inline void writeOutputsFPGA(uint16_t data)
{
  uint8_t toSend0 = 0x3f & (data >> 8);
  uint8_t toSend1 = 0x00ff & data;

  spi_transaction_t transaction = {
    .cmd = toSend0,
    .length = 1*8,
    .tx_buffer = &toSend1,
    .rx_buffer = nullptr,
  };

  //Set to spi
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);

  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  //Set to drive gpio 
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);
  // ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "",
  // BYTE_TO_BINARY(toSend[0]), BYTE_TO_BINARY(toSend[1]));
}

uint8_t reflectByte(uint8_t data)
{
    uint32_t tmp = 0;

    for (uint8_t i = 0; i < 8; i++)
    {
      tmp = tmp << 1;
      tmp |= data & 0x01;
      data = data >> 1;
    }

    return tmp;
}

// Soft CRC-8
// @param data data to calculate CRC-8
uint8_t checkCRC(uint32_t data)
{
    const uint8_t packetSize = 32; // 24 data + 8 crc
    const uint32_t divisior = 0x07000000;

    for (uint8_t i = 0; i < packetSize - 8; i++)
    {
        if ((data & 0x80000000) != 0)
        {
            data = uint32_t((data << 1) ^ divisior);
            //data ^= divisior;
        }
        else
          data = data << 1;
    }

  // ESP_LOGI("CRC", "Remainder: %d", data);

  return data>>=24;
}

// Check CRC-8, using ESPIDF API
// @param crc
inline uint8_t checkCRC(uint8_t crc, const uint8_t * buf, uint32_t len)
{
  uint8_t crcTemp = 0;
  crcTemp = ~crc8_le(~0, buf, 3);
  return crcTemp ^ crc;
}

// Check CRC-8, using ESPIDF API
// @param buf buffer with CRC
// @param len length of buffer with CRC
// @return if 0 then CRC is good
inline uint8_t checkCRC(const uint8_t * buf, uint32_t len)
{
  uint8_t crcTemp = 0;
  crcTemp = ~crc8_le(~0, buf, len);
  return crcTemp;
}

// Generate CRC-8, using ESPIDF API
// @param buf buffer to start calculate crc
// @param len buffer length in byte
// @return CRC-8 value
uint8_t genCRC(const uint8_t * buf, uint32_t len)
{
  return ~crc8_be(~0, buf, len);
}

// Write 1 bytes and read 3 bytes from FPGA. This function generates table with received data.
// @param data Data to be send
// @param received Pointer to table, function save in it received data
// @return CRC reminder, if equals 0 packet is good
inline uint8_t readExtFPGA(uint8_t data, uint8_t* received)
{
  uint8_t toSend = 0x80 | (0x0f & data); 

  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 4*8,
    .rxlength = 3*8,
    .rx_buffer = received,
  };
  
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);
  
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
  
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);

  uint8_t crcdata[4];
  crcdata[0] = toSend;
  crcdata[1] = received[0];
  crcdata[2] = received[1];
  crcdata[3] = received[2];


  // ESP_LOGI("FPGA", "Send:     %d, \t 0x%x, "  BYTE_TO_BINARY_PATTERN "", toSend, toSend, BYTE_TO_BINARY(toSend));
  // ESP_LOGI("FPGA", "Received: %d, \t 0x%x, " BYTE_TO_BINARY_PATTERN "", received[0], received[0], BYTE_TO_BINARY(received[0]));
  // ESP_LOGI("FPGA", "Received: %d, \t 0x%x, " BYTE_TO_BINARY_PATTERN "", received[1], received[1], BYTE_TO_BINARY(received[1]));
  // ESP_LOGI("FPGA", "Received: %d, \t 0x%x, " BYTE_TO_BINARY_PATTERN "", received[2], received[2], BYTE_TO_BINARY(received[2]));

  // uint32_t dataToCheckPrepared = 
  // (reflectByte(toSend)<<24) | 
  // (reflectByte(received[0])<<16) | 
  // (reflectByte(received[1])<<8) | 
  // reflectByte(received[2]); 

  // ESP_LOGI("TEST", "Soft CRC status: %d", checkCRC(dataToCheckPrepared));
  // ESP_LOGI("TEST", "Hard CRC status: %d", checkCRC(crcdata[3], crcdata, 3));
  ESP_LOGI("TEST", "Hard CRC status: %d", checkCRC(crcdata, 4));
  
  return checkCRC(crcdata, 4);
}

// Write 4 bytes
// @param toSend array of 4 elements with data to send
// @note toSend[3] should be empty, crc is generate inside 
// @note toSend[0] 2 MSbs are reserved, inside is proper validation
inline void writeExtFPGA(uint8_t* toSend)
{
  toSend[0] = 0x3f & toSend[0];
  toSend[3] = (genCRC(toSend, 3));

  // ESP_LOGI("FPGA", "FPGA Send: %x %x %x %x", toSend[0], toSend[1], toSend[2], toSend[3]);

  spi_transaction_t transaction = {
    .cmd = toSend[0],
    .length = 3*8,
    .tx_buffer = toSend + 1,
    .rx_buffer = nullptr,
  };
  
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);
  
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);
}

// Receive data using `readExtFPGA`, then check received data 
// if data is good set flag `CONTROL_FLAG_FPGA_CONNECTED` in `controllerStatus`
inline void checkFPGAId()
{
  uint8_t received[3] = {0};

  if(readExtFPGA(0, received) == 0)
  {
    if(received[1] != 0xff)
    {
      ESP_LOGI("FPGA", "FPGA programmed properly!");
      controllerStatus.controlFlags |= CONTROL_FLAG_FPGA_CONNECTED;
    }
  }
  else
  {
    ESP_LOGE("CRC", "BAD CRC!");
  }
}

// Write encapsulated I2C data
// @param data additional send data, only 2 LSbs
// @param i2c_data set I2C SDA state, only 1 LSb 
// @param i2c_clk set I2C CLK state, only 1LSb
// @param receivedData additional received data (port c)
// @return received I2C state
// @note For more info chcek fpga readme
inline uint8_t writeRawI2CFPGA(uint8_t data, uint8_t i2c_data, uint8_t i2c_clk, uint8_t* receivedData)
{
  uint8_t toSend = 0x90 | ((0x03 & data)<<2) | ((0x01 & i2c_data)<<1) | (0x01 & i2c_clk); 
  uint8_t received;

  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 1 * 8,
    .rxlength = 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &received,
  };

  //Set to spi
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);

  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  //Set to drive gpio 
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);

  // ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  received: " BYTE_TO_BINARY_PATTERN "",
  // BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(received));
  if(receivedData)
    *receivedData = received & 0x7f;

  return received >> 7;
}

// Write one byte to I2C device connected to FPGA, blocking funtion - waits to write all data
// @param data write additional data
// @param i2cByte write byte to I2C device
// @param flags set flags to define if this byte is first/middle/last, use one of FPGA_FLAG_I2C_*
// @note First byte must have flag: FPGA_FLAG_I2C_FIRST_BYTE, last byte must have flag: FPGA_FLAG_I2C_LAST_BYTE
// @note Testing purpose, use writeByteI2CFPGA
inline void writeRawByteI2CFPGA(uint8_t data, uint8_t i2cByte, uint8_t flags = 0)
{
  //Set to spi
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);

  uint8_t toSend = 0;
  uint8_t received = 0;
  const uint8_t constSend = 0x90 | ((0x03 & data)<<2);

  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 1 * 8,
    .rxlength = 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &received,
  };

  // write init
  if(flags & FPGA_FLAG_I2C_FIRST_BYTE)
  {
    transaction.cmd = constSend | 1; 
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
  }

  transaction.cmd = constSend; 
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  // write data
  for (uint8_t i = 0; i < 8; i++)
  {
    uint8_t i2cBit = 0x01 & (i2cByte>>(7-i));

    transaction.cmd = constSend | (i2cBit<<1);
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
    //small delay
    // ets_delay_us(10);
    transaction.cmd = constSend | (i2cBit<<1) | 1;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
    //small delay
    // ets_delay_us(5);
    transaction.cmd = constSend | (i2cBit<<1);
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
    //small delay
    // ets_delay_us(10);
    
    // i2cByte = i2cByte >> 1;
  }
  
  // wait for ACK
  transaction.cmd = constSend | 2; 
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  transaction.cmd = constSend | 3; 
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  transaction.cmd = constSend | 2; 
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  // set default state
  transaction.cmd = constSend | 2;
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
  
  if(flags & FPGA_FLAG_I2C_LAST_BYTE)
  {
    transaction.cmd = constSend ;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

    transaction.cmd = constSend | 1;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

    transaction.cmd = constSend | 3;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
  }

  //Set to drive gpio 
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);
}

// Read one byte from I2C device connected to FPGA, blocking funtion - waits to read all data
// @param data write additional data
// @param flags set flags to define if this byte is first/middle/last, use one of FPGA_FLAG_I2C_*
// @return received byte from I2C device
// @note First byte must be written to device not red, last byte must have flag: FPGA_FLAG_I2C_LAST_BYTE
// @note Testing purpose, use readByteI2CFPGA
inline uint8_t readRawByteI2CFPGA(uint8_t data, uint8_t flags = 0)
{
  //Set to spi
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);

  uint8_t toSend = 0;
  uint8_t received = 0;
  uint8_t i2cReceived = 0;
  const uint8_t constSend = 0x90 | ((0x03 & data)<<2);

  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 1 * 8,
    .rxlength = 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &received,
  };

  // send init
  if(flags & FPGA_FLAG_I2C_FIRST_BYTE)
  {
    transaction.cmd = constSend | 1; 
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
  }

  transaction.cmd = constSend; 
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  // read data
  for (uint8_t i = 0; i < 8; i++)
  {
    transaction.cmd = constSend | 2;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
    //small delay
    // ets_delay_us(10);
    transaction.cmd = constSend | 3;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
    i2cReceived = (i2cReceived << 1) | (received >> 7);
    //small delay
    // ets_delay_us(5);
    transaction.cmd = constSend | 2;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
    //small delay
    // ets_delay_us(10);
  }
  
  // set ACK
  transaction.cmd = constSend; 
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  transaction.cmd = constSend | 1; 
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  transaction.cmd = constSend; 
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  // set default state
  transaction.cmd = constSend | 2;
  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));


  if(flags & FPGA_FLAG_I2C_LAST_BYTE)
  {
    transaction.cmd = constSend ;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

    transaction.cmd = constSend | 1;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

    transaction.cmd = constSend | 3;
    ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
  }

  //Set to drive gpio 
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);

  return i2cReceived;
}



// Queue contain data to write from fpga to the I2C device
struct QueueDataI2C
{
  uint8_t bytesOfData[FPGA_MAX_I2C_QUEUE_SIZE] = { 0 };
  uint8_t bytesOfFlags[FPGA_MAX_I2C_QUEUE_SIZE] = { 0 };

  uint8_t queueCounterData = 0;
  uint8_t enqueueIndex = 0;
  uint8_t dequeueIndex = 0;

  // @return 0 on success
  uint8_t enqueue(uint8_t data, uint8_t flags)
  {
    if(queueCounterData == FPGA_MAX_I2C_QUEUE_SIZE)
      return 1;

    bytesOfData[enqueueIndex] = data;
    bytesOfFlags[enqueueIndex] = flags;

    enqueueIndex++;
    queueCounterData++;
    // enqueueIndex = enqueueIndex % FPGA_MAX_I2C_QUEUE_SIZE;//which is faster?
    if(enqueueIndex == FPGA_MAX_I2C_QUEUE_SIZE)
      enqueueIndex = 0;

    return 0;
  }

  // @return 0 on success
  uint8_t dequeue(uint8_t* data, uint8_t* flags)
  {
    if(queueCounterData == 0)
      return 1;
    
    *data = bytesOfData[dequeueIndex];
    *flags = bytesOfFlags[dequeueIndex];

    queueCounterData--;
    dequeueIndex++;
    // dequeueIndex = enqueueIndex % FPGA_MAX_I2C_QUEUE_SIZE;//which is faster?
    if(dequeueIndex == FPGA_MAX_I2C_QUEUE_SIZE)
      dequeueIndex = 0;

    return 0;
  }
} queueDataI2C;

// Queue contain data to write to fpga
struct QueueRawDataToWriteTPGA
{
  uint8_t bytesOfData[FPGA_MAX_QUEUE_SIZE] = { 0 };
  uint8_t bytesOfFlags[FPGA_MAX_QUEUE_SIZE] = { 0 };

  uint8_t queueCounterData = 0;
  uint8_t enqueueIndex = 0;
  uint8_t dequeueIndex = 0;

  // @return 0 on success
  uint8_t enqueue(uint8_t data, uint8_t flags = 0)
  {
    if(queueCounterData == FPGA_MAX_QUEUE_SIZE)
      return 1;

    bytesOfData[enqueueIndex] = data;
    bytesOfFlags[enqueueIndex] = flags;

    enqueueIndex++;
    queueCounterData++;
    // enqueueIndex = enqueueIndex % FPGA_MAX_I2C_QUEUE_SIZE;//which is faster?
    if(enqueueIndex == FPGA_MAX_QUEUE_SIZE)
      enqueueIndex = 0;

    return 0;
  }

  // @return 0 on success
  uint8_t dequeue(uint8_t* data, uint8_t* flags)
  {
    if(queueCounterData == 0)
      return 1;
    
    *data = bytesOfData[dequeueIndex];
    *flags = bytesOfFlags[dequeueIndex];

    queueCounterData--;
    dequeueIndex++;
    // dequeueIndex = enqueueIndex % FPGA_MAX_I2C_QUEUE_SIZE;//which is faster?
    if(dequeueIndex == FPGA_MAX_QUEUE_SIZE)
      dequeueIndex = 0;

    return 0;
  }
} queueRawDataToWriteTPGA;

// TODO change to queue?
uint8_t receivedDataFPGA = 0;
uint8_t receivedByteFlag = 0; //Temp

// Add write byte to queue, this is no bloking function
// @param toSend byte to be send
// @param flags set flags to define if this byte is first/middle/last, use one of FPGA_FLAG_I2C_*
inline void writeByteI2CFPGA(uint8_t toSend, uint8_t flags)
{
  queueDataI2C.enqueue(toSend, flags);
}

// Add read byte to queue, this is no bloking function
// @param flags set flags to define if this byte is middle/last, use one of FPGA_FLAG_I2C_*
inline void readByteI2CFPGA(uint8_t flags)
{
  queueDataI2C.enqueue(0xff, flags | FPGA_FLAG_I2C_RECEIVE_BYTES);
}

// Change byte from queue with data fpga<->I2C_device to bytes to write to fpga
// @param additionalData only 2 LSb, additional bits wich drive 2 outputs on fpga 
void generateI2CPacketsToWriteFPGA(uint8_t additionalData = 0)
{
  uint8_t i2cByte;
  uint8_t flags = 0;
  queueDataI2C.dequeue(&i2cByte, &flags);
  uint8_t sendFlags=0; 
  
  const uint8_t constSend = 0x90 | ((0x03 & additionalData)<<2);

  if(flags & FPGA_FLAG_I2C_RECEIVE_BYTES)
  {
    sendFlags = FPGA_FLAG_RECEIVE_BIT;
  }

  if(flags & FPGA_FLAG_I2C_FIRST_BYTE)
  {
    queueRawDataToWriteTPGA.enqueue(constSend | 1);
  }

  for (uint8_t i = 0; i < 8; i++)
  {
    uint8_t i2cBit = 0x01 & (i2cByte>>(7-i));

    queueRawDataToWriteTPGA.enqueue(constSend | (i2cBit<<1));
    if(i == 7 && (sendFlags & FPGA_FLAG_RECEIVE_BIT)) // last bit
      sendFlags |= FPGA_FLAG_RECEIVE_LAST_BIT;
    queueRawDataToWriteTPGA.enqueue(constSend | (i2cBit<<1) | 1, sendFlags);

    queueRawDataToWriteTPGA.enqueue(constSend | (i2cBit<<1));
  }

  // set ACK
  if(flags & FPGA_FLAG_I2C_RECEIVE_BYTES)
  {
    queueRawDataToWriteTPGA.enqueue(constSend );
    queueRawDataToWriteTPGA.enqueue(constSend | 1);
    queueRawDataToWriteTPGA.enqueue(constSend);
  }
  else // wait for ACK
  {
    queueRawDataToWriteTPGA.enqueue(constSend | 2);
    queueRawDataToWriteTPGA.enqueue(constSend | 3, FPGA_FLAG_ACK_BIT);
    queueRawDataToWriteTPGA.enqueue(constSend | 2);
  }

  // set default state
  queueRawDataToWriteTPGA.enqueue(constSend | 2);

  if(flags & FPGA_FLAG_I2C_LAST_BYTE)
  {
    queueRawDataToWriteTPGA.enqueue(constSend);
    queueRawDataToWriteTPGA.enqueue(constSend | 1);
    queueRawDataToWriteTPGA.enqueue(constSend | 3);
  }
}

// If can write data to fpga
// @param additionalData only 2 LSb, additional bits wich drive 2 outputs on fpga 
void processSendFPGA(uint8_t additionalData)
{
  if(queueRawDataToWriteTPGA.queueCounterData == 0)
  {
    if(queueDataI2C.queueCounterData > 0)
      generateI2CPacketsToWriteFPGA(additionalData);
    return;
  }
  
  uint8_t toSend;
  uint8_t received;
  uint8_t flags;

  queueRawDataToWriteTPGA.dequeue(&toSend, &flags);
  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 1 * 8,
    .rxlength = 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &received,
  };

  // ESP_LOGI("QUEUE_FPGA", "Data in FPGA queue(%d):", queueRawDataToWriteTPGA.queueCounterData);
  // for (uint8_t i = 0; i < queueRawDataToWriteTPGA.queueCounterData; i++)
  // {
  //   ESP_LOGI("QUEUE_FPGA", "en: 0x%x de: 0x%x data: 0x%x", 
  //           queueRawDataToWriteTPGA.enqueueIndex, 
  //           queueRawDataToWriteTPGA.dequeueIndex, 
  //           queueRawDataToWriteTPGA.bytesToWrite[(queueRawDataToWriteTPGA.dequeueIndex + i) % FPGA_MAX_I2C_QUEUE_SIZE]
  //           );
  // }
  //ESP_LOGI("QUEUE_FPGA", "to send: %x, flags: %x", toSend, flags);

  //Set to spi
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);

  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));
  //Set to drive gpio 
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);

  if(flags & FPGA_FLAG_RECEIVE_BIT)
  {
    receivedDataFPGA = (receivedDataFPGA << 1) | (received >> 7);
    if(flags & FPGA_FLAG_RECEIVE_LAST_BIT)
    {
      // TODO set proper flag
      receivedByteFlag = 1;
      ESP_LOGI("QUEUE_FPGA", "received: %x", receivedDataFPGA);
    }
  }

  if(flags & FPGA_FLAG_ACK_BIT)
  {
    if(received & 0x80)
    {
      // TODO add to errors No ACK
      ESP_LOGE("QUEUE_FPGA", "No ACK");
    }
  }
}

// Testing

uint8_t bufCounter = 0;
TickType_t buf = 0;
TickType_t buf2 = 0;

void testWriteI2C()
{
    if(xTaskGetTickCount() - buf2 > 5000)
    {
      writeByteI2CFPGA(0x08, FPGA_FLAG_I2C_FIRST_BYTE);
      if(bufCounter % 2 == 0)
        writeByteI2CFPGA('q', FPGA_FLAG_I2C_LAST_BYTE);
      else
        writeByteI2CFPGA('a', FPGA_FLAG_I2C_LAST_BYTE);

      ESP_LOGI("QUEUE_I2C", "Led changed");

      bufCounter++;
      buf2 = xTaskGetTickCount();
    }
}

void testReadI2C()
{
  if(xTaskGetTickCount() - buf > 10000)
    {
      // writeByteI2CFPGA(0x08, FPGA_FLAG_I2C_FIRST_BYTE);
      // if(bufCounter % 2 == 0)
      //   writeByteI2CFPGA('q', FPGA_FLAG_I2C_LAST_BYTE);
      // else
      //   writeByteI2CFPGA('a', FPGA_FLAG_I2C_LAST_BYTE);
      writeByteI2CFPGA(0x09, FPGA_FLAG_I2C_FIRST_BYTE);
      readByteI2CFPGA(FPGA_FLAG_I2C_LAST_BYTE);


        ESP_LOGI("QUEUE_I2C", "Data in I2C queue(%d):", queueDataI2C.queueCounterData);
        for (uint8_t i = 0; i < queueDataI2C.queueCounterData; i++)
        {
          ESP_LOGI("QUEUE_I2C", "en: 0x%x de: 0x%x data: 0x%x flags: 0x%x", 
                  queueDataI2C.enqueueIndex, 
                  queueDataI2C.dequeueIndex, 
                  queueDataI2C.bytesOfData[(queueDataI2C.dequeueIndex + i) % FPGA_MAX_I2C_QUEUE_SIZE],
                  queueDataI2C.bytesOfFlags[(queueDataI2C.dequeueIndex + i) % FPGA_MAX_I2C_QUEUE_SIZE]
                  );
        }
        
      // bufCounter++;
      buf = xTaskGetTickCount();
    }
}