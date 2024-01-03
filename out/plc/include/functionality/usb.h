/*---------------------------------------------*/
/*----------------------USB--------------------*/
/*---------------------------------------------*/


#pragma once
#include "../includes.h"
#include "../defines.h"
#include "hal/usb_serial_jtag_ll.h"
#include "driver/usb_serial_jtag.h"
#include "soc/usb_serial_jtag_struct.h"
#include "filesystem.h"

// //temp
// #include "esp32-hal-adc.h"
// #include "soc/apb_saradc_reg.h"

extern DigitalInputsStructure inputs[32];
extern ControllerStatus controllerStatus;
extern inline void writeOutputs();
extern inline void readInputs();

uint8_t usbMode = USB_MODE_IDLE;
void initUSB()
{
  //Only testing purpose
  #ifdef ARDUINO_USB_CDC_ON_BOOT
  Serial.setRxBufferSize(4096);
  Serial.begin(115200); 
  Serial.setDebugOutput(true);
  #else

  //  usb_serial_jtag_driver_config_t con = {
  //   .tx_buffer_size = 256,
  //   .rx_buffer_size = 256,
  // };
  // ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&con));
  ESP_ERROR_CHECK(usb_serial_jtag_driver_uninstall());
  // REG_CLR_BIT(USB_SERIAL_JTAG_INT_ENA_REG, USB_SERIAL_JTAG_SERIAL_IN_EMPTY_INT_ENA);
  #endif
}

//Only once after in the end of controller initialization
//Adds delay up to 70s (10s to wait for command, 60s is max time of reciving data)
inline void receiveLDSave()
{
  #ifdef DEBUG
  const char* TASK_TAG = "LD_RECEIVER";
  ESP_LOGI(TASK_TAG, "Wait for ld save");
  #endif

  TickType_t xLastWakeTime = xTaskGetTickCount() + 10000 / portTICK_PERIOD_MS; 
  uint8_t buffer[64];
  uint8_t byteCount = 0;

  bool isReceivingLD = false;
  bool isReceived = false;

  //Wait for command
  while (xLastWakeTime > xTaskGetTickCount())
  {
    byteCount = 0;
    while(USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail == 1) {
      buffer[byteCount] = USB_SERIAL_JTAG.ep1.rdwr_byte;
      byteCount++;
    }
    if(byteCount == 0)
      continue;

    if (buffer[0] == USB_COMMAND_WRITE_LD) {
      openLDFileEspidf("wb");
      if(fileLD == NULL) {
        USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
        USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_ERROR;
        USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
        USB_SERIAL_JTAG.ep1_conf.wr_done = 1;

        isReceivingLD = false;
        ESP_LOGE("USB", "Can't open ld file!");
        break;
      } else {
        USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
        USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_OK;
        USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
        USB_SERIAL_JTAG.ep1_conf.wr_done = 1;

        isReceivingLD = true;
        break;
      }
    }
  }

  if(!isReceivingLD) {
    ESP_LOGW(TASK_TAG, "LD save NOT received");
    return;
  }
  
  //Has 60s to receive all data
  xLastWakeTime = xTaskGetTickCount() + 60000 / portTICK_PERIOD_MS; 
  while (xLastWakeTime > xTaskGetTickCount()) {
    byteCount = 0;
    while(USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail == 1) { //if data rx available
      buffer[byteCount] = USB_SERIAL_JTAG.ep1.rdwr_byte;
      byteCount++;
    }
    if (byteCount == 0)
      continue;
    
    if (byteCount > 2) {
      if (buffer[0] == USB_COMMAND_WRITE_LD_END &&
          buffer[1] == 0xfe &&
          buffer[2] == USB_COMMAND_WRITE_LD_END) {
        closeLDFileEspidf();
        USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
        USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_OK;
        USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
        USB_SERIAL_JTAG.ep1_conf.wr_done = 1;  

        isReceived = true;
        // ESP_LOGI(TASK_TAG, "LD save received");

        break;
      }
    }
    writeToLDFile(buffer, byteCount);
    // fwrite(buffer, sizeof(uint8_t), byteCount, fileLD);
  }
  
  if(!isReceived)
    ESP_LOGW(TASK_TAG, "LD save NOT received");
}

