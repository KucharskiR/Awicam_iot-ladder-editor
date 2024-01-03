#include "includes.h"
#include "defines.h"
#include "functionality/multiio.h"
#include "functionality/extensionboards.h"
#include "functionality/filesystem.h"
#include "functionality/fpga.h"
#include "functionality/usb.h"
#include "functionality/webserver.h"
#include "functionality/status.h"

#include "ladder2pin.h"

// Initialize global variables
volatile uint8_t boardsNumber = 0;
DigitalInputsStructure inputs[32] = {0};
ControllerStatus controllerStatus;

/*---------------------------------------------*/
/*------------Initialize controller------------*/
/*---------------------------------------------*/
void initController(void) 
{
  inputs[0].firmwareVersion = FIRMWARE_VERSION;
  
/*------- Configure USB -------*/
 initUSB();
  
  
  vTaskDelay(1000 / portTICK_PERIOD_MS);
  #ifdef DEBUG
  static const char *TASK_TAG = "MAIN_TASK";
	ESP_LOGI(TASK_TAG, "---------MAIN-------- \n");
	ESP_LOGI(TASK_TAG, "portTick_PERIOD_MS %d\n", (int)portTICK_PERIOD_MS);
  #else
  Serial.setDebugOutput(false);
  #endif


  #ifdef DEBUG
	ESP_LOGI(TASK_TAG, "usbTask created!");
  #endif


/*----- Configure Extensions UART ------*/
  ESP_ERROR_CHECK(uart_driver_delete(UART_BB));
  const uart_config_t uart_config = {
      .baud_rate = 3062500,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .source_clk = UART_SCLK_APB,
  };
  // We won't use a buffer for sending data.
  ESP_ERROR_CHECK(uart_param_config(UART_BB, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_BB, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_driver_install(UART_BB, RX_BUF_SIZE * 2, 0, 0, NULL, 0));

  ESP_ERROR_CHECK(uart_isr_free(UART_BB)); //clear default interupts
  ESP_ERROR_CHECK(uart_isr_register(UART_BB, rxExtensionMoudeInterrupt, NULL, ESP_INTR_FLAG_IRAM, NULL));
  ESP_ERROR_CHECK(uart_enable_rx_intr(UART_BB));

  uart_intr_config_t uartIntrConfig = {
    .intr_enable_mask = UART_RXFIFO_TOUT_INT_ENA_M | UART_RXFIFO_FULL_INT_ENA_M,
    // .rx_timeout_thresh = 30,
    .rxfifo_full_thresh = 4 // chwilowo na sztywno
  };
  ESP_ERROR_CHECK(uart_intr_config(UART_BB, &uartIntrConfig));
  

/*------ Configure GPIO -------*/
  gpio_pad_select_gpio(INPUT1_PIN);
  gpio_pad_select_gpio(INPUT2_PIN);
  gpio_pad_select_gpio(INPUT3_PIN);
  gpio_pad_select_gpio(INPUT4_PIN);
  gpio_pad_select_gpio(INPUT5_PIN);
  gpio_set_direction(INPUT1_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(INPUT2_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(INPUT3_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(INPUT4_PIN, GPIO_MODE_INPUT);
  gpio_set_direction(INPUT5_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(INPUT1_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(INPUT2_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(INPUT3_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(INPUT4_PIN, GPIO_PULLUP_ONLY);
  gpio_set_pull_mode(INPUT5_PIN, GPIO_PULLUP_ONLY);
  #ifdef W1VC64R_BOARD
  gpio_pad_select_gpio(INPUT6_PIN);
  gpio_set_direction(INPUT6_PIN, GPIO_MODE_INPUT);
  gpio_set_pull_mode(INPUT6_PIN, GPIO_PULLUP_ONLY);
  #endif

  gpio_pad_select_gpio(OUTPUT1_PIN); //To trzeba dodać, bez tego dziala w pio ale arduino tego wymaga
  gpio_pad_select_gpio(OUTPUT2_PIN); 
  gpio_set_direction(OUTPUT1_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(OUTPUT2_PIN, GPIO_MODE_OUTPUT);
  
  // Ustawianie tych pinów następuje po wysłaniu danych SPI
  // gpio_pad_select_gpio(OUTPUT3_PIN); 
  // gpio_set_direction(OUTPUT3_PIN, GPIO_MODE_OUTPUT);
  // gpio_pad_select_gpio(OUTPUT4_PIN); 
  // gpio_set_direction(OUTPUT4_PIN, GPIO_MODE_OUTPUT);
  
/*------- Configure SPI -------*/
  // SPI.end();
  spi_bus_initialize(SPI2_HOST, &spi_conifg, SPI_DMA_DISABLED);

/*----------- SPIFFS -----------*/
  initMemoryESPIDF();


/*-------- Progam FPGA ---------*/
  #if defined(W1VC128R_BOARD) || defined(W1VC1616R_BOARD)
  programFPGA();
  #endif

/*----- Init MultiIO ------*/
  initMultiIO();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  xTaskCreate(multiIOTask, "multiIOTask", 1024*2, NULL, configMAX_PRIORITIES - 4, NULL);

/*----- Check Lower Board ------*/
  // identifyLowerBoard();


/*--------- Connect BB ---------*/
  ESP_ERROR_CHECK(uart_flush(UART_BB));
  // xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES - 1, NULL);
  #ifdef DEBUG
  ESP_LOGI(TASK_TAG, "uart_rx_task created!");
  #endif
  initExtensionModules();
  vTaskDelay(100 / portTICK_PERIOD_MS);
  
  #ifdef FORCE_SEND_TO_MAX_EXTENSION_MODULES
  boardsNumber = 31; //For testing
  #endif
  
  if (boardsNumber != 0) {
    ESP_ERROR_CHECK(uart_disable_rx_intr(UART_BB));
    ESP_ERROR_CHECK(uart_isr_free(UART_BB)); //clear default interupts
    uartIntrConfig.rxfifo_full_thresh = 4 * boardsNumber; //interrupt only when all data recived
    ESP_ERROR_CHECK(uart_intr_config(UART_BB, &uartIntrConfig));
    ESP_ERROR_CHECK(uart_isr_register(UART_BB, rxExtensionMoudeInterrupt, NULL, ESP_INTR_FLAG_IRAM, NULL));
    ESP_ERROR_CHECK(uart_enable_rx_intr(UART_BB));
  }

/*-------------- AP ------------*/
  initAP();
  // initSTA();
  start_webserver();

  //Init temperature sensor
  temp_sensor_config_t tempSensorConfig {
    .dac_offset = TSENS_DAC_L2,
    .clk_div = 6
  };
  temp_sensor_set_config(tempSensorConfig);
  temp_sensor_start();

  //  receiveLDSave();
  
  controllerStatus.controlFlags = CONTROL_FLAG_RUN_LD_PROGRAMM;
  inputs[0].deviceInitTime = xTaskGetTickCount();
}


/*---------------------------------------------*/
/*-----------------Read inputs-----------------*/
/*---------------------------------------------*/
inline void readInputs()
{
  //clear
  inputs[0].digitalInputStates = 0;
  
  #if defined(W1VC128R_BOARD)
    // SPI -> FPGA
    // Only if TOP board has 12 input ports
    // Read port B -> Read IN6-IN12 
    
  //Set to spi
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);

  uint8_t inputsFPGA = readInputsFPGA(static_cast<uint8_t>(inputs[0].digitalOutputStates >> 12));

  //Set to drive gpio 
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);

  gpio_set_level(OUTPUT3_PIN, (~inputs[0].digitalOutputStates & 0x0004));
  gpio_set_level(OUTPUT4_PIN, (~inputs[0].digitalOutputStates & 0x0008));
  inputs[0].digitalInputStates |= ((inputsFPGA & 0x7f) << 5);  //Piny są odwrócone względem komunikacji do fpga ale bym tak zostawił by było mniej obliczeń
  /*|  //IN6
                                  (inputsFPGA << 6) |  //IN7
                                  (inputsFPGA << 7) |  //IN8
                                  (inputsFPGA << 8) |  //IN9
                                  (inputsFPGA << 9) |  //IN10
                                  (inputsFPGA << 10) |*/ //IN11
  //                                 (inputsFPGA << 11);  //IN12
  #elif defined(W1VC1616R_BOARD)
  #error "FPGA can't read 12 ports now"
  uint16_t inputsFPGA = readInputsFPGA(static_cast<uint8_t>(inputs[0].digitalOutputStates >> 12));
  inputs[0].digitalInputStates |= (inputsFPGA << 5) |  //IN6
                                  (inputsFPGA << 6) |  //IN7
                                  (inputsFPGA << 7) |  //IN8
                                  (inputsFPGA << 8) |  //IN9
                                  (inputsFPGA << 9) |  //IN10
                                  (inputsFPGA << 10) | //IN11
                                  (inputsFPGA << 11);  //IN12
                                  (inputsFPGA << 12);  //IN13
                                  (inputsFPGA << 13);  //IN14
                                  (inputsFPGA << 14);  //IN15
                                  (inputsFPGA << 15);  //IN16
  #elif defined(W1VC64R_BOARD)
    inputs[0].digitalInputStates |= (gpio_get_level(INPUT6_PIN) << 5);
  #endif
  
  // GPIO -> ESP32
  inputs[0].digitalInputStates |= gpio_get_level(INPUT1_PIN) |
                                (gpio_get_level(INPUT2_PIN) << 1) |
                                (gpio_get_level(INPUT3_PIN) << 2) |
                                (gpio_get_level(INPUT4_PIN) << 3) |
                                (gpio_get_level(INPUT5_PIN) << 4);
                                

  //Wait to recive anwser from extension boards
  // UART -> BB   
  // a gdyby tu dać process usb    
}

/*---------------------------------------------*/
/*----------------Write outputs----------------*/
/*---------------------------------------------*/
inline void writeOutputs()
{
  if(controllerStatus.controlFlags & CONTROL_FLAG_DISABLE_OUTPUT)
    return;

  if(boardsNumber > 0)
    sendDigitalOutputs();

  // SPI -> FPGA
  #if defined(W1VC128R_BOARD) || defined(W1VC1616R_BOARD)
  //Set to spi
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, FSPICLK_OUT_IDX, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, FSPICS0_OUT_IDX, 0x000000ff);

  writeOutputsFPGA((inputs[0].digitalOutputStates >> 4));
  
  //Set to drive gpio 
  REG_SET_BITS(GPIO_FUNC2_OUT_SEL_CFG_REG, 128, 0x000000ff);
  REG_SET_BITS(GPIO_FUNC8_OUT_SEL_CFG_REG, 128, 0x000000ff);
  #endif

  // GPIO
  gpio_set_level(OUTPUT1_PIN, inputs[0].digitalOutputStates & 0x0001);
  gpio_set_level(OUTPUT2_PIN, inputs[0].digitalOutputStates & 0x0002);
  gpio_set_level(OUTPUT3_PIN, (~inputs[0].digitalOutputStates & 0x0004));
  gpio_set_level(OUTPUT4_PIN, (~inputs[0].digitalOutputStates & 0x0008));
  
  // ESP_LOGI("out", "Outputs:" BYTE_TO_BINARY_PATTERN " " BYTE_TO_BINARY_PATTERN " ", BYTE_TO_BINARY(inputs[0].digitalOutputStates>>8), BYTE_TO_BINARY(inputs[0].digitalOutputStates));
  // xSemaphoreGive(inputsMutex);
}

/*---------------------------------------------*/
/*-------------Ladder Diagram Task-------------*/
/*---------------------------------------------*/

extern inline void ladderDiagramProgram();

void ladderDiagramTask(void* arg)
{
  recivedAll = true; // Add this to ladder generator
  TickType_t xLastWakeTime = xTaskGetTickCount(); // Add this to ladder generator
  TickType_t timeWait = 1 / portTICK_PERIOD_MS;

  while(1) {
    readInputs();
    
    usbJTAG();
    // yield();
    while(recivedAll == false && boardsNumber > 0) {}

    if(xTaskDelayUntil(&xLastWakeTime, timeWait) == pdFALSE) {
      xLastWakeTime = xTaskGetTickCount();
       timeWait = 2 / portTICK_PERIOD_MS;
      // recivedAll = true;
      // continue;
      controllerStatus.errorOccur(ERROR_FLAG_EXECUTION_TOO_LONG);
    }
    else
    {
      xLastWakeTime = xTaskGetTickCount();
       timeWait = 1 / portTICK_PERIOD_MS;
    }

    if(controllerStatus.controlFlags & CONTROL_FLAG_RUN_LD_PROGRAMM) {
      ladderDiagramProgram();
    }
    writeOutputs();
  }
}