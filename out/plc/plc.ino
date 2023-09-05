// AWICAM Ladder Editor (0.0.2)
//
// Copyright (C) 2021  Leonardo Fernandes
//
// https://github.com/leofds/iot-ladder-editor
//
// Project: blink

// Device 
#define W1VC_128R_BOARD
#include "include/controller.h"



// Outputs defines
Ladder2Pin LD_Q0_1(1, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_2(2, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_3(3, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_4(4, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_5(5, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_6(6, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_7(7, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q0_8(8, &inputs[0].digitalOutputStates);
Ladder2Pin LD_Q1_1(1, &inputs[1].digitalOutputStates);
Ladder2Pin LD_Q1_2(2, &inputs[1].digitalOutputStates);

// Inputs defines
Ladder2Pin LD_I0_1(1, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_2(2, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_3(3, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_4(4, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_5(5, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_6(6, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_7(7, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_8(8, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_9(9, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_10(10, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_11(11, &inputs[0].digitalInputStates);
Ladder2Pin LD_I0_12(12, &inputs[0].digitalInputStates);
Ladder2Pin LD_I1_1(1, &inputs[1].digitalInputStates);
Ladder2Pin LD_I1_2(2, &inputs[1].digitalInputStates);
Ladder2Pin LD_I1_3(3, &inputs[1].digitalInputStates);

// Timer struct
typedef struct {
  int32_t PRE;
  int32_t AC;
  int32_t B;
  int32_t DN;
  int32_t EN;
  uint64_t TT;
} LD_TIMER;

union {
  uint32_t p[2];
  uint64_t v;
} LD_TIME;

uint64_t getTime(){
  return LD_TIME.v;
}


LD_TIMER LD_T1;
LD_TIMER LD_T2;

void refreshTime64bit(){
  unsigned long now = millis();
  if(now < LD_TIME.p[0]){
    LD_TIME.p[1]++;
  }
  LD_TIME.p[0] = now;
}

void rung001(void){
  uint8_t _LD_S0;
  uint64_t _LD_T1;
  uint64_t _LD_T2;
  uint64_t _LD_T3;
  uint64_t _LD_T4;
  _LD_S0 = 1;
  LD_T1.EN = _LD_S0;
  if(!_LD_S0){
    LD_T1.DN = 0;
    LD_T1.AC = 0;
    LD_T1.TT =  getTime();
  }else{
    if(!LD_T1.DN){
      _LD_T1 =  getTime();
      _LD_T2 = _LD_T1 - LD_T1.TT;
      if(_LD_T2 >= LD_T1.B){
        LD_T1.TT = _LD_T1;
        LD_T1.AC = LD_T1.AC + 1;
        if(LD_T1.AC >= LD_T1.PRE){
          LD_T1.DN = 1;
        }
      }
    }
  }
  _LD_S0 = LD_T1.DN;
  LD_Q0_8 = _LD_S0;
  LD_T2.EN = _LD_S0;
  if(!_LD_S0){
    LD_T2.DN = 0;
    LD_T2.AC = 0;
    LD_T2.TT =  getTime();
  }else{
    if(!LD_T2.DN){
      _LD_T3 =  getTime();
      _LD_T4 = _LD_T3 - LD_T2.TT;
      if(_LD_T4 >= LD_T2.B){
        LD_T2.TT = _LD_T3;
        LD_T2.AC = LD_T2.AC + 1;
        if(LD_T2.AC >= LD_T2.PRE){
          LD_T2.DN = 1;
        }
      }
    }
  }
  _LD_S0 = LD_T2.DN;
  if(_LD_S0){
    LD_T1.DN = 0;
    LD_T1.AC = 0;
    LD_T1.EN = 0;
    LD_T1.TT =  getTime();
  }
}

void initContext(void){
  LD_T2.EN = 0;
  LD_T2.AC = 0;
  LD_T2.PRE = 1;
  LD_T2.B = 200;
  LD_T2.DN = 0;
  LD_T2.TT =  getTime();
  LD_T1.EN = 0;
  LD_T1.AC = 0;
  LD_T1.PRE = 1;
  LD_T1.B = 200;
  LD_T1.DN = 0;
  LD_T1.TT =  getTime();
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

