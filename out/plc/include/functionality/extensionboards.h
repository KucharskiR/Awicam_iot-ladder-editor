/*---------------------------------------------*/
/*----------------UART Extension---------------*/
/*---------------------------------------------*/

// Data packet:
// byte0: RCAA AAAP 
// byte1: output_states / input_states / command
// byte2: output_states / input_states / 0?
// byte3: byte0 xor byte1 xor byte2
//
// R -> recived bit - extension module change it when recived data
// C -> command bit - if esp set to 1 then in byte1 is command
// A -> address bit - if address = 0x1f then initialize procedure occure
// P -> parity bit
//
// esp send outputs, extension module anwser with inputs
//
// More information in extension module repository


#pragma once
#include "../includes.h"
#include "../defines.h"
#include "status.h"

extern DigitalInputsStructure inputs[32];
extern volatile uint8_t boardsNumber;
extern ControllerStatus controllerStatus;

const uint8_t PARITY_EVEN_TABLE[128] = {
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
    };

static const int RX_BUF_SIZE = 1024;

//Start transmit to start recive equals ~17us
uint32_t lastSendTime = 0;
volatile bool recivedAll = true;
volatile uint32_t errorsUart = 0;

int sendInitCommand() 
{
  uint8_t bytes[4] = {63, 0, 0, 63};
  const int txBytes = uart_write_bytes(UART_BB, bytes, 4);
  return txBytes;
}

