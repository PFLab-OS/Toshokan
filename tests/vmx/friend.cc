#include <toshokan/friend/offload.h>
#include "shared.h"

int64_t SHARED_SYMBOL(vmx) = VMX_NOTAVAILABLE;

void friend_main() {
  uint64_t eax, ebx, ecx, edx;
	asm volatile("cpuid"
	    : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
	    : "0" (1)
	    : "memory");

  if (ecx & (1<<5)) {
    SHARED_SYMBOL(vmx) = VMX_AVAILABLE;
  }
}
