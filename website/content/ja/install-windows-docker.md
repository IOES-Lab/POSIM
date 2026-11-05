# Windows：WSL と Docker

Docker Desktop で POSIM を動かし、WSLg を通じて Windows デスクトップに Gazebo を表示します。このページの Docker コマンドは、すべて **WSL 内の Ubuntu ターミナル**で実行してください。

## 1. WSL とグラフィックスを準備する

[Windows へのインストール](install-windows.md)の手順 1、2 に従い、WSL2 に Ubuntu 26.04 をインストールして、WSL と **Windows 側のグラフィックスドライバー**を更新します。NVIDIA 製 GPU も同様です。WSL 内に Linux 用ディスプレイドライバーをインストールしないでください。

[WSL2 バックエンドの Docker Desktop](https://docs.docker.com/desktop/features/wsl/) をインストールし、**Settings → Resources → WSL Integration → Ubuntu-26.04** を有効にします。Docker Desktop のエンジンを使用するため、Ubuntu 内に別の Docker Engine は不要です。

サーバーと画面表示用ソケットを確認します。

{{CODE:install-windows-docker:0}}

ソケットが存在しない場合は、先に [WSLg の設定](https://learn.microsoft.com/en-us/windows/wsl/tutorials/gui-apps)を完了してください。

## 2. POSIM をダウンロードする

{{CODE:install-windows-docker:1}}

タグとソースからのビルドは [Docker 環境](docker.md)で説明しています。以下の設定はソフトウェアレンダリングを使用します。対応する NVIDIA GPU を使う場合は、[GPU 利用の前提条件](docker.md#graphics-and-gpu-access)も確認してください。

## 3. コンテナを作成する

{{CODE:install-windows-docker:2}}

自分のファイルは `posim-data` ボリュームに対応する `/work` に保存します。`docker stop posim` で停止し、`docker start posim` で再開できます。

続く例では [Ubuntu ターミナルを 2 つ](terminals.md)開き、両方を同じコンテナに接続します。Gazebo は Windows のウィンドウとして表示されます。

## 4. デモワールドを起動する

コンテナに入ります。

{{CODE:install-windows-docker:3}}

次のコマンドでインストール結果を確認します。

{{CODE:install-windows-docker:4}}

Gazebo に海のワールドが表示されれば準備完了です。実行中はターミナルを開いたままにし、停止するには **Ctrl+C** を押します。次は[最初のロボット](quickstart.md)を動かします。
