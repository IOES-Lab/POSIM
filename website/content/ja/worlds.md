# ワールド一覧

ワールドは `models/posim_worlds/worlds` にあります。起動時は、ファイル名から `.world` を除いた名前を指定します。

## ワールドを起動する

Ubuntu ネイティブ、WSL、Docker、RDP のいずれでも、[インストール方法に応じて準備した POSIM ターミナル](terminals.md)で実行します。

{{CODE:worlds:0}}

ファイル名と内部のワールド名は異なる場合があります。たとえば `posim_ocean_waves.world` の内部名は `oceans_waves` で、Gazebo のトピックやサービスのパスにはこちらを使います。パスを指定する前に `<world name="…">` を確認してください。

## 実験に合うワールドを選ぶ

| 実験 | ワールド |
|---|---|
| 最初のロボット起動 | `posim_ocean_waves` |
| 水中カメラの画像効果 | `camera_tutorial` |
| DVL の計測 | `dvl_world` |
| USBL の問い合せ | `usbl_tutorial` |
| 海流の設定 | `ocean_current_plugin` |
| 沿岸・地理座標のシーン | `posim_Santorini` |
| ソナー | `posim_multibeam_sonar` と名前に sonar を含む海洋ワールド |
| マニピュレーション | `posim_bimanual_example`、`posim_plug_and_socket`、`posim_electrical_mating` |

ソナー用ワールドには[ソナーの実行環境](sonar-tuning.md)が必要です。Fuel を参照するモデルは、初回起動時にダウンロードされる場合があります。

## SDF ファイル

ファイルを開くと、システム、モデル、原点、照明、カメラ設定を確認できます。

| ワールドの SDF |
|---|
{{WORLD_CATALOG}}

## 別の環境を作る

適した SDF を `models/posim_worlds/worlds` にコピーし、ファイルとワールドに新しい名前を付けます。インストール済みのリソースを解決できる URI を使ってください。ワークスペースを再ビルドして環境を読み込み、新しい名前で起動します。[高さマップによる地形](heightmaps.md)と[オブジェクト](objects.md)も参照してください。

## Fuel のダウンロードを待つ

`posim_integrated` と `posim_ocean_models` はリモートモデルを使います。初回のダウンロードが終わるまで待ち、読み込み中に起動を繰り返さないでください。`verbose:=true` を指定すると、リソースの読み込み状況を確認できます。

同じ環境の別のターミナルで、読み込まれたシーンを取得します。

{{CODE:worlds:1}}

応答に必要なモデルが含まれること、シミュレーション時刻が進むこと、センサーが計測値を出力することを確認します。ダウンロードが止まった場合は、読み込みメッセージと Gazebo を実行するコンピューターまたはコンテナのネットワーク接続を調べます。

## 起動引数を確認する

{{CODE:worlds:2}}

| 引数 | 用途 | センサー起動時の既定値 |
|---|---|---|
| `namespace` | インストール済みモデルのディレクトリ | 空。明示的に指定 |
| `world_name` | ワールドのファイル | `empty.sdf` |
| `paused` | 一時停止状態で起動 | `true` |
| `gui` / `headless` | Gazebo GUI を表示 / 非表示 | `true` / `false` |
| `use_sim_time` | ROS ノードでシミュレーション時刻を使用 | `true` |
| `x`, `y`, `z` | 初期位置、m | 各 `0.0` |
| `roll`, `pitch`, `yaw` | 初期姿勢、rad | 各 `0.0` |
| `debug` / `verbosity_level` | 詳細ログ / ログレベル | `false` / `1` |
| `use_ned_frame` | NED 座標系を使用 | `false` |

センサーの出力を得るには `paused:=false` を指定します。ロボットやワールドの引数は、それぞれの起動ファイルで `--show-args` を使って確認してください。
