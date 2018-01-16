// [expermental] depftom loader
// use "cat <file> > /dev/depftom" instead!
//
// how to build
// g++ -static load.cc -o load

#include <iostream>
#include <stdio.h>
using namespace std;

int main(int argc, const char **argv) {
  FILE *rfp, *wfp;

  if (argc != 2) {
    cerr << "usage: load <target file>" << endl;
    return 1;
  }

  rfp = fopen(argv[1], "rb");
  if (rfp == NULL) {
    cout << "depftom_loader: failed to open [" << argv[1] << "]" << endl;
    return 1;
  } else {
    cerr << "depftom_loader: open [" << argv[1] << "]" << endl;
  }

  wfp = fopen("/dev/depftom", "wb");
  if (wfp == NULL) {
    cerr << "depftom_loader: failed to open [/sys/kernel/file/file]" << endl;
    return 1;
  }

  size_t total = 0;
  size_t len;
  char buf[4096];
  while((len = fread(buf, 1, 4096, rfp)) != 0) {
    while(len != 0) {
      size_t tmp_len = fwrite(buf, 1, len, wfp);
      if (tmp_len == 0) {
	cerr << "depftom_loader: failed to write to [/sys/kernel/file/file]" << endl;
	return 1;
      }
      total += tmp_len;
      len -= tmp_len;
    }
  }

  cout << "depftom_loader: wrote " << total << "bytes" << endl;
  
  fclose(rfp);
  fclose(wfp);
  
  return 0;
}
