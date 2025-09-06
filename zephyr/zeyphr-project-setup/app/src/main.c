/*
 * Copyright (c) 2022 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <app_version.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>

int main(void) {
  printk("Address of sample %p\n", (void *)__rom_region_start);
  printk("Hello sysbuild with mcuboot! %s\n", CONFIG_BOARD);

  while (true) {
    k_sleep(K_MSEC(1000)); // sleep for 1 second
    printk("%s\n", APP_VERSION_STRING);
  }
  return 0;
}
