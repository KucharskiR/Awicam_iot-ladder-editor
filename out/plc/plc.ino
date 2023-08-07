// AWICAM Ladder Editor (0.0.2)
//
// Copyright (C) 2021  Leonardo Fernandes
//
// https://github.com/leofds/iot-ladder-editor
//
// Project: test
#include "include/main.h"


#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define PIN_Q0_1 2
#define PIN_Q0_2 4
#define PIN_Q0_3 12
#define PIN_Q0_4 13
#define PIN_Q0_5 33
#define PIN_Q0_6 25
#define PIN_Q0_7 26
#define PIN_Q0_8 27
#define PIN_I0_1 14
#define PIN_I0_2 16
#define PIN_I0_3 17
#define PIN_I0_4 18
#define PIN_I0_5 19
#define PIN_I0_6 21
#define PIN_I0_7 22
#define PIN_I0_8 23
#define PIN_I0_9 25
#define PIN_I0_10 26
#define PIN_I0_11 27
#define PIN_I0_12 28

union {
  uint32_t p[2];
  uint64_t v;
} LD_TIME;

uint64_t getTime(){
  return LD_TIME.v;
}

uint8_t LD_I0_8 = 0;
uint8_t LD_Q0_3 = 0;
uint8_t LD_Q0_1 = 0;
uint8_t LD_Q0_2 = 0;
uint8_t LD_Q0_4 = 0;
uint8_t LD_I0_1 = 0;
uint8_t LD_I0_2 = 0;
uint8_t LD_I0_3 = 0;
uint8_t LD_I0_4 = 0;
uint8_t LD_I0_5 = 0;


void refreshTime64bit(){
  unsigned long now = millis();
  if(now < LD_TIME.p[0]){
    LD_TIME.p[1]++;
  }
  LD_TIME.p[0] = now;
}

void readInputs(){
 inputs[0].digitalInputStates = gpio_get_level((gpio_num_t)LD_I0_1) |
							(gpio_get_level((gpio_num_t)LD_I0_2) << 1) | 
							(gpio_get_level((gpio_num_t)LD_I0_3) << 2) | 
							(gpio_get_level((gpio_num_t)LD_I0_4) << 3) | 
							(gpio_get_level((gpio_num_t)LD_I0_5) << 4);
while(recivedAll == false) {}
}

void writeOutputs(){
  for (uint8_t i = 1; i < boardsNumber + 1; i++)
    SendDigitalOutputs(i, inputs[i].digitalOutputStates);

  gpio_set_level((gpio_num_t)LD_Q0_1, inputs[0].digitalOutputStates & 0x0001);
  gpio_set_level((gpio_num_t)LD_Q0_2, inputs[0].digitalOutputStates & 0x0002);

}

void rung001(void){
  uint8_t _LD_S0;
  _LD_S0 = 1;
  if(!LD_I0_8){
    _LD_S0 = 0;
  }
  LD_Q0_3 = _LD_S0;
}

void initContext(void){
}



void TaskScan(void *pvParameters){
  for(;;){
    vTaskDelay(1);
    readInputs();
    refreshTime64bit();
    rung001();
    writeOutputs();
  }
}



void loop() {
}

