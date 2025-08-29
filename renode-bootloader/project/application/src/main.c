#include <stdio.h>

#include "cmsis_shim.h"
#include "hal/logging.h"
#include "hal/uart.h"
#include <stdint.h>
#include <sys/types.h>

#include "bootutil/bootutil.h"

#include "shell_port.h"

#include <string.h>

//! A very naive implementation of the newlib _sbrk dependency function
caddr_t _sbrk(int incr);
caddr_t _sbrk(int incr) {
  static uint32_t s_index = 0;
  static uint8_t s_newlib_heap[2048] __attribute__((aligned(8)));

  if ((s_index + (uint32_t)incr) <= sizeof(s_newlib_heap)) {
    EXAMPLE_LOG("Out of Memory!");
    return 0;
  }

  caddr_t result = (caddr_t)&s_newlib_heap[s_index];
  s_index += (uint32_t)incr;
  return result;
}

__attribute__((noinline)) static void prv_enable_vfp(void) {
  volatile uint32_t *cpacr = (volatile uint32_t *)0xE000ED88;
  *cpacr |= 0xf << 20;
}


extern int cli_idn(int argc, char *argv[]);

int main(void) {
  prv_enable_vfp();
  uart_boot();

  cli_idn(0, NULL);
  // succesfully completed init, mark image as stable

  int rc = boot_set_confirmed();
  if (rc != 0) {
    EXAMPLE_LOG("boot_set_confirmed() failed with error code %d", rc);
    // Optionally handle failure case (e.g., reset or retry)
  }

  EXAMPLE_LOG("==Main Application Booted==");

  shell_processing_loop();

  __builtin_unreachable();
}
