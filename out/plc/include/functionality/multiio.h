/*---------------------------------------------*/
/*--------------------MultiIO------------------*/
/*---------------------------------------------*/
//Packet: <addresss> <comanddd> <datadata> <datadata>

#pragma once
#include "../includes.h"
#include "../defines.h"

extern DigitalInputsStructure inputs[32];

enum MultiIOStates {
  Recive,
  Send,
  RequestSend
};

uint8_t modeMultiIO = MultiIOStates::Recive;
uint8_t writeBuf[4] = {0};


inline void processMultiIOData(const char*TASK_TAG, uint8_t data[])
{
  if(data[0] == MULTI_IO_MY_ADDRESS) {
    switch (data[1])
    {
    case MULTI_IO_COMMAND_INTRODUCE:
        ESP_LOGI(TASK_TAG, "Lower board signature %d", data[3]);
        inputs[0].lowerBoardId = data[3];
      break;

    default:
        ESP_LOGW(TASK_TAG, "Command NOT exists!");
      break;
    }
  }
}

void multiIOTask(void* arg)
{
  static const char *TASK_TAG = "MULTIIO_TASK";

  uint8_t* readBuf = (uint8_t*)malloc(64);
  uint8_t processBuf[4];

  char* tmp;

  while(1) {
    if(modeMultiIO == MultiIOStates::Send) {
      modeMultiIO = MultiIOStates::Recive;

      //Change to sending
      ESP_ERROR_CHECK(uart_set_pin(MULTI_IO, MULTI_IO_PIN, -1, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
      const int sendBytes = uart_write_bytes(MULTI_IO, writeBuf, 4);
      if(sendBytes > 0)
        ESP_LOGI(TASK_TAG, "Sent %d bytes", sendBytes);
      else
        ESP_LOGE(TASK_TAG, "Send error!");

      ESP_ERROR_CHECK(uart_wait_tx_done(MULTI_IO, 1 / portTICK_PERIOD_MS));

      //Change to reciving
      ESP_ERROR_CHECK(uart_set_pin(MULTI_IO, -1, MULTI_IO_PIN,  UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
      uart_flush(MULTI_IO); // clen rx buffer after send
      // vTaskDelay(5 / portTICK_PERIOD_MS);
      
      // vTaskGetRunTimeStats(tmp);
      // ESP_LOGI("EM", "%s", tmp);
      // free(tmp);
    } else if(modeMultiIO == MultiIOStates::Recive || modeMultiIO == MultiIOStates::RequestSend) {
      const int readBytes = uart_read_bytes(MULTI_IO, readBuf, 64, 1 / portTICK_PERIOD_MS);
      if(readBytes > 0) {
        ESP_LOGI(TASK_TAG, "Read %d bytes", readBytes);

        ESP_LOGI(TASK_TAG, "First bytes read: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " " ,BYTE_TO_BINARY(readBuf[0]),BYTE_TO_BINARY(readBuf[1]));
        ESP_LOGI(TASK_TAG, "Last bytes read: " BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " ",BYTE_TO_BINARY(readBuf[2]),BYTE_TO_BINARY(readBuf[3]));

        if(readBytes != 4) {
          ESP_LOGI(TASK_TAG, "Red bytes is not correct");
          continue;
        }

        //process recived data
        processMultiIOData(TASK_TAG, readBuf);
      }
      else if(readBytes < 0 ) {
        ESP_LOGE("MULITIO_RX_TASK", "ERROR");
      }
      else {
        if(modeMultiIO == MultiIOStates::RequestSend)
          modeMultiIO = MultiIOStates::Send;
      }
    }
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }  
}

inline void writeMultiIO(uint32_t toSend)
{
  modeMultiIO = MultiIOStates::Send;
  writeBuf[0] = toSend;
  writeBuf[1] = toSend>>8;
  writeBuf[2] = toSend>>16;
  writeBuf[3] = toSend>>24;
  ESP_LOGI("SEND", "send");
}

inline void writeMultiIO(uint8_t address, uint8_t command, uint16_t data)
{
  modeMultiIO = MultiIOStates::RequestSend;
  writeBuf[0] = address;
  writeBuf[1] = command;
  writeBuf[2] = data;
  writeBuf[3] = data>>8;
}

inline void initMultiIO()
{
  ESP_ERROR_CHECK(uart_driver_delete(MULTI_IO));
  const uart_config_t uart_lower_board_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_APB,
  };
  ESP_ERROR_CHECK(uart_driver_install(MULTI_IO, 1024, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_param_config(MULTI_IO, &uart_lower_board_config));

  ESP_ERROR_CHECK(uart_set_pin(MULTI_IO, -1, MULTI_IO_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_LOGI("MultiIO", "Initialized");
}


void testMultiIOTask(void* arg)
{
  TickType_t lastTickCount = xTaskGetTickCount();
  uint16_t val = 0;
  
  while (1) {
   // uint8_t bb[4] = {0xaa,0xaa,0xaa,0xaa};
    uint32_t bb = 0xacacacad;
    writeMultiIO(MULTI_IO_LOWER_BOARD_ADDRESS, MULTI_IO_COMMAND_SET_DIGIT, val++);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

inline void runTestTasks()
{
  xTaskCreate(testMultiIOTask, "testMultiIOTask", 1024 * 2, NULL, configMAX_PRIORITIES - 5, NULL);
}