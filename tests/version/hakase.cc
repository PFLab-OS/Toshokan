#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>

int test_main() {
  FILE *version_fp = fopen("/sys/module/friend_loader/info/version", "r");
  if (!version_fp) {
    perror("failed to open `version`");
    return 0;
  }

  char buf[256];
  buf[fread(buf, 1, 255, version_fp)] = '\0';
  if (!strstr(buf, "v0.03")) {
    std::cerr << "error: version mismatched"
              << std::endl;
    return 0;
  }
  fclose(version_fp);

  return 1;
}
