# 概要

POSIMは、ROS 2とGazeboを使うオープンソースの海洋ロボティクスシミュレーションライブラリです。海洋環境、水上・水中ロボット、センサー、制御実験を実行できます。[UUV Simulator](https://uuvsimulator.github.io/)を基盤とする[Project DAVE](https://field-robotics-lab.github.io/dave.doc/)から派生しています。

{{TUTORIALS}}

{{OVERVIEW}}

## まずはここから

[実行環境を選び](requirements.md)、[Ubuntuへのネイティブインストール](install.md)、[LinuxのDocker](install-ubuntu-docker.md)、[WindowsのWSL](install-windows.md)、[WSLのDocker](install-windows-docker.md)、または[macOSのDockerとRDP](install-macos.md)の手順を進めてください。Ubuntuへのネイティブインストールを推奨します。

インストール後は[ターミナルを準備](terminals.md)し、[初めてのシミュレーション](quickstart.md)を実行します。

- ROS：**ROS 2 Lyrical** — LTS、サポート終了：[2031年5月](https://docs.ros.org/en/lyrical/Releases/Release-Lyrical-Luth.html)
- Gazebo Simシミュレーター：**Gazebo Jetty** — LTS、サポート終了：[2031年5月](https://gazebosim.org/docs/latest/releases/#release-list)

## シミュレーションできること

[機能とサンプル](features.md)では、各機能の実装と使い方を紹介しています。

| 目的 | ガイド |
| --- | --- |
| 海洋環境と海底を準備する | [ワールド](worlds.md)、[海流](currents.md)、[水中表現](underwater-rendering.md) |
| ロボットを選ぶ | [ROV](rovs.md)、[Slocum](gliders.md)、[水上ロボット](surface.md) |
| センサーを確認する | [カメラ](camera.md)、[GNSS](gnss.md)、[水圧](pressure.md)、[DVL](dvl.md)、[USBL](usbl.md)、[ソナー](sonar.md) |
| 制御や記録を行う | [ROS](ros.md)、[Python](python-control.md)、[スラスターとジョイスティック](robot-control.md)、[センサーデータ](visualization.md) |

## シミュレーションの構成

1. ワールドのSDFで環境とシステムを定義します。
2. モデルのSDFでリンク、衝突形状、慣性、センサーを記述します。
3. launchファイルでGazebo、モデル、ROSブリッジを起動します。
4. ROSノードがセンサーデータを受信し、制御指令を送信します。

独自の実験には[ワールド作成](custom-worlds.md)、[ロボット追加](custom-robots.md)、[センサー設定](custom-sensors.md)を参照してください。各プラグインのリファレンスには、クイックスタート、SDFの記述例、パラメーターを掲載しています。

## WWW-POSIM

[WWW-POSIM](https://www-posim.vercel.app/)は、POSIMに地理情報に基づく地形を生成するPOSIM-Terrain、経路を計画するPOSIM-Routing、機体を制御するPOSIM-Controlを組み合わせたアプリケーションです。GUIとWeb・デスクトップアプリを加え、シーンの準備、ロボット操作、シミュレーションデータの確認を画面上で行えます。

以下の動画で主な機能を紹介しています。インストールと操作方法は[WWW-POSIMのドキュメント](https://www-posim.vercel.app/guide/en.html)を参照してください。

{{INTRO_VIDEO}}

[YouTubeで紹介動画を見る](https://www.youtube.com/watch?v=_cQtl5JzN2E)。
