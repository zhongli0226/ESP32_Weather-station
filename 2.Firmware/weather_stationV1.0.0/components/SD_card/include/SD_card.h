#ifndef __SD_CARD_H
#define __SD_CARD_H

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"

#define SPI_DMA_CHAN    2
#define MOUNT_POINT "/sdcard"//根目录

//在测试SD和SPI模式时，请记住，一旦在SPI模式下初始化了卡，在不接通卡电源的情况下就无法在SD模式下将其重新初始化。
#define PIN_NUM_MISO		19
#define PIN_NUM_MOSI		23
#define PIN_NUM_CLK			18
#define PIN_NUM_CS			5


extern void Init_SDcard();
extern void get_fatfs_usage(size_t* out_total_bytes, size_t* out_free_bytes);

#endif