#ifndef APP_PCH_H
#define APP_PCH_H

#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <freertos/task.h>
#include <freertos/queue.h>
// #include "sdkconfig.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "app_board.h"
#include "app_machine.h"
#include "wifi.h"
#include "text.h"
#include "epd.h"

#include <stdio.h>

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include "services/gap/ble_svc_gap.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "nvs_flash.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"


#include "text.h"
#include "app_board.h"
#include "app_machine.h"
#include "epd.h"
#include "wifi.h"
#include "splash.h"
#include "ble.h"
#include "font.h"


#endif