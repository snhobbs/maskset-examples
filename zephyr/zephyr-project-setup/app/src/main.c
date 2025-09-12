/*
 * Copyright (c) 2022 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zephyr/storage/flash_map.h"
#include <assert.h>
#include <bootutil/bootutil_public.h>
#include <zephyr/app_version.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/shell/shell.h>
#include <zephyr/storage/flash_map.h>

static int version_cmd_handler(const struct shell *sh, size_t argc, char **argv,
                               void *data) {
  shell_print(sh, "%s\n", APP_VERSION_STRING);
  return 0;
}

SHELL_CMD_REGISTER(version, NULL, "Print application version.",
                   version_cmd_handler);

int main(void) {
  printk("Address of sample %p\n", (void *)__rom_region_start);
  printk("Hello sysbuild with mcuboot! %s\n", CONFIG_BOARD);
  int slot = 0;
  //  boot_request_upgrade_multi(slot, 0);
  //  int rc = boot_set_pending(0);
  //  assert(rc == 0);
  printk("Swap Type: %d\n", mcuboot_swap_type());
  printk("VERSION: %s\n", APP_VERSION_STRING);

  while (true) {
    k_sleep(K_MSEC(1000)); // sleep for 1 second
  }
  return 0;
}
