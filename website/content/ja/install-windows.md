# Windows：WSLへのインストール

Windows Subsystem for Linux 2（WSL2）上のUbuntu 26.04にPOSIMをインストールします。GazeboのウィンドウはWSLgによってWindowsデスクトップに表示されます。構築済みイメージを使う場合は[WSLとDocker](install-windows-docker.md)を選んでください。

## 1. WSLとUbuntuをインストールする

管理者としてPowerShellを開き、次を実行します。

{{CODE:install-windows:0}}

再起動を求められたらWindowsを再起動します。Ubuntuを開き、Linuxのユーザー名とパスワードを設定してください。PowerShellで、UbuntuがWSL 2を使用していることを確認します。

{{CODE:install-windows:1}}

既存のWSLを利用する場合やインストールに失敗する場合は、[Microsoftの手順](https://learn.microsoft.com/ja-jp/windows/wsl/install)を参照してください。

## 2. グラフィックスドライバーとWSLgを準備する

Intel・AMD・NVIDIA GPUの**Windows用ドライバー**をインストールまたは更新します。NVIDIAの場合も必要です。WSLgはWindows側のドライバーでLinuxのGUIを描画します。[GUIアプリの設定手順](https://learn.microsoft.com/ja-jp/windows/wsl/tutorials/gui-apps)を確認してください。

NVIDIA では [CUDA on WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html) に従い、Windows 用ドライバーを導入します。**WSL 内に Linux 用ディスプレイドライバーをインストールしないでください。** CUDA Toolkit が必要なのは、ソナーの CUDA バックエンドをビルドする場合です。

PowerShellでWSLを更新します。

{{CODE:install-windows:2}}

再起動が必要な場合は、WSL内で作業を保存してから実施し、その後Ubuntuを開き直します。

## 3. UbuntuのターミナルでPOSIMをインストールする

LinuxのコマンドはWindows Terminalの**Ubuntu**プロファイルで実行します。ディストリビューションを確認してください。

{{CODE:install-windows:3}}

[Ubuntuへのネイティブインストール](install.md)のソース取得からビルドまでを実行します。ワークスペースはLinuxのホームディレクトリ内の`~/posim_ws`に作成してください。インストーラーは`sudo`を使い、Ubuntu内のパッケージを更新します。

画像トピックは見えるのに画像が届かない場合は[トラブルシューティング](troubleshooting.md)を参照してください。以降の例で使う2つのウィンドウは、[ターミナルの準備](terminals.md)で説明しています。

## 4. デモワールドを起動する

Ubuntuのターミナルで実行します。WindowsデスクトップにGazeboが開き、海のシーンが表示されれば成功です。初回はFuelモデルをダウンロードすることがあります。別の例を起動する前にCtrl+Cで停止してください。

{{CODE:install-windows:4}}
