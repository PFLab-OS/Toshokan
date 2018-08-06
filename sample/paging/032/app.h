#pragma once
struct Context {
  Context (*next)(Context c);
  int i;
};
