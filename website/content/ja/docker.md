# Docker環境

POSIMイメージにはUbuntu、ROS 2 Lyrical、Gazebo Jetty、POSIM、ArduSub、MAVROS、QGroundControlが含まれています。画面表示とドライバの準備は、[実行環境の選択](requirements.md)から各OSの手順を確認してください。

## 配布済みイメージを取得する

[Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags)から取得します。DockerがCPUに合ったAMD64版またはARM64版を選びます。

{{CODE:docker:0}}

| タグ | 用途 |
|---|---|
| `latest` | メインブランチから公開されたビルド。AMD64とARM64に対応 |
| `sha-<12-character-commit>` | 特定のソースリビジョン。Docker Hubに存在するタグをコピーして使用 |
| SHAタグの末尾が`-amd64`または`-arm64` | アーキテクチャを明示して選択 |
| 公開されているリリース番号 | リリースタグに対応するバージョン |

イメージのラベルには完全なソースリビジョンが記録されています。同じイメージで実験を再現する場合は、ダイジェストも保存します。

{{CODE:docker:1}}

選択したリビジョンに対応するドキュメントを参照してください。新しいイメージを取得しても、既存のコンテナは更新されません。

## ソースからビルドする

ホスト側で実行します。

{{CODE:docker:2}}

AMD64の場合：

{{CODE:docker:3}}

ARM64の場合：

{{CODE:docker:4}}

OS別のインストール手順では、`ioeslab/posim:latest`を作成したローカルタグに置き換えます。どちらも手元のソースと固定リビジョンの波浪ライブラリをビルドします。ARM64版にはXFCEデスクトップとRDPサーバーも含まれます。

## OSに合った方法で起動する

- [LinuxとDocker](install-ubuntu-docker.md)：Linuxデスクトップの画面に表示します。
- [WindowsとWSL・Docker](install-windows-docker.md)：WSLgでLinuxのウィンドウを表示します。
- [macOSとDocker・RDP](install-macos.md)：コンテナ内のLinuxデスクトップに接続します。

各手順で`posim`という名前のコンテナを作成します。コマンドを入力する場所とワークスペースの読み込み方は、[ターミナルの準備](terminals.md)を参照してください。

## ファイルを保存する

ワークスペースはAMD64版では`/opt/posim_ws`、ARM64版では`/home/docker/posim_ws`にあります。`POSIM_WS`または`POSIM_UNDERLAY`で場所を確認できます。

自作のSDFや記録データは`/work`に保存します。この場所は`posim-data`ボリュームに対応します。それ以外のファイルも停止・再起動では残りますが、コンテナを削除すると消えます。ソースの変更はGitに記録するか、交換前に外部へコピーしてください。ボリュームは明示的に削除するまで残ります。

## 停止・再開・更新 {#update-an-existing-container}

まずシミュレーションのターミナルで**Ctrl+C**を押します。その後、ホスト側で実行します。

{{CODE:docker:5}}

更新する場合は作業内容を保存し、コンテナを停止してから新しいイメージを取得します。停止中のコンテナを削除し、OS別の作成コマンドをもう一度実行してください。`posim-data`を再利用すれば`/work`のファイルを引き継げます。

## グラフィックスとGPU {#graphics-and-gpu-access}

Linuxではホストのドライバと[NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)を設定します。Windowsでは[Docker DesktopとWSL2のGPU設定](https://docs.docker.com/desktop/features/gpu/)に従います。CUDAソナーにはイメージ内のライブラリも必要です。[ソナーの設定](sonar-tuning.md)を参照してください。最初の表示確認はソフトウェアレンダリングを使うため、GPU利用時より遅くなることがあります。
