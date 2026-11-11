# 実行環境を選ぶ

現在のパソコンのOSに合わせて選んでください。Gazeboの描画、ROSのツール、NVIDIA/CUDAデバイスを直接利用できるため、**Ubuntu 26.04へのネイティブインストールを推奨します**。

## インストール方法

| OS | 実行方法 | ガイド |
| --- | --- | --- |
| Ubuntu 26.04 | Ubuntu上でPOSIMを直接ビルドして実行 | [Ubuntuネイティブ](install.md) |
| UbuntuまたはDockerが対応する他のLinux、x86-64 | UbuntuベースのPOSIMイメージを使用し、Linuxデスクトップに表示 | [LinuxのDocker](install-ubuntu-docker.md) |
| Windows 10/11、x86-64 | WSL2のUbuntu 26.04でビルドし、WSLgでウィンドウを表示 | [WindowsのWSL](install-windows.md) |
| Windows 10/11、x86-64 | WSL2対応のDocker DesktopでPOSIMイメージを実行し、WSLgで表示 | [WSLとDocker](install-windows-docker.md) |
| Apple Silicon搭載macOS | ARM64イメージをDockerで実行し、RDPでLinuxデスクトップに接続 | [macOSのDockerとRDP](install-macos.md) |

DockerイメージにはUbuntuのユーザー環境が含まれます。ホスト側のLinuxは、Dockerが対応する別のバージョンやディストリビューションでも利用できます。Intel Mac上でのARM64エミュレーションは対象外です。

## ソフトウェアと描画

どの方法でも**ROS 2 Lyrical**と**Gazebo Sim — Gazebo Jetty**を使用します。ネイティブ環境ではUbuntuにインストールし、Dockerではイメージに含まれるものを使用します。グラフィックスドライバーは各ガイドに従ってホスト側にインストールしてください。

Gazeboとカラー・深度カメラには描画環境が必要です。まずはロボット1台と標準のセンサーで始めましょう。[CUDAマルチビームソナー](sonar-tuning.md)には、対応するNVIDIA GPU、CUDAツール、ソナーのライブラリも必要です。macOSのDockerでは、Apple MetalをLinuxのNVIDIA/CUDAデバイスとして使用できません。

## リソースとダウンロード

必要なリソースは機体数、地形の細かさ、画像解像度、センサー更新頻度によって変わります。設定を増やす前にメモリ、GPU負荷、リアルタイム係数を確認してください。ソース、ビルド結果、Dockerイメージ、Fuelモデル、記録データ用のディスク容量も確保します。

インストールにはROS/Gazeboのパッケージリポジトリ、GitHub、Docker Hubへの接続が必要です。一部のワールドは初回起動時にFuelモデルを取得します。デモワールドが動いたら[ターミナルの準備](terminals.md)へ進んでください。
