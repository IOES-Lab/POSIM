# マルチビームソナー

POSIM のマルチビームソナーは、点散乱モデルで距離ごとの反射強度とソナー画像を生成します。同じプラグインで CPU、WGPU、CUDA による計算を選べます。WGPU は Linux の Vulkan、または macOS ネイティブ環境の Metal を通じて、複数メーカーの GPU を利用します。[計算環境の準備と選択](sonar-tuning.md)を参照してください。

## サンプルを動かす {#run-a-sonar-example}

[ターミナル A と B](terminals.md)を準備し、未導入なら[ソナーパッケージをビルド](sonar-tuning.md#build-the-sonar-packages)します。A でシミュレーションを起動し、B で出力を確認します。最初のコマンドはバックエンドを自動選択します。専用の例は WGPU を指定し、RViz も起動します。

{{CODE:sonar:0}}

`posim_multibeam_sonar_demo`にも独立したサンプルがあります。

{{CODE:sonar:1}}

例は 1 つずつ実行してください。`wgpu` を `cpu`、`cuda`、`auto` に替えると計算方法を選べます。ロボット搭載の例では、適切なソナーワールドで `bluerov2_heavy_multibeam_sonar` を選びます。

## 出力を確認する

ターミナルBで実行します。

{{CODE:sonar:2}}

- 画像のQoSは`reliable`です。RVizのImage表示も同じ設定にします。
- `/sensor/multibeam_sonar/point_cloud`にはPointCloud2表示を追加します。
- 生の音響データは`marine_acoustic_msgs/msg/ProjectedSonarImage`です。
- BlueViewの起動設定はGazeboの時刻をROSの`/clock`へ橋渡しします。`use_sim_time`を使うノードでは、ワールドが動作中で時刻が進んでいることを確認してください。

## センサーの設定 {#sensor-configuration}

次のシステムを`<world>`直下に追加します。

{{CODE:sonar:3}}

P900センサーはモデルの`<link>`内に配置します。

{{CODE:sonar:4}}

この例は、水平方向512ビーム・約130°、垂直方向300本の音線・約12°、距離範囲0.1–10 mを指定します。周波数は900 kHz、帯域幅は29.9 kHz、音速は1500 m/sです。30 Hzは要求する更新頻度であり、実際の受信頻度は計算量と描画処理に左右されます。全体の設定は[センサーモデル](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/blueview_p900/model.sdf)を参照してください。

## `<spec>`のパラメーター

| パラメーター | 意味 | コードの既定値 | P900の例 |
| --- | --- | --- | --- |
| `verticalFOV` | 音響ビームの垂直幅、度 | 10 | 12 |
| `sonarFreq` | 音響周波数、Hz | 900000 | 900000 |
| `bandwidth` | 信号の帯域幅、Hz | 29500000 | 29900 |
| `soundSpeed` | 音速、m/s | 1500 | 1500 |
| `sourceLevel` | 音源レベル、dB | 220 | 220 |
| `maxDistance` | 処理対象の距離、m。音線の範囲とそろえる | 60 | 10 |
| `raySkips` | 垂直方向のサンプリング間隔。正の整数 | 10 | 10 |
| `sensorGain` | 読み込み・表示のみで、画像生成には使われない | 0.02 | 0.02 |
| `blazingSonarImage` | 実際の時計から乱数シードを生成。`false`では1234に固定 | false | true |
| `pointCloudTopicName` | Gazeboの点群トピックの接尾辞 | `point_cloud` | 同じ |
| `sonarImageRawTopicName` | ROSの生データトピックの接尾辞 | `sonar_image_raw` | 同じ |
| `sonarImageTopicName` | ROSの画像トピックの接尾辞 | `sonar_image` | 同じ |
| `frameName` | 光学座標系のID | `forward_sonar_optical_link` | 同じ |
| `writeLog` | 実行ごとの専用ディレクトリにCSVを保存 | false | false |
| `writeFrameInterval` | CSVを保存する間隔、フレーム数。正の整数 | 10 | 5 |
| `debugFlag` | コンソールへの診断出力 | false | false |
| `plotScaler` | 読み込み・表示のみで、画像生成には使われない | 10 | 省略 |

### トピックと座標系

接尾辞はセンサーの`<topic>`に付加されます。この例では`/sensor/multibeam_sonar`です。Gazeboの点群型は`gz.msgs.PointCloudPacked`で、付属のROSブリッジが`sensor_msgs/msg/PointCloud2`へ変換します。`pointCloudTopicName`を`sensor_config.py`と一致させ、`frameName`にソナーの光学座標系を指定してください。

### 音線の配置

`<ray>`の既定値は、水平方向256ビーム、垂直方向3本の音線、水平角±π/4、垂直角±π/8、距離範囲0.1–5 mです。角度はラジアンで指定し、`degrees="true"`の場合は度を使います。`<gz:multibeam_sonar>`内の任意項目`<reference_frame>`は、位置と回転が0の姿勢を既定値とします。

`<ray>`と`<spec>`の両ブロックを残してください。既定値はブロック内の各項目に適用されます。音線の距離範囲と`maxDistance`をそろえます。パラメーターの読み込みと出力の詳細は[実装](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar/MultibeamSonarSensor.cc)で確認できます。

### 実験条件をそろえる

処理性能を測るときは、記録処理も実験対象である場合を除いてCSV保存を無効にします。[ソナーの設定と調整](sonar-tuning.md)を参照し、比較する実験では音響設定と`blazingSonarImage`の値を記録してください。

### 計算バックエンド {#compute-backend}

| 選択値 | 動作 |
| --- | --- |
| `auto` | CUDA、WGPU、CPU の順で利用可能なものを選択。既定値 |
| `wgpu` | 対応する Vulkan/Metal アダプターを使用。利用できなければセンサーの初期化に失敗 |
| `cuda` | NVIDIA GPU とビルド・インストール済みの CUDA バックエンドが必要。利用できなければ初期化に失敗 |
| `cpu` | 音響反射を CPU で計算。Gazebo の描画環境は引き続き必要 |

専用の起動コマンドでは `compute_backend` を指定します。ほかの起動コマンドは環境変数 `POSIM_SONAR_COMPUTE_BACKEND` を読みます。起動時の `Using sonar compute backend:` で実際の選択を確認してください。[ソナーの設定と性能調整](sonar-tuning.md)では、アダプターの選択、動作条件、数値計算の違いを説明します。

## センサーを追加する

1. `models/posim_sensor_models/description/blueview_p900/`を新しいセンサー名でコピーします。
2. 対応する`config/<センサー名>/sensor_config.py`を用意し、モデル・リンク・座標系の名前とブリッジのトピックをそろえます。
3. サンプル数、角度、距離範囲を設定します。ワールドに上記の2つのシステムを組み込みます。その他の設定は[ソナー用ワールドのSDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_multibeam_sonar.world)を参照してください。
4. 計算バックエンドと描画環境を準備し、ワークスペースを再ビルドして環境を読み込みます。[起動引数](quickstart.md)で新しいセンサーを選びます。

## 生データを保存・表示する

`writeLog=true`では、センサーごとにシミュレーターの作業ディレクトリ内へ`SonarRawData_<run>_<suffix>/`を作成し、そのパスを表示します。連番のCSVと`SonarRawData_beam_angles.csv`を保存し、既存の記録は上書きしません。保存間隔が1未満なら1に補正します。ディレクトリを作成できない場合は記録を無効にしてエラーを出力します。

表示されたディレクトリへ移動し、最初のフレームを開きます。

{{CODE:sonar:6}}

PNGとして保存する場合：

{{CODE:sonar:7}}

[表示ツール](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_gz_multibeam_sonar/multibeam_sonar_demo/scripts/plotdata.py)はCUDAなしでもインストールできます。ソナー画像と、等間隔に選んだ5本のビームのプロファイルを生成します。ビームが5本未満ならすべて表示します。

| オプション | 内容 |
| --- | --- |
| `--range-max` | 表示する最大距離、m |
| `--beam-step` | N本おきにビームを表示 |
| `--dynamic-range` | 最大値から下の表示範囲、dB。既定値60 |

色は各フレームの最大値に対する相対値です。実験間の比較にはCSVの値を使ってください。最初の列は距離、残りはビームごとの複素反射データです。対応するビーム角度ファイルを一緒に保管し、ディスク使用量も確認してください。

## シーンと観測例

{{MEDIA:sonar}}

図・動画：POSIM Notion Wiki。DAVEとソナーモデルの出典は[引用とライセンス](citation.md)を参照してください。コマンドとトピック名は、このページのコードブロックを使用します。
