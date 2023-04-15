#include <stdio.h>
#include "SD_card.h"
#include "Wifi.h"
#include "string.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"




void Init_SDcard()
{
    esp_err_t ret;                                                                    // ESP错误定义
    sdmmc_card_t *card;                                                               // SD / MMC卡信息结构
    const char mount_point[] = MOUNT_POINT;                                           // 根目录
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {                                 // 文件系统挂载配置
                                                     .format_if_mount_failed = false, // 如果挂载失败：true会重新分区和格式化/false不会重新分区和格式化
                                                     .max_files = 5,                  // 打开文件最大数量
                                                     .allocation_unit_size = 16 * 1024};
    //用于SD over SPI驱动的默认sdmmc_host_t结构初始化器
    //使用SPI模式，最大频率设置为20MHz
    //'slot'应该被设置为由`sdspi_host_init_device()'初始化的sdspi设备。
    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    // SPI总线初始化
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CHAN);

    if (ret != ESP_OK)
    {
        printf("Failed to initialize bus.\r\n");
        return;
    }
    else
    {
        printf("Successed to initialize SPI bus.\r\n");
    }
    // 这将初始化没有卡检测（CD）和写保护（WP）信号的插槽。
    // 如果您的主板有这些信号，请修改slot_config.gpio_cd和slot_config.gpio_wp。
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;
    // 挂载文件系统

    //使用VFS(虚拟文件系统)注册SD卡上的FAT文件系统的便利函数
    /*
      这是一个多合一的函数，做了以下工作。
      用host_config中的配置初始化SDMMC驱动或SPI驱动.
      用slot_config中的配置初始化SD卡.
      使用FATFS库在SD卡上挂载FAT分区，配置在mount_config。
      在VFS中注册FATFS库，其前缀由base_prefix变量给出。

      mount_point-用于注册分区的路径（例如："/sdcard"）
      host-指向描述SDMMC主机的结构的指针。
           当使用SDMMC外围设备时，这个结构可以用SDMMC_HOST_DEFAULT()来初始化宏指令。
           当使用SPI外围设备时，这个结构可以用SDSPI_HOST_DEFAULT()来初始化宏指令。
      slot_config-指向具有插槽配置的结构的指针。
                  对于SDMMC外围设备，传递一个指向sdmmc_slot_config_t的指针，使用SDMMC_SLOT_CONFIG_DEFAULT()初始化的结构。
                  对于SPI外设，传递一个指向sdspi_device_config_t的指针，使用SDSPI_DEVICE_CONFIG_DEFAULT()进行初始化。
      mount_config-指针指向带有额外参数的结构，用于安装FATFS.
      card-如果不是NULL，指向卡片信息结构的指针将通过这个参数返回.
      返回：
      成功-ESP_OK
      如果esp_vfs_fat_sdmmc_mount已经被调用-ESP_ERR_INVALID_STATE。
      如果不能分配内存-ESP_ERR_NO_MEM。
      如果分区不能被安装-ESP_FAIL。
      来自SDMMC或SPI驱动、SDMMC协议或FATFS驱动的其他错误代码
    */
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            printf("Failed to mount filesystem.%s\r\n",
                   "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else
        {
            printf("Failed to initialize the card %s  (%s). ",
                   "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    else
    {
        printf("Successed to initialize SD.\r\n");
    }
    // TF卡已经初始化，打印TF卡属性
    sdmmc_card_print_info(stdout, card);
    // Print FAT FS size information
    size_t bytes_total, bytes_free;
    get_fatfs_usage(&bytes_total, &bytes_free);

    printf("FAT FS Total: %d MB, Free: %d MB \r\n", bytes_total / 1024, bytes_free / 1024);

    // 读取文件
    printf("Reading file\r\n");
    FILE *f = fopen(MOUNT_POINT "/wifi.cfg", "r"); // 读取方式打开文件
    if (f == NULL)
    {
        printf("Failed to open file for reading\r\n");
        return;
    }

    fgets(wifi_ssid, sizeof(wifi_ssid), f); // 读取一行数据
    char *pos = strchr(wifi_ssid, '\r');    // 在字符串中查找换行 注意在不同系统下的末尾不同，win下后结尾为\r\n linux下为\n
    if (pos)
    {
        *pos = '\0'; // 替换为结束符
    }
    // printf("Read from file: %s\r\n", wifi_ssid);

    fgets(wifi_password, sizeof(wifi_password), f); // 读取一行数据
    pos = strchr(wifi_password, '\r');             // 在字符串中查找换行
    if (pos)
    {
        *pos = '\0'; // 替换为结束符
    }
    // printf("Read from file: %s\r\n", wifi_password);
    fgets(loaction, sizeof(loaction), f); // 读取一行数据
    pos = strchr(loaction, '\r');             // 在字符串中查找换行
    if (pos)
    {
        *pos = '\0'; // 替换为结束符
    }
    fclose(f); // 关闭文件
    printf("wifi id: %s\r\n", wifi_ssid);
    printf("wifi password: %s\r\n", wifi_password);
    printf("location: %s\r\n", loaction);
    // // 卸载分区并禁用SDMMC或SPI外设
    // esp_vfs_fat_sdcard_unmount(mount_point, card);
    // printf("Card unmounted\r\n");
    // //卸载总线
    // spi_bus_free(host.slot);
}

void get_fatfs_usage(size_t *out_total_bytes, size_t *out_free_bytes)
{
    FATFS *fs;
    size_t free_clusters;
    int res = f_getfree("0:", &free_clusters, &fs); //
    assert(res == FR_OK);
    size_t total_sectors = (fs->n_fatent - 2) * fs->csize;
    size_t free_sectors = free_clusters * fs->csize;

    size_t sd_total = total_sectors / 1024;
    size_t sd_total_KB = sd_total * fs->ssize;
    size_t sd_total_MB = (sd_total * fs->ssize) / 1024;
    size_t sd_free = free_sectors / 1024;
    size_t sd_free_KB = sd_free * fs->ssize;
    size_t sd_free_MB = (sd_free * fs->ssize) / 1024;

    // printf("SD Cart sd_total_KB %d KByte\r\n ", sd_total_KB);
    // printf("SD Cart sd_total_MB %d MByte\r\n ", sd_total_MB);
    // printf("SD Cart sd_free_KB %d KByte\r\n ", sd_free_KB);
    // printf("SD Cart sd_free_MB %d MByte\r\n ", sd_free_MB);

    // 假设总大小小于4GiB，对于SPI Flash应该为true
    if (out_total_bytes != NULL)
    {
        *out_total_bytes = sd_total_KB;
    }
    if (out_free_bytes != NULL)
    {
        *out_free_bytes = sd_free_KB;
    }
}