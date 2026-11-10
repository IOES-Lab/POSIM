# Linux：Dockerでインストールする

Ubuntuのバージョンが異なる場合や、別のLinuxディストリビューションを使う場合は、配布済みのPOSIMイメージを利用できます。この手順は**x86-64デスクトップのAMD64版**を対象とし、X11またはXWaylandで表示します。Ubuntu 26.04では[ネイティブインストール](install.md)を推奨します。

## 1. Dockerと画面表示を準備する

使用するディストリビューションに合わせて[Docker Engine](https://docs.docker.com/engine/install/)をインストールします。[Ubuntu向けの手順](https://docs.docker.com/engine/install/ubuntu/)もあります。自分のアカウントで`docker`を実行できない場合は、[インストール後の設定](https://docs.docker.com/engine/install/linux-postinstall/)を行ってください。

デスクトップのターミナルを開き、`xauth`をインストールします。Ubuntuでは次のように実行します。

{{CODE:install-ubuntu-docker:0}}

ディスプレイのアドレスとDockerサーバーのバージョンを確認します。

{{CODE:install-ubuntu-docker:1}}

## 2. POSIMを取得する

{{CODE:install-ubuntu-docker:2}}

リビジョンの選択やソースビルドは[Docker環境](docker.md)を参照してください。最初の起動はソフトウェアレンダリングを使うため、GPUを使ったネイティブ環境より遅くなることがあります。

## 3. コンテナからの表示を許可する

現在のセッションのX11認証情報を、自分のアカウントだけが読めるファイルにコピーします。

{{CODE:install-ubuntu-docker:3}}

最後のコマンドはファイルにデータがあることを確認します。失敗した場合は、現在のデスクトップセッションのターミナルを使っていることと、XWaylandが利用できることを確認してください。認証情報を用意してから次へ進みます。

## 4. コンテナを作成する

{{CODE:install-ubuntu-docker:4}}

自作モデルと記録データは`/work`の`posim-data`ボリュームに保存します。停止は`docker stop posim`、再開は`docker start posim`です。Linuxに再ログインした後で表示できなくなった場合は、認証ファイルを更新してください。

各例を実行するときは、同じコンテナにつながる[2つのターミナルを準備](terminals.md)します。ハードウェアアクセラレーションやCUDAソナーを使う前に、[GPU設定](docker.md#graphics-and-gpu-access)を確認してください。

## 5. デモワールドを起動する

コンテナ内のターミナルを開きます。

{{CODE:install-ubuntu-docker:5}}

その中で最後の動作確認を行います。

{{CODE:install-ubuntu-docker:6}}

Gazeboに海のワールドが表示されます。実行中はターミナルを開いたままにし、停止するときは**Ctrl+C**を押します。続いて[最初のシミュレーション](quickstart.md)へ進んでください。
