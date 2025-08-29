//! @file
//!
//! @brief
//! Shell commands for experimenting with breakpoints

#include "bootutil/bootutil_public.h"
#include "shell/shell.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bootutil/bootutil.h"
#include "hal/logging.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

static void prv_reboot(void) {
  // NVIC_SystemReset
  *(volatile uint32_t *)0xE000ED0C = 0x5FAUL << 16 | 0x4;

  __builtin_unreachable();
}

static int prv_reboot_cli(int argc, char *argv[]) {
  prv_reboot();
  return 0;
}

static int prv_swap_images(int argc, char *argv[]) {
  EXAMPLE_LOG("Triggering Image Swap");

  const int permanent = 0;
  const int result = boot_set_pending(permanent);
  if (result == 0) {
    prv_reboot();
  } else {
    EXAMPLE_LOG("Swap failed with result: %l", result);
  }

  return result;
}

#ifndef IDN
#ifndef FIRMWARE_IDN
#define FIRMWARE_IDN "UNDEFINED"
#endif
#define IDN "Maskset," FIRMWARE_IDN ",1,0.1.0-" __TIME__
#endif

static constexpr const char idn[] = IDN;

int cli_idn(int argc, char *argv[]) {
  EXAMPLE_LOG(idn);
  return 0;
}

static int cli_magic_number_check(int argc, char *argv[]) {
  for (size_t slot = 0; slot < (MCUBOOT_IMAGE_NUMBER + 1); slot++) {
    const struct flash_area *fap;
    struct boot_swap_state state;
    int rc;

    rc =
        boot_read_swap_state_by_id(flash_area_id_from_image_slot(slot), &state);
    if (rc != 0) {
      EXAMPLE_LOG("Failed to read swap state for slot %d: %d", (int)slot, rc);
      continue;
    }

    EXAMPLE_LOG("Slot %d: magic=0x%02x swap_type=%d copy_done=%d image_ok=%d",
                (int)slot, (unsigned)state.magic, (int)state.swap_type,
                (int)state.copy_done, (int)state.image_ok);
  }

  return 0;
}

static const sShellCommand s_shell_commands[] = {
    {"*IDN?", cli_idn, "Identity response"},
    {"MAGIC?", cli_magic_number_check, "Check magic numbers"},
    {"swap_images", prv_swap_images, "Swap images"},
    {"reboot", prv_reboot_cli, "Reboot System"},
    {"help", shell_help_handler, "Lists all commands"},
};

const sShellCommand *const g_shell_commands = s_shell_commands;
const size_t g_num_shell_commands = ARRAY_SIZE(s_shell_commands);
