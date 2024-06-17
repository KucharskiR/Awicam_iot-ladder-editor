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
#include "functionality/usb_commands.h"

extern DigitalInputsStructure inputs[32];
extern ControllerStatus controllerStatus;
extern volatile uint8_t boardsNumber;
extern uint8_t genCRC(const uint8_t * buf, uint32_t len);

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
// inline void receiveLDSave()
// {
//   #ifdef DEBUG
//   const char* TASK_TAG = "LD_RECEIVER";
//   ESP_LOGI(TASK_TAG, "Wait for ld save");
//   #endif
//   TickType_t xLastWakeTime = xTaskGetTickCount() + 10000 / portTICK_PERIOD_MS; 
//   uint8_t buffer[64];
//   uint8_t byteCount = 0;
//   bool isReceivingLD = false;
//   bool isReceived = false;
//   //Wait for command
//   while (xLastWakeTime > xTaskGetTickCount())
//   {
//     byteCount = 0;
//     while(USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail == 1) {
//       buffer[byteCount] = USB_SERIAL_JTAG.ep1.rdwr_byte;
//       byteCount++;
//     }
//     if(byteCount == 0)
//       continue;
//     if (buffer[0] == USB_COMMAND_WRITE_LD) {
//       openLDFileEspidf("wb");
//       if(fileLD == NULL) {
//         USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
//         USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_ERROR;
//         USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
//         USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
//         isReceivingLD = false;
//         ESP_LOGE("USB", "Can't open ld file!");
//         break;
//       } else {
//         USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
//         USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_OK;
//         USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
//         USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
//         isReceivingLD = true;
//         break;
//       }
//     }
//   }
//   if(!isReceivingLD) {
//     ESP_LOGW(TASK_TAG, "LD save NOT received");
//     return;
//   }
//   //Has 60s to receive all data
//   xLastWakeTime = xTaskGetTickCount() + 60000 / portTICK_PERIOD_MS; 
//   while (xLastWakeTime > xTaskGetTickCount()) {
//     byteCount = 0;
//     while(USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail == 1) { //if data rx available
//       buffer[byteCount] = USB_SERIAL_JTAG.ep1.rdwr_byte;
//       byteCount++;
//     }
//     if (byteCount == 0)
//       continue;
//     if (byteCount > 2) {
//       if (buffer[0] == USB_COMMAND_WRITE_LD_END &&
//           buffer[1] == 0xfe &&
//           buffer[2] == USB_COMMAND_WRITE_LD_END) {
//         closeLDFileEspidf();
//         USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
//         USB_SERIAL_JTAG.ep1.rdwr_byte = USB_ESP_OK;
//         USB_SERIAL_JTAG.ep1.rdwr_byte = 0xff;
//         USB_SERIAL_JTAG.ep1_conf.wr_done = 1;  
//         isReceived = true;
//         // ESP_LOGI(TASK_TAG, "LD save received");
//         break;
//       }
//     }
//     writeToLDFile(buffer, byteCount);
//     // fwrite(buffer, sizeof(uint8_t), byteCount, fileLD);
//   }
//   if(!isReceived)
//     ESP_LOGW(TASK_TAG, "LD save NOT received");
// }

//temp
uint32_t partOffset = 0;
uint32_t ldPartFileSize = 0;

//Check len and crc
inline bool checkBytesCrcUSB(uint8_t* buffer, uint8_t  byteCount)
{
  
  if(genCRC(buffer, byteCount - 1) == buffer[byteCount - 1])
    return true;
  return false;
}

inline bool checkBytesLengthUSB(uint8_t* buffer, uint8_t  byteCount)
{
  if(byteCount < 3)
    return false;
  
  if(buffer[1] != byteCount)
    return false;

  return true;
}

inline void preparePacketUSB(uint8_t* received, uint8_t receivedBytes, uint8_t* toSend, uint8_t* toSendBytes)
{
  (*toSendBytes)+=3; // cmdbyte + lenbyte + crcbyte
  // buffer[0] = buffer[0]; // cmd
  toSend[0] = received[0];
  toSend[1] = *toSendBytes;

  toSend[(*toSendBytes)-1] = genCRC(toSend, *toSendBytes-1);
}

// Function use only to write code - copy and paste
inline uint8_t templat(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) {
  return 0;
}



