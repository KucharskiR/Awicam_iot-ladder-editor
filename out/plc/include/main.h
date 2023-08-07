#include "include/controller.h"

/* Komunikacja 
inputs
  \/
drabinka
  \/
outputs
*/


// inline void readInputs()
// {
//   // SPI -> FPGA
  
//   // GPIO -> ESP32
//   inputs[0].digitalInputStates = gpio_get_level(INPUT1_PIN) |
//                                 (gpio_get_level(INPUT2_PIN) << 1) |
//                                 (gpio_get_level(INPUT3_PIN) << 2) |
//                                 (gpio_get_level(INPUT4_PIN) << 3) |
//                                 (gpio_get_level(INPUT5_PIN) << 4);// |
//                                 // (gpio_get_level(INPUT6_PIN) << 5);
//   // UART -> BB                                
//   while(recivedAll == false) {}
// }
// inline void writeOutputs()
// {
//   // UART -> BB
//   for (uint8_t i = 1; i < boardsNumber + 1; i++)
//     SendDigitalOutputs(i, inputs[i].digitalOutputStates);
//   // SPI -> FPGA

//   // GPIO
//   gpio_set_level(OUTPUT1_PIN, inputs[0].digitalOutputStates & 0x0001);
//   gpio_set_level(OUTPUT2_PIN, inputs[0].digitalOutputStates & 0x0002);
//   // gpio_set_level(OUTPUT3_PIN, inputs[0].digitalOutputStates & 0x0004);
//   // gpio_set_level(OUTPUT4_PIN, inputs[0].digitalOutputStates & 0x0008);
// }

//To działa tylko wtedy gdy zapewnianione zostaną 4 moduły rozszerzeń

void testLadderDiagramProgram()
{
  if(xTaskGetTickCount() - storedTime > 500)
  {
    inputs[1].digitalOutputStates = ~inputs[1].digitalOutputStates;
    storedTime = xTaskGetTickCount();
  }

  if(xTaskGetTickCount() - storedTime1 > 50)
  {
    inputs[2].digitalOutputStates = ~inputs[2].digitalOutputStates;
    storedTime1 = xTaskGetTickCount();
  }

  for (int i = 3; i < boardsNumber + 1; i++)
  {
    if(inputs[i].digitalInputStates & 0x0001)
    {
      if(i == boardsNumber)
        inputs[3].digitalOutputStates = 0xffff;
      else
        inputs[i + 1].digitalOutputStates = 0xffff;
    }
    else
    {
      if(i == boardsNumber)
        inputs[3].digitalOutputStates = 0x0000;
      else
        inputs[i + 1].digitalOutputStates = 0x0000;
    }  
  }
}

// void setup()
// {
//   // czasem czeka na otwarcie portu
//   Serial.setRxBufferSize(4096);
//   Serial.begin(115200); 
  
//   vTaskDelay(1000 / portTICK_PERIOD_MS);
//   #ifdef DEBUG
//   Serial.setDebugOutput(true);
//   static const char *TASK_TAG = "MAIN_TASK";
// 	ESP_LOGI(TASK_TAG, "---------MAIN-------- \n");
// 	ESP_LOGI(TASK_TAG, "portTick_PERIOD_MS %d\n", (int)portTICK_PERIOD_MS);
//   #else
//   Serial.setDebugOutput(false);
//   #endif

	
//   initController();  //init z controllera
//   xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES - 1, NULL);
//   #ifdef DEBUG
//   ESP_LOGI(TASK_TAG, "uart_rx_task created!");
//   #endif

//   Update.onProgress(updateCallback);
//   xTaskCreate(usbTask, "usbTask", 4096*2, NULL, configMAX_PRIORITIES - 5, NULL);
//   #ifdef DEBUG
// 	ESP_LOGI(TASK_TAG, "usbTask created!");
//   #endif

// 	// initAP();
//   initMemory();
//   // initWebServer();

// /*-------- Progam FPGA ---------*/
//   vTaskDelay(1000 / portTICK_PERIOD_MS);
//   programFPGA();
//   #ifdef DEBUG
// 	ESP_LOGI(TASK_TAG, "Program FPGA done!");
//   #endif


//   vTaskDelay(1000 / portTICK_PERIOD_MS);



//   while(1) 
//   {
//     readInputs();
//     vTaskDelay(1 / portTICK_PERIOD_MS);
//     testLadderDiagramProgram();
//     writeOutputs();
  
//     // for(int i = 1; i < boardsNumber + 1; i++) {
//     // SendDigitalOutputs(i, 0xFFFF);
//     // vTaskDelay(500 / portTICK_PERIOD_MS);
//     // SendDigitalOutputs(i, 0x0000);
//     // vTaskDelay(500 / portTICK_PERIOD_MS);
//     // }
//   }
// }

// void loop()
// {
   
// }