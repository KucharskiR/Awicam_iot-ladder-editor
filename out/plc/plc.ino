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

#define LD_Q0_1(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0001)) | ((value & 0x01)))))
#define LD_Q0_2(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0002)) | ((value & 0x01) << 1))))
#define LD_Q0_3(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0004)) | ((value & 0x01) << 2))))
#define LD_Q0_4(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0008)) | ((value & 0x01) << 3))))
#define LD_Q0_5(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0010)) | ((value & 0x01) << 4))))
#define LD_Q0_6(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0020)) | ((value & 0x01) << 5))))
#define LD_Q0_7(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0040)) | ((value & 0x01) << 6))))
#define LD_Q0_8(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0080)) | ((value & 0x01) << 7))))
#define LD_Q0_9(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0100)) | ((value & 0x01) << 8))))
#define LD_Q0_10(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0200)) | ((value & 0x01) << 9))))
#define LD_Q0_11(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0400)) | ((value & 0x01) << 10))))
#define LD_Q0_12(value) ((inputs[0].digitalOutputStates = ((inputs[0].digitalOutputStates & ~(0x0800)) | ((value & 0x01) << 11))))

#define LD_I0_1 ((inputs[0].digitalInputStates & 0x0001))
#define LD_I0_2 (((inputs[0].digitalInputStates>>1) & 0x0001))
#define LD_I0_3 (((inputs[0].digitalInputStates>>2) & 0x0001))
#define LD_I0_4 (((inputs[0].digitalInputStates>>3) & 0x0001))
#define LD_I0_5 (((inputs[0].digitalInputStates>>4) & 0x0001))
#define LD_I0_6 (((inputs[0].digitalInputStates>>5) & 0x0001))
#define LD_I0_7 (((inputs[0].digitalInputStates>>6) & 0x0001))
#define LD_I0_8 (((inputs[0].digitalInputStates>>7) & 0x0001))
#define LD_I0_9 (((inputs[0].digitalInputStates>>8) & 0x0001))
#define LD_I0_10 (((inputs[0].digitalInputStates>>9) & 0x0001))
#define LD_I0_11 (((inputs[0].digitalInputStates>>10) & 0x0001))
#define LD_I0_12 (((inputs[0].digitalInputStates>>11) & 0x0001))

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

