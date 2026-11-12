# 水上ロボットと波

Wave Sim の波と ArduRover のボートモードを使って WAM-V を動かします。標準の POSIM インストーラーと Docker ビルド手順には、外部の波ライブラリのビルドが含まれます。

## 標準チュートリアル

標準の POSIM には、カメラ、ライダー、設定済み RViz を備えた WAM-V のブイコースが含まれます。以下の追加イメージは不要です。[準備済みの POSIM ターミナル](terminals.md)で実行します。

{{CODE:quickstart:4}}

シーン、ライダー表示、QGroundControl の映像と操作は[最初のシミュレーション](quickstart.md)を参照してください。

## WAM-V を準備する

この追加イメージは ARM64 のデスクトップ構成を使います。Apple Silicon Mac で [Docker と RDP](install-macos.md)を準備し、POSIM リポジトリのコピーから macOS のターミナルで実行します。

{{CODE:surface:0}}

RDP で **127.0.0.1:3390** に `docker` アカウントで接続します。Linux デスクトップ内のターミナルで実行してください。

{{CODE:surface:1}}

イメージには ArduSub と ArduRover の別々の実行ファイルがあります。この例は `FRAME_CLASS=2` の ArduRover を使います。車両名、生成ファイル、制御ポートが固定のため、**1 環境につき 1 インスタンス**を実行してください。複数艇を動かすには、ファイル、名前、制御 ID、ポートを分けるか、独立した環境を用意します。

制御器はモデルの生成と IMU の出力を待ちます。生成に失敗するか 180 秒を超えた場合、または生成後 180 秒以内に IMU が届かない場合は、該当段階を表示してエラー終了します。再実行前に Gazebo とセンサーのメッセージを確認してください。

## モデル、制御、航法

`extras/surface/wamv.py` が外部モデルを組み込みます。船尾の 2 基のプロペラは出力 1 と 3 で駆動し、艇は水面から開始します。

外部航法アダプターは **Gazebo が計算した位置・姿勢と速度**を MAVROS に送ります。ミッションを送る前に、制御器の状態、航法データの有効性、推力の向きを確認してください。

受信機のノイズ、浸水、再捕捉を調べる場合は、[海洋 GNSS](gnss.md)とアンテナ、適切な航法アダプターを追加します。この例の外部姿勢アダプターとは別の航法入力です。

## 波を調整する

`/world/<world_name>/waves` に `gz.msgs.Param` を送ります。名前には SDF の `<world name="…">` を使います。動作中のトピックを一覧表示します。

{{CODE:surface:2}}

[WAM-V 起動ファイル](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/wamv.launch.py)の既定名は `posim_surface` で、波のトピックは `/world/posim_surface/waves` です。準備したコンテナ内で名前を変える例です。

{{CODE:surface:3}}

波のトピックは `/world/harbour/waves` になり、ROS の IMU は `/model/wamv/imu` のままです。`world_name` は生成するワールドの名前を設定します。地形ファイルの選択や別インスタンスの分離には使いません。

海底は z = −10 m の平面です。変更する場合は[ハイトマップによる地形](heightmaps.md)を参照してください。

| パラメーター（`double`） | 単位 | 用途 |
|---|---|---|
| `wind_speed` | m/s | 波モデルの風速 |
| `wind_angle` | 度 | 風向 |
| `steepness` | 無次元 | 波の急峻度 |

描画と流体力学の設定を一致させます。[海流](currents.md)は別途設定します。ライブラリのインストールに加えて、ワールドに波のシステムを組み込み、設定する必要があります。

## 制約と依存関係

追加 Dockerfile は ARM64 構成に対応します。船体、推進、抵抗のパラメーターは対象艇と運用条件に合わせて調整してください。終了時に MAVROS や Gazebo が監視プロセスによる停止を必要とする場合があります。次の起動前に両方が終了したことを確認します。

外部ライブラリのリビジョンは `extras/surface/dependency.json` で固定します。インストーラーは `/opt/waves` にビルドし、ソースを `/opt/asv_wave_sim` に保存します。Git サブモジュールではなくビルド依存関係です。[統合ガイド](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md)と[ライセンス](citation.md)も参照してください。
