/*---------------------------------------------*/
/*------------------Acess point----------------*/
/*---------------------------------------------*/
// Korzystanie z web powoduje widoczne opóźnienia do ~1ms
// Wydaje się dobrze działać gdy stany są ustawiane co 2ms

#pragma once
#include "../includes.h"
#include "../defines.h"
#include "esp_wifi.h"
#include "esp_https_server.h"
#include "mdns.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;
const char* TAG = "WEB";

static void wifi_event_handler(void*arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
    //    esp_wifi_connect();
    // } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
    //     if (s_retry_num < 5) {
    //         esp_wifi_connect();
    //         s_retry_num++;
    //         // ESP_LOGI(TAG, "retry to connect to the AP");
    //     } else {
    //       ;
    //         // xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    //     }
    //     // ESP_LOGI(TAG,"connect to the AP fail");
    // } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    //     ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    //     // ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
    //     s_retry_num = 0;
    //     // xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    // }
  // ESP_LOGI("WIFI", "Event: %d", event_id);
  // if (event_id == WIFI_EVENT_AP_STACONNECTED) {
  //     wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
  //     // ESP_LOGI("WiFi", "station "MACSTR" join, AID=%d", MAC2STR(event->mac), event->aid);
  // } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
  //     wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
  //     // ESP_LOGI("WiFi", "station "MACSTR" leave, AID=%d", MAC2STR(event->mac), event->aid);
  // }
}

inline void initSTA()
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t* wifiAP = esp_netif_create_default_wifi_sta();

  // esp_netif_ip_info_t addr;
  // IP4_ADDR(&addr.ip, 192, 168, 4, 1);
  // IP4_ADDR(&addr.gw, 192, 168, 4, 1);
  // IP4_ADDR(&addr.netmask, 255, 255, 255, 0);
  // ESP_ERROR_CHECK(esp_netif_dhcpc_stop(wifiAP));
  // ESP_ERROR_CHECK(esp_netif_set_ip_info(wifiAP, &addr));
  // ESP_ERROR_CHECK(esp_netif_dhcpc_start(wifiAP));
  

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg)); 


  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));                                      

  wifi_config_t wifi_config = {
      .sta = {
        "Nie_dla_Ciebie_NET",
        "1dosto21",
        .threshold = {
          .authmode = WIFI_AUTH_WPA2_PSK
        }
      }
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());
  vTaskDelay(1000/portTICK_PERIOD_MS);
  //  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
  //           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
  //           pdFALSE,
  //           pdFALSE,
  //           portMAX_DELAY);

  //   /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
  //    * happened. */
  //   if (bits & WIFI_CONNECTED_BIT) {
  //       ESP_LOGI("TAG", "connected to ap ");
  //   } else if (bits & WIFI_FAIL_BIT) {
  //       ESP_LOGI("sda", "Failed to connec");
  //   } else {
  //       ESP_LOGE("TAG", "UNEXPECTED EVENT");
  //   }

  ESP_LOGE("WIFI_HANDLER", "wifi_init finished. SSID:%s password:%s channel:%d", AP_SSID, AP_PASS, 5);
}

inline void initAP()
{
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_t* wifiAP = esp_netif_create_default_wifi_ap();

  esp_netif_ip_info_t addr;
  IP4_ADDR(&addr.ip, 192, 168, 4, 1);
  // IP4_ADDR(&addr.gw, 192, 168, 4, 1);
  IP4_ADDR(&addr.netmask, 255, 255, 255, 0);
  // ESP_ERROR_CHECK(esp_netif_dhcpc_stop(wifiAP));
  // ESP_ERROR_CHECK(esp_netif_set_ip_info(wifiAP, &addr));
  // ESP_ERROR_CHECK(esp_netif_dhcpc_start(wifiAP));
  

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &wifi_event_handler,
                                                      NULL,
                                                      NULL));

  wifi_config_t wifi_config = {
      .ap = {
          AP_SSID, //.ssid = 
          AP_PASS, //.password =
          .ssid_len = strlen(AP_SSID),
          .channel = 5,
          .authmode = WIFI_AUTH_WPA2_PSK,
          .max_connection = 5,
      },
  };
  if (strlen(AP_PASS) == 0) {
      wifi_config.ap.authmode = WIFI_AUTH_OPEN;
  }

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI("WIFI_HANDLER", "wifi_init_softap finished. SSID:%s password:%s channel:%d", AP_SSID, AP_PASS, 5);

  //initialize mDNS service
  esp_err_t err = mdns_init();
  if (err) {
      ESP_LOGE("WIFI_HANDLER", "MDNS Init failed: %d\n", err);
      return;
  }

  //set hostname
  mdns_hostname_set("awicam-controller");
  //set default instance
  mdns_instance_name_set("AWICAM PLC");
}


