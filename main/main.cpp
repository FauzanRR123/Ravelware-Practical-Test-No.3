#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#define SENSOR_COUNT 5

static const char *TAG = "SOAL_3";

// Shared Data Buffer & Mutex
int sharedDataBuffer[SENSOR_COUNT];
SemaphoreHandle_t bufferMutex;

// Task A: Sensor
void sensor_task(void *pvParameters) {
    while (1) {
        int temp_data[SENSOR_COUNT];
        for (int i = 0; i < SENSOR_COUNT; i++) {
            temp_data[i] = (i + 1) * 100;
        }

        if (xSemaphoreTake(bufferMutex, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Sensor Task: Updating buffer...");
            for (int i = 0; i < SENSOR_COUNT; i++) {
                sharedDataBuffer[i] = temp_data[i];
            }
            xSemaphoreGive(bufferMutex); 
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

// Task B: Communication
void communication_task(void *pvParameters) {
    while (1) {
        int data_to_send[SENSOR_COUNT];
        bool has_data = false;

        if (xSemaphoreTake(bufferMutex, portMAX_DELAY) == pdTRUE) {
            for (int i = 0; i < SENSOR_COUNT; i++) {
                data_to_send[i] = sharedDataBuffer[i];
            }
            has_data = true;
            xSemaphoreGive(bufferMutex); 
        }

        if (has_data) {
            ESP_LOGI(TAG, "Communication Task: Sending data...");
            for (int i = 0; i < SENSOR_COUNT; i++) {
                ESP_LOGI(TAG, " -> Data Sensor %d: %d terkirim", i + 1, data_to_send[i]);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(3000)); 
    }
}

// Wajib pakai extern "C" karena file Anda .cpp
extern "C" void app_main(void) {
    ESP_LOGI(TAG, "=== Sistem Dimulai ===");

    bufferMutex = xSemaphoreCreateMutex();

    if (bufferMutex != NULL) {
        xTaskCreate(sensor_task, "SensorTask", 2048, NULL, 5, NULL);
        xTaskCreate(communication_task, "CommTask", 2048, NULL, 4, NULL);
    } else {
        ESP_LOGE(TAG, "Gagal membuat Mutex!");
    }
}