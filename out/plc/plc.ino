// AWICAM Ladder Editor (0.0.2)
//
// Copyright (C) 2021  Leonardo Fernandes
//
// https://github.com/leofds/iot-ladder-editor
//
// Project: blink
#include "include/controller.h"


// Device 
#define W1VC_128R_BOARD

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

void refreshTime64bit(){
  unsigned long now = millis();
  if(now < LD_TIME.p[0]){
    LD_TIME.p[1]++;
  }
  LD_TIME.p[0] = now;
}

void rung001(void){
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

void init(){
  LD_TIME.v = 0;
  refreshTime64bit();
}
void setup()
{
  initController();

  init();
  initContext();
  while(1) 
  {
    readInputs();
    refreshTime64bit();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    rung001();
    writeOutputs();
  }
}
void loop() {
}

