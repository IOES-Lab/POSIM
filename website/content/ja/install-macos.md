# macOS：DockerとRDP

Apple Silicon搭載のMacでPOSIMのARM64イメージを起動し、RDPでLinuxデスクトップに接続します。Gazeboと各例のターミナルは、このデスクトップ内で使います。

## 1. DockerとRDPクライアントを準備する

[Docker Desktop for Mac](https://docs.docker.com/desktop/setup/install/mac-install/)をインストールして起動します。接続用に[macOS版Windows App](https://learn.microsoft.com/en-us/windows-app/get-started-connect-devices-desktops-apps)もインストールしてください。

macOSの「ターミナル」で確認します。

{{CODE:install-macos:0}}

Apple Siliconでは`arm64`が表示されます。2つ目のコマンドではDockerサーバーのバージョンが表示されることを確認します。

## 2. イメージを取得して起動する

DockerがARM64版を自動的に選択します。

{{CODE:install-macos:1}}

RDPポートには、このMacからだけ接続できます。自作モデルと記録データは`/work`に保存すると、`posim-data`ボリュームに残ります。バージョンの選択とソースビルドは[Docker環境](docker.md)を参照してください。

## 3. Linuxデスクトップに接続する

Windows Appで**127.0.0.1:3389**をPCとして追加します。xrdpのログイン画面で**Xorg**を選び、ユーザー名**docker**、パスワード**docker**でログインします。

Linuxデスクトップでターミナルを開き、ローカル開発用アカウントのパスワードを変更します。

{{CODE:install-macos:2}}

POSIMのコマンドは、このLinuxデスクトップ内で実行します。macOSのターミナルはDockerの管理に使ってください。[ターミナルの準備](terminals.md)では、ワークスペースの場所も説明しています。

## 4. 停止して再接続する

RDPのウィンドウを閉じてもシミュレーションは続きます。まず実行中のターミナルで**Ctrl+C**を押し、その後macOS側で操作します。

{{CODE:install-macos:3}}

コンテナ起動後は Windows App から再接続します。Linux の描画はソフトウェアで行うため、複雑なシーンでは遅くなる場合があります。ソナーは[パッケージをビルド・確認](sonar-tuning.md#build-the-sonar-packages)してから CPU で始めてください。Metal は macOS ネイティブプロセスで利用でき、この Linux コンテナには提供されません。最初の確認には[カメラと圧力センサーの例](features.md)も使えます。

## 5. デモワールドで動作を確認する

**RDPデスクトップ内のターミナル**で、最後の動作確認を行います。

{{CODE:install-macos:4}}

Gazeboに海のワールドが表示されれば準備完了です。実行中はターミナルを開いたままにしてください。**Ctrl+C**で停止してから、[最初のロボットシミュレーション](quickstart.md)へ進みます。

{{MEDIA:install-macos}}
