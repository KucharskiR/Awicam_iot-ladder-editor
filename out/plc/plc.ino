// AWICAM Ladder Editor (0.0.2)
//
// Copyright (C) 2021  Leonardo Fernandes
//
// https://github.com/leofds/iot-ladder-editor
//
// Project: null

// Device 
#define W1VC_64R_BOARD
#include "include/controller.h"



// Outputs defines
Ladder2Pin LD_Q0_1(1, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_2(2, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_3(3, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_4(4, &inputs[0].digitalOutputStates);

// Inputs defines
Ladder2Pin LD_I0_1(1, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_2(2, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_3(3, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_4(4, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_5(5, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_6(6, &inputs[0].digitalInputStates);

union {
  uint32_t p[2];
  uint64_t v;
} LD_TIME;

uint64_t getTime(){
  return LD_TIME.v;
}



void refreshTime64bit(){
  unsigned long now = millis();
  if(now < LD_TIME.p[0]){
    LD_TIME.p[1]++;
  }
  LD_TIME.p[0] = now;
}

void rung001(void){
  uint8_t _LD_S0;
  _LD_S0 = 1;
}

void initContext(void){
}

void init(){
  LD_TIME.v = 0;
  refreshTime64bit();
}
void ladderDiagramTask(void* arg)
{
  while(1) 
  {
    readInputs();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    refreshTime64bit();
    rung001();
    writeOutputs();
  }
}void setup()
{
  initController();

  init();
  initContext();

  xTaskCreate(ladderDiagramTask, "ladderDiagramTask", 2048, NULL, configMAX_PRIORITIES - 2, NULL);
}
void loop() {
}

