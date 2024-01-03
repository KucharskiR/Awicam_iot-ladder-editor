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

spi_device_handle_t fpga;

spi_bus_config_t spi_conifg = {//Default MSB
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

// Read inputs states
// @return recived byte
inline uint8_t readInputsFPGA(uint8_t data)
{
  uint8_t toSend = 0x80 | (0x0f & data); 
  uint8_t recived;

  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 1*8,
    .rxlength = 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &recived,
  };


  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  // ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  recived: " BYTE_TO_BINARY_PATTERN "",
  // BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(recived));

  return recived;
}

inline void writeOutputsFPGA(uint16_t data)
{
  uint8_t toSend0 = 0x3f & (data >> 8) ;
  uint8_t toSend1 = 0x00ff & data;

  spi_transaction_t transaction = {
    .cmd = toSend0,
    .length = 1*8,
    .tx_buffer = &toSend1,
    .rx_buffer = nullptr,
  };

  ESP_ERROR_CHECK(spi_device_transmit(fpga, &transaction));

  // ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "",
  // BYTE_TO_BINARY(toSend[0]), BYTE_TO_BINARY(toSend[1]));
}


//-----------------------------------
//--------------Testing--------------
//-----------------------------------

// Etap 1 wysyła 14 bitów i ustawia o=z
void writeFPGAstage1(uint16_t data)
{
  uint8_t dataa = 0x00ff & data;
  uint8_t cmd = 0x3f & (data>> 8) ;

  spi_transaction_t transaction = {
    .cmd = cmd,
    .length = 1*8,
    .tx_buffer = &dataa,
    .rx_buffer = nullptr,
  };

  spi_device_transmit(fpga, &transaction);

  ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "",
  BYTE_TO_BINARY(cmd), BYTE_TO_BINARY(dataa));
}

// Etap 2 wysłanie bajtu i odebranie bajtu
// >1>0> 0>0> AB>AA>A9>A8 <B7<B6<B5<B4<B3<B2<B1<B0.
//  K A PORT
void writeFPGAstage2(uint8_t data)
{
  uint8_t toSend = 0x80 | (0x0f & data); 
  uint8_t recived;

  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 1*8,
    .rxlength = 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &recived,
  };


  spi_device_transmit(fpga, &transaction);

  ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  recived: " BYTE_TO_BINARY_PATTERN "",
  BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(recived));
}

// Etap 3      
// data -> AD AC 
// >1>0>0>1>SD>SC>AD>AC <SA<C6<C5<C4<C3<C2<C1<C0
//Nie testowane
void writeFPGAstage3(uint8_t data, uint8_t i2c)
{
  uint8_t toSend = 0x90 | ((0x03 & i2c)<<2) | (0x03 & data); 
  uint8_t recived;

  spi_transaction_t transaction = {
    .cmd = toSend,
    .length = 1*8,
    .rxlength = 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &recived,
  };


  spi_device_transmit(fpga, &transaction);


  ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  recived: " BYTE_TO_BINARY_PATTERN "",
  BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(recived));
}


//Not work
void writeFPGAi2c(uint8_t sendData, uint8_t reciveData, uint8_t numOfSend, uint8_t numOfRecive)
{
  /*
  Default: 
  SDA->1 SCL->1
  
  one bit:
  SDA->x SCL->0
  SDA->x SCL->1
  SDA->y SCL->0
  
  
  */
  uint8_t toSend;
  uint8_t recived;

  spi_transaction_t transaction = {
    .length = 2*8,
    .rxlength = 8,
    .tx_buffer = &toSend,
    .rx_buffer = &recived,
  };


  for (uint8_t i = 0; i < numOfSend; i++)
  {
    toSend = 0x90 | ((0x01 & (sendData>>(8-numOfSend)))<<2); //| (0x03 & data)
    toSend = 0x90 | ((0x01 & (sendData>>(8-numOfSend)))<<2) | 0x04; //| (0x03 & data)
    spi_device_transmit(fpga, &transaction);
  }
  for (uint8_t i = 0; i < numOfRecive; i++)
  {
    toSend = 0x98; //| (0x03 & data)
    toSend = 0x9c; //| (0x03 & data)
    spi_device_transmit(fpga, &transaction);
    reciveData = reciveData>>1 | recived & 0x80;
  }
  for (uint8_t i = 0; i < numOfRecive; i++)
  {
    toSend = 0x98; //| (0x03 & data)
    toSend = 0x9c; //| (0x03 & data)
    spi_device_transmit(fpga, &transaction);
    reciveData = reciveData>>1 | recived & 0x80;
  }

  toSend = 0x9c;
  spi_device_transmit(fpga, &transaction);

  ESP_LOGI("FPGA_SPI", "I2C send: " BYTE_TO_BINARY_PATTERN "  recived: " BYTE_TO_BINARY_PATTERN "",
  BYTE_TO_BINARY(sendData), BYTE_TO_BINARY(reciveData));
}

