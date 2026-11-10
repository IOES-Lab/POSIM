# スラスターとジョイスティックによる制御

ロボットに合う指令インターフェースを選びます。先にセンサーデータを確認し、制御器は一度に 1 つだけ有効にしてください。

| 機体 | 指令の経路 | ガイド |
|---|---|---|
| REXROV | ROS の力指令 → Gazebo スラスター | [Python](python-control.md) |
| BlueROV2 | ROS／ジョイスティック → MAVROS → ArduSub | [QGC と映像](qgroundcontrol.md)・[ROS／ジョイスティック](rovs.md) |
| WAM-V | 水上機体の統合機能と制御器 | [水上ロボット](surface.md) |

## REXROV のスラスター

[準備済みターミナル](terminals.md)の A で REXROV を動かし、B から指令を送ります。設定には、`std_msgs/msg/Float64` 型としてブリッジされる 8 つの `cmd_thrust` トピックがあります。値の単位はニュートンです。スラスター 1 を中立にする指令は次のとおりです。

{{CODE:robot-control:0}}

各スラスターの位置と軸に合わせて力を配分します。機体の速度制御には複数のスラスターの協調が必要です。終了時は、使用したすべてのスラスターにゼロを送ります。

## BlueROV2 と ArduSub

[BlueROV2 の例](rovs.md#start-bluerov2)を起動し、オートパイロットの接続を待ってインターフェースを確認します。

{{CODE:robot-control:1}}

Durability の `volatile` は新しく配信される状態を対象にし、フィルターは接続確立後の `connected: true` を待ちます。待機が続く場合は、MAVROS と ArduSub の起動出力、接続設定を確認してください。

アダプターは `/joy` と `/keyboard/joy` を MAVROS 指令に変換します。

- 配信周波数：**20 Hz**。
- 入力タイムアウト：**0.3 秒**。

指令元を 1 つに絞り、アーム状態と航法状態を確認します。キーボードと WebSocket の設定は [ROV と BlueROV2](rovs.md)で説明しています。
