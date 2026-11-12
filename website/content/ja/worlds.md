# ワールド一覧

ワールドは `models/posim_worlds/worlds` にあります。起動時は、ファイル名から `.world` を除いた名前を指定します。

海洋デモには `model://calm_waves` を含みます。Wave Sim の FFT 波は風速入力 3 m/s、周期タイル 256 m、表示範囲 768 × 768 m を使い、水面は上下から見えます。ネイティブ水中効果は RGB センサー画像に適用され、浅い水中のチュートリアルでは Gazebo のワールド表示にも適用されます。標準のインストーラーとイメージに Wave Sim を含みます。水中機体の浮力、海洋 GNSS、光学境界は表示上の波高とは別に平均水面 z = 0 を使います。WAM-V は波と連成する Wave Sim の流体力を使います。水のない USBL 例と独立したソナー試験ワールドはそれぞれの形状を維持します。

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

適した SDF を `models/posim_worlds/worlds` にコピーし、ファイルとワールドに新しい名前を付けます。インストール済みのリソースを解決できる URI を使ってください。ワークスペースを再ビルドして環境を読み込み、新しい名前で起動します。[ハイトマップによる地形](heightmaps.md)と[オブジェクト](objects.md)も参照してください。

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
| `use_ned_frame` | `world_ned` 変換を配信（ENU → NED） | `false` |

センサーの出力を得るには `paused:=false` を指定します。ロボットやワールドの引数は、それぞれの起動ファイルで `--show-args` を使って確認してください。
