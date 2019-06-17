
# hakaseとfriend間のsymbol解決

特殊なシンタックスによって記述する事により、hakase上からfriendのグローバル変数を参照する事が可能です。また、この参照はhakaseバイナリのコンパイル時に（静的に）行われます。

## シンタックス
hakase及びfriend上において、共有変数名に`SHARED_SYMBOL()`を付けるだけです。
この際、双方において`<toshokan/symbol.h>`をincludeしてください。

### 変数宣言

```cc
// friend
int SHARED_SYMBOL(variable);
```
**必ずfriend上でグローバル変数として宣言してください。**

ローカル変数や、hakase上で宣言した場合、正常に動作しません。

### 変数参照
hakase上、friend上で共通です。

```cc
// hakase
void func() {
     SHARED_SYMBOL(variable) = 0;
     int i = SHARED_SYMBOL(variable);
}
```

```cc
// friend
void func() {
     SHARED_SYMBOL(variable) = 0;
     int i = SHARED_SYMBOL(variable);
}
```

## ベタープラクティス
`shared.h`を作成し、hakaseとfriendの双方からインクルードする事によって、hakaseとfriend間での型の不一致を防ぐ事ができます。

```cc
// shared.h
#pragma once
#include <toshokan/symbol.h>

extern int SHARED_SYMBOL(variable);
```

## サンプル
本リポジトリをクローンし、このREADME.mdがあるディレクトリ上で`make`する事で、サンプルプログラムを実行できます。