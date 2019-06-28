
# friendからhakaseへのfunction offloading

hakase上でのみ実行できるコードや関数（例：libc関数）をfriendコード上で呼び出したかのように記述する事ができます。ただし、実際には関数はhakase上で実行されます。

標準ライブラリ関数とhakase上で自前で定義した関数の双方とも、オフロード可能です。

## シンタックス　
### OFFLOAD
friend上で`<toshokan/friend/export.h>`をincludeし、offloadしたいコードブロックを`OFFLOAD({ <コード> });`と記述してください。

hakase側では、初期化処理終了後に`offloader_tryreceive()`を呼び出し続けてください。

```cc
// hakase
#include <toshokan/hakase/hakase.h>
#include <stdio.h>

int main(int argc, const char **argv) {
  // do initialization
  while(true) {
    offloader_tryreceive();
    usleep(100);
    // break after friend binary finished its execution.
  }
}
```

```cc
// friend
#include <toshokan/friend/offload.h>

void func() {
  int x = 0;
  OFFLOAD({
    // executed on hakase
    x = 1;
  });
  // x == 1
}
```

`OFFLOAD({})`ブロック内では、friendコードのスコープ内の変数、関数に自由にアクセスする事ができます。

### EXPORT_SYMBOL
`OFFLOAD({})`ブロック内でhakaseの関数を呼び出すためには、別途EXPORT_SYMBOLという仕組みが必要になります。friendはコンパイル時にhakaseの関数を参照する事ができないため、hakaseの関数は関数ポインタ経由で呼び出す事になります。そのための一連の処理を簡略化する仕組みがEXPORT_SYMBOLです。

まず、hakaseとfriendの双方で`<toshokan/export.h>`をincludeし、`EXPORTED_SYMBOL()`を用いて関数ポインタを宣言してください。

hakase側では関数の宣言後、或いは定義後に`EXPORT_SYMBOL()`を記述してください。標準ライブラリ関数の場合は、該当するヘッダを事前にincludeしてください。

friend側では、`EXPORTED_SYMBOL()`を用いて関数ポインタを定義してください。関数呼び出し時も、`EXPORTED_SYMBOL()`を用いてください。

```cc
// hakase
#include <toshokan/export.h>
#include <stdio.h>

extern int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
EXPORT_SYMBOL(printf);

```

```cc
// friend
#include <toshokan/export.h>
#include <toshokan/friend/offload.h>

extern int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
int (*EXPORTED_SYMBOL(printf))(const char *format, ...);

void func() {
  OFFLOAD({
    EXPORTED_SYMBOL(printf)("%s\n", "Hello World!");
  });
}
```

## ベタープラクティス
`SHARED_SYMBOL()`と同様、`shared.h`を作成し、hakaseとfriendの双方からインクルードする事によって、hakaseとfriend間での型の不一致を防ぐ事ができます。

```cc
// shared.h
#pragma once
#include <toshokan/export.h>

extern int (*EXPORTED_SYMBOL(printf))(const char *format, ...);
```

## offloader_tryreceive()
offloader_tryreceive()は、offloadリクエストがfriendから発行されている際に、それをキャッチして実行するための関数です。friend側がoffloadリクエストを出しても、offloader_tryreceive()が実行されていなければ関数はオフロードされません。offloadリクエストが存在しない場合は、offloader_tryreceive()は即座に終了します。

offloader_tryreceive()はブロッキング型の関数では無いため、期待したリクエストを受け取るまで繰り返し実行する（ポーリング処理する）必要があります。同時に、offloadリクエストを出したfriendもまた、offloader_tryreceive()によるhakase側でのリクエストの処理が終了するまではビジーウェイトを行います。

複数のfriendコアから同時にoffloadリクエストが発行されそうになった場合、調停機構が働き、一つのoffloadリクエストのみが発行されます。offloadリクエストを発行できなかった他のコアは、offloadリクエストが終了するまでビジーウェイトを行います。

## メモリアクセス等の制約について
friendとhakaseの非対称性に常に注意してください。`OFFLOAD({})`ブロック内ではhakaseが実行できるコードのみを記述してください。具体的には、`OFFLOAD({})`ブロック内で特権命令等を実行する事はできません。

また逆に、`OFFLOAD({})`ブロックを用いてhakase上のアドレスを返してもfriendからそのメモリにアクセスできるわけではありません。

```cc
// hakase
int hakase_variable;

int *hakase_func() {
  return &hakase_variable;
}

EXPORT_SYMBOL(hakase_func);
```

```cc
// friend
int *(*EXPORTED_SYMBOL(hakase_func))();

void func() {
  int *i;
  OFFLOAD({
    i = EXPORTED_SYMBOL(hakase_func)();
  });

  // i == &hakase_variable;
  
  *i = 123; // hakase_variableに123を書き込もうとしているが、これは上手くいかない
}
```

また、friend上で仮想メモリと物理メモリがストレートマップされている必要があります。ただしこれは、friend上でページテーブルを再設定等しない限り問題にはなりません。


## サンプル
本リポジトリをクローンし、このREADME.mdがあるディレクトリ上で`make`する事で、サンプルプログラムを実行できます。