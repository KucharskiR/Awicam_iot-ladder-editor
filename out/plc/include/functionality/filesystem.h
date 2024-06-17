/*---------------------------------------------*/
/*---------------------SPIFFS------------------*/
/*---------------------------------------------*/
#pragma once
#include "../includes.h"
#include "../defines.h"
#include "nvs_flash.h"
#include "dirent.h"


inline void initMemoryESPIDF()
{
  esp_vfs_spiffs_conf_t config{
    .base_path = "/spiffs",
    .partition_label = "spiffs",
    .max_files = 5,
    .format_if_mount_failed = false,
  };
  esp_err_t ret = esp_vfs_spiffs_register(&config);
    if (ret != ESP_OK) {
      
        if (ret == ESP_FAIL) {
            ESP_LOGE("SPIFFS", "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE("SPIFFS", "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE("SPIFFS", "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }
  
  if(esp_spiffs_mounted("spiffs"))
    ESP_LOGI("SPIFFS", "Fs mounted!");
  else
    ESP_LOGI("SPIFFS", "Fs NOT mounted!");

    esp_err_t rett = nvs_flash_init();
    if (rett == ESP_ERR_NVS_NO_FREE_PAGES || rett == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      rett = nvs_flash_init();
    }
    ESP_ERROR_CHECK(rett);
}


inline FILE* openFPGAFile() 
{
  return fopen("/spiffs/fpga.bin", "rb");
}

inline void closeFPGAFile(FILE* fileFPGA)
{
  fclose(fileFPGA);
}

inline size_t readFPGAFile(FILE* fileFPGA, uint8_t* data, size_t size)
{
  return fread(data, sizeof(uint8_t), size, fileFPGA);
}

//--------------------
// Ladder diagram file
FILE* fileLD;
// IMPORTANT NOTE to write data to flash interrupts have to be in IRAM and using data in DRAM or have to be disabled
inline void openLDFileEspidf(const char *mode = "r")
{
  fileLD = fopen("/spiffs/ladder_program.ld", mode);
}

inline void closeLDFileEspidf()
{
  fclose(fileLD);
}

inline long getLDFileEspidfSize()
{
  if(fseek(fileLD, 0, SEEK_END) != 0)
    return -1;
  long fileSize = ftell(fileLD);
  if(fseek(fileLD, 0, SEEK_SET) != 0)
    return -1;
  
  return fileSize;
}

/// @brief Calculates maximum size to write ladder diagram save
/// @return maximum ladder diagram size
/// @note Be sure the file is open!
size_t getLDMaxFileSize()
{
  size_t totalBytes = 0;
  size_t usedBytes = 0;
  esp_spiffs_info("spiffs", &totalBytes, &usedBytes);

  return totalBytes - usedBytes  + static_cast<size_t>(getLDFileEspidfSize());
}

// Works but generates delay
inline size_t writeToLDFile(const uint8_t* data, uint32_t length)
{
  size_t s = fwrite(data, sizeof(uint8_t), length, fileLD);
  // fflush(fileLD); //uneccesary
  //fsync(fileno(fileLD));
  return s;
}

//Read up to 64 bytes
inline size_t readFromLDFile(uint8_t* data)
{
  return fread(data, sizeof(uint8_t), 64, fileLD);
}

inline size_t readFromLDFile(uint8_t* data, size_t count)
{
  return fread(data, sizeof(uint8_t), count, fileLD);
}

//Time testing of opening and closing file
//~Time[us]  openclose  read
// arduino   ~1300      ~37 
// espidf    ~361       ~36
//espidf is faster than arduino
//Testing functions are in previous commit