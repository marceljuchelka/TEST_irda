/*
 * main.c
 *
 *  Created on: 15. 2. 2022
 *      Author: marcel
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_task_wdt.h"
#include "driver/ir_rx.h"
#include "sdkconfig.h"

static const char *TAG = "main";

#define IR_RX_IO_NUM 12
#define IR_RX_BUF_LEN 128 // The actual allocated memory size is sizeof(uint32_t)*BUF_LEN. If the allocation is too small, the old infrared data may be overwritten.

/**
 * @brief check whether the ir cmd and addr obey the protocol
 *
 * @param nec_code nec ir code that received
 *
 * @return
 *     - ESP_OK success
 *     - ESP_ERR_INVALID_ARG Parameter error
 */
static esp_err_t ir_rx_nec_code_check(ir_rx_nec_data_t nec_code)
{

    if ((nec_code.addr1 != ((~nec_code.addr2) & 0xff))) {
        return ESP_FAIL;
    }

    if ((nec_code.cmd1 != ((~nec_code.cmd2) & 0xff))) {
        return ESP_FAIL;
    }

    return ESP_OK;
}

void ir_rx_task(void *arg)
{
    ir_rx_nec_data_t ir_data;
    ir_rx_config_t ir_rx_config = {
        .io_num = IR_RX_IO_NUM,
        .buf_len = IR_RX_BUF_LEN
    };
    ir_rx_init(&ir_rx_config);

    while (1) {
        ir_data.val = 0;
        ir_rx_recv_data(&ir_data, 1, portMAX_DELAY);
        ESP_LOGI(TAG, "addr1: 0x%x, addr2: 0x%x, cmd1: 0x%x, cmd2: 0x%x", ir_data.addr1, ir_data.addr2, ir_data.cmd1, ir_data.cmd2);

        if (ESP_OK == ir_rx_nec_code_check(ir_data)) {
            ESP_LOGI(TAG, "ir rx nec data:  0x%x", ir_data.cmd1);
        } else {
            ESP_LOGI(TAG, "Non-standard nec infrared protocol");
        }
    }

    vTaskDelete(NULL);
}

void print_text(void *pvParameters ){
	static uint8_t pocitadlo = 0;
	char *TAG = "print_text";

	for(;;){
		ESP_LOGI(TAG, "volna halda: %d\n   pocitadlo %d \n", esp_get_free_heap_size(), pocitadlo++);
		esp_task_wdt_reset();
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}



void app_main()
{
    xTaskCreate(ir_rx_task, "ir_rx_task", 2048, NULL, 5, NULL);
    xTaskCreate(print_text, "print_text", 2048, NULL, 1, NULL);
	for(;;){

	}
}
