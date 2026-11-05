# ターミナルの準備

**ターミナル**は、コマンドを入力して実行結果を確認するウィンドウです。このガイドでは、通常のターミナルを2つ開き、シミュレーションを実行する方を**ターミナルA**、センサーデータの確認や操作に使う方を**ターミナルB**と呼びます。AとBは役割を表す名前で、別のアプリではありません。

## インストール方法に合った場所で開く

| インストール方法 | ターミナルA・Bの開き方 |
|---|---|
| [Ubuntuへのネイティブインストール](install.md) | Ubuntuの「端末」を2つ開く |
| [LinuxのDocker](install-ubuntu-docker.md) | Linuxのターミナルを2つ開き、両方から同じコンテナに入る |
| [WindowsのWSL](install-windows.md) | Windows Terminalで同じWSLディストリビューションのUbuntuタブを2つ開く |
| [WindowsのWSLとDocker](install-windows-docker.md) | Ubuntuタブを2つ開き、両方から同じコンテナに入る |
| [macOSのDockerとRDP](install-macos.md) | RDPで接続したLinuxデスクトップ内でターミナルを2つ開く |

Linux・WindowsのDocker環境では、ホスト側の各ターミナルで次を実行し、`posim`コンテナに入ります。

{{CODE:terminals:0}}

macOSで利用するRDPデスクトップは、すでにコンテナ内にあります。ROSのコマンドはそのLinuxターミナルで実行してください。PowerShellやmacOSのターミナルは、コンテナの起動などホスト側の操作に使います。

## ワークスペースとは

**ROSワークスペース**は、ソースコードとビルドしたパッケージをまとめるディレクトリです。`src/`にソースコード、`build/`にビルド途中のファイル、`install/`にROSが読み込むパッケージが入ります。

| インストール方法 | パス |
|---|---|
| Ubuntu・WSLへのネイティブインストール | `~/posim_ws` |
| AMD64 Dockerイメージ | `/opt/posim_ws` |
| ARM64 RDPイメージ | `/home/docker/posim_ws` |

`~`はLinuxユーザーのホームディレクトリを表します。以下の`POSIM_WS`は、ワークスペースのパスを保持する変数です。

## AとBの両方で環境を読み込む

**Ubuntu・WSLへのネイティブインストールの場合：**

{{CODE:terminals:1}}

**Dockerの場合（RDPデスクトップ内も同じ）：**

{{CODE:terminals:2}}

`source`は、ROSやPOSIMを見つけるためのパスと設定をターミナルに読み込むコマンドです。新しいターミナルを開くたびに実行してください。

## 起動前に確認する

{{CODE:terminals:3}}

どちらもワークスペース内の`install/`を指していれば準備完了です。パッケージが見つからない場合はインストール手順を確認してください。`ros_gz_sim`が`/opt/ros/lyrical`を指す場合、ワークスペース側のパッケージが読み込まれていません。

次は[初めてのシミュレーション](quickstart.md)です。メッセージを出力し続けているコマンドは実行中です。**そのターミナルでCtrl+C**を押して停止し、入力待ちの表示に戻ってから次のコマンドを実行します。
