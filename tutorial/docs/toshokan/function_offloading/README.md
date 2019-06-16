
# friendからhakaseへのfunction offloading

hakase上でのみ実行できる関数（例：libc関数）をfriendコード上で呼び出したかのように記述する事ができます。ただし、実際には関数はhakase上で実行されます。

**v0.02において、関数の戻り値は取得できません。** オフロード先の関数の戻り値が必要な場合は、ラッパ関数を用意し、戻り値を格納するための変数のポインタを渡す、等の対応が必要になります。

標準ライブラリ関数とhakase上で自前で定義した関数の双方とも、オフロード可能です。

## シンタックス
hakase側では`<toshokan/hakase/export.h>`をincludeしてください。宣言後、或いは定義後に`EXPORT_SYMBOL()`を記述してください。標準ライブラリ関数の場合は、該当するヘッダを事前にincludeしてください。

friend側では、`<toshokan/friend/export.h>`と`<toshokan/friend/offload.h>`の両方をincludeしてください。更に`EXPORTED_SYMBOL()`を用いて関数宣言を行ってください。関数呼び出し時は、`OFFLOAD_FUNC()`を用いてください。

### printfの場合
```cc
// hakase
#include <toshokan/hakase/export.h>
#include <stdio.h>

EXPORT_SYMBOL(printf);
```

```cc
// friend
#include <toshokan/friend/export.h>
#include <toshokan/friend/offload.h>

int (*EXPORTED_SYMBOL(printf))(const char *format, ...);

void func() {
  OFFLOAD_FUNC(printf, "%s\n", "Hello World!");
}
```

## メモリアクセス制約について
関数の引数や戻り値にポインタを指定する場合、ポインタが指す先のアドレスはfriendメモリ内でなければなりません。例えば、以下のような事はできません。

```cc
// hakase
int hakase_variable;

void hakase_func(int **ptr_i) {
  *ptr_i = &hakase_variable;
}

EXPORT_SYMBOL(hakase_func);
```

```cc
// friend
void (*EXPORTED_SYMBOL(hakase_func))(int **i);

void func() {
  int *i;
  OFFLOAD_FUNC(hakase_func, &i);

  // i == &hakase_variable;
  
  *i = 123; // hakase_variableに123を書き込もうとしているが、これは上手くいかない
}
```

また、friend上で仮想メモリと物理メモリがストレートマップされている必要があります。ただしこれは、friend上でページテーブルを再設定等しない限り問題にはなりません。


## サンプル
本リポジトリをクローンし、このREADME.mdがあるディレクトリ上で`make`する事で、サンプルプログラムを実行できます。