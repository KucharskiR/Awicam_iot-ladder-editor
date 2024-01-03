#pragma once
#include "../includes.h"
#include "../defines.h"

// Control flags

#define CONTROL_FLAG_RUN_LD_PROGRAMM 0x01
#define CONTROL_FLAG_DISABLE_OUTPUT 0x02

// How to store errors?
#define MAX_STORED_ERRORS 64
#define ERROR_FLAG_EXECUTION_TOO_LONG 0x01

#define ERROR_FLAG_NO_EXTENSION_MODULES 0x30
#define ERROR_FLAG_EXT_MODULE_INCORRECT_XOR 0x31
#define ERROR_FLAG_EXT_MODULE_INCORRECT_PARITY_BIT 0x32
#define ERROR_FLAG_EXT_MODULE_NO_RESPONSE_BIT 0x33


struct ControllerStatus {
  uint32_t controlFlags;
  uint32_t errorFlags;
  
  struct ErrorOccurS
  {
    int32_t time;
    uint32_t code;
  };

  ErrorOccurS errorHistory[MAX_STORED_ERRORS] = {0};
  uint32_t errorIterator = 0;
  uint32_t errorCounter = 0;
  void errorOccur(uint32_t errorCode)
  {
    if(errorIterator >= MAX_STORED_ERRORS)
      errorIterator = 0;

    errorHistory[errorIterator].time = esp_timer_get_time() >> 16;
    errorHistory[errorIterator].code = errorCode;

    errorIterator++;
    errorCounter++;
  }
};