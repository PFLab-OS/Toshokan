{% import 'build_misc/macro.tpl' as helper %}
# QEMUモニタを用いたデバッグ

{{ helper.sample_info() }}

## QEMUモニタの起動方法
Toshokanを実行中に別のターミナル画面で`make monitor`を実行する事で、QEMUモニタに接続できます。或いは、QEMUコンテナ上でlocalhost:4445へTCPアクセスする事でも接続できます。

接続すると、以下のようなメッセージが表示されます。
```
QEMU 2.12.0 monitor - type 'help' for more information
(qemu) 
```

**ATTENTION:QEMUが起動していない時は`make monitor`が失敗します。実機実行中も、QEMUが起動していないので失敗します。**

## QEMUの終了方法
`q`をタイプする事で、QEMUを終了する事ができます。QEMUの終了は、hakaseとfriendが動作する仮想マシン全体を停止させる事を意味します。

## 簡単なデバッグ

サンプルのfriend.ccは、`hlt`命令を使用し、プロセッサコアを停止させるものです。`hlt`命令は特定の条件下で停止したプロセッサコアを再開させますが、無限ループによってサイド`hlt`命令が呼ばれるようになっています。このコードを実行した時のfriendコアのステータスをQEMUモニタを用いて確認してみましょう。

```cc
// friend.cc
void friend_main() {
  while(true) {
    asm volatile("hlt;");
  }
}
```

まず`make`を実行し、Toshokanを起動しましょう。`>>> running hakase.bin on remote`と表示されたら、make実行中のターミナルを閉じずに、別のターミナルで`make monitor`を実行します。

QEMUモニタ接続時はデフォルトでcpu1が選択されています。`cpu 3`等と実行する事で、cpuを切り替える事ができます。** cpu0はhakaseコアに該当し、friendコードのデバッグを行えません。**

QEMUモニタ上で`info registers`を実行してみましょう。

```
(qemu) info registers
info registers
RAX=00000001000000f0 RBX=0000000100000228 RCX=0000000100006000 RDX=0000000000000000
RSI=0000000000000000 RDI=0000000000071000 RBP=0000000100000228 RSP=0000000100007fd8
R8 =0000000000000000 R9 =0000000000000000 R10=0000000000000000 R11=0000000000000000
R12=00000001000050b4 R13=0000000000000000 R14=0000000000000000 R15=0000000000000000
RIP=00000001000000f1 RFL=00000046 [---Z-P-] CPL=0 II=0 A20=1 SMM=0 HLT=1
ES =0000 0000000000000000 0000ffff 00009300 DPL=0 DS   [-WA]
CS =0010 0000000000000000 00000000 00209a00 DPL=0 CS64 [-R-]
SS =0018 0000000000000000 00000000 00009300 DPL=0 DS   [-WA]
DS =0018 0000000000000000 00000000 00009300 DPL=0 DS   [-WA]
FS =0000 0000000000000000 0000ffff 00009300 DPL=0 DS   [-WA]
GS =0000 0000000000000000 0000ffff 00009300 DPL=0 DS   [-WA]
LDT=0000 0000000000000000 0000ffff 00008200 DPL=0 LDT
TR =0000 0000000000000000 0000ffff 00008b00 DPL=0 TSS64-busy
GDT=     00000000000700a8 0000002f
IDT=     0000000000000000 0000ffff
CR0=80000011 CR2=0000000000000000 CR3=0000000000071000 CR4=000000b0
DR0=0000000000000000 DR1=0000000000000000 DR2=0000000000000000 DR3=0000000000000000
DR6=00000000ffff0ff0 DR7=0000000000000400
EFER=0000000000000500
FCW=037f FSW=0000 [ST=0] FTW=00 MXCSR=00001f80
FPR0=0000000000000000 0000 FPR1=0000000000000000 0000
FPR2=0000000000000000 0000 FPR3=0000000000000000 0000
FPR4=0000000000000000 0000 FPR5=0000000000000000 0000
FPR6=0000000000000000 0000 FPR7=0000000000000000 0000
XMM00=00000000000000000000000000000000 XMM01=8b200a3f54ab9d23e1e8ccb76358fede
XMM02=8d81752b3800279007517879f728ec8e XMM03=00000000000000000000000000000000
XMM04=00000000000000000000000000000000 XMM05=00000000000000000000000000000000
XMM06=00000000000000000000000000000000 XMM07=00000000000000000000000000000000
XMM08=00000000000000000000000000000000 XMM09=00000000000000000000000000000000
XMM10=00000000000000000000000000000000 XMM11=00000000000000000000000000000000
XMM12=00000000000000000000000000000000 XMM13=00000000000000000000000000000000
XMM14=00000000000000000000000000000000 XMM15=00000000000000000000000000000000
```