char* indexData;
size_t indexDataLen;
esp_err_t getRootHandle(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_set_hdr(req, "Content-Encoding", "gzip");
  httpd_resp_send(req, indexData, indexDataLen);
  return ESP_OK;
}

esp_err_t getDevices(httpd_req_t *req)
{
  //Data package:
  // <boards 8>
  // [
  //   <deviceInitTime 4x8>
  //   <deviceType 8>
  //   <firmareVersion 8>
  //   <numberOfAnalogInputs 8>
  // ] x boardsNumber + 1, max 32

  char res[225];

  res[0] = boardsNumber;
  uint32_t resSize = 1;

  const uint32_t offset = 1;
  for (uint8_t i = 0; i < boardsNumber + 1; i++)
  {
    res[offset + (i * 7)] = (inputs[i].deviceInitTime);
    res[offset + (i * 7) + 1] = (inputs[i].deviceInitTime>>8);
    res[offset + (i * 7) + 2] = (inputs[i].deviceInitTime>>16);
    res[offset + (i * 7) + 3] = (inputs[i].deviceInitTime>>24);
    res[offset + (i * 7) + 4] = (inputs[i].deviceType); 
    res[offset + (i * 7) + 5] = (inputs[i].firmwareVersion);
    res[offset + (i * 7) + 6] = (inputs[i].numberOfAnalogInputs);
    resSize += 7;
  }
  httpd_resp_send(req, res, resSize);
  return ESP_OK;
}

esp_err_t getControllerStatus(httpd_req_t *req)
{
  //Data package:
  // <deviceTime 8x8>
  // <deviceTemp 8>
  // <deviceErrorsCount 4x8>

  char res[225];
  int64_t deviceTime = esp_timer_get_time();

  res[0] = deviceTime;
  res[1] = deviceTime >> 8;
  res[2] = deviceTime >> 16;
  res[3] = deviceTime >> 24;
  res[4] = deviceTime >> 32;
  res[5] = deviceTime >> 40;
  res[6] = deviceTime >> 48;
  res[7] = deviceTime >> 56;

  uint32_t tsensRaw = 0;
  temp_sensor_read_raw(&tsensRaw);

  res[8] = tsensRaw;

  res[9] = controllerStatus.errorCounter;
  res[10] = controllerStatus.errorCounter >> 8;
  res[11] = controllerStatus.errorCounter >> 16;
  res[12] = controllerStatus.errorCounter >> 24;

  uint32_t resSize = 13;

  httpd_resp_send(req, res, resSize);
  return ESP_OK;
}

