# ドップラー速度計（DVL）

DVL は海底または周囲の水に対する速度を推定します。POSIM は Gazebo の DVL システムと ROS ブリッジを使い、`posim_interfaces/msg/DVL` を出力します。

## センサーを起動する

[インストール方法に合わせて準備したターミナル A](terminals.md)で実行します。

{{CODE:dvl:0}}

{{MEDIA:dvl:0}}

準備済みのターミナル B で Gazebo のデータを確認し、**Ctrl+C** で表示を終了してから ROS のデータを確認します。

{{CODE:dvl:1}}

別のセンサーを作る場合は、そのモデルで定義したトピックを使ってください。

## SDF に組み込む

次のシステムを `<world>` の直下に置きます。

{{CODE:dvl:2}}

POSIM ブリッジはセンサーの `<model>` 内に置きます。トピックを `<link>` 内の DVL の `<topic>` と一致させます。

{{CODE:dvl:3}}

[DVL500-300 の完全な SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_sensor_models/description/nortek_dvl500_300/model.sdf)は `type="custom" gz:type="dvl"`、4 本のビーム、追跡モードを定義します。[ワールド](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/dvl_world.world)には海底形状があります。

## パラメーターと単位

`DVLBridge` のパラメーターは `topic` のみで、既定値は `/dvl/velocity` です。同じパスで Gazebo メッセージを受信し、ROS メッセージを出力します。次の項目は Gazebo センサーの設定で、値は DVL500-300 の例です。

| センサーの SDF 項目 | 意味 | 例 |
|---|---|---|
| `topic`, `update_rate` | 出力先と要求周波数、Hz | `/dvl/velocity`, 8 |
| `gz:dvl/type` | 送受波器の配列 | `phased_array` |
| `arrangement` の `aperture`, `rotation`, `tilt` | ビーム角度。`degrees="true"` なら度 | 開き角 3°、回転 −135/135/45/−45°、傾斜 25° |
| `minimum_range`, `maximum_range` | 音響測距範囲、m | 0.3, 200 |
| `reference_frame` | 基準姿勢。位置 m、回転 rad | `0 0 0 3.14 0 -1.57` |
| `tracking/bottom_mode/when`, `tracking/water_mass_mode/when` | 対地・対水モードの選択 | 両方 `best` |
| 追跡の `noise/stddev` | 速度ノイズの標準偏差、m/s | 対地 0.005、対水 0.015 |
| 水塊の `water_velocity` | x/y/z 成分、m/s | 0/0/0 |
| 水塊の `boundaries/near`, `boundaries/far`, `bins` | 計測範囲 m とセル数 | 10, 100, 10 |
| 追跡の `visualize` | 計測形状を表示 | true |

変換の詳細は[ブリッジ実装](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/DVLBridge.cc)にあります。深度センサーは Gazebo の描画を使うため、[実行環境の準備](requirements.md)を済ませてください。

## 利用できるモデル

| モデル名 | 系列 |
|---|---|
| `nortek_dvl500_300`, `nortek_dvl500_6000` | Nortek DVL500 |
| `nortek_dvl1000_300`, `nortek_dvl1000_4000` | Nortek DVL1000 |
| `sonardyne_syrinx600` | Sonardyne |
| `teledyne_explorer1000`, `teledyne_explorer4000` | Teledyne Explorer |
| `teledyne_whn` | Teledyne WHN |
| `nortek_dvl500_300_with_multibeam_sonar` | DVL と [ソナー](sonar-tuning.md) |

各 SDF の計測設定とビーム形状を確認してください。

## 座標系と共分散

この例は `<reference_frame>` で ENU を前方・右舷・下方の座標系に変換します。ブリッジは Gazebo のフレーム名を保持し、区切りの `::` を `/` に置換します。推定器の座標系をそろえ、対地速度と対水速度を区別してください。

合成速度と各ビームは ROS の 6×6 twist 共分散を持ちます。左上の 3×3 が計測された並進速度の共分散で、単位は m²/s² です。残りは角速度を推定しないため 0 となっており、不確かさゼロの計測値を意味しません。

## 観測値を確認する

`ros2 interface show posim_interfaces/msg/DVL` で定義を確認します。停止中と制御された移動中に速度、座標系、状態を調べます。海底形状、距離、ビームの交差位置が計測に影響します。モデルの `visualize` を有効にするとビームを表示できます。

## DVL のビームと出力

{{MEDIA:dvl:1}}
{{MEDIA:dvl:2}}
{{MEDIA:dvl:3}}
{{MEDIA:dvl:4}}

画像の出典：POSIM Notion Wiki。DAVE の帰属は[引用とライセンス](citation.md)を参照してください。実行にはこのページのコマンドとトピック名を使います。
