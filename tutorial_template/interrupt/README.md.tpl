# 割り込み、例外

1. [CPUによるインストラクションポインタ変更](./jump/)

    割り込みや例外を非常に簡略化すると、「次に実行するインストラクションをCPU側が強制的に変更する事」です。具体的にその例を見てみましょう。

1. [例外からの復帰](./exception_recovery/)

    CPU内で発生したエラーを解消し、例外ハンドラから元の実行パスに復帰してみます。

<!--
MEMO
- レジスタ退避
- I/O割り込み
- プロファイリング、デバッグレジスタ
-->