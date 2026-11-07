# 機能と使用例

POSIM は Gazebo のシステム、ロボットモデル、ROS インターフェースを提供します。ワールドの SDF と起動設定で、シミュレーションに使うシステムを選びます。

## 環境と描画

| 機能 | 実装 | ガイド |
|---|---|---|
| 水中のかすみ、RGB 吸収、日光の減衰 | `gazebo/posim_gz_rendering` | [水中描画](underwater-rendering.md) |
| 海底の集光模様と浮遊粒子 | `gazebo/posim_gz_rendering` | [効果の有効化](underwater-rendering.md) |
| 海底テクスチャ、計測グリッド、等深線 | `gazebo/posim_gz_rendering`、地形側のマップ | [材質と間隔](underwater-rendering.md) |
| スペクトル波と波を考慮した WAM-V の流体力学 | 外部の Wave Sim、`extras/surface` による統合 | [水上ロボット](surface.md) |
| 一様流、深度依存の海流、潮流 | `posim_gz_world_plugins`、`posim_gz_model_plugins`、`posim_ros_gz_plugins` | [海流](currents.md) |
| 地理座標の原点と座標変換 | `posim_ros_gz_plugins` | [球面座標](coordinates.md) |
| 海底、オブジェクト、作業シーン | `models/posim_worlds`、`posim_object_models` | [ワールド](worlds.md)、[オブジェクト](objects.md)、[地形](heightmaps.md) |

水中の光学効果は PBR 表面に適用され、数値としての深度は維持されます。海面は独自の材質で描画します。マリンスノーは手続き的に生成する視覚表現で、初期状態では無効です。海面高さの前提とカメラの動作は[水中描画ガイド](underwater-rendering.md)で説明しています。

## センサー、ロボット、制御

| 機能 | 実装 | ガイド |
|---|---|---|
| 海洋 GNSS、潜水による受信喪失、再捕捉、遅延 | `gazebo/posim_gz_gnss` | [GNSS](gnss.md) |
| RGBD に基づく水中画像の減衰 | `posim_gz_sensor_plugins/UnderwaterCamera` | [カメラ](camera.md) |
| DVL 計測と ROS 変換 | Gazebo DVL、`posim_ros_gz_plugins` | [DVL](dvl.md) |
| 静水圧と計算した深度 | `posim_gz_sensor_plugins` | [圧力](pressure.md) |
| USBL の音響問い合せと測位 | `posim_gz_sensor_plugins` | [USBL](usbl.md) |
| CUDA マルチビームソナー | `gazebo/posim_gz_multibeam_sonar` | [ソナー](sonar.md)、[設定と性能](sonar-tuning.md) |
| REXROV、BlueROV2 の各構成、Slocum グライダー | `models/posim_robot_models` | [ROV](rovs.md)、[グライダー](gliders.md)、[ロボット追加](custom-robots.md) |
| ArduRover による WAM-V 統合と ROS 制御 | `extras/surface`、起動設定、`posim_interfaces` | [水上艇](surface.md)、[ROS 2](ros.md) |

利用できるセンサーは、インストール済みのプラグインとモデル設定で決まります。GNSS のプロファイルは取り付け位置を記述するため、組み込む際にはリンクと受信機を追加してください。CUDA ソナーには対応 GPU とツールチェーンが必要です。各センサーのクイックスタートで実際の計測出力を確認できます。

## ソースディレクトリ

以下は POSIM リポジトリからの相対パスです。

| ディレクトリ | 内容 |
|---|---|
| `models/posim_worlds` | ワールドと環境リソース |
| `models/posim_robot_models` | ロボットの機構、センサー、個別の ROS 設定 |
| `models/posim_sensor_models` | 単独センサーの記述と使用例 |
| `gazebo/` | 海洋環境とセンサーのプラグイン |
| `posim_interfaces` | POSIM のメッセージとサービス定義 |
| `examples/posim_demos` | ワールド、モデル、ROS を結び付ける起動ファイル |

[最初のシミュレーション](quickstart.md)で、これらを組み合わせて実行できます。変更する場合は[ロボットを追加する](custom-robots.md)と[ワールドを作る](custom-worlds.md)を参照してください。

## macOS 用の Ogre 差し替えライブラリ

[Ogre ビルドツール](https://github.com/IOES-Lab/POSIM/blob/main/extras/ogre/README.md)は、macOS arm64 と Ogre-Next 2.3.3 の組み合わせ向けに独立した Ogre コアをビルドします。対応 SDK の設定と、ロードされたライブラリの確認手順に従ってください。

外部コンポーネントの条件は[ライセンス](citation.md)を参照してください。
