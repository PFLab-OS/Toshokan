#pragma once

#define SHARED_SYMBOL(declaration) \
  declaration __attribute__((section(".shsym")))