esp_err_t getDevicesState(httpd_req_t* req)
{
  //Data package:
  // <boards 8>
  // [
  //   <lastDigitalUpdateTime 4x8>
  //   <inputs 2x8>
  //   <outputs 2x8>
  //   [
  //      <analogUpdateTime 4x8>
  //      <analogValue 2x8>
  //   ] x 4
  // ] x boardsNumber + 1, max 32

  uint32_t resSize = 5;
  char res[1300];

  res[0] = boardsNumber;
  res[1] = errorsUart;
  res[2] = errorsUart>>8;
  res[3] = errorsUart>>16;
  res[4] = errorsUart>>24;

  const uint32_t offset = 5;
  for (uint8_t i = 0; i < boardsNumber + 1; i++)
  {
    res[offset + i * 32] = inputs[i].lastDigitalInputUpdateTime;
    res[offset + (i * 32) + 1] = inputs[i].lastDigitalInputUpdateTime >> 8;
    res[offset + (i * 32) + 2] = inputs[i].lastDigitalInputUpdateTime >> 16;
    res[offset + (i * 32) + 3] = inputs[i].lastDigitalInputUpdateTime >> 24;

    res[offset + (i * 32) + 4] = inputs[i].digitalInputStates;
    res[offset + (i * 32) + 5] = inputs[i].digitalInputStates >> 8;
    
    res[offset + (i * 32) + 6] = inputs[i].digitalOutputStates;
    res[offset + (i * 32) + 7] = inputs[i].digitalOutputStates >> 8;
    
    res[offset + (i * 32) + 8] = inputs[i].aIUpdateTime[0];
    res[offset + (i * 32) + 9] = inputs[i].aIUpdateTime[0] >> 8;
    res[offset + (i * 32) + 10] = inputs[i].aIUpdateTime[0] >> 16;
    res[offset + (i * 32) + 11] = inputs[i].aIUpdateTime[0] >> 24;
    res[offset + (i * 32) + 12] = inputs[i].aIValue[0];
    res[offset + (i * 32) + 13] = inputs[i].aIValue[0] >> 8;
    res[offset + (i * 32) + 14] = inputs[i].aIUpdateTime[1];
    res[offset + (i * 32) + 15] = inputs[i].aIUpdateTime[1] >> 8;
    res[offset + (i * 32) + 16] = inputs[i].aIUpdateTime[1] >> 16;
    res[offset + (i * 32) + 17] = inputs[i].aIUpdateTime[1] >> 24;
    res[offset + (i * 32) + 18] = inputs[i].aIValue[1];
    res[offset + (i * 32) + 19] = inputs[i].aIValue[1] >> 8;
    res[offset + (i * 32) + 20] = inputs[i].aIUpdateTime[2];
    res[offset + (i * 32) + 21] = inputs[i].aIUpdateTime[2] >> 8;
    res[offset + (i * 32) + 22] = inputs[i].aIUpdateTime[2] >> 16;
    res[offset + (i * 32) + 23] = inputs[i].aIUpdateTime[2] >> 24;
    res[offset + (i * 32) + 24] = inputs[i].aIValue[2];
    res[offset + (i * 32) + 25] = inputs[i].aIValue[2] >> 8;
    res[offset + (i * 32) + 26] = inputs[i].aIUpdateTime[3];
    res[offset + (i * 32) + 27] = inputs[i].aIUpdateTime[3] >> 8;
    res[offset + (i * 32) + 28] = inputs[i].aIUpdateTime[3] >> 16;
    res[offset + (i * 32) + 29] = inputs[i].aIUpdateTime[3] >> 24;
    res[offset + (i * 32) + 30] = inputs[i].aIValue[3];
    res[offset + (i * 32) + 31] = inputs[i].aIValue[3] >> 8;

    resSize += 32;
  }
  
  httpd_resp_send(req, res, resSize);
  return ESP_OK;
}

esp_err_t getControllerState(httpd_req_t* req)
{
  //Data package:
  // <temperature 4x8>
  // [
  //   <lastDigitalUpdateTime 4x8>
  //   <inputs 2x8>
  //   <outputs 2x8>
  //   [
  //      <analogUpdateTime 4x8>
  //      <analogValue 2x8>
  //   ] x 4
  // ] x boardsNumber + 1, max 32
  
  uint32_t resSize = 4;
  char res[100];
  
  uint32_t tsensRaw = 0;
  temp_sensor_read_raw(&tsensRaw);

  uint32_t deviceTime = xTaskGetTickCount();
  
  res[0] = tsensRaw;

  res[1] = deviceTime;
  res[2] = deviceTime >> 8;
  res[3] = deviceTime >> 16;
  res[4] = deviceTime >> 24;

  httpd_resp_send(req, res, 5);

  return ESP_OK;
}

