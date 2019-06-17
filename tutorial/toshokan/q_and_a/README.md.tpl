# 困った時は

### dockerの実行でエラーが出ます。

```
Cannot connect to the Docker daemon at unix:///var/run/docker.sock. Is the docker daemon running?
```

上記のようなエラーが出る場合、dockerコンテナを実行できる権限が足りてない可能性があります。以下のいずれかで対処可能です。

- sudo等を用い、root権限で実行する。
- ユーザをdockerグループに所属させる。（詳細はググって）

### dockerのオプションでエラーが出ます。

aptやyum等のパッケージマネージャでdockerをインストールしている場合、古いバージョンのdockerがインストールされている可能性があります。アンインストールした後、公式HPから最新のdockerをインストールしてください。

### さっきまで動いていて、コードを変更してないのに、突然エラーが出たり、途中で止まったりします。

とりあえずdockerの再起動、それでもダメならマシンの再起動を試してみましょう。

### friendバイナリが意図した通りに動きません。

function offloadingを用いてprintfデバッグするか、QEMUモニタを用いてデバッグしましょう。

### QEMUモニタからレジスタを確認すると、滅茶苦茶な値を示しています。

キャッチできない例外する等が発生して、CPUリセットが掛かっているのかもしれません。多くの場合、friend上で割当領域外のメモリにアクセスしている事が原因と思われます。また、0除算やページテーブルの権限設定等によっても同様の事象が発生します。詳細はIntel Software Developer's ManualのEXCEPTION AND INTERRUPT REFERENCEの章を参照してみてください。

### バグを見つけました。

pull requestを出すか、[@liva_jy](https://twitter.com/liva_jy)に報告してください。たぶんLivaが喜びます。