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
gpio_set_direction((gpio_num_t)LD_Q1, GPIO_MODE_OUTPUT);
gpio_set_direction((gpio_num_t)LD_Q2, GPIO_MODE_OUTPUT);
gpio_set_direction((gpio_num_t)LD_Q3, GPIO_MODE_OUTPUT);
gpio_set_direction((gpio_num_t)LD_Q4, GPIO_MODE_OUTPUT);
gpio_set_direction((gpio_num_t)LD_I1, GPIO_MODE_INPUT);
gpio_set_direction((gpio_num_t)LD_I2, GPIO_MODE_INPUT);
gpio_set_direction((gpio_num_t)LD_I3, GPIO_MODE_INPUT);
gpio_set_direction((gpio_num_t)LD_I4, GPIO_MODE_INPUT);
gpio_set_direction((gpio_num_t)LD_I5, GPIO_MODE_INPUT);

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

void init(){
  LD_TIME.v = 0;
  refreshTime64bit();
  pinMode(PIN_I0_8, INPUT);
  pinMode(PIN_Q0_3, OUTPUT);
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
void setup()
{
  // czasem czeka na otwarcie portu
  Serial.setRxBufferSize(4096);
  Serial.begin(115200); 
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  #ifdef DEBUG
  Serial.setDebugOutput(true);
  static const char *TASK_TAG = "MAIN_TASK";
	ESP_LOGI(TASK_TAG, "---------MAIN-------- \n");
	ESP_LOGI(TASK_TAG, "portTick_PERIOD_MS %d\n", (int)portTICK_PERIOD_MS);
  #else
  Serial.setDebugOutput(false);
  #endif

	
  initController();  //init z controllera
  xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES - 1, NULL);
  #ifdef DEBUG
  ESP_LOGI(TASK_TAG, "uart_rx_task created!");
  #endif

  Update.onProgress(updateCallback);
  xTaskCreate(usbTask, "usbTask", 4096*2, NULL, configMAX_PRIORITIES - 5, NULL);
  #ifdef DEBUG
	ESP_LOGI(TASK_TAG, "usbTask created!");
  #endif

	// initAP();
  initMemory();
  // initWebServer();

/*-------- Progam FPGA ---------*/
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  programFPGA();
  #ifdef DEBUG
	ESP_LOGI(TASK_TAG, "Program FPGA done!");
  #endif


  vTaskDelay(1000 / portTICK_PERIOD_MS);



  while(1) 
  {
    readInputs();
    vTaskDelay(1 / portTICK_PERIOD_MS);
    testLadderDiagramProgram();
    writeOutputs();
  
    // for(int i = 1; i < boardsNumber + 1; i++) {
    // SendDigitalOutputs(i, 0xFFFF);
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    // SendDigitalOutputs(i, 0x0000);
    // vTaskDelay(500 / portTICK_PERIOD_MS);
    // }
  }
}
void loop() {
}

