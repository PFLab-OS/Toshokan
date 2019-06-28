#include <toshokan/hakase/export.h>
#include <toshokan/hakase/hakase.h>
#include "shared.h"

int state;

int func(int i, int j)
{
  state = i - j;
  return 100;
}

EXPORT_SYMBOL(func);

int test_main()
{
  int r;
  r = setup();
  if (r != 0)
  {
    return r;
  }

  SHARED_SYMBOL(sync_flag) = 0;
  state = 0;

  boot(1);

  while (SHARED_SYMBOL(sync_flag) == 0)
  {
    offloader_tryreceive();
    asm volatile("pause" ::
                     : "memory");
  }

  return (state == 1);
}
