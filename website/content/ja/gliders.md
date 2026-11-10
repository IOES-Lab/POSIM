# Slocum 水中グライダー

`glider_slocum` には機体、スラスター、流体力学モデル、IMU・航法・圧力のインターフェースが含まれます。水中グライダーの実験や独自機体の出発点として使えます。

## 海のワールドで起動する

インストール方法に合った[準備済みの POSIM ターミナル](terminals.md)で実行します。

{{CODE:gliders:0}}

空のワールドでモデルを確認する場合は `world_name:=empty.sdf z:=0.2` を指定します。

## 観測データを確認する

{{CODE:gliders:1}}

IMU と NavSat の Gazebo トピックは `/model/glider_slocum/` 以下にあります。対応する ROS トピックと型はブリッジ設定で確認してください。モデルは[圧力プラグイン](pressure.md)と Gazebo のオドメトリ配信機能も読み込みます。

機体と環境を起動してから、自分の制御ノードを接続し、潜航や滑空の実験を行います。

## モデルを変更する

[Slocum の SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/glider_slocum/model.sdf) をブリッジ設定と併せて確認します。質量、慣性、衝突形状、流体力学係数の整合性を保って変更してください。推定器や制御器を接続する前に座標系の規約を確認します。

比較用に元のモデルを残します。別のモデルの作成は[ロボットの追加](custom-robots.md)、観測と指令は [ROS](ros.md)を参照してください。

## グライダーの動作を見る

{{MEDIA:gliders}}

図と動画：POSIM Notion Wiki。DAVE 文書の帰属表示は[引用とライセンス](citation.md)にあります。実行には本ページのコマンドとトピック名を使ってください。