inline uint8_t usbGetDeviceInfo(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  if(receivedBytes != 4) {
    toSendDataPtr[0] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }
  
  union Some {
      uint32_t data;
      uint8_t raw[4];
  } s;
  
  s.data = inputs[received[2]].deviceInitTime;


  toSendDataPtr[0] =  inputs[received[2]].deviceType;
  toSendDataPtr[1] =  inputs[received[2]].firmwareVersion;
  toSendDataPtr[2] =  inputs[received[2]].numberOfAnalogInputs;
  memcpy(&(toSendDataPtr[3]), s.raw, 4);

  // toSendDataPtr[6] =  inputs[received[2]].deviceInitTime & 0x000000ff;
  // toSendDataPtr[5] = (inputs[received[2]].deviceInitTime>>8) & 0x000000ff;
  // toSendDataPtr[4] = (inputs[received[2]].deviceInitTime>>16) & 0x000000ff;
  // toSendDataPtr[3] = (inputs[received[2]].deviceInitTime>>24) & 0x000000ff;
  *toSendBytes = 7;
  return 0;
}

inline uint8_t usbSetNeutralMode(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) 
{
  if(receivedBytes != 3) {
    toSendDataPtr[0] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  controllerStatus.controlFlags &= ~CONTROL_FLAG_RUN_LD_PROGRAMM;
  
  return 0;
}

inline uint8_t usbUnsetNeutralMode(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) 
{
  if(receivedBytes != 3) {
    toSendDataPtr[0] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  controllerStatus.controlFlags |= CONTROL_FLAG_RUN_LD_PROGRAMM;

  return 0;
}


inline uint8_t usbGetExtensionModulesCount(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) 
{
  if(receivedBytes != 3) {
    toSendDataPtr[0] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  toSendDataPtr[0] = boardsNumber;
  *toSendBytes = 1;

  return 0;
}

inline uint8_t usbGetDigitalOutputs(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) 
{
  if(receivedBytes != 4) {
    toSendDataPtr[0] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  uint8_t deviceId = received[2];
    if(deviceId > boardsNumber) {
    return USB_ERROR_COMMAND_ERROR;
  }

  toSendDataPtr[0] = static_cast<uint8_t>(inputs[deviceId].digitalOutputStates);
  toSendDataPtr[1] = static_cast<uint8_t>(inputs[deviceId].digitalOutputStates>>8);
  *toSendBytes = 2;

  return 0;
}

inline uint8_t usbGetDigitalInputs(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) 
{
  if(receivedBytes != 4) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  uint8_t deviceId = received[2];
  if(deviceId > boardsNumber) {
    return USB_ERROR_COMMAND_ERROR;
  }

  toSendDataPtr[0] = static_cast<uint8_t>(inputs[deviceId].digitalInputStates);
  toSendDataPtr[1] = static_cast<uint8_t>(inputs[deviceId].digitalInputStates>>8);
  *toSendBytes = 2;

  return 0;
}

inline uint8_t usbGetTemperature(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) 
{
  if(receivedBytes != 3) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  uint32_t tsensRaw = 0;
  if(esp_err_t err = temp_sensor_read_raw(&tsensRaw) != ESP_OK) {
    toSendDataPtr[1] = static_cast<uint8_t>(err);
    // toSendDataPtr[2] = static_cast<uint8_t>(err>>8);
    // toSendDataPtr[3] = static_cast<uint8_t>(err>>16);
    // toSendDataPtr[4] = static_cast<uint8_t>(err>>24);
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }
  
  toSendDataPtr[0] = static_cast<uint8_t>(tsensRaw);
  *toSendBytes = 1;
  return 0;
}

inline uint8_t usbGetDeviceTime(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) 
{
  if(receivedBytes != 3) {
    toSendDataPtr[0] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  union Some{
    int64_t data;
    uint8_t raw[8];
  } s;
  s.data = esp_timer_get_time();
  memcpy(&(toSendDataPtr[0]), s.raw, 8);
  
  // int64_t deviceTime = 

  // toSendDataPtr[0] = deviceTime;
  // toSendDataPtr[1] = deviceTime >> 8;
  // toSendDataPtr[2] = deviceTime >> 16;
  // toSendDataPtr[3] = deviceTime >> 24;
  // toSendDataPtr[4] = deviceTime >> 32;
  // toSendDataPtr[5] = deviceTime >> 40;
  // toSendDataPtr[6] = deviceTime >> 48;
  // toSendDataPtr[7] = deviceTime >> 56;

  *toSendBytes = 8;

  return 0;
}

inline uint8_t usbGetLastErrors(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes) 
{
  if(receivedBytes != 3) {
    toSendDataPtr[0] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  toSendDataPtr[0] = controllerStatus.errorCounter;
  toSendDataPtr[1] = controllerStatus.errorCounter >> 8;
  toSendDataPtr[2] = controllerStatus.errorCounter >> 16;
  toSendDataPtr[3] = controllerStatus.errorCounter >> 24;
  *toSendBytes = 4;
  // if(controllerStatus.errorCounter == 0)
  //   return 0;

  // if(controllerStatus.errorCounter > 4)
  // {
  //   toSendDataPtr[4] = 5;
  //   *toSendBytes++;

  //   for (uint32_t i = controllerStatus.errorCounter - 5; i < controllerStatus.errorCounter; i++)
  //   {
  //     //send time and error
  //    toSendDataPtr[4 + 8]  = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].time;
  //    toSendDataPtr[5]  = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].time >> 8;
  //    toSendDataPtr[6]  = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].time >> 16;
  //    toSendDataPtr[7]  = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].time >> 24;
  //    toSendDataPtr[8]  = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].code ;
  //    toSendDataPtr[9]  = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].code >> 8;;
  //    toSendDataPtr[10]  = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].code >> 16;
  //    toSendDataPtr[11]  = controllerStatus.errorHistory[i % MAX_STORED_ERRORS].code >> 24;
  //   *toSendBytes+=8;
     
  //   }
  //     USB_SERIAL_JTAG.ep1_conf.wr_done = 1;  
  // }
  // else
  // {
  //   USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorCounter;
  //   for (uint32_t i = 0; i < controllerStatus.errorCounter; i++)
  //   {
  //     USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].time;
  //     USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].time >> 8;
  //     USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].time >> 16;
  //     USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].time >> 24;
  //     USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].code ;
  //     USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].code >> 8;;
  //     USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].code >> 16;
  //     USB_SERIAL_JTAG.ep1.rdwr_byte = controllerStatus.errorHistory[i].code >> 24;
  //   }
  //     USB_SERIAL_JTAG.ep1_conf.wr_done = 1;  
  // }

  return 0;
}

//------------------
// Ladder Diagram
//------------------

inline uint8_t usbInitWriteLd(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  if(receivedBytes != 7) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  // if(fileLD != nullptr) {
  //   // closeLDFileEspidf();
  //   toSendDataPtr[1] = 0xaa;
  //   *toSendBytes = 1;
  //   if(ferror(fileLD)) {
  //     toSendDataPtr[2] = 0xee;
  //     *toSendBytes = 2;
  //   }
  //   if(feof(fileLD)) {
  //     toSendDataPtr[2] = 0xdd;
  //     *toSendBytes = 2;
  //   }
  //   return USB_ERROR_COMMAND_ERROR;
  // }

  openLDFileEspidf("wb");
  if(fileLD == nullptr) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_NULL_FILE;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  size_t ldFileSize = received[2] | (received[3]<<8) | (received[4]<<16) | (received[5]<<24);
  if(ldFileSize > getLDMaxFileSize()) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_FILE_TOO_BIG;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  *toSendBytes = 0;
  return 0;
}

inline uint8_t usbWriteLd(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  if(fileLD == nullptr) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_NULL_FILE;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  size_t wrote = writeToLDFile(&(received[2]), receivedBytes - 3);

  toSendDataPtr[0] = static_cast<uint8_t>(wrote);
  toSendDataPtr[1] = static_cast<uint8_t>(wrote>>8);
  toSendDataPtr[2] = static_cast<uint8_t>(wrote>>16);
  toSendDataPtr[3] = static_cast<uint8_t>(wrote>>24);

  *toSendBytes = 4;
  return 0;
}

inline uint8_t usbEndWriteLd(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  if(fileLD == nullptr) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_NULL_FILE;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }
  
  long wrote = getLDFileEspidfSize();
  closeLDFileEspidf();

  toSendDataPtr[0] = static_cast<uint8_t>(wrote);
  toSendDataPtr[1] = static_cast<uint8_t>(wrote>>8);
  toSendDataPtr[2] = static_cast<uint8_t>(wrote>>16);
  toSendDataPtr[3] = static_cast<uint8_t>(wrote>>24);

  *toSendBytes = 4;

  return 0;
}

inline uint8_t usbInitReadLd(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  if(receivedBytes != 3) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_BAD_LENGTH;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  openLDFileEspidf("rb");
  if(fileLD == nullptr) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_NULL_FILE;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  if(getLDFileEspidfSize() == -1)
  {
    toSendDataPtr[1] = USB_COMMAND_ERROR_NULL_FILE;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  long ldFileSize = getLDFileEspidfSize();
  toSendDataPtr[0] = static_cast<uint8_t>(ldFileSize);
  toSendDataPtr[1] = static_cast<uint8_t>(ldFileSize>>8);
  toSendDataPtr[2] = static_cast<uint8_t>(ldFileSize>>16);
  toSendDataPtr[3] = static_cast<uint8_t>(ldFileSize>>24);

  *toSendBytes = 4;
  return 0;
}

inline uint8_t usbReadLd(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  if(fileLD == nullptr) {
    toSendDataPtr[1] = USB_COMMAND_ERROR_NULL_FILE;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }
  size_t read = readFromLDFile(toSendDataPtr, 60);
  *toSendBytes = read;
  return 0;
}

inline uint8_t usbEndReadLd(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  closeLDFileEspidf();
  return 0;
}

//read partition test

inline uint8_t usbInitReadLdPartition(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  if(receivedBytes != 3) {
    toSendDataPtr[1] = 0xaa;
    *toSendBytes = 1;
    return USB_ERROR_COMMAND_ERROR;
  }

  const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_UNDEFINED, "ldpart");
  esp_err_t err = esp_partition_read_raw(partition, 0, &(toSendDataPtr[1]), 4);
  if(err != ESP_OK)
  {
    toSendDataPtr[1] =  err & 0x000000ff;
    toSendDataPtr[2] = (err>>8) & 0x000000ff;
    toSendDataPtr[3] = (err>>16) & 0x000000ff;
    toSendDataPtr[4] = (err>>24) & 0x000000ff;
    *toSendBytes = 4;
    return USB_ERROR_COMMAND_ERROR;
  }

  ldPartFileSize = (toSendBytes[4]<<24) | (toSendBytes[3]<<16) | (toSendBytes[2]<<8) | toSendBytes[1];
  partOffset = 4;
  
  toSendDataPtr[0] = USB_COMMAND_INIT_READ_LD;
  *toSendBytes = 5;
  return 0;
}

inline uint8_t usbReadLdPartition(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  if(partOffset > ldPartFileSize)
    return USB_ERROR_COMMAND_ERROR;
  uint32_t toread = (partOffset + 60 < ldPartFileSize) ? 60 : ldPartFileSize - partOffset;
  const esp_partition_t* partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_UNDEFINED, "ldpart");
  esp_err_t err = esp_partition_read_raw(partition, partOffset, toSendDataPtr, toread);
  *toSendBytes = toread;

  if(err != ESP_OK)
  {
    toSendDataPtr[1] =  err & 0x000000ff;
    toSendDataPtr[2] = (err>>8) & 0x000000ff;
    toSendDataPtr[3] = (err>>16) & 0x000000ff;
    toSendDataPtr[4] = (err>>24) & 0x000000ff;
    *toSendBytes = 4;
    return USB_ERROR_COMMAND_ERROR;
  }

  partOffset+=toread;
  return 0;
}

inline uint8_t usbEndReadLdPartition(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  toSendDataPtr[0] = USB_COMMAND_END_READ_LD;
  *toSendBytes = 1;

  return 0;
}


//------------------
//  Process packet
//------------------

//<0x80 | cmd> <len> <main error code> <error code> <crc>
// eg: bad crc
//<0x80 | cmd> <len> <error_code> <crc>
//
// eg: bad somthing in command
//<0x80 | cmd> <len> <error_code> <command_error> <crc>
// 
inline void generateError(uint8_t* toSendDataPtr, uint8_t *toSendBytes, uint8_t errorCode)
{
  toSendDataPtr[0] = errorCode;
  (*toSendBytes)++;
}

inline uint8_t processCommandUSB(uint8_t* received, uint8_t receivedBytes, uint8_t* toSendDataPtr, uint8_t *toSendBytes)
{
  switch(received[0])
  {
    case USB_COMMAND_ECHO:
      *toSendBytes = 0;
      for (size_t i = 0; i < receivedBytes - 3; i++)
      {
        (*toSendBytes)++;
        toSendDataPtr[i] = received[i+2];
      }
      break;
    case USB_COMMAND_GET_DEVICE_INFO:
      return usbGetDeviceInfo(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_SET_NEUTRAL_MODE:
      return usbSetNeutralMode(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_UNSET_NEUTRAL_MODE:
      return usbUnsetNeutralMode(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_GET_EXTENSION_MODULES_COUNT: 
      return usbGetExtensionModulesCount(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_GET_DIGITAL_OUTPUTS:
      return usbGetDigitalOutputs(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_GET_DIGITAL_INPUTS:
      return usbGetDigitalInputs(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_GET_DEVICE_TEMPERATURE:
      return usbGetTemperature(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_GET_DEVICE_TIME:
      return usbGetDeviceTime(received, receivedBytes, toSendDataPtr, toSendBytes);
    
    // case USB_COMMAND_GET_ERRORS:
      // return usbGetLastErrors(received, receivedBytes, toSendDataPtr, toSendBytes);

    case USB_COMMAND_INIT_WRITE_LD:
      return usbInitWriteLd(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_WRITE_LD:
      return usbWriteLd(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_END_WRITE_LD:
      return usbEndWriteLd(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_INIT_READ_LD:
      // return usbInitReadLdPartition(received, receivedBytes, toSendDataPtr, toSendBytes);
      return usbInitReadLd(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_READ_LD:
      // return usbReadLdPartition(received, receivedBytes, toSendDataPtr, toSendBytes);
      return usbReadLd(received, receivedBytes, toSendDataPtr, toSendBytes);
    case USB_COMMAND_END_READ_LD:
      // return usbEndReadLdPartition(received, receivedBytes, toSendDataPtr, toSendBytes);
      return usbEndReadLd(received, receivedBytes, toSendDataPtr, toSendBytes);
    default:
      return USB_ERROR_NO_COMMAND;
  }
  return 0;
}

#ifndef TEST_USB
inline void receiveBytesUSB(uint8_t* buffer, uint8_t* byteCount)
{
  while(USB_SERIAL_JTAG.ep1_conf.serial_out_ep_data_avail == 1) { //if data rx available
    buffer[*byteCount] = USB_SERIAL_JTAG.ep1.rdwr_byte;
    (*byteCount)++;
  }
}

inline void sendBytesUSB(uint8_t* buffer, uint8_t byteCount)
{
  for (uint8_t i = 0; i < byteCount; i++)
    USB_SERIAL_JTAG.ep1.rdwr_byte = buffer[i];
  USB_SERIAL_JTAG.ep1_conf.wr_done = 1;
}
#else
extern void receiveBytesUSB(uint8_t* buffer, uint8_t*byteCount);
extern void sendBytesUSB(uint8_t* buffer, uint8_t byteCount);
#endif

void usb()
{
  uint8_t receivedBytes = 0;
  uint8_t received[64]{0};
  uint8_t toSendBytes = 0;
  uint8_t toSend[64]{0};
  uint8_t* toSendDataPtr = &(toSend[2]);
  uint8_t err = 0;
  
  receiveBytesUSB(received, &receivedBytes);

  if(receivedBytes == 0)
    return;

  if(!checkBytesLengthUSB(received, receivedBytes))
  {
    generateError(toSendDataPtr, &toSendBytes, USB_ERROR_BAD_LENGTH);
    received[0] |= 0x80;
    goto skipToPreparePacket;
  }

  if(!checkBytesCrcUSB(received, receivedBytes))
  {
    generateError(toSendDataPtr, &toSendBytes, USB_ERROR_BAD_CRC);
    received[0] |= 0x80;
    goto skipToPreparePacket;
  }

  err = processCommandUSB(received, receivedBytes, toSendDataPtr, &toSendBytes);
  if(err > 0)
  {
    generateError(toSendDataPtr, &toSendBytes, err);
    received[0] |= 0x80;
    goto skipToPreparePacket;
  }

skipToPreparePacket:
  preparePacketUSB(received, receivedBytes, toSend, &toSendBytes);
  
  sendBytesUSB(toSend, toSendBytes);
}

  

void updateCallback(size_t a, size_t b)
{
    ESP_LOGI("Update", "Progress: %d / %d", a, b);
}


