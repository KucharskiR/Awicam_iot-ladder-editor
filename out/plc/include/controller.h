#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_task_wdt.h"
// #include "esp_wifi.h"
// #include "nvs_flash.h"
// #include "esp_http_server.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_partition.h"
// #include "esp_rom_gpio.h"
#include "esp_ota_ops.h"
// #include <WebServer.h>
#include "esp_spiffs.h"
#include "SPIFFS.h"
#include "esp32-hal-gpio.h"
// #include "esp32-hal-spi.h"

#include <Arduino.h>
#include <SPI.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiAP.h>
// #include <ESPAsyncWebServer.h>

#include "defines.h"


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


uint8_t i = 0;
uint8_t boardsNumber = 0;

struct digitalInputsStructure {
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
};
// Inputs and outputs structure   inputs[0]->ESP32
digitalInputsStructure inputs[32] = {0};

const uint8_t PARITY_EVEN_TABLE[128] = {
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1};

static const int RX_BUF_SIZE = 1024;
uint8_t bytes[4];

spi_device_handle_t fpga;




/*---------------------------------------------*/
/*--------------UART communication-------------*/
/*---------------------------------------------*/

//Start transmit to start recive equals ~17us
uint32_t lastSendTime = 0;
volatile bool recivedAll = true;
int SendInitCommand() {
  uint8_t bytes[4] = {63, 2, 0, 61};
  const int txBytes = uart_write_bytes(UART_BB, bytes, 4);
  return txBytes;
}
int SendIntroduceCommand(uint8_t deviceNumber) {
  //deviceNumber 1-31
  uint8_t bytes[4];
  uint8_t tempByte0 = (DATA_COMMAND_FLAG_MASK >> 1) | deviceNumber;
  bytes[0] = (tempByte0 << 1) | PARITY_EVEN_TABLE[tempByte0];
  bytes[1] = INTRODUCE;
  bytes[2] = 0;
  bytes[3] = bytes[0] ^ bytes[1] ^ bytes[2];
  const int txBytes = uart_write_bytes(UART_BB, bytes, 4);
  #ifdef DEBUG
  ESP_LOGI("Send Introduction Request", "Introduce, quartet %d %d %d %d\n", bytes[0], bytes[1], bytes[2], bytes[3]);
  // for(int i = 0; i < 4; i++)
  // ESP_LOGI("Send Introduction Request", "Introduce, quartet[%d]: "BYTE_TO_BINARY_PATTERN"", i, BYTE_TO_BINARY(bytes[i]));
  #endif
  return txBytes;
}
int SendDigitalOutputs(uint8_t deviceNumber, uint16_t outputs) {
  uint8_t tmp[4];
  // uint8_t bytes[4];
  tmp[0] = (deviceNumber << 1) | PARITY_EVEN_TABLE[deviceNumber];
  tmp[1] = (uint8_t) outputs;
  tmp[2] = (uint8_t)(outputs >> 8);
  tmp[3] = tmp[0] ^ tmp[1] ^ tmp[2];
  
  #ifdef DEBUG
  #ifndef NO_DEBUG_UART
  static const char *TASK_TAG = "SEND DO";
  ESP_LOGI(TASK_TAG, "byte[0]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(tmp[0]));
  ESP_LOGI(TASK_TAG, "byte[1]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(tmp[1]));
  ESP_LOGI(TASK_TAG, "byte[2]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(tmp[2]));
  ESP_LOGI(TASK_TAG, "byte[3]: " BYTE_TO_BINARY_PATTERN "\n",(uint8_t)BYTE_TO_BINARY(tmp[3]));
  #endif
  #endif
  const int txBytes = uart_write_bytes(UART_BB, (void*)tmp, 4);
  if(lastSendTime == 0)
    lastSendTime = xTaskGetTickCount();
  return txBytes;
}
int SendAnalogInputSetup(uint8_t deviceNumber, uint8_t numberOfAI){
  uint8_t bytes[4];
  uint8_t tempByte0 = (DATA_COMMAND_FLAG_MASK >> 1) | deviceNumber;
  bytes[0] = (tempByte0 << 1) | PARITY_EVEN_TABLE[tempByte0];
  bytes[1] = (numberOfAI << 4) | 0x02;
  bytes[2] = 0;
  bytes[3] = bytes[0] ^ bytes[1] ^ bytes[2];
  const int txBytes = uart_write_bytes(UART_BB, bytes, 4);
  return txBytes;
}
int SendAnalogRead(uint8_t deviceNumber, uint8_t analogInput){
  uint8_t bytes[4];
  uint8_t tempByte0 = (DATA_COMMAND_FLAG_MASK >> 1) | deviceNumber;
  bytes[0] = (tempByte0 << 1) | PARITY_EVEN_TABLE[tempByte0];
  bytes[1] = 10 + analogInput;
  bytes[2] = 0;
  bytes[3] = bytes[0] ^ bytes[1] ^ bytes[2];
  const int txBytes = uart_write_bytes(UART_BB, bytes, 4);
  return txBytes;
}
int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART_BB, data, len);
	#ifdef DEBUG
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
	#endif
    return txBytes;
}
int sendCommand(uint8_t deviceNumber, uint8_t command)
{
  uint8_t bytes[4];
  uint8_t tempByte0 = (DATA_COMMAND_FLAG_MASK >> 1) | deviceNumber;
  bytes[0] = (tempByte0 << 1) | PARITY_EVEN_TABLE[tempByte0];
  bytes[1] = command;
  bytes[2] = 0;
  bytes[3] = bytes[0] ^ bytes[1] ^ bytes[2];
  const int txBytes = uart_write_bytes(UART_BB, bytes, 4);

  #ifdef DEBUG
  #ifndef NO_DEBUG_UART
  static const char *TASK_TAG = "SEND COMMAND";
  ESP_LOGI(TASK_TAG, "COMMAND NUM: %d", command);
  ESP_LOGI(TASK_TAG, "byte[0]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(bytes[0]));
  ESP_LOGI(TASK_TAG, "byte[1]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(bytes[1]));
  ESP_LOGI(TASK_TAG, "byte[2]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(bytes[2]));
  ESP_LOGI(TASK_TAG, "byte[3]: " BYTE_TO_BINARY_PATTERN "\n",(uint8_t)BYTE_TO_BINARY(bytes[3]));
  #endif
  #endif

  return txBytes;
}

//static void tx_task(void *arg)
//{
//    static const char *TX_TASK_TAG = "TX_TASK";
//    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
//    while (1) {
//        sendData(TX_TASK_TAG, "Hello world");
//        vTaskDelay(300 / portTICK_PERIOD_MS);
//    }
//}
// void checkQuartet(const char* logName, uint8_t quartet[]){
//   uint8_t calculatedXOR = quartet[0] ^ quartet[1] ^ quartet[2];
//   if(calculatedXOR == quartet[3])
//   {
//     uint8_t parityBitRecived = quartet[0] & 1;
//     uint8_t calculatedParityBit = PARITY_EVEN_TABLE[quartet[0] >> 1];
//     if(parityBitRecived == calculatedParityBit)
//     {
//       uint8_t address = (quartet[0] & ADDRESS_MASK) >> 1;
//       if(address == (INIT_ADDRESS >> 1)) 
//       {
//         //INIT
//         boardsNumber = (quartet[1] >> 1) - 1;
//         if(inputs) {
//         	free(inputs);
//         }
//         if(boardsNumber) {
//         	inputs = (digitalInputsStructure*)malloc(boardsNumber * sizeof(digitalInputsStructure));
//           // inputs = new digitalInputsStructure[boardsNumber];
//         }
//         #ifdef DEBUG
//         //ESP_LOGI(logName, "sizeof(struct digitalInputsStructure): %d \n", sizeof(struct digitalInputsStructure));
//         //ESP_LOGI(logName, "sizeof(inputs): %d \n", sizeof(inputs));
//         //ESP_LOGI(logName, "inputs: %d \n", inputs);
//         ESP_LOGI(logName, "-----INIT-----");
//         ESP_LOGI(logName, "Boards number: %d", boardsNumber);
//         //Serial.println(boardsNumber);
//         #endif
//       }
//       else
//       {
//         uint8_t responseBit = quartet[0] & REQUEST_RESPONSE_FLAG_MASK;
//         if(responseBit){
//         #ifdef DEBUG
//         //Serial.println();
//         ESP_LOGI(logName, "Device address: %d", address);
//         //Serial.println(address);
//         #endif
//         uint8_t dataCommandBit = quartet[0] & DATA_COMMAND_FLAG_MASK;
//         address--;
//         if(dataCommandBit){
//           //cmd
//           uint8_t commandNumber = quartet[1] & 0x0F;
//           switch(commandNumber) {
//               case 1:  // 0x01 introduce
//                 // ESP_LOGI(logName, "Test introduce address: %d", address);
//                 inputs[address].deviceType = 0;// static_cast<uint8_t>( quartet[1] >> 4);
//                 inputs[address].deviceInitTime = xTaskGetTickCount();
//                 inputs[address].firmwareVersion = quartet[2];
//                 #ifdef DEBUG
//                 ESP_LOGI(logName, "Introduce, Time: %d", (int)inputs[address].deviceInitTime);
//                 ESP_LOGI(logName, "Introduce, DeviceType: %d", inputs[address].deviceType);
//                 ESP_LOGI(logName, "Introduce, FW Version: %d", inputs[address].firmwareVersion);
//                 //Serial.println(inputs[address].deviceType);
//                 #endif
//                 break;
//               case SETUP_ANALOG_INPUT:  // 0x02 setup analog inputs
//                 inputs[address].numberOfAnalogInputs = quartet[1] >> 4;
//                 #ifdef DEBUG
//                 ESP_LOGI(logName, "AI setup. Number of analof inputs: %d\n", inputs[address].numberOfAnalogInputs);
//                 //Serial.println(inputs[address].numberOfAnalogInputs);
//                 #endif
//                 break;
//               case ANALOG_READ_1: // 0x0B analog read 1
//                 inputs[address].aIUpdateTime[0] = xTaskGetTickCount();
//                 inputs[address].aIValue[0] = ((quartet[1] & 0xF0) << 4) + quartet[2];
//                 #ifdef DEBUG
//                 ESP_LOGI(logName, "AI 1 value: %d\n", inputs[address].aIValue[0]);
//                 //Serial.println(inputs[address].aIValue[0]);
//                 #endif
//                 break;
//               case ANALOG_READ_2: // 0x0C analog read 2
//                 inputs[address].aIUpdateTime[1] = xTaskGetTickCount();
//                 inputs[address].aIValue[1] = ((quartet[1] & 0xF0) << 4) + quartet[2];
//                 #ifdef DEBUG
//                 ESP_LOGI(logName, "AI 2 value: %d\n", inputs[address].aIValue[1]);
//                 //Serial.println(inputs[address].aIValue[1]);
//                 #endif
//                 break;
//               case ANALOG_READ_3: // 0x0D analog read 3
//                 inputs[address].aIUpdateTime[2] = xTaskGetTickCount();
//                 inputs[address].aIValue[2] = ((quartet[1] & 0xF0) << 4) + quartet[2];
//                 #ifdef DEBUG
//                 ESP_LOGI(logName, "AI 3 value: %d\n", inputs[address].aIValue[2]);
//                 //Serial.println(inputs[address].aIValue[2]);
//                 #endif
//                 break;
//               case ANALOG_READ_4: // 0x0E analog read 4
//                 inputs[address].aIUpdateTime[3] = xTaskGetTickCount();
//                 inputs[address].aIValue[3] = ((quartet[1] & 0xF0) << 4) + quartet[2];
//                 #ifdef DEBUG
//                 ESP_LOGI(logName, "AI 4 value: %d\n", inputs[address].aIValue[3]);
//                 //Serial.println(inputs[address].aIValue[3]);
//                 #endif
//                 break;
//             }
//           }else{
//             //data eq. to digital inputs
//             inputs[address].lastDigitalInputUpdateTime = xTaskGetTickCount();
//             inputs[address].digitalInputStates = (quartet[2] << 8) + quartet[1];
//             #ifdef DEBUG
//             ESP_LOGI(logName, "-----DI-----");
//             ESP_LOGI(logName, "Update time %d     ", (int)inputs[address].lastDigitalInputUpdateTime);
//             ESP_LOGI(logName, "Input states: %d", inputs[address].digitalInputStates);
//             ESP_LOGI(logName, "Input states HIGH BYTE: " BYTE_TO_BINARY_PATTERN "",(uint8_t)BYTE_TO_BINARY(inputs[address].digitalInputStates>>8));
//             ESP_LOGI(logName, "Input states  LOW BYTE: " BYTE_TO_BINARY_PATTERN "",(uint8_t)BYTE_TO_BINARY(inputs[address].digitalInputStates));
//             #endif
//           }
//         }else{
//           //crc ok but response bit not set
//           #ifdef DEBUG
//         	ESP_LOGI(logName, "Received original message (no response bit set!)\n");
//           #endif
//         }
//       }
//     }
//     else
//     {
//       #ifdef DEBUG
//       ESP_LOGI(logName, "Parity bit is not correct!");
//       #endif
//     }
//   }
//   else
//   {
//     #ifdef DEBUG
//     ESP_LOGI(logName, "XOR byte is not correct!");
//     #endif
//   }
// }
volatile uint32_t errorsUart = 0;
void checkQuartet(const char* logName, uint8_t quartet[]) {
  uint8_t calculatedXOR = quartet[0] ^ quartet[1] ^ quartet[2];
  if(calculatedXOR != quartet[3]) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    // uart_flush(UART_BB);
    errorsUart |= 0x0001;
    #ifdef DEBUG
    ESP_LOGI(logName, "XOR byte is not correct!");
    ESP_LOGI(logName, "byte[0]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(quartet[0]));
    ESP_LOGI(logName, "byte[1]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(quartet[1]));
    ESP_LOGI(logName, "byte[2]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(quartet[2]));
    ESP_LOGI(logName, "byte[3]: " BYTE_TO_BINARY_PATTERN "\n",(uint8_t)BYTE_TO_BINARY(quartet[3]));
    #endif
    return;
  }

  uint8_t parityBitRecived = quartet[0] & 1;
  uint8_t calculatedParityBit = PARITY_EVEN_TABLE[quartet[0] >> 1];
  if(parityBitRecived != calculatedParityBit) {
    // uart_flush(UART_BB);
    errorsUart |= 0x0001;
    #ifdef DEBUG
    ESP_LOGI(logName, "Parity bit is not correct!");
    #endif
    return;
  }

  uint8_t address = (quartet[0] & ADDRESS_MASK) >> 1;
  if(address == (INIT_ADDRESS >> 1)) {
    //INIT
    boardsNumber = (quartet[1] >> 1) - 1;
    #ifdef DEBUG
    //ESP_LOGI(logName, "sizeof(struct digitalInputsStructure): %d \n", sizeof(struct digitalInputsStructure));
    //ESP_LOGI(logName, "sizeof(inputs): %d \n", sizeof(inputs));
    //ESP_LOGI(logName, "inputs: %d \n", inputs);
    ESP_LOGI(logName, "-----INIT-----");
    ESP_LOGI(logName, "Boards number: %d", boardsNumber);
    //Serial.println(boardsNumber);
    #endif
  }
  else
  {
    uint8_t responseBit = quartet[0] & REQUEST_RESPONSE_FLAG_MASK;
    if(responseBit) {
    #ifdef DEBUG
    //Serial.println();
    // ESP_LOGI(logName, "Device address: %d", address);
    //Serial.println(address);
    #endif
    uint8_t dataCommandBit = quartet[0] & DATA_COMMAND_FLAG_MASK;
    // address--;
    if(dataCommandBit) {
      //cmd
      uint8_t commandNumber = quartet[1] & 0x0F;
      switch(commandNumber) {
          case INTRODUCE:  // 0x01 introduce
            // ESP_LOGI(logName, "Test introduce address: %d", address);
            inputs[address].deviceType = 0;// static_cast<uint8_t>( quartet[1] >> 4);
            inputs[address].deviceInitTime = xTaskGetTickCount();
            inputs[address].firmwareVersion = quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "Introduce, Time: %d", (int)inputs[address].deviceInitTime);
            ESP_LOGI(logName, "Introduce, DeviceType: %d", inputs[address].deviceType);
            ESP_LOGI(logName, "Introduce, FW Version: %d", inputs[address].firmwareVersion);
            //Serial.println(inputs[address].deviceType);
            #endif
            break;
          case SETUP_ANALOG_INPUT:  // 0x02 setup analog inputs
            inputs[address].numberOfAnalogInputs = quartet[1] >> 4;
            #ifdef DEBUG
            ESP_LOGI(logName, "AI setup. Number of analof inputs: %d\n", inputs[address].numberOfAnalogInputs);
            //Serial.println(inputs[address].numberOfAnalogInputs);
            #endif
            break;
          case ANALOG_READ_1: // 0x0B analog read 1
            inputs[address].aIUpdateTime[0] = xTaskGetTickCount();
            inputs[address].aIValue[0] = ((quartet[1] & 0xF0) << 4) + quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "AI 1 value: %d\n", inputs[address].aIValue[0]);
            //Serial.println(inputs[address].aIValue[0]);
            #endif
            break;
          case ANALOG_READ_2: // 0x0C analog read 2
            inputs[address].aIUpdateTime[1] = xTaskGetTickCount();
            inputs[address].aIValue[1] = ((quartet[1] & 0xF0) << 4) + quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "AI 2 value: %d\n", inputs[address].aIValue[1]);
            //Serial.println(inputs[address].aIValue[1]);
            #endif
            break;
          case ANALOG_READ_3: // 0x0D analog read 3
            inputs[address].aIUpdateTime[2] = xTaskGetTickCount();
            inputs[address].aIValue[2] = ((quartet[1] & 0xF0) << 4) + quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "AI 3 value: %d\n", inputs[address].aIValue[2]);
            //Serial.println(inputs[address].aIValue[2]);
            #endif
            break;
          case ANALOG_READ_4: // 0x0E analog read 4
            inputs[address].aIUpdateTime[3] = xTaskGetTickCount();
            inputs[address].aIValue[3] = ((quartet[1] & 0xF0) << 4) + quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "AI 4 value: %d\n", inputs[address].aIValue[3]);
            //Serial.println(inputs[address].aIValue[3]);
            #endif
            break;
        }
      } else {
        //data eq. to digital inputs
        inputs[address].lastDigitalInputUpdateTime = xTaskGetTickCount();
        inputs[address].digitalInputStates = (quartet[2] << 8) + quartet[1];
        #ifdef DEBUG
        #ifndef NO_DEBUG_UART
        ESP_LOGI(logName, "-----DI-----");
        ESP_LOGI(logName, "Update time %d     ", (int)inputs[address].lastDigitalInputUpdateTime);
        ESP_LOGI(logName, "Input states: %d", inputs[address].digitalInputStates);
        ESP_LOGI(logName, "Input states HIGH BYTE: " BYTE_TO_BINARY_PATTERN "",(uint8_t)BYTE_TO_BINARY(inputs[address].digitalInputStates>>8));
        ESP_LOGI(logName, "Input states  LOW BYTE: " BYTE_TO_BINARY_PATTERN "",(uint8_t)BYTE_TO_BINARY(inputs[address].digitalInputStates));
        #endif
        #endif
      }
    } else {
      //crc ok but response bit not set
      errorsUart |= 0x0001;
      #ifdef DEBUG
      ESP_LOGI(logName, "Received original message (no response bit set!)\n");
      #endif
    }
  }
}
static void rx_task(void *arg)
{
	static const char *RX_TASK_TAG = "RX_TASK";
	esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
	uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE);
	uint8_t number = 0;
	ESP_LOGI(RX_TASK_TAG, "Welcome to RX Task");
  // uart_flush
	while(1) {
		const int rxBytes = uart_read_bytes(UART_BB, data, RX_BUF_SIZE, 1 / portTICK_PERIOD_MS);
		if(rxBytes > 0) {
      recivedAll = false;
      errorsUart = errorsUart<<1;
      #ifdef DEBUG
      #ifndef NO_DEBUG_UART
			ESP_LOGI(RX_TASK_TAG, "\nReceived %d bytes", rxBytes);
      ESP_LOGI(RX_TASK_TAG, "byte[0]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(data[0]));
      ESP_LOGI(RX_TASK_TAG, "byte[1]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(data[1]));
      ESP_LOGI(RX_TASK_TAG, "byte[2]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(data[2]));
      ESP_LOGI(RX_TASK_TAG, "byte[3]: " BYTE_TO_BINARY_PATTERN "\n",(uint8_t)BYTE_TO_BINARY(data[3]));
      #endif
      #endif

			for(int i = 0; i < rxBytes; i++) {
				bytes[number++] = data[i];
				if(number > 3) {
					number = 0;
          checkQuartet(RX_TASK_TAG, bytes);
          lastSendTime = 0;
          if(errorsUart & 0x0001)
          {
            uart_flush(UART_BB);
            break;
          }
			  }
			}
		}
    else
    {
      if(xTaskGetTickCount() - lastSendTime > SEND_RESPONSE_WAIT)
      {
        if(lastSendTime != 0)
        {
          errorsUart = errorsUart<<1;
          errorsUart |= 0x0001;
          #ifdef DEBUG
          ESP_LOGI(RX_TASK_TAG, "Sent data but no response!");
          uart_flush(UART_BB);
          lastSendTime = 0;
          recivedAll = true;
          #endif
        }
        else if(recivedAll==false)
          recivedAll = true;
      }
    }
	}
	free(data);
//    static const char *RX_TASK_TAG = "RX_TASK";
//    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
//    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
//    while (1) {
//        const int rxBytes = uart_read_bytes(UART_BB, data, RX_BUF_SIZE, 100 / portTICK_RATE_MS);
//        if (rxBytes > 0) {
//            data[rxBytes] = 0;
//            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: %s'", rxBytes, data);
//            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
//        }
//    }
//    free(data);
}

void initExtensionModules()
{
  const char* TASK_TAG = "EXT_MODULES_INIT";
  
  for (uint8_t i = 0; i < 5; i++)
  {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    #ifdef DEBUG
    ESP_LOGI(TASK_TAG, "Send init command, send %d bytes", SendInitCommand());
    #else
    SendInitCommand();
    #endif
  }

  if(boardsNumber == 0)
  {
    #ifdef DEBUG
    ESP_LOGI(TASK_TAG, "No extension modules found");
    #endif  
    return;
  }

  for(int i = 1; i < boardsNumber + 1; i++)
  {
    SendIntroduceCommand(i);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  for(int i = 1; i < boardsNumber + 1; i++)
  {
    SendDigitalOutputs(i, 0x00);
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }

  #ifdef DEBUG
  ESP_LOGI(TASK_TAG, "Initialized, boards number: %d \n", boardsNumber);
  #endif
}

//---------SPIFFS-----------
void initMemory()
{
  // esp_vfs_spiffs_conf_t config{
  //   .base_path = "/spiffs",
  //   .partition_label = "spiffs",
  //   .max_files = 5,
  //   .format_if_mount_failed = true,
  // };
  // esp_err_t ret = esp_vfs_spiffs_register(&config);
  //   if (ret != ESP_OK) {
  //       if (ret == ESP_FAIL) {
  //           ESP_LOGE("SPIFFS", "Failed to mount or format filesystem");
  //       } else if (ret == ESP_ERR_NOT_FOUND) {
  //           ESP_LOGE("SPIFFS", "Failed to find SPIFFS partition");
  //       } else {
  //           ESP_LOGE("SPIFFS", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
  //       }
  //       return;
  //   }
  // if(esp_spiffs_mounted("spiffs"))
  //   ESP_LOGI("SPIFFS", "Fs mounted!");
  // else
  //   ESP_LOGI("SPIFFS", "Fs NOT mounted!");
  if(SPIFFS.begin(false, "/spiffs", 5, "spiffs"))
      ESP_LOGI("SPIFFS", "Fs mounted!");
    else
      ESP_LOGI("SPIFFS", "Fs NOT mounted!");
}



/*---------------------------------------------*/
/*--------------FPGA communication-------------*/
/*---------------------------------------------*/

void programFPGA()
{
  #if defined(DEBUG) && !defined(NO_DEBUG_SPI)
  const char* TASK_TAG = "PROGAM_FPGA";
  #endif
  
  char* buffer = new char[104161];
  if(!SPIFFS.exists("/fpga.bin"))
  {
    ESP_LOGI("PROGRAM_FPGA", "FPGA config file NOT exists!");
    return;
  }
  fs::File configFPGA = SPIFFS.open("/fpga.bin", "r");
  configFPGA.readBytes(buffer, 104161);
  size_t fileSize = configFPGA.size();
  configFPGA.close();

  #if defined(DEBUG) && !defined(NO_DEBUG_SPI)
  ESP_LOGI(TASK_TAG, "File loaded to memory, fragment:");
  ESP_LOGI(TASK_TAG, "%#02x %#02x %#02x %#02x %#02x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
  #endif

  spi_device_interface_config_t device_conifg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,                  //SPI mode 0
    .clock_speed_hz = FPGA_SPI_SPEED_HZ,  // 10 MHz
    .spics_io_num = -1, //FPGA_SPI_CS,         // CS Pin
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
  #ifdef TEST_SPI
  gpio_set_level(FPGA_SPI_CS, 0);
  #else
  gpio_set_level(FPGA_SPI_CS, 1);
  #endif
  gpio_set_level(FPGA_SPI_CLK, 1);
  //wait minimum 200ns
  vTaskDelay(0.3 / portTICK_PERIOD_MS);
  gpio_set_level(FPGA_CRESET_B, 1);
  //wait minimum 1200ns -> clear internal configuration
  vTaskDelay(1.3 / portTICK_PERIOD_MS);
  
  //send 8 dummy clocks
  #ifdef TEST_SPI
  gpio_set_level(FPGA_SPI_CS, 1);
  #else
  gpio_set_level(FPGA_SPI_CS, 0);
  #endif
  
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
  
  #ifdef TEST_SPI
  gpio_set_level(FPGA_SPI_CS, 0);
  #else
  gpio_set_level(FPGA_SPI_CS, 1);
  #endif
  while (sent < fileSize)
  {
    size_t length = (sent + 64 < fileSize) ? 64 : fileSize - sent;
    spi_transaction_t transactionProgram ={
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
  #ifdef TEST_SPI
  gpio_set_level(FPGA_SPI_CS, 1);
  #else
  gpio_set_level(FPGA_SPI_CS, 0);
  #endif

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
  #ifdef DEBUG
  ESP_LOGI("PROGRAM_FPGA", "Program FPGA done!");
  #endif
}
//After programming reinit spi device
void postProgramFPGA()
{
  spi_device_interface_config_t device_conifg = {
    .command_bits = 0,
    .address_bits = 0,
    .dummy_bits = 0,
    .mode = 0,                  //SPI mode 0
    .clock_speed_hz = FPGA_SPI_SPEED_HZ, 
    .spics_io_num = FPGA_SPI_CS,         // CS Pin
  #ifdef TEST_SPI
    .flags = SPI_DEVICE_HALFDUPLEX  | SPI_DEVICE_3WIRE,
  #else
    .flags = SPI_DEVICE_HALFDUPLEX | SPI_DEVICE_3WIRE | SPI_DEVICE_POSITIVE_CS,
  #endif
    .queue_size = 1,
    .pre_cb = NULL,
    .post_cb = NULL,
  };
  spi_bus_add_device(SPI2_HOST, &device_conifg, &fpga);
}

// Read and write ports
inline uint8_t readInputsFPGA(uint8_t data)
{
  uint8_t toSend = 0x80 | (0x0f & data); 
  uint8_t recived;

  spi_transaction_t transaction = {
    .length = 2*8,
    .rxlength = 8,
    .tx_buffer = &toSend,
    .rx_buffer = &recived,
  };


  spi_device_transmit(fpga, &transaction);

  // ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  recived: " BYTE_TO_BINARY_PATTERN "",
  // BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(recived));

  return recived;
}
inline void writeOutputsFPGA(uint16_t data)
{
  uint8_t toSend[2] = {0};
  toSend[0] = 0x3f & (data >> 8) ;
  toSend[1] = 0x00ff & data;

  spi_transaction_t transaction = {
    .length = 2*8,
    .tx_buffer = toSend,
    .rx_buffer = nullptr,
  };

  spi_device_transmit(fpga, &transaction);

  // ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "",
  // BYTE_TO_BINARY(toSend[0]), BYTE_TO_BINARY(toSend[1]));
}


//-------Testing-------

// Etap 1 wysyła 14 bitów i ustawia o=z
void writeFPGAstage1(uint16_t data)
{
  uint8_t toSend[2] = {0};
  toSend[0] = 0x3f & (data>> 8) ;
  toSend[1] = 0x00ff & data;

  spi_transaction_t transaction = {
    .length = 2*8,
    .tx_buffer = toSend,
    .rx_buffer = nullptr,
  };

  spi_device_transmit(fpga, &transaction);

  ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN "",
  BYTE_TO_BINARY(toSend[0]), BYTE_TO_BINARY(toSend[1]));
}

// Etap 2 wysłanie bajtu i odebranie bajtu
// >1>0> 0>0> AB>AA>A9>A8 <B7<B6<B5<B4<B3<B2<B1<B0.
//  K A PORT
void writeFPGAstage2(uint8_t data)
{
  uint8_t toSend = 0x80 | (0x0f & data); 
  uint8_t recived;

  spi_transaction_t transaction = {
    .length = 2*8,
    .rxlength = 8,
    .tx_buffer = &toSend,
    .rx_buffer = &recived,
  };


  spi_device_transmit(fpga, &transaction);

  ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  recived: " BYTE_TO_BINARY_PATTERN "",
  BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(recived));
}

// Etap 3      
// data -> AD AC 
// >1>0>0>1>SD>SC>AD>AC <SA<C6<C5<C4<C3<C2<C1<C0
void writeFPGAstage3(uint8_t data, uint8_t i2c)
{
  uint8_t toSend = 0x90 | ((0x03 & i2c)<<2) | (0x03 & data); 
  uint8_t recived;

  spi_transaction_t transaction = {
    .length = 2*8,
    .rxlength = 8,
    .tx_buffer = &toSend,
    .rx_buffer = &recived,
  };


  spi_device_transmit(fpga, &transaction);


  ESP_LOGI("FPGA_SPI", "send: " BYTE_TO_BINARY_PATTERN "  recived: " BYTE_TO_BINARY_PATTERN "",
  BYTE_TO_BINARY(toSend), BYTE_TO_BINARY(recived));
}

//MSB
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


//Send to EEPROM
// void testFPGAi2c()
// {
  // START
  // Clk: 1 Data: falling
// }

void testFPGA()
{
  uint8_t dataToSend2 = 0x00;
    uint16_t buff = 0b0000000001000000;

  while(1)
  {
    // Test stage 1
    ESP_LOGI("TEST_FPGA", "Test stage 1");
    
    for(int i = 0; i < 16; i++)
    {
      if(buff != 0b0010000000000000)
        buff = buff << 1;  
      else
        buff = 0b0000000001000000;


      writeFPGAstage1(buff);

      vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    
    // Test stage 2
    writeFPGAstage1(0xffff);
    ESP_LOGI("TEST_FPGA", "Test stage 2");
    dataToSend2 = 0x00;
    for(uint8_t i = 0; i < 16; i++)
    {
      writeFPGAstage2(dataToSend2);
      dataToSend2 = dataToSend2 + 1;
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }


    // Test stage 3
    // Clk:  0b10 0001 0010 0100 1001 0111
    // Data: 0b10 1000 0111 1110 0011 1000
    //                       | <- tu chyba mozna zmienic 
    writeFPGAstage1(0xffff);
    dataToSend2 = 1;
    ESP_LOGI("TEST_FPGA", "Test stage 3");
    uint32_t i2cClk =  0b110101010101010101010101011;
    uint32_t i2cData = 0b110100000111111100001111001;
    for(uint8_t i = 0; i < 27; i++)
    {
      writeFPGAstage3(dataToSend2, ((i2cData & 0x01)<<1) | (i2cClk & 0x01));
      dataToSend2 = ~dataToSend2;
//      i2cData = ~(0x01 & i2cData) | 0xfe & i2cData;
      i2cClk = i2cClk>>1;
      i2cData = i2cData>>1;

      // vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}



/*---------------------------------------------*/
/*------------------Acess point----------------*/
/*---------------------------------------------*/

// #include "sites/indexSite.h"
// WebServer server(80);
// AsyncWebServer server(80);
// inline void initAP()
// {
//   IPAddress ip(192, 168, 4, 1);
//   IPAddress gate(192, 168, 4, 1);
//   IPAddress subnet(255, 255, 255, 0);
//   WiFi.softAPConfig(ip, gate, subnet);
//   WiFi.mode(WIFI_AP);
//   WiFi.softAP(AP_SSID, AP_PASS);
//   // WiFi.disconnect();
// }
// inline void initWebServer()
// {
//   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
//     request->send(SPIFFS, "/indexSite.html");
//     });

//   server.on("/getDevices", HTTP_POST, [](AsyncWebServerRequest *request){
//     String res = "{ \"boardsNumber\":\"" + String(boardsNumber) + "\", \"boards\":[";
//     for (uint8_t i = 0; i < boardsNumber + 1; i++)
//     {
//       res += "{";
//       res += "\"deviceInitTime\": \"" + String(inputs[i].deviceInitTime) + "\",";
//       res += "\"deviceType\": \"" + String(inputs[i].deviceType) + "\",";
//       res += "\"firmwareVersion\": \"" + String(inputs[i].firmwareVersion) + "\",";
//       res += "\"numberOfAnalogInputs\": \"" + String(inputs[i].numberOfAnalogInputs) + "\"";
//       res += "}";
//       if(i != boardsNumber)
//         res+=",";
//     }
//     res += "]}\n";

//     request->send_P(200, "application/json", res.c_str());
//   });

//   server.on("/getDevicesState", HTTP_POST, [](AsyncWebServerRequest *request){
//     String res = "{ \"boardsNumber\":\""+String(boardsNumber)+"\", \"errorUart\":\"" + String(errorsUart) + "\", \"boards\":[";
//     for (uint8_t i = 0; i < boardsNumber + 1; i++)
//     {
//       res += "{";
//       res += "\"lastDigitalInputUpdateTime\": \"" + String(inputs[i].lastDigitalInputUpdateTime) + "\",";
//       res += "\"digitalInputStates\": \"" + String(inputs[i].digitalInputStates) + "\",";
//       res += "\"digitalOutputStates\": \"" + String(inputs[i].digitalOutputStates) + "\",";
//       res += "\"aIUpdateTime0\": \"" + String(inputs[i].aIUpdateTime[0]) + "\",";
//       res += "\"aIValue0\": \"" + String(inputs[i].aIValue[0]) + "\",";
//       res += "\"aIUpdateTime1\": \"" + String(inputs[i].aIUpdateTime[1]) + "\",";
//       res += "\"aIValue1\": \"" + String(inputs[i].aIValue[1]) + "\",";
//       res += "\"aIUpdateTime2\": \"" + String(inputs[i].aIUpdateTime[2]) + "\",";
//       res += "\"aIValue2\": \"" + String(inputs[i].aIValue[2]) + "\",";
//       res += "\"aIUpdateTime3\": \"" + String(inputs[i].aIUpdateTime[3]) + "\",";
//       res += "\"aIValue3\": \"" + String(inputs[i].aIValue[3]) + "\"";
//       res += "}";
//       if(i != boardsNumber)
//         res+=",";
//     }
//     res += "]}\n";

//     request->send_P(200, "application/json", res.c_str());
//   });

//   server.begin();
// }
// void handleClientTask(void*arg)
// {
//   while (1)
//   {
//     server.handleClient();
//     vTaskDelay(10 / portTICK_PERIOD_MS);    
//   }
// }


//------LD Program---------
// uint32_t storedTime = 0;
// uint32_t storedTime1 = 0;
// void (*ladderDiagramProgram)(void* arg) = nullptr; 
// reinterpret_cast<void(*)()>(0x42020094);
// void saveLDProgram()
// {
//   ESP_LOGI("TASK_SAVE_LD", "Start programming");
//   Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH);
//   uint32_t storedTime = xTaskGetTickCount();
//   while(xTaskGetTickCount()-storedTime <= 3000)
//   {
//     if(int num = Serial.available() > 0)
//     {
//       ESP_LOGI("TASK_SAVE_LD", "Programming recived: %d bytes", num);
//       uint8_t *buffer = new uint8_t[num];
//       Serial.readBytes(buffer, num);
//       Update.write(buffer, num);
//       delete[] buffer;
//       storedTime = xTaskGetTickCount();
//     }
//   }
//   vTaskDelay(1000 / portTICK_PERIOD_MS);
//   Update.end(true);
//   ESP_LOGI("TASK_SAVE_LD", "End programming");
// }
// void saveLDProgram2()
// {
//   const esp_partition_t* partition;
//   partition = esp_partition_find_first(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, "app1");
//   if(partition==nullptr)
//   {
//     ESP_LOGI("TASK_SAVE_LD", "No partition");
//     return;
//   }//1107431200
//   ladderDiagramProgram = reinterpret_cast<void(*)()>(partition->address + 1107431200); //1108803732 + 0x42020094
//   // ESP_LOGI("TASK_SAVE_LD", "Function address: %d", fun);  //1107431200
//   // Update.begin()                                       //1107431200
//   Update.end();
//   // asm("MOVE r1, 1");
//   // partition->address
//   // ESP.partitionWrite(partition, );
// }
// #define ADDRESS_FUN 0x42030000
// #define ADDRESS_FUN 0x150000
// uint16_t dataToWrite[] = {0x95, 0x47, 0xA3, 0x02, 0xF5, 0x00, 0x83, 0x47, 0x45, 0x00, 0x85, 0x07, 0x23, 0x02, 0xF5, 0x00, 0x1C, 0x45, 0x89, 0x07, 0x1C, 0xC5, 0x82, 0x80};
// void test()
// {
//     esp_err_t err = spi_flash_write(ADDRESS_FUN, dataToWrite, 24);
//     if(err == ESP_OK)
//     {
//         ESP_LOGI("FLASH FUN FUN", "writted");
//         ladderDiagramProgram = reinterpret_cast<void(*)(void* arg)>(ADDRESS_FUN);
//     }
//     else
//         ESP_LOGI("FLASH FUN FUN", "ERROR!");
//     ESP_LOGI("FLASH FUN FUN", "error code: %s", esp_err_to_name(err));
// }


//---------USB------------
void usbTask(void* arg)
{
  uint8_t* buffer = new uint8_t[4];
  while (1)
  {
    if(Serial.available() > 0)
    {
      Serial.read(buffer, 4);

      if(buffer[0] == 0xff) // update software
      {
        uint32_t bytesToWrite = (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
        ESP_LOGI("Update", "Start update: %d", bytesToWrite); 
        if(!Update.begin(bytesToWrite, U_FLASH))
        {
            ESP_LOGI("Update", "No free space"); 
            Serial.flush();
            continue;
        }
        uint32_t remaing = bytesToWrite;
        uint32_t written = 0;
        uint8_t *bufferBytes = new uint8_t[4096];
        while(written < bytesToWrite)
        {
          uint32_t redBytes = Serial.readBytes(bufferBytes, 4096);
          if(redBytes)
          {
            Update.write(bufferBytes, redBytes);
            written += redBytes;
          }
          yield();
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        delete[] buffer;

        // while(!(Serial.available() > 0)) {}
        // uint32_t written = Update.writeStream(Serial);

        if(written == bytesToWrite)
        {
            ESP_LOGI("Update", "wrote %d", written);
        }
        else
        {
          ESP_LOGI("Update", "Error: wrote %d but should %d", written, bytesToWrite);
          Serial.flush();
          continue;
        }
        
        if(!Update.end())
        {
          ESP_LOGI("Update", "Error update.end()");
          Serial.flush();
          continue;
        }

        // esp_ota_set_boot_partition(esp_ota_get_next_update_partition(NULL));
        yield();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        yield();
        ESP_LOGI("Update", "ESP restart");
        ESP.restart();
      }
      // else if(buffer[0] == 0xfe) // update website
      // {
      //   uint32_t bytesToWrite = (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
      //   ESP_LOGI("Update", "Start update website: %d", bytesToWrite); 
      //   SPIFFS.remove("/indexSite.html");
      //   File website = SPIFFS.open("/indexSite.html", "rw", true);
      //   uint32_t remaing = bytesToWrite;
      //   uint32_t written = 0;
      //   uint8_t *bufferBytes = new uint8_t[4096];
      //   while(written < bytesToWrite)
      //   {
      //     uint32_t redBytes = Serial.readBytes(bufferBytes, 4096);
      //     if(redBytes)
      //     {
      //       website.write(bufferBytes, redBytes);
      //       written += redBytes;
      //     }
      //     yield();
      //     vTaskDelay(100 / portTICK_PERIOD_MS);
      //   }
      //   delete[] buffer;
      //   website.close();
      // }      
      else
          ESP_LOGI("USB", "Command unknown!");
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
}

void updateCallback(size_t a, size_t b)
{
    ESP_LOGI("Update", "Progress: %d / %d", a, b);
}




/*---------------------------------------------*/
/*------------Initialize controller------------*/
/*---------------------------------------------*/
void initController(void) {
//-----Configure USB Serial-----
  // czasem czeka na otwarcie portu
  Serial.setRxBufferSize(4096);
  Serial.begin(115200); 
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  #ifdef DEBUG
  Serial.setDebugOutput(true);
  static const char *TASK_TAG = "MAIN_TASK";
	ESP_LOGI(TASK_TAG, "---------MAIN-------- \n");
	ESP_LOGI(TASK_TAG, "portTick_PERIOD_MS %d\n", (int)portTICK_PERIOD_MS);
  #else
  Serial.setDebugOutput(false);
  #endif


//--------Configure UART--------
  uart_driver_delete(UART_BB);
  const uart_config_t uart_config = {
      .baud_rate = 3062500,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_APB,
  };
  // We won't use a buffer for sending data.
  uart_driver_install(UART_BB, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
  uart_param_config(UART_BB, &uart_config);
  uart_set_pin(UART_BB, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

//---------Configure SPI--------
  SPI.end();
  spi_bus_config_t spi_conifg={//Default MSB
    .mosi_io_num = FPGA_SPI_MOSI,
    .miso_io_num = FPGA_SPI_MISO,
    .sclk_io_num = FPGA_SPI_CLK,
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
  };
  spi_bus_initialize(SPI2_HOST, &spi_conifg, SPI_DMA_DISABLED);

//--------Configure GPIO--------
  gpio_pad_select_gpio(INPUT1_PIN);
  gpio_pad_select_gpio(INPUT2_PIN);
  gpio_pad_select_gpio(INPUT3_PIN);
  gpio_pad_select_gpio(INPUT4_PIN);
  gpio_pad_select_gpio(INPUT5_PIN);
  gpio_set_direction(INPUT1_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(INPUT2_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(INPUT3_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(INPUT4_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(INPUT5_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(INPUT1_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(INPUT2_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(INPUT3_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(INPUT4_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(INPUT5_PIN, GPIO_PULLUP_ONLY);
  #ifdef W1VC64R_BOARD
  gpio_pad_select_gpio(INPUT6_PIN);
  gpio_set_direction(INPUT6_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(INPUT6_PIN, GPIO_PULLUP_ONLY);
  #endif

  gpio_pad_select_gpio(OUTPUT1_PIN); //To trzeba dodać, bez tego dziala w pio ale arduino tego wymaga
  gpio_pad_select_gpio(OUTPUT2_PIN); 
  gpio_pad_select_gpio(OUTPUT3_PIN); 
  // gpio_pad_select_gpio(OUTPUT4_PIN); 
  gpio_set_direction(OUTPUT1_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(OUTPUT2_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(OUTPUT3_PIN, GPIO_MODE_OUTPUT);
  // gpio_set_direction(OUTPUT4_PIN, GPIO_MODE_OUTPUT);


  inputs[0].firmwareVersion = FIRMWARE_VERSION;


  xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES - 1, NULL);
  #ifdef DEBUG
  ESP_LOGI(TASK_TAG, "uart_rx_task created!");
  #endif

  Update.onProgress(updateCallback);
  xTaskCreate(usbTask, "usbTask", 4096*2, NULL, configMAX_PRIORITIES - 5, NULL);
  #ifdef DEBUG
	ESP_LOGI(TASK_TAG, "usbTask created!");
  #endif

	// initAP();
  initMemory();
  // initWebServer();

/*-------- Progam FPGA ---------*/
  #if defined(W1VC128R_BOARD) || defined(W1VC1616R_BOARD)
  programFPGA();
  postProgramFPGA();
  #endif
}


/*---------------------------------------------*/
/*-----------------Read inputs-----------------*/
/*---------------------------------------------*/
inline void readInputs()
{
  //clear
  inputs[0].digitalInputStates = 0;

  
  #ifdef W1VC128R_BOARD
    // SPI -> FPGA
    // Only if TOP board has 12 input ports
    // Read port B -> Read IN6-IN12 
    uint8_t inputsFPGA = readInputsFPGA(static_cast<uint8_t>(inputs[0].digitalOutputStates >> 12));
    inputs[0].digitalInputStates |= ((inputsFPGA & 0x7f) << 5);  //Piny są odwrócone względem komunikacji do fpga ale bym tak zostawił by było mniej obliczeń
  #else
    inputs[0].digitalInputStates |= (gpio_get_level(INPUT6_PIN) << 5);
  #endif
  
  // GPIO -> ESP32
  inputs[0].digitalInputStates |= gpio_get_level(INPUT1_PIN) |
                                (gpio_get_level(INPUT2_PIN) << 1) |
                                (gpio_get_level(INPUT3_PIN) << 2) |
                                (gpio_get_level(INPUT4_PIN) << 3) |
                                (gpio_get_level(INPUT5_PIN) << 4);
  // UART -> BB                                
  while(recivedAll == false) {}
}

/*---------------------------------------------*/
/*----------------Write outputs----------------*/
/*---------------------------------------------*/
inline void writeOutputs()
{
  // UART -> BB
  for (uint8_t i = 1; i < boardsNumber + 1; i++)
    SendDigitalOutputs(i, inputs[i].digitalOutputStates);

  // SPI -> FPGA
  #if defined(W1VC128R_BOARD) || defined(W1VC1616R_BOARD)
  writeOutputsFPGA((inputs[0].digitalOutputStates >> 4));
  // writeOutputsFPGA((inputs[0].digitalOutputStates << 2)); // Tymczasowo
  #endif

  // GPIO
  gpio_set_level(OUTPUT1_PIN, inputs[0].digitalOutputStates & 0x0001);
  gpio_set_level(OUTPUT2_PIN, inputs[0].digitalOutputStates & 0x0002);
  gpio_set_level(OUTPUT3_PIN, inputs[0].digitalOutputStates & 0x0004);
  gpio_set_level(OUTPUT4_PIN, inputs[0].digitalOutputStates & 0x0008);
}