# ROS のターミナルとトピック

ターミナル A でシミュレーターを動かし、B でデータの確認や指令の送信を行います。両方で同じ ROS ディストリビューションと POSIM ワークスペースを読み込んでください。

## データの流れを理解する

<div class="docs-flow"><div><strong>ROS プログラムとコマンド</strong><span>購読、サービス、制御入力</span></div><div><strong>ROS インターフェース</strong><span>ros_gz ブリッジまたはプラグインの配信機能</span></div><div><strong>Gazebo と POSIM プラグイン</strong><span>物理、ロボットの状態、計測値</span></div></div>

計測値は ROS プログラムへ、指令はシミュレーターへ送られます。Gazebo のトピックを ROS で使うには、ブリッジまたは ROS 対応プラグインによる配信が必要です。ロボットの起動時に、対応するブリッジ設定が読み込まれます。

## A と B を準備する

インストール方法に合った[ターミナルの準備](terminals.md)を行います。A で[最初のシミュレーション](quickstart.md)を動かしたまま、B で以下のコマンドを実行してください。

以下のトピックはソナーなしの BlueROV2 水中例に対応します。ソナー付きでは `bluerov2` を `bluerov2_heavy_multibeam_sonar` に、水上の例では `wamv` に置き換えます。

## 接続構成を調べる

`--no-daemon` は CLI デーモンのキャッシュを使わず、ROS に直接問い合わせます。

{{CODE:ros:0}}

`ros2 topic list` は ROS のトピック、`gz topic -l` は Gazebo Transport のトピックを表示します。プログラムを接続する前に、メッセージ型、配信元、購読先、QoS を確認します。

## 計測値を観察する

{{CODE:ros:1}}

位置がほぼ一定でも、新しいタイムスタンプのデータが届けば観測は続いています。**B で Ctrl+C** を押すと表示だけが止まり、A のシミュレーションは継続します。1 件だけ受信する場合は `--once` を使います。シミュレーション時刻を使うノードには `use_sim_time:=true` と `/clock` の受信が必要です。

## 別の ROS コンピューターを接続する

互換性のある ROS・メッセージパッケージ、同じドメイン設定、DDS の探索とデータ通信が通るネットワークを用意します。POSIM 独自のメッセージには `posim_interfaces` が必要です。配信元があるのにデータが届かない場合は[トラブルシューティング](troubleshooting.md)を確認してください。

次は [Python による制御](python-control.md)、[スラスターとジョイスティック](robot-control.md)、[表示と記録](visualization.md)へ進みます。