void writeFPGAstage4(uint8_t data)
{
  uint32_t toSend = 0x80 | (0x0f & data); 
  // uint8_t recived[3] ={0};
  uint32_t recived = 0;

  spi_transaction_t transaction = {
    .cmd = (uint16_t)toSend, //Bez bitow danych i z tx_buffer nie dziala tak jak powinno
    .length = 4 * 8, //Nie jestem pewien czy dobrze
    .rxlength = 3 * 8,
    // .tx_buffer = &toSend,
    .rx_buffer = &recived,
  };

  spi_device_transmit(fpga, &transaction);

  ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  recived: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " ",
  BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(recived), BYTE_TO_BINARY(recived>>8), BYTE_TO_BINARY(recived>>16));
}

//Send to EEPROM
// void testFPGAi2c()
// {
  // START
  // Clk: 1 Data: falling
// }

void testFPGA()
{
  spi_bus_initialize(SPI2_HOST, &spi_conifg, SPI_DMA_DISABLED);
  spi_bus_add_device(SPI2_HOST, &fpga_conifg, &fpga);

  uint8_t dataToSend2 = 0x00;
    uint16_t buff = 0b0000000000000100;

  while(1)
  {
    // writeFPGAstage2(0b01010101);
    // writeFPGAstage4(0xff);
    // Test stage 1
    ESP_LOGI("TEST_FPGA", "Test stage 1");
    
    // for(int i = 0; i < 32; i++)
    // {
    //   if(buff != 0b0010000000000000)
    //     buff = buff << 1;  
    //   else
    //     buff = 0b00000000000000100;

      // buff=0xcccc;
      // writeFPGAstage1(buff);

    //   vTaskDelay(100 / portTICK_PERIOD_MS);
    // }

    
    // Test stage 2
    // writeFPGAstage1(0xffff);
    // ESP_LOGI("TEST_FPGA", "Test stage 2");
    dataToSend2 = 0b00001100;
    // for(uint8_t i = 0; i < 16; i++)
    // {
      writeFPGAstage2(dataToSend2);
    //   // dataToSend2 = dataToSend2 + 1;
    //   vTaskDelay(100 / portTICK_PERIOD_MS);
    // }


//     // Test stage 3
//     // Clk:  0b10 0001 0010 0100 1001 0111
//     // Data: 0b10 1000 0111 1110 0011 1000
//     //                       | <- tu chyba mozna zmienic 
//     writeFPGAstage1(0xffff);
//     dataToSend2 = 1;
//     ESP_LOGI("TEST_FPGA", "Test stage 3");
//     uint32_t i2cClk =  0b110101010101010101010101011;
//     uint32_t i2cData = 0b110100000111111100001111001;
//     for(uint8_t i = 0; i < 27; i++)
//     {
//       writeFPGAstage3(dataToSend2, ((i2cData & 0x01)<<1) | (i2cClk & 0x01));
//       dataToSend2 = ~dataToSend2;
// //      i2cData = ~(0x01 & i2cData) | 0xfe & i2cData;
//       i2cClk = i2cClk>>1;
//       i2cData = i2cData>>1;

//       // vTaskDelay(100 / portTICK_PERIOD_MS);
//     }
    //Test stage 4


    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}