//deviceNumber 0-30
int sendIntroduceCommand(uint8_t deviceNumber) 
{
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

// @return Send bytes 
int sendDigitalOutputs(uint8_t deviceNumber, uint16_t outputs) 
{
  uint8_t bytes[4];
  bytes[0] = (deviceNumber << 1) | PARITY_EVEN_TABLE[deviceNumber];
  bytes[1] = (uint8_t) outputs;
  bytes[2] = (uint8_t)(outputs >> 8);
  bytes[3] = bytes[0] ^ bytes[1] ^ bytes[2];
  
  #ifdef DEBUG
  #ifndef NO_DEBUG_UART
  static const char *TASK_TAG = "SEND DO";
  ESP_LOGI(TASK_TAG, "byte[0]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(tmp[0]));
  ESP_LOGI(TASK_TAG, "byte[1]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(tmp[1]));
  ESP_LOGI(TASK_TAG, "byte[2]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(tmp[2]));
  ESP_LOGI(TASK_TAG, "byte[3]: " BYTE_TO_BINARY_PATTERN "\n",(uint8_t)BYTE_TO_BINARY(tmp[3]));
  #endif
  #endif
  const int txBytes = uart_write_bytes(UART_BB, (void*)bytes, 4);
  if(lastSendTime == 0)
    lastSendTime = xTaskGetTickCount();
  return txBytes;
}

// Send outputs for all extension modules
// @return Send bytes 
int sendDigitalOutputs()
{
  recivedAll = false;
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  timer_start(TIMER_GROUP_0, TIMER_0);

  // Set all data in one packet...
  uint8_t buf2send[4*31] = {0};
  for (int i = 0; i < boardsNumber; i++)
  {
    buf2send[i * 4] = ((i) << 1) | PARITY_EVEN_TABLE[i];
    buf2send[i * 4 + 1] =(uint8_t)inputs[i + 1].digitalOutputStates;
    buf2send[i * 4 + 2] =(uint8_t)(inputs[i + 1].digitalOutputStates >> 8);
    buf2send[i * 4 + 3] = buf2send[i * 4] ^ buf2send[i * 4 + 1]^buf2send[i * 4 + 2];
  }
  
  // ...send packet with data to all extension modules
  const int txBytes = uart_write_bytes(UART_BB, (void*)buf2send, 4 * boardsNumber);
  if(lastSendTime == 0)
    lastSendTime = xTaskGetTickCount();
  
  return txBytes;
}


int sendAnalogInputSetup(uint8_t deviceNumber, uint8_t numberOfAI)
{
  uint8_t bytes[4];
  uint8_t tempByte0 = (DATA_COMMAND_FLAG_MASK >> 1) | deviceNumber;
  bytes[0] = (tempByte0 << 1) | PARITY_EVEN_TABLE[tempByte0];
  bytes[1] = (numberOfAI << 4) | 0x02;
  bytes[2] = 0;
  bytes[3] = bytes[0] ^ bytes[1] ^ bytes[2];
  const int txBytes = uart_write_bytes(UART_BB, bytes, 4);
  return txBytes;
}
int sendAnalogRead(uint8_t deviceNumber, uint8_t analogInput)
{
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

// @return Send bytes 
int sendCommand(uint8_t deviceNumber, uint8_t command, uint8_t data = 0)
{
  uint8_t bytes[4];
  uint8_t tempByte0 = (DATA_COMMAND_FLAG_MASK >> 1) | deviceNumber;
  bytes[0] = (tempByte0 << 1) | PARITY_EVEN_TABLE[tempByte0];
  bytes[1] = command;
  bytes[2] = data;
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

static void IRAM_ATTR checkQuartet(const char* logName, uint8_t quartet[]) 
{
  uint8_t calculatedXOR = quartet[0] ^ quartet[1] ^ quartet[2];
  if(calculatedXOR != quartet[3]) {
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
    // uart_flush(UART_BB);
    // errorsUart |= 0x0001;
    controllerStatus.errorOccur(ERROR_FLAG_EXT_MODULE_INCORRECT_XOR);
    // #ifdef DEBUG
    // ESP_LOGI(logName, "XOR byte is not correct!");
    // ESP_LOGI(logName, "byte[0]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(quartet[0]));
    // ESP_LOGI(logName, "byte[1]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(quartet[1]));
    // ESP_LOGI(logName, "byte[2]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(quartet[2]));
    // ESP_LOGI(logName, "byte[3]: " BYTE_TO_BINARY_PATTERN "\n",(uint8_t)BYTE_TO_BINARY(quartet[3]));
    // #endif
    return;
  }

  uint8_t parityBitRecived = quartet[0] & 1;
  uint8_t calculatedParityBit = PARITY_EVEN_TABLE[quartet[0] >> 1];
  if(parityBitRecived != calculatedParityBit) {
    // uart_flush(UART_BB);
    // errorsUart |= 0x0001;
    controllerStatus.errorOccur(ERROR_FLAG_EXT_MODULE_INCORRECT_PARITY_BIT);
    #ifdef DEBUG
    // ESP_LOGI(logName, "Parity bit is not correct!");
    #endif
    return;
  }

  uint8_t address = (quartet[0] & ADDRESS_MASK) >> 1;
  if(address == INIT_ADDRESS) {
    //INIT
    boardsNumber = (quartet[1] >> 1);
    #ifdef DEBUG
    //ESP_LOGI(logName, "sizeof(struct DigitalInputsStructure): %d \n", sizeof(struct DigitalInputsStructure));
    //ESP_LOGI(logName, "sizeof(inputs): %d \n", sizeof(inputs));
    //ESP_LOGI(logName, "inputs: %d \n", inputs);
    ESP_LOGI(logName, "-----INIT-----");
    ESP_LOGI(logName, "Boards number: %d", boardsNumber);
    #endif
  } else {
    uint8_t responseBit = quartet[0] & REQUEST_RESPONSE_FLAG_MASK;
    if(responseBit) {
    #ifdef DEBUG
    // ESP_LOGI(logName, "Device address: %d", address);
    #endif
    uint8_t dataCommandBit = quartet[0] & DATA_COMMAND_FLAG_MASK;
    // address--;
    if(dataCommandBit) {
      //cmd
      uint8_t commandNumber = quartet[1] & 0x0F;
      switch(commandNumber) {
          case INTRODUCE:  // 0x01 introduce
            // ESP_LOGI(logName, "Test introduce address: %d", address);
            inputs[address + 1].deviceType = 0;// static_cast<uint8_t>( quartet[1] >> 4);
            inputs[address + 1].deviceInitTime = xTaskGetTickCount();
            inputs[address + 1].firmwareVersion = quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "Introduce, Time: %d", (int)inputs[address + 1].deviceInitTime);
            ESP_LOGI(logName, "Introduce, DeviceType: %d", inputs[address + 1].deviceType);
            ESP_LOGI(logName, "Introduce, FW Version: %d", inputs[address + 1].firmwareVersion);
            //Serial.println(inputs[address].deviceType);
            #endif
            break;
          case SETUP_ANALOG_INPUT:  // 0x02 setup analog inputs
            inputs[address+1].numberOfAnalogInputs = quartet[1] >> 4;
            #ifdef DEBUG
            ESP_LOGI(logName, "AI setup. Number of analof inputs: %d\n", inputs[address + 1].numberOfAnalogInputs);
            //Serial.println(inputs[address].numberOfAnalogInputs);
            #endif
            break;
          case ANALOG_READ_1: // 0x0B analog read 1
            inputs[address + 1].aIUpdateTime[0] = xTaskGetTickCount();
            inputs[address + 1].aIValue[0] = ((quartet[1] & 0xF0) << 4) + quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "AI 1 value: %d\n", inputs[address + 1].aIValue[0]);
            //Serial.println(inputs[address].aIValue[0]);
            #endif
            break;
          case ANALOG_READ_2: // 0x0C analog read 2
            inputs[address + 1].aIUpdateTime[1] = xTaskGetTickCount();
            inputs[address + 1].aIValue[1] = ((quartet[1] & 0xF0) << 4) + quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "AI 2 value: %d\n", inputs[address + 1].aIValue[1]);
            //Serial.println(inputs[address].aIValue[1]);
            #endif
            break;
          case ANALOG_READ_3: // 0x0D analog read 3
            inputs[address + 1].aIUpdateTime[2] = xTaskGetTickCount();
            inputs[address + 1].aIValue[2] = ((quartet[1] & 0xF0) << 4) + quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "AI 3 value: %d\n", inputs[address + 1].aIValue[2]);
            //Serial.println(inputs[address].aIValue[2]);
            #endif
            break;
          case ANALOG_READ_4: // 0x0E analog read 4
            inputs[address + 1].aIUpdateTime[3] = xTaskGetTickCount();
            inputs[address + 1].aIValue[3] = ((quartet[1] & 0xF0) << 4) + quartet[2];
            #ifdef DEBUG
            ESP_LOGI(logName, "AI 4 value: %d\n", inputs[address + 1].aIValue[3]);
            //Serial.println(inputs[address].aIValue[3]);
            #endif
            break;
        }
      } else {
        inputs[address + 1].lastDigitalInputUpdateTime = xTaskGetTickCount();
        inputs[address + 1].digitalInputStates = (quartet[2] << 8) + quartet[1];
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
      // errorsUart |= 0x0001;
      controllerStatus.errorOccur(ERROR_FLAG_EXT_MODULE_NO_RESPONSE_BIT);
      #ifdef DEBUG
      // ESP_LOGI(logName, "Received original message (no response bit set!)\n");
      #endif
    }
  }
}

// static void IRAM_ATTR rx_task(void *arg)
// {
// 	static const char *RX_TASK_TAG = "RX_TASK";
// 	esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
// 	uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE);
// 	uint8_t number = 0;
// 	ESP_LOGI(RX_TASK_TAG, "Welcome to RX Task");
//   // uart_flush
// 	while(1) {
// 		const int rxBytes = uart_read_bytes(UART_BB, data, RX_BUF_SIZE, 1 / portTICK_PERIOD_MS);
// 		if(rxBytes > 0) {
//       recivedAll = false;
//       errorsUart = errorsUart<<1;
//       #ifdef DEBUG
//       #ifndef NO_DEBUG_UART
// 			ESP_LOGI(RX_TASK_TAG, "\nReceived %d bytes", rxBytes);
//       ESP_LOGI(RX_TASK_TAG, "byte[0]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(data[0]));
//       ESP_LOGI(RX_TASK_TAG, "byte[1]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(data[1]));
//       ESP_LOGI(RX_TASK_TAG, "byte[2]: " BYTE_TO_BINARY_PATTERN "",  (uint8_t)BYTE_TO_BINARY(data[2]));
//       ESP_LOGI(RX_TASK_TAG, "byte[3]: " BYTE_TO_BINARY_PATTERN "\n",(uint8_t)BYTE_TO_BINARY(data[3]));
//       #endif
//       #endif
// 			for(int i = 0; i < rxBytes; i++) {
// 				bytes[number++] = data[i];
// 				if(number > 3) {
// 					number = 0;
//           checkQuartet(RX_TASK_TAG, bytes);
//           lastSendTime = 0;
//           if(errorsUart & 0x0001)
//           {
//             uart_flush(UART_BB);
//             break;
//           }
// 			  }
// 			}
// 		}
//     else
//     {
//       if(xTaskGetTickCount() - lastSendTime > SEND_RESPONSE_WAIT)
//       {
//         if(lastSendTime != 0)
//         {
//           errorsUart = errorsUart<<1;
//           errorsUart |= 0x0001;
//           #ifdef DEBUG
//           // ESP_LOGI(RX_TASK_TAG, "Sent data but no response!");
//           // uart_flush(UART_BB);
//           // lastSendTime = 0;
//           #endif
//           recivedAll = true;
//         }
//         else if(recivedAll==false)
//           recivedAll = true;
//       }
//     }
// 	}
// 	free(data);
// //    static const char *RX_TASK_TAG = "RX_TASK";
// //    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
// //    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
// //    while (1) {
// //        const int rxBytes = uart_read_bytes(UART_BB, data, RX_BUF_SIZE, 100 / portTICK_RATE_MS);
// //        if (rxBytes > 0) {
// //            data[rxBytes] = 0;
// //            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: %s'", rxBytes, data);
// //            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
// //        }
// //    }
// //    free(data);
// }


static bool IRAM_ATTR noUARTReceivedInterrupt(void* arg)
{
  if(recivedAll == false)
  {
    recivedAll = true;
    controllerStatus.errorFlags |= ERROR_FLAG_NO_EXTENSION_MODULES;
    // ESP_LOGE("EXT_MODULE", "No response in time");
  }

  timer_pause(TIMER_GROUP_0, TIMER_0);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
  return pdTRUE;
}

static void IRAM_ATTR rxExtensionMoudeInterrupt(void* arg)
{
  const char* TASK_TAG = "EXT_MODULE_INTR";
  uint16_t status = UART0.int_st.val;
  uint16_t rxBytes = UART0.status.rxfifo_cnt;

  if(status == UART_RXFIFO_FULL_INT_RAW)
  {
    UART0.int_clr.rxfifo_full = 1;

    controllerStatus.errorFlags &= ~ERROR_FLAG_NO_EXTENSION_MODULES; 
    uint8_t recivedQuartet[4];
    for(uint16_t i = 0; i < rxBytes; i++)
    {
      recivedQuartet[i % 4] = UART0.ahb_fifo.rw_byte;
      if(i % 4 == 3)
        checkQuartet(TASK_TAG, recivedQuartet);
    }
    recivedAll = true;
    // timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0);
    // timer_pause(TIMER_GROUP_0, TIMER_0);
  }
  else
  {
    UART0.int_clr.val = status;
  }
}

inline void initExtensionModules()
{
  const char* TASK_TAG = "EXT_MODULES_INIT";
  
  //Init timer
  timer_config_t timerConfig = {
    .alarm_en = TIMER_ALARM_EN,
    .counter_en = TIMER_PAUSE,
    .counter_dir = TIMER_COUNT_UP,
    .auto_reload = TIMER_AUTORELOAD_EN,
    .divider = 8
  };
  timer_init(TIMER_GROUP_0, TIMER_0, &timerConfig);          
  timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 0); 
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_0, 8000); //not sure value is good
  timer_enable_intr(TIMER_GROUP_0, TIMER_0);
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_0, noUARTReceivedInterrupt, NULL, ESP_INTR_FLAG_IRAM);
  ESP_LOGI(TASK_TAG, "Interrupt started");
  timer_start(TIMER_GROUP_0, TIMER_0);
  



  // recivedAll = false;
  for (uint8_t i = 0; i < 2; i++) {
    #if defined(DEBUG)
    ESP_LOGI(TASK_TAG, "Send init command, send %d bytes", sendInitCommand());
    #else
    SendInitCommand();
    #endif
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    if(boardsNumber != 0)
      break;
  }

  // while(recivedAll == false) { }
  // recivedAll = false;

  if(boardsNumber == 0) {
    ESP_LOGI(TASK_TAG, "No extension modules connected");
    return;
  }

  for(int i = 0; i < boardsNumber; i++) {
    sendIntroduceCommand(i);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  sendDigitalOutputs();


  // ESP_LOGI(TASK_TAG, "Initialized extension modules, number: %d \n", boardsNumber);
}