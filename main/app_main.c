#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "Smartconfig.h"
#include "w5500_driver.h"
#include "Http.h"
#include "Nvs.h"
#include "Mqtt.h"
#include "Bluetooth.h"
#include "Json_parse.h"
#include "Human.h"
#include "Uart0.h"
#include "Led.h"
#include "E2prom.h"
#include "RtcUsr.h"
#include "sht30dis.h"
#include "user_app.h"
#include "ota.h"
#include "user_key.h"

void app_main(void)
{

    if (Check_First_Key())
    {
        ota_back();
    }

    Net_sta_group = xEventGroupCreate();
    xMutex_Http_Send = xSemaphoreCreateMutex(); //创建HTTP发送互斥信号
    Send_Mqtt_Queue = xQueueCreate(1, sizeof(creat_json));
    Led_Init();
    user_app_key_init();
    E2prom_Init();
    Read_Metadate_E2p();
    Read_Product_E2p();
    Uart0_Init();
    Human_Init();

    printf("FIRMWARE=%s\n", FIRMWARE);

    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ble_app_init();

    if (memcmp(ProductId, "ubibot-ms1p", 12) == 0)
    {
        w5500_user_int();
    }
    else
    {
        net_mode = NET_WIFI;
        E2P_WriteOneByte(NET_MODE_ADD, net_mode); //写入net_mode
    }

    init_wifi();
    initialise_http();
    initialise_mqtt();

    /* 判断是否有序列号和product id */
    if ((strlen(SerialNum) == 0) || (strlen(ProductId) == 0) || (strlen(WEB_SERVER) == 0)) //未获取到序列号或productid，未烧写序列号
    {
        while (1)
        {
            ESP_LOGE("Init", "no SerialNum or product id!");
            No_ser_flag = true;
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    }
}