/* Function for starting the webserver */
httpd_handle_t start_webserver(void)
{
  FILE* file = fopen("/spiffs/indexSite.html.gz", "rb");
  if(file == NULL)
  {
    ESP_LOGE("WEB", "NO FILE");
  }
  indexData = new char[4096];
  
  indexDataLen = fread(indexData, sizeof(char), 4096, file);
  fclose(file);


  httpd_uri_t uriGetRoot = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = getRootHandle,
    .user_ctx = NULL
  };

  httpd_uri_t uriPostGetDevices = {
    .uri      = "/getDevices",
    .method   = HTTP_POST,
    .handler  = getDevices,
    .user_ctx = NULL
  };

   httpd_uri_t uriPostGetDevicesState = {
    .uri      = "/getDevicesState",
    .method   = HTTP_POST,
    .handler  = getDevicesState,
    .user_ctx = NULL
  };

  httpd_uri_t uriPostGetControllerStatus = {
    .uri      = "/getControllerStatus",
    .method   = HTTP_POST,
    .handler  = getControllerStatus,
    .user_ctx = NULL
  };


  /* Generate default configuration */
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  /* Empty handle to esp_http_server */
  httpd_handle_t server = NULL;

  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_register_uri_handler(server, &uriGetRoot);
    httpd_register_uri_handler(server, &uriPostGetDevices);
    httpd_register_uri_handler(server, &uriPostGetDevicesState);
    httpd_register_uri_handler(server, &uriPostGetControllerStatus);
  } else {
    ESP_LOGE("WEB", "Some thing is wrong");
  }
  /* If server failed to start, handle will be NULL */
  return server;
}



//---------------
//Testing purpose:
//----------------

void measureCharArray()
{
    char *res = new char[4096];
    char *resT = new char[20];
    sprintf(res,"{ \"boardsNumber\":%d\"\", \"boards\":[", 31);
    for (uint8_t i = 0; i < 31 + 1; i++)
    {
        strcat(res, "{\"deviceInitTime\": \"");
        itoa(56516, resT, 20);
        strcat(res, (const char*)resT);
        strcat(res,  "\",\"deviceType\": \"");
        itoa(1, resT, 20);
        strcat(res, resT);
        strcat(res, "\",\"firmwareVersion\": \"");
        itoa(1, resT, 20);
        strcat(res, resT);
        strcat(res, "\",\"numberOfAnalogInputs\": \"");
        itoa(4, resT, 20);
        strcat(res, resT);
        strcat(res, "\"}");
      if(i != 31)
        strcat(res, ",");
    }
    strcat(res, "]}\n");
    delete[] res;
    delete[] resT;
}