現在のRIP（インストラクションポインタ）が0x1000000f1を指している事を示しています。

仮にfriend.ccの通りfriendコアがhltによって停止しているとすれば、RIPはhltの次のインストラクションを指しているはずです。（これはCPUの仕様です）hlt命令は1byteなので、0x1000000f0をディスアセンブルすれば、そこにhlt命令が存在するはずです。

```
(qemu) x /10i 0x1000000f0
x /10i 0x1000000f0
0x1000000f0:  f4                       hlt
0x1000000f1:  eb fd                    jmp      0x1000000f0
0x1000000f3:  48 b9 00 60 00 00 01 00  movabsq  $0x100006000, %rcx
0x1000000fb:  00 00
0x1000000fd:  b8 00 40 00 00           movl     $0x4000, %eax
0x100000102:  48 bb 28 01 00 00 01 00  movabsq  $0x100000128, %rbx
0x10000010a:  00 00
0x10000010c:  f0 0f c1 03              lock xaddl %eax, (%rbx)
0x100000110:  eb 48                    jmp      0x10000015a
0x100000112:  b9 b8 00 40 00           movl     $0x4000b8, %ecx
0x100000117:  00 48 bb                 addb     %cl, -0x45(%rax)
0x10000011a:  28 01                    subb     %al, (%rcx)
(qemu)
```

確かに0x1000000f0にhlt命令が存在します。

$eip（ripでない事に注意）によってRIPのレジスタ値を参照する事ができるので、以下のようにアドレスの計算を省略する事もできます。

```
(qemu) x /10i $eip-1
x /10i $eip-1
0x1000000f0:  f4                       hlt
0x1000000f1:  eb fd                    jmp      0x1000000f0
0x1000000f3:  48 b9 00 60 00 00 01 00  movabsq  $0x100006000, %rcx
0x1000000fb:  00 00
0x1000000fd:  b8 00 40 00 00           movl     $0x4000, %eax
0x100000102:  48 bb 28 01 00 00 01 00  movabsq  $0x100000128, %rbx
0x10000010a:  00 00
0x10000010c:  f0 0f c1 03              lock xaddl %eax, (%rbx)
0x100000110:  eb 48                    jmp      0x10000015a
0x100000112:  b9 b8 00 40 00           movl     $0x4000b8, %ecx
0x100000117:  00 48 bb                 addb     %cl, -0x45(%rax)
0x10000011a:  28 01                    subb     %al, (%rcx)
```

ディスアセンブルでなく、生のバイナリデータを見たい場合は、以下の通りです。

```
(qemu) x /10x $eip-1
x /10x $eip-1
00000001000000f0: 0x48fdebf4 0x006000b9 0x00000100 0x4000b800
0000000100000100: 0xbb480000 0x00000128 0x00000001 0x03c10ff0
0000000100000110: 0x01489848 0xc48948c8
```

ディスアセンブル時に表示されているバイトコードと全く同じバイナリが得られています。バイナリ出力がリトルエンディアンである事には注意してください。

次に、このhlt命令が本当に`friend_main()`内のhltであるのかを調べてみましょう。ここではQEMUモニタは不要です。シェル上で`.misc/wrapper/objdump -d friend.bin`とし、friendバイナリをディスアセンブルしてみましょう。

```
$ .misc/wrapper/objdump -d friend.bin

friend.bin:     file format elf64-x86-64


Disassembly of section .text:

00000001000000f0 <_Z11friend_mainv>:
   1000000f0:	 f4			hlt
   1000000f1: eb fd                	jmp    1000000f0 <_Z11friend_mainv>

（後略）
```

関数名がマングルされてしまっていますが、`_Z11friend_mainv`が`friend_main()`です。そして、正しく先程の0x1000000f0にhlt命令が埋め込まれている事が確認できます。

## 最後に
QEMUモニタの詳細は[このページ](https://en.wikibooks.org/wiki/QEMU/Monitor)が参考になります。また、[公式ドキュメント](https://en.wikibooks.org/wiki/QEMU/Monitor)を参照するのも良いでしょう。
