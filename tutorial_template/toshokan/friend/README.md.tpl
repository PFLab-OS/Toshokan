{% import 'macro.tpl' as helper %}

# friend上でできる事、できない事
Toshokanでコードを書く際の基礎事項をざっくりと解説します。

## friend上でできて、hakase上ではできない事

cli instructionはRing0（特権レベル）でのみ実行を許可されています。friend上では以下のようなインラインアセンブリ構文を呼んでも問題ありませんが、hakase上でこれを実行すると、アプリケーションがクラッシュします。

```cc
// friend only
asm volatile("cli;");
```

また、friend上ではOFFLOADセクションを使う事ができます。（後述）

## hakase上でできて、friend上ではできない事

printf等のlibc関数やLinuxシステムコール等はfriend上では呼び出せません。（コンパイル時に失敗します）

```cc
// hakase only
printf("Hello World!");
```

## OFFLOADセクション

friend内のOFFLOADセクションでは、hakase上で実行できるコードを実行でき、逆にhakase上で実行できないコードは実行できません。また、OFFLOADセクションからhakase上の関数を呼び出すためには、EXPORTED_SYMBOL()を利用する必要があります。

```cc
// friend only
OFFLOAD({
  // do not write the following code in a OFFLOAD section
  // asm volatile("cli;");

  // printf is allowed but do not forget to use EXPORTED_SYMBOL
  EXPORTED_SYMBOL(printf)("Hello World!");
});
```

詳しくは[別ページの解説](../offloading/)を参照してください。

## hakaseとfriendの両方でできる事
SHARED_SYMBOL()が付加された変数は、hakaseとfriendの両方からアクセスする事ができます。

```cc
// you can use this symbol on both hakase and friend.
SHARED_SYMBOL(variable) = 1;
return SHARED_SYMBOL(variable);
```

詳しくは[別ページの解説](../symbol_resolution/)を参照してください。