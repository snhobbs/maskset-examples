/*
 * Copyright (c) 2022 Nordic Semiconductor
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/app_version.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/kernel.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/shell/shell.h>

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

  while (true) {
    k_sleep(K_MSEC(1000)); // sleep for 1 second
  }
  return 0;
}
