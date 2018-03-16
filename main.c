/*
 * OTA demo
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp/uart.h>

#include <sysparam.h>  //include this in your own code
#include <rboot-api.h> //include this in your own code

#ifndef VERSION
 #error You must set VERSION=x.y.z of the ota-demo code to match github version tag x.y.z
#endif

void print_text_value(char *key, char *value) {
    printf("  '%s' = '%s'\n", key, value);
}

void print_binary_value(char *key, uint8_t *value, size_t len) {
    size_t i;

    printf("  %s:", key);
    for (i = 0; i < len; i++) {
        if (!(i & 0x0f)) {
            printf("\n   ");
        }
        printf(" %02x", value[i]);
    }
    printf("\n");
}

sysparam_status_t dump_params(void) {
    sysparam_status_t status;
    sysparam_iter_t iter;

    status = sysparam_iter_start(&iter);
    if (status < 0) return status;
    while (true) {
        status = sysparam_iter_next(&iter);
        if (status != SYSPARAM_OK) break;
        if (!iter.binary) {
            print_text_value(iter.key, (char *)iter.value);
        } else {
            print_binary_value(iter.key, iter.value, iter.value_len);
        }
    }
    sysparam_iter_end(&iter);

    if (status == SYSPARAM_NOTFOUND) {
        // This is the normal status when we've reached the end of all entries.
        return SYSPARAM_OK;
    } else {
        // Something apparently went wrong
        return status;
    }
}


void ota_task(void *arg) {
    sysparam_status_t status;
    uint32_t base_addr, num_sectors;
    
    vTaskDelay(1000); //10 seconds to allow connecting a console
    printf("ota-demo code version %s compiled %s %s\n", VERSION, __DATE__, __TIME__);
    status = sysparam_get_info(&base_addr, &num_sectors);
    if (status == SYSPARAM_OK) {
        printf("[current sysparam region is at 0X%08X (%d sectors)]\n", base_addr, num_sectors);
    } else {
        printf("[NOTE: No current sysparam region (initialization problem during boot?)]\n");
    }
    dump_params();
    
    sysparam_set_string("ota_repo", "HomeACcessoryKid/ota-demo");
    sysparam_set_string("ota_version", "0.0.0");
    sysparam_set_string("ota_file", "main.bin");
    dump_params();
    printf("In 30 seconds will reboot to the OTA updater\n");
    vTaskDelay(3000); //30 seconds

    rboot_set_temp_rom(1); //select the OTA routine
    sdk_system_restart();

    vTaskDelete(NULL);
}

void user_init(void) {
//    uart_set_baud(0, 74880);
    uart_set_baud(0, 115200);

    xTaskCreate(ota_task,"ota",4096,NULL,1,NULL);
    printf("user-init-done\n");
}
