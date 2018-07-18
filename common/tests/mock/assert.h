#pragma once

void assert_func();
#define assert(x) if (!(x)) { assert_func(); }
