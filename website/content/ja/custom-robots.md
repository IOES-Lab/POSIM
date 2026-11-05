# ロボットを追加する

構成が似ている POSIM ロボットを出発点にします。SDF は機構とシステムを記述し、ROS 設定は計測と指令を結び付けます。

## 作業環境

Ubuntu ネイティブ、WSL、Docker の[準備済み Linux ターミナル](terminals.md)を使います。macOS では RDP デスクトップ内で作業します。編集とビルドもその環境内で行い、`POSIM_WS` にはワークスペースのパスを設定します。

## 1. 参考モデルをコピーする

[REXROV](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/rexrov) は Gazebo のスラスタを使います。[BlueROV2](https://github.com/IOES-Lab/POSIM/tree/main/models/posim_robot_models/description/bluerov2) は ArduSub/MAVROS と連携します。[Slocum](gliders.md)では異なる機体構成を確認できます。

メッシュ、ブリッジ設定、トピック名をまとめて扱えるよう、`$POSIM_WS` で REXROV を一式コピーします。

{{CODE:custom-robots:0}}

次のプログラムを `copy_robot.py` として保存し、`python3 copy_robot.py` で実行します。`my_robot` が既に存在する場合は停止します。

{{CODE:custom-robots:1}}

作成される構成は次のとおりです。

{{CODE:custom-robots:2}}

`my_robot` は任意に選ぶ識別子です。起動ファイルは `description/<namespace>/model.sdf` と対応する設定を探します。

## 2. 機構とリソースを定義する

最初の起動ではコピー元の機構を維持します。船体を変更する際は、次をまとめて確認してください。

| SDF のパス | 単位 | 確認事項 |
|---|---|---|
| `model/@name`、リンク名、ジョイント名 | 識別子 | プラグインと ROS 設定の名前に一致 |
| `link/inertial/mass` | kg | 動く各リンクの質量が正であること |
| `link/inertial/pose` | m, rad | 重心と慣性座標系の向き |
| `link/inertial/inertia/ixx`, `iyy`, `izz` | kg·m² | 慣性座標系の各軸まわりの慣性モーメント |
| `link/inertial/inertia/ixy`, `ixz`, `iyz` | kg·m² | 非対角成分。テンソル全体の物理的な妥当性 |
| `link/visual`, `link/collision` | m, rad | メッシュの縮尺と形状の一致 |
| `joint/axis`、スラスタ設定 | プラグインによる | 駆動方向と推力配分 |

STL が提供するのは形状です。質量と力学は SDF に設定し、メッシュの単位と縮尺を確認します。既存の例に従ってインストール済みリソースの URI を使います。複雑な表示形状が接触計算を重くする場合は、衝突形状を簡略化してください。

## 3. システムと ROS を接続する

流体力学、浮力、スラスタ軸、センサー姿勢を対象機体に合わせます。係数には実際の物性に合う値を使ってください。

[海流](currents.md)、[センサーを追加する](custom-sensors.md)、各プラグインのリファレンスを参照します。緯度・経度が必要な場合は[球面座標](coordinates.md)で原点を設定します。

`config/<namespace>/robot_config.py` のエンティティ名、ジョイント名、トピック、座標系、ポートを整合させます。`description`、`meshes`、`config` とリソース検索設定は既存のパッケージがインストールします。生成された `install/` 内ではなく、ソースを編集してください。

## 4. ビルドして試す

環境を読み込んだワークスペースで実行します。

{{CODE:custom-robots:3}}

各軸に小さな指令を送り、次を確認します。ソースディレクトリの外からも起動してください。

- インストール済みリソースのパス。
- 質量、慣性、静止時の姿勢。
- 衝突形状と接触への応答。
- 座標系とメッセージ内容。

別の準備済みターミナルで、名前を変更したモデルの状態を読みます。

{{CODE:custom-robots:4}}

位置と速度を受信できるはずです。まずコピーを動かし、次に機構、その後にセンサーや推進系を変更します。[センサーデータ](visualization.md)で実験を比較し、リソースが見つからない場合は[トラブルシューティング](troubleshooting.md)を参照してください。
