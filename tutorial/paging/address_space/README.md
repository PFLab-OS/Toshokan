
# 仮想メモリと物理メモリ

## サンプル
本リポジトリをクローンし、このREADME.mdがあるディレクトリ上で`make`する事で、サンプルプログラムを実行できます。

ひとまず、`make`を実行してみましょう。

```
$ make
Silent mode is enabled by default. You can turn it off with 'make V=1'.
>>> build friend binary
　　（中略）
>>> send the binary to remote
>>> run hakase.bin on remote
ready
```

上記のような出力が得られ、readyが表示されたらOKです。以下の操作は、readyを表示させたまま、別のターミナル上で行ってください。

## QEMUモニタ上での物理メモリの確認



## QEMUモニタ上での仮想メモリの確認


## 補足
`make`を実行したターミナル（readyと表示されたまま停止しているターミナル）は、Ctrl+Cで終了する事ができます。