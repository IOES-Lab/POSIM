# QGroundControl で操作する

QGroundControl（QGC）を BlueROV2 に接続し、仮想ジョイスティックで操作しながら前方カメラの映像を確認します。[最初のシミュレーション](quickstart.md)を試してから、[ターミナル A と B](terminals.md)を準備してください。

## 1. POSIM と同じ Linux 環境で QGC を開く

| インストール方法 | QGC の実行場所 |
|---|---|
| Ubuntu ネイティブ | Ubuntu に Linux 版アプリをインストールして実行します。 |
| Windows の WSL ネイティブ | POSIM と同じ Ubuntu ディストリビューションに Linux 版をインストールして実行します。 |
| Linux・Windows の Docker | POSIM コンテナ内の同梱アプリを実行します。 |
| macOS の Docker・RDP | RDP デスクトップのターミナルから同梱アプリを実行します。 |

ネイティブ環境では、[QGC 公式インストールガイド](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/getting_started/download_and_install.html)に従って CPU アーキテクチャに合う Linux 版を入れます。Docker ではターミナル B で実行します。

{{CODE:qgroundcontrol:0}}

末尾の `&` により、QGC を開いたまま次のコマンドを入力できます。別のターミナルから起動した場合は、そのターミナルも開いておきます。初回の設定画面が表示されたら、単位を **Metric System** にしてください。

QGC、MAVROS、Gazebo は同じ Linux 環境内で通信します。テレメトリーは **UDP 14550**、映像は **UDP 5600** を使うため、この構成では Docker の追加ポート公開は不要です。macOS からは RDP でこの環境に接続します。

## 2. 仮想ジョイスティックを設定する

**Application Settings → Fly View → Virtual Joystick** で **Virtual Joystick** と **Auto-Center Throttle** をオンにし、**Left-Handed Mode** はオフにします。スティックを離すと指令が中央に戻る設定です。

{{MEDIA:qgroundcontrol:0}}

## 3. 映像の受信を設定する

**Application Settings → Video** を開きます。

| 設定項目 | 値 |
|---|---|
| Video Source | UDP h.264 Video Stream |
| UDP URL | `127.0.0.1:5600` |
| Aspect Ratio | `0`（縦横比を自動設定） |
| Disable Video Stream When Disarmed | オフ |
| Auto-reconnect | オン |

{{MEDIA:qgroundcontrol:1}}

## 4. ロボットと映像配信を開始する

前のシミュレーションを停止し、準備済みのターミナル A で実行します。

{{CODE:qgroundcontrol:1}}

Gazebo に海底のシーンが開き、QGC が ArduSub に接続します。初期化が終わるのを待ち、ターミナル B で映像配信を開始します。

{{CODE:qgroundcontrol:2}}

**Fly View** に戻り、小さな映像プレビューをクリックして拡大します。

{{MEDIA:qgroundcontrol:2}}

カメラは**シミュレーション時間で 5 Hz、640 × 480 ピクセル**の画像を生成します。ソフトウェアレンダリングでは実際の表示速度が下がることがあります。BlueROV2 は正の浮力を持つため、アームしていなくても上昇し、対象物が画面外に移る場合があります。

`qgc_video:=true` は、この起動に H.264 送信機能を追加します。配信するのは通常の RGB カメラです。[水中カメラフィルター](camera.md)は別の ROS 出力を持ちます。

## 5. アームして操作し、停止する

1. **Ready** を確認し、**Manual** を選びます。
2. 機体の状態メニューを開き、操作が完了するまで **Arm** を長押しします。
3. 右スティックを短時間だけ前後に動かします。
4. スティックを離すと指令が中立に戻ります。海流や慣性による移動は続くことがあります。
5. 終了時は状態メニューの **Disarm** を長押しします。

指令の競合を防ぐため、キーボード操作と Web ジョイスティックは無効のままにします。ターミナル B で映像送信を停止します。

{{CODE:qgroundcontrol:3}}

ターミナル A で **Ctrl+C** を押し、QGC を閉じます。

## 接続や映像を確認できない場合

- **機体が表示されない：** QGC と POSIM が同じ Linux 環境で動いているか、別の QGC が UDP 14550 を使用していないか確認します。
- **Waiting for video のまま：** H.264 の入力、UDP URL、`qgc_video:=true` を確認し、B から送信を有効にして数フレーム待ちます。
- **別のポートを使う：** `qgc_video_port:=5602` と QGC の `127.0.0.1:5602` を対応させます。
- **`qgc_video` がない：** POSIM を更新するか、[新しいイメージでコンテナを作り直します](docker.md#update-an-existing-container)。
- **ファームウェアやシリアルポートの通知：** このシミュレーションは同梱の ArduSub とネットワーク接続を使います。ファームウェアの書き込みやシリアル接続の設定は不要です。

[QGC 公式の映像設定](https://docs.qgroundcontrol.com/master/en/qgc-user-guide/settings_view/video.html)と[スラスターの ROS インターフェース](robot-control.md)も参照してください。
