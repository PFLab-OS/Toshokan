#pragma once

template <class T>
inline T align(T val, T align_val) {
  return (val / align_val) * align_val;
}