void measureTimeCharArray()
{
    const unsigned MEASUREMENTS = 5000;
    uint64_t start = esp_timer_get_time();

    for (int retries = 0; retries < MEASUREMENTS; retries++) {
        measureCharArray();
    }

    uint64_t end = esp_timer_get_time();
    String s = "iterations took " + String((end - start)/1000) + " ms";
    
    ESP_LOGI("awd","%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS, (end - start)/1000, (end - start)/MEASUREMENTS);
}

void measureCharArray2()
{
  char res[1300];
  res[0] = boardsNumber;
  res[1] = errorsUart;
  res[2] = errorsUart>>8;
  res[3] = errorsUart>>16;
  res[4] = errorsUart>>24;

  const uint32_t offset = 5;
  for (uint8_t i = 0; i < boardsNumber; i++)
  {
    res[offset + i * 32] = inputs[i].lastDigitalInputUpdateTime;
    res[offset + (i * 32) + 1] = inputs[i].lastDigitalInputUpdateTime >> 8;
    res[offset + (i * 32) + 2] = inputs[i].lastDigitalInputUpdateTime >> 16;
    res[offset + (i * 32) + 3] = inputs[i].lastDigitalInputUpdateTime >> 24;

    res[offset + (i * 32) + 4] = inputs[i].digitalInputStates;
    res[offset + (i * 32) + 5] = inputs[i].digitalInputStates >> 8;
    
    res[offset + (i * 32) + 6] = inputs[i].digitalOutputStates;
    res[offset + (i * 32) + 7] = inputs[i].digitalOutputStates >> 8;
    
    res[offset + (i * 32) + 8] = inputs[i].aIUpdateTime[0];
    res[offset + (i * 32) + 9] = inputs[i].aIUpdateTime[0] >> 8;
    res[offset + (i * 32) + 10] = inputs[i].aIUpdateTime[0] >> 16;
    res[offset + (i * 32) + 11] = inputs[i].aIUpdateTime[0] >> 24;
    res[offset + (i * 32) + 12] = inputs[i].aIValue[0];
    res[offset + (i * 32) + 13] = inputs[i].aIValue[0] >> 8;
    res[offset + (i * 32) + 14] = inputs[i].aIUpdateTime[1];
    res[offset + (i * 32) + 15] = inputs[i].aIUpdateTime[1] >> 8;
    res[offset + (i * 32) + 16] = inputs[i].aIUpdateTime[1] >> 16;
    res[offset + (i * 32) + 17] = inputs[i].aIUpdateTime[1] >> 24;
    res[offset + (i * 32) + 18] = inputs[i].aIValue[1];
    res[offset + (i * 32) + 19] = inputs[i].aIValue[1] >> 8;
    res[offset + (i * 32) + 20] = inputs[i].aIUpdateTime[2];
    res[offset + (i * 32) + 21] = inputs[i].aIUpdateTime[2] >> 8;
    res[offset + (i * 32) + 22] = inputs[i].aIUpdateTime[2] >> 16;
    res[offset + (i * 32) + 23] = inputs[i].aIUpdateTime[2] >> 24;
    res[offset + (i * 32) + 24] = inputs[i].aIValue[2];
    res[offset + (i * 32) + 25] = inputs[i].aIValue[2] >> 8;
    res[offset + (i * 32) + 26] = inputs[i].aIUpdateTime[3];
    res[offset + (i * 32) + 27] = inputs[i].aIUpdateTime[3] >> 8;
    res[offset + (i * 32) + 28] = inputs[i].aIUpdateTime[3] >> 16;
    res[offset + (i * 32) + 29] = inputs[i].aIUpdateTime[3] >> 24;
    res[offset + (i * 32) + 30] = inputs[i].aIValue[3];
    res[offset + (i * 32) + 31] = inputs[i].aIValue[3] >> 8;
  }
}

void measureTimeCharArray2()
{
    const unsigned MEASUREMENTS = 5000;
    uint64_t start = esp_timer_get_time();

    for (int retries = 0; retries < MEASUREMENTS; retries++) {
        measureCharArray2();
    }

    uint64_t end = esp_timer_get_time();
    String s = "iterations took " + String((end - start)/1000) + " ms";
    
    ESP_LOGI("awd","%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS, (end - start)/1000, (end - start)/MEASUREMENTS);
}

void measureString()
{
    String res = "{ \"boardsNumber\":\"" + String(31) + "\", \"boards\":[";
    for (uint8_t i = 0; i < 31 + 1; i++)
    {
      res += "{";
      res += "\"deviceInitTime\": \"" + String(56516) + "\",";
      res += "\"deviceType\": \"" + String(1) + "\",";
      res += "\"firmwareVersion\": \"" + String(1) + "\",";
      res += "\"numberOfAnalogInputs\": \"" + String(4) + "\"";
      res += "}";
      if(i != 31)
        res+=",";
    }
    res += "]}\n";
}

void measureTimeString()
{
    const unsigned MEASUREMENTS = 5000;
    uint64_t start = esp_timer_get_time();

    for (int retries = 0; retries < MEASUREMENTS; retries++) {
        measureString(); // This is the thing you need to measure
    }

    uint64_t end = esp_timer_get_time();
    String s = "iterations took " + String((end - start) / 1000) + " ms";
    
    ESP_LOGI("awd","%u iterations took %llu milliseconds (%llu microseconds per invocation)\n",
           MEASUREMENTS, (end - start)/1000, (end - start)/MEASUREMENTS);
}