const uint8_t maxIterationNoResponse = 100; //~100ms
uint8_t iterationCount = 0;
uint8_t sendBuffer[64];


//Read usb data and send if needed
void  usbJTAG()
{
  const char* taskTag = "USB";
  uint8_t buffffer[64] = {0};
  uint8_t byteCount = 0;

  if(iterationCount > maxIterationNoResponse) {
    iterationCount = 0;
    usbMode = USB_MODE_IDLE;
    ESP_LOGI(taskTag, "Timed out, go to idle mode");
  }

  switch(usbMode)
  {
  case USB_MODE_IDLE:
    while(USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail == 1) { //if data rx available
      buffffer[byteCount] = USB_SERIAL_JTAG.ep1.rdwr_byte;
      byteCount++;
    }
    if(byteCount == 0)
      return;

    switch (buffffer[0])
    {
    case USB_COMMAND_WRITE_LD:
      usbMode = USB_MODE_BEFORE_RECEIVE_LD_SAVE;
      break;
    case USB_COMMAND_READ_LD:
      usbMode =  USB_MODE_BEFORE_SEND_LD_SAVE;
      break;

    case USB_COMMAND_SET_NEUTRAL_MODE:
      for (uint8_t i = 0; i < 32; i++)
        inputs[i].digitalOutputStates = 0;
      
      controllerStatus.controlFlags &= ~CONTROL_FLAG_RUN_LD_PROGRAMM;

      usbMode = USB_MODE_IDLE;
      break;
    case USB_COMMAND_UNSET_NEUTRAL_MODE:
      
      controllerStatus.controlFlags |= CONTROL_FLAG_RUN_LD_PROGRAMM;

      usbMode = USB_MODE_IDLE;
      break;
    case USB_COMMAND_GET_ERRORS:
      {
        USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorCounter;
        USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorCounter >> 8;
        USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorCounter >> 16;
        USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorCounter >> 24;
        if(controllerStatus.errorCounter == 0)
        {
          //send no error
          USB_SERIAL_JTAG.ep1_conf.wr_done = 1;  
          return;
        }

        if(controllerStatus.errorCounter > 4)
        {
          USB_SERIAL_JTAG.ep1.rdwr_byte = 5;
          for (uint32_t i = controllerStatus.errorCounter - 5; i < controllerStatus.errorCounter; i++)
          {
            //send time and error
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].time;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].time >> 8;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].time >> 16;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].time >> 24;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].code ;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].code >> 8;;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].code >> 16;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].code >> 24;
          }
            USB_SERIAL_JTAG.ep1_conf.wr_done = 1;  
        }
        else
        {
          USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorCounter;
          for (uint32_t i = 0; i < controllerStatus.errorCounter; i++)
          {
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].time;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].time >> 8;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].time >> 16;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].time >> 24;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].code ;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].code >> 8;;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].code >> 16;
            USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].code >> 24;
          }
            USB_SERIAL_JTAG.ep1_conf.wr_done = 1;  
        }
      }  
      break;
    case USB_COMMAND_GET_EXTENSION_MODULES_COUNT:
      USB_SERIAL_JTAG.ep1.rdwr_byte = boardsNumber;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
      break;
    case USB_COMMAND_GET_DEVICE_INFO:
      if(byteCount != 2)
        return; //Set error

      USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].deviceType;
      USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].firmwareVersion;
      USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].numberOfAnalogInputs;
      USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].deviceInitTime & 0x000000ff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = (inputs[buffffer[1]].deviceInitTime>>8) & 0x000000ff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = (inputs[buffffer[1]].deviceInitTime>>16) & 0x000000ff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = (inputs[buffffer[1]].deviceInitTime >> 24) & 0x000000ff;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
    break;
    case USB_COMMAND_GET_DEVICE_STATUS:

    break;
    case USB_COMMAND_GET_DIGITAL_OUTPUTS:
      if(byteCount != 2)
        return; //Set error

      if(buffffer[1] > boardsNumber) {
        //Error no board connected
      }

      USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].digitalOutputStates & 0x00ff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].digitalOutputStates >> 8;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
      break;
    case USB_COMMAND_GET_DIGITAL_INPUTS:
      if(byteCount != 2)
        return; //Set error

      if(buffffer[1] > boardsNumber) {
        //Error no board connected
      }

      USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].digitalInputStates & 0x00ff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].digitalInputStates >> 8;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
      break;
    case USB_COMMAND_GET_ANALOG_INPUTS:
      if(byteCount != 2)
        return; //Set error

      if(buffffer[1] > boardsNumber) {
        //Error no board connected
      }

      for (uint8_t analogId = 0; analogId < inputs[buffffer[1]].numberOfAnalogInputs; analogId++) {
        USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].aIValue[analogId] & 0x00ff;
        USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[buffffer[1]].aIValue[analogId] >> 8;
      }
      
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
    break;
    case USB_COMMAND_GET_DIGITAL_INPUTS_ALL:
    //idk why not work properly
    // while(USB_SERIAL_JTAG.ep1_conf.serial_in_ep_data_free == 0 && USB_SERIAL_JTAG.ep1_conf.wr_done == 1 && REG_GET_BIT(USB_SERIAL_JTAG_INT_RAW_REG, USB_SERIAL_JTAG_SERIAL_IN_EMPTY_INT_RAW) != 0)
    //   {;}
    //  ;
    // REG_CLR_BIT(USB_SERIAL_JTAG_INT_RAW_REG, USB_SERIAL_JTAG_SERIAL_IN_EMPTY_INT_RAW);
    // ESP_LOGI("wa", "Awd");
    // if(USB_SERIAL_JTAG.ep1_conf.wr_done == 1)
    // break;/* code */
      // for (uint8_t j = 0; j < 32; j++) {
      //   USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[j].digitalInputStates & 0x00ff;
      //   USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[j].digitalInputStates>>8;
      // }
      // USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
      // {
      // inputs[0].digitalOutputStates = ~inputs[0].digitalOutputStates;
      // for (uint8_t j = 0; j < 32; j++)
      // {
      //   sendBuffer[j*2] = inputs[j].digitalInputStates & 0x00ff;
      //   sendBuffer[j*2+1] = inputs[j].digitalInputStates>>8;
      // }
      // int ff = usb_serial_jtag_ll_write_txfifo(sendBuffer, 64);
      // if(ff != 64)
      //   inputs[0].digitalOutputStates = ~inputs[0].digitalOutputStates;
      // usb_serial_jtag_ll_txfifo_flush();
      // }
      break;
    case USB_COMMAND_GET_DIGITAL_OUTPUTS_ALL:
    //idk why not work properly
      for (uint8_t j = 0; j < 32; j++) {
        USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[j].digitalOutputStates;
        USB_SERIAL_JTAG.ep1.rdwr_byte = inputs[j].digitalOutputStates>>8;
      }
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
    break;
    case USB_COMMAND_ECHO:
      for (uint8_t j = 0; j < byteCount; j++)
        USB_SERIAL_JTAG.ep1.rdwr_byte = buffffer[j];
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
      
      break;
    case USB_COMMAND_GET_DEVICE_TEMPERATURE:
      {
        uint32_t tsensRaw = 0;
        temp_sensor_read_raw(&tsensRaw);
        USB_SERIAL_JTAG.ep1.rdwr_byte = tsensRaw;
        USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
      }
      break;
    case USB_COMMAND_GET_DEVICE_TIME:
      {
        int64_t deviceTime = esp_timer_get_time();
        // uint32_t deviceTime = xTaskGetTickCount();
        
        USB_SERIAL_JTAG.ep1.rdwr_byte = deviceTime;
        USB_SERIAL_JTAG.ep1.rdwr_byte = deviceTime >> 8;
        USB_SERIAL_JTAG.ep1.rdwr_byte = deviceTime >> 16;
        USB_SERIAL_JTAG.ep1.rdwr_byte = deviceTime >> 24;
        USB_SERIAL_JTAG.ep1.rdwr_byte = deviceTime >> 32;
        USB_SERIAL_JTAG.ep1.rdwr_byte = deviceTime >> 40;
        USB_SERIAL_JTAG.ep1.rdwr_byte = deviceTime >> 48;
        USB_SERIAL_JTAG.ep1.rdwr_byte = deviceTime >> 56;
        USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
      }
    break;
    default:
      ESP_LOGE(taskTag, "Received bad command");
      break;
    }
    break;
  case USB_MODE_BEFORE_RECEIVE_LD_SAVE:
    openLDFileEspidf("wb");
    if(fileLD == NULL) {
      usbMode = USB_MODE_IDLE;

      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_ERROR;
      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;

      ESP_LOGE(taskTag, "Can't open ld file!");
    } else {
      usbMode = USB_MODE_RECEIVE_LD_SAVE;

      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_OK; //albo USB_ESP_READY_TO_RECEIVE ?
      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
    }
    break;
  case USB_MODE_RECEIVE_LD_SAVE:
    iterationCount++;
    
    while(USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail == 1) { //if data rx available
      buffffer[byteCount] = USB_SERIAL_JTAG.ep1.rdwr_byte;
      byteCount++;
    }
    if(byteCount == 0)
      return;
    
    iterationCount = 0;

    if(byteCount > 2) {
      if (buffffer[0] == USB_COMMAND_WRITE_LD_END &&
          buffffer[1] == 0xfe &&
          buffffer[2] == USB_COMMAND_WRITE_LD_END) {
        closeLDFileEspidf();
        usbMode = USB_MODE_IDLE;
        USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
        USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_OK;
        USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
        USB_SERIAL_JTAG.ep1_conf.wr_done = 1;  
        // REG_SET_BIT(USB_SERIAL_JTAG_EP1_CONF_REG, USB_SERIAL_JTAG_WR_DONE);

        return;
      }
    }
    writeToLDFile(buffffer, byteCount);

    // USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
    // USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_READY_TO_RECEIVE_PACKET;
    // USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
    // // USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
    // REG_SET_BIT(USB_SERIAL_JTAG_EP1_CONF_REG, USB_SERIAL_JTAG_WR_DONE);

    break;
  case USB_MODE_BEFORE_SEND_LD_SAVE:
    usbMode = USB_MODE_SEND_LD_SAVE; 
    openLDFileEspidf("rb");
    if(fileLD == NULL) {
      usbMode = USB_MODE_IDLE;

      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_ERROR;
      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;

      ESP_LOGE(taskTag, "Can't open ld file!");
    } else {
      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_OK;
      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1; //to czy...
    // REG_SET_BIT(USB_SERIAL_JTAG_EP1_CONF_REG, USB_SERIAL_JTAG_WR_DONE); // ...to nie ma znaczenia
    }
    break;
  case USB_MODE_SEND_LD_SAVE:
    if(USB_SERIAL_JTAG.ep1_conf.wr_done == 1)
      break;
    
    size_t readFilesBytes = readFromLDFile(buffffer);
    if(readFilesBytes == 0) {
      closeLDFileEspidf();
      usbMode = USB_MODE_IDLE;

      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_OK;
      USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
      USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
      // REG_SET_BIT(USB_SERIAL_JTAG_EP1_CONF_REG, USB_SERIAL_JTAG_WR_DONE);
      // ESP_LOGI("USB", "LD file sent");
      return;
    
    }

    for (uint8_t j = 0; j < readFilesBytes; j++)
      USB_SERIAL_JTAG.ep1.rdwr_byte = buffffer[j];
    USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
    break;
  }
}

void updateCallback(size_t a, size_t b)
{
    ESP_LOGI("Update", "Progress: %d / %d", a, b);
}


