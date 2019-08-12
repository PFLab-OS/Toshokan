{% import 'macro.tpl' as helper %}
# CPUによるインストラクションポインタ変更

{{ helper.sample_info() }}

今回のサンプルでは、コンソールから1-2を入力する事により、異なるサンプルコードを実行できます。以下の説明に対応するサンプルを実行してください。

## 例外
例外とは、ソフトウェア実行中に発生した予期せぬエラーを、ソフトウェアに通知するための仕組みです。もちろん、エラーが発生しているコードは実行を継続できないので、通知受取専用のコード（ハンドラ）を呼び出す事によって通知します。

割り込みは、ハードウェア（CPUやCPU以外のデバイス）からの通知をソフトウェアに伝えるための仕組みです。例外と割り込みは、発生要因が異なる（前者はCPU内部のエラー、後者はハードウェアからの通知）ものの、ほぼ同じ仕組みで処理できます。

## ０除算例外
普通のアプリケーションでは、0で数字を割るとクラッシュしてしまう事を皆さんもご存知でしょう。この時、システムの内部で起きているのは、CPUによる０除算例外の発行（とそれに伴うOSによるアプリケーション強制終了処理）です。


０除算をしてしまうようなコードの実行をCPUが継続する事はできないので、「これはCPUが対処できないエラーである」という事を例外を通じてソフトウェアに通知する必要があるわけです。

今回は０除算例外を通じて、例外や割り込みの基本を見てみましょう。

## 1. ただの割り算
サンプルの1を実行してみます。これは０除算等関係無く、単に1/1の割り算を実行する物です。

この時、wait_input()の戻り値は1になるので、以下の様なコードが実行されます。途中のインラインアセンブリコードは、単なる1/1です。（今後の都合で敢えてインラインアセンブリで記述しています）

```cc
// friend.cc
void friend_main() {
  int num = 0;
  // 中略
  switch(wait_input()) {
  case 1:
    num = 1;
    asm volatile("divl %2" ::"a"(1), "d"(0), "m"(num));  // 1 / 1
    break;
  }
}

```

もちろん、このコードは何の問題も無く動きます。最後まで動作する事を確認してみてください。

## 2. 0除算
今回は実際に0除算をしてみて、CPUが例外を発行する所を見てみましょう。

サンプル２ではwait_input()の戻り値が2なので、numの値が0になり、1/0を行います。

```cc
// friend.cc
void friend_main() {
  int num = 0;
  // 中略
  switch(wait_input()) {
  case 2:
    num = 0;
    asm volatile("divl %2" ::"a"(1), "d"(0), "m"(num));  // 1 / 0
    break;
  }
}

```

この際の例外ハンドラとして、int_handlerルーチン（int.Sで定義）の先頭アドレスを設定しています。

```cc
// friend.cc
setup_inthandler(int_handler);
```

`setup_inthandler()`の中身は少し複雑なので、今回は解説しません。`setup_inthandler()`でしているような処理（+lidt命令）を使えば、例外ハンドラや割り込みハンドラを登録できるのだ、とだけ今は思っておいてください。

ではこれを実行してみてください。

どうでしょうか？実行が終了しませんね。1/1をするサンプル１と殆ど変わらないコード（変えた部分は割り算の割る数を0にしただけ）ですが、全く異なる結果になりました。

では、QEMUモニタで、CPUが現在どのインストラクションを実行しているか調べてみましょう。サンプルを実行したまま、別のターミナルで、`make monitor`を開き、`info registers`でRIPの値を取得してみてください。

```
$ make monitor
Silent mode is enabled by default. You can turn it off with 'make V=1'.
>>> connecting to QEMU monitor
QEMU 2.12.0 monitor - type 'help' for more information
(qemu) info registers
info registers
RAX=0000000000000001 RBX=0000000040007000 RCX=0000000000000001 RDX=0000000000000000
RSI=000000000000ffff RDI=0000000040002000 RBP=0000000040008fd0 RSP=0000000040008f98
R8 =0000000000000000 R9 =0000000000000000 R10=0000000000000000 R11=0000000000000000
R12=0000000000000000 R13=0000000000000000 R14=0000000000000000 R15=0000000000000000
RIP=00000000400002c1 RFL=00000046 [---Z-P-] CPL=0 II=0 A20=1 SMM=0 HLT=1
ES =0000 0000000000000000 0000ffff 00009300 DPL=0 DS   [-WA]
CS =0010 0000000000000000 00000000 00209a00 DPL=0 CS64 [-R-]
SS =0018 0000000000000000 00000000 00009300 DPL=0 DS   [-WA]
DS =0018 0000000000000000 00000000 00009300 DPL=0 DS   [-WA]
FS =0000 0000000000000000 0000ffff 00009300 DPL=0 DS   [-WA]
GS =0000 0000000000000000 0000ffff 00009300 DPL=0 DS   [-WA]
LDT=0000 0000000000000000 0000ffff 00008200 DPL=0 LDT
TR =0000 0000000000000000 0000ffff 00008b00 DPL=0 TSS64-busy
GDT=     0000000040000300 0000002f
IDT=     0000000040001040 000003ff
CR0=80000011 CR2=0000000000000000 CR3=0000000040003000 CR4=000000b0
DR0=0000000000000000 DR1=0000000000000000 DR2=0000000000000000 DR3=0000000000000000 
DR6=00000000ffff0ff0 DR7=0000000000000400
EFER=0000000000000500
FCW=037f FSW=0000 [ST=0] FTW=00 MXCSR=00001f80
FPR0=0000000000000000 0000 FPR1=0000000000000000 0000
FPR2=0000000000000000 0000 FPR3=0000000000000000 0000
FPR4=0000000000000000 0000 FPR5=0000000000000000 0000
FPR6=0000000000000000 0000 FPR7=0000000000000000 0000
XMM00=00000000000000000000000000000000 XMM01=00000000000000000000000000000000
XMM02=00000000000000000000000000000000 XMM03=00000000000000000000000000000000
XMM04=00000000000000000000000000000000 XMM05=00000000000000000000000000000000
XMM06=00000000000000000000000000000000 XMM07=00000000000000000000000000000000
XMM08=00000000000000000000000000000000 XMM09=00000000000000000000000000000000
XMM10=00000000000000000000000000000000 XMM11=00000000000000000000000000000000
XMM12=00000000000000000000000000000000 XMM13=00000000000000000000000000000000
XMM14=00000000000000000000000000000000 XMM15=00000000000000000000000000000000
```

ここでは0x400002c1となりました。このアドレスがソースコードのどの行に対応するのか、調べてみましょう。（アドレスは適宜皆さんが得たRIPの値に置き換えてください）

```
$ bin/addr2line -e friend.bin 400002c1
```

int.Sの5行目、と出てきたでしょうか？これはint_handlerルーチン内であり、期待通り0除算例外が発生して、事前に登録していたint_handlerルーチンが呼び出された事が分かります。


## 今回のまとめ
- 割り込みや例外とは、非常に簡略化すると「何らかの要因に起因した、CPUによるインストラクション変更」である。
- 割り込みや例外が発生すると、事前に設定していたハンドラへ強制的にジャンプする。
