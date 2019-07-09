#include <toshokan/friend/offload.h>
#include "shared.h"

int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
int SHARED_SYMBOL(variable) = 0;

void friend_main() {
  /*
   * do not run these code on friend
   *

   printf("hello world\n");

   */
  asm volatile("cli;");
  OFFLOAD({
    /*
     * do not run these code in offloading section
     *

     asm volatile("cli;");

    */
    EXPORTED_SYMBOL(printf)("Hello World!\n");
  });
  SHARED_SYMBOL(variable) = 1;
}
