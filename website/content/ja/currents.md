# 海流

POSIMはワールドの海流を生成し、各ロボットの位置で流速を計算します。ROSサービスで設定を変更できます。空間的に一様な海流と、深さによって変わる海流では、トピックと設定が異なります。

## サンプルを動かす

使用するPOSIM環境（Ubuntuネイティブ、WSL、Docker、macOSのリモートデスクトップ）で[ターミナルAとB](terminals.md)を準備します。Aで起動します。

{{CODE:currents:0}}

BでGazebo、ROSの順に確認します。最初の表示をCtrl+Cで止めてから、次のコマンドを実行してください。

{{CODE:currents:1}}

{{CODE:currents:2}}

## ワールドの海流を設定する

2つのプラグインを`<world>`直下に配置します。ワールドの海流プラグインをROSプラグインより先に置いてください。全体の構成は[サンプルワールド](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/ocean_current_plugin.world)を参照できます。

{{CODE:currents:3}}

両方に同じ、空ではない`namespace`を指定します。`constant_current`と`transient_current`の両ブロックが必要です。必須設定がない場合は配信を無効にしてエラーを出力します。一様流の速さ・水平角・垂直角には、それぞれ独立したガウス・マルコフモデルを使います。

| パラメーター | 意味と既定値 |
| --- | --- |
| 両プラグインの`namespace` | 共通の名前空間。この例では`hydrodynamics` |
| `constant_current/use_constant_current` | 一様流を配信するか。`false` |
| `constant_current/topic` | 一様流のGazeboトピック。`ocean_current` |
| `transient_current/topic_stratified` | 名前空間内の層別Gazeboトピック。`stratified_current_velocity` |
| `transient_current/databasefileName` | インストール済み`posim_worlds/worlds`内のCSV。`transientOceanCurrentDatabase.csv` |
| `transient_current/databasefilePath` | 任意のCSVパス。ファイル名指定より優先 |
| 各確率モデルの`mean` | 速さ（m/s）または角度（rad）の初期平均値。0 |
| `min`, `max` | 下限・上限。−1、1。`min < max`とし、平均値を範囲内に設定 |
| `noiseAmp` | ランダムな増分の振幅。0 |
| `mu` | ガウス・マルコフモデルの減衰係数。0 |
| `tidal_oscillation` | 任意の潮流データ設定ブロック |

## 深度別の流速プロファイルを用意する

[サンプルCSV](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/transientOceanCurrentDatabase.csv)の列は、ワールドX軸の速度（m/s）、ワールドY軸の速度（m/s）、深さ（m）の順です。従来のキー`velocity_north`、`velocity_east`、`velocity_down`は、それぞれワールドX、Y、Z軸に直接対応します。

- 先頭3行の説明・見出しを残します。
- 少なくとも2層を用意し、各データ行に有限の数値を3つ指定します。
- 深さは0以上で、厳密に増加する順に並べます。
- ファイルがない場合や形式が不正な場合は、海流の配信を無効にします。

ワールドのROSプラグインはデータベース全体を配信します。モデル側のプラグインが機体の深さに合わせて補間します。流体力の計算では、機体の速度に対する相対的な流れが使われます。

## ロボットにプロファイルを適用する

REXROVのサンプルは初期状態では一様流`/ocean_current`を使います。深度プロファイルを使うには、[REXROVモデル](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_robot_models/description/rexrov/model.sdf)でコメントアウトされている次のブロックを、重複させずに`<model>`直下で有効にします。

{{CODE:currents:4}}

既存の`gz-sim-hydrodynamics-system`にも`<namespace>rexrov</namespace>`を追加します。そのプラグインのリンク、密度、抗力係数は保ちます。ワークスペースを再ビルドし、環境を読み込んでください。

| モデルのパラメーター | 意味と既定値 |
| --- | --- |
| `namespace` | 必須。流体力プラグインの名前空間と一致させる。この例では`rexrov` |
| `flow_velocity_topic` | `/model/<namespace>/`以下の接尾辞。`ocean_current` |
| `transient_current/topic_stratified` | データベースのROS入力。`hydrodynamics/stratified_current_velocity_topic_database` |
| `velocity_north`, `velocity_east`, `velocity_down` | 各成分の任意の確率設定。省略した成分にはノイズを加えない |
| 各成分の`mean` | 初期速度、m/s。0。データ受信後は補間した速度に置き換える |
| `mu` | 0～1の減衰係数。経過したシミュレーション時間（秒）を使って適用。既定値0 |
| `noiseAmp` | 補間速度から許容する偏差、m/s。0 |
| `noiseFreq` | 更新ごとのランダム増分の振幅。0。Hz単位の周波数ではない |
| `tide_oscillation` | 提供された潮流データをモデルに適用するか。`false` |

結果は`/model/rexrov/ocean_current`に、Gazeboでは`gz.msgs.Vector3d`、ROSでは`geometry_msgs/msg/TwistStamped`として配信されます。出力値と機体の応答を確認してください。

深さはモデルのワールド座標から求め、水面より上では0とします。層の間は補間し、範囲外では最も近い層の値を使います。一時停止中や同じ時刻が繰り返された場合は更新しません。不正なデータベースメッセージは全体を拒否し、最後の有効なプロファイルを維持します。

## システムとトピック

| システム | 役割 |
| --- | --- |
| `posim_gz_world_plugins::OceanCurrentWorldPlugin` | ワールドの海流と深度別データを生成 |
| モデルの海流プラグイン | ロボットの位置で流速を補間 |
| `posim_ros_gz_plugins::OceanCurrentPlugin` | ROSデータ配信と設定サービスを提供 |

| サンプルのトピック | 通信系と型 |
| --- | --- |
| `/ocean_current` | Gazebo、`gz.msgs.Vector3d` |
| `/hydrodynamics/stratified_current_velocity` | Gazebo、層別データ |
| `/hydrodynamics/stratifiedCurrentVelocityTopic` | ROS、`posim_interfaces/msg/StratifiedCurrentVelocity` |
| `/hydrodynamics/currentVelocityTopic` | ROS、`geometry_msgs/msg/TwistStamped` |
| `/hydrodynamics/stratified_current_velocity_topic_database` | ROS、`posim_interfaces/msg/StratifiedCurrentDatabase` |

SDFの名前を確認し、流体力プラグインを意図した海流の出力先に接続してください。

## ROSサービスで流れを調整する

Bでサービスを調べ、水平な0.3 m/sの流れを設定します。

{{CODE:currents:5}}

速さはm/s、角度はラジアンです。応答の`success`と配信値を確認します。層ごとの設定インターフェースは次のコマンドで確認できます。

{{CODE:currents:6}}

モデルを読み取り、範囲を変更して、最初の層を設定する例です。

{{CODE:currents:7}}

| `/hydrodynamics`以下のサービス | インターフェース | 入力フィールド |
| --- | --- | --- |
| `get_current_velocity_model`, `get_current_horz_angle_model`, `get_current_vert_angle_model` | `GetCurrentModel` | 空のリクエスト |
| `set_current_velocity_model`, `set_current_horz_angle_model`, `set_current_vert_angle_model` | `SetCurrentModel` | `mean`, `min`, `max`, `noise`, `mu` |
| `set_current_velocity` | `SetCurrentVelocity` | `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_current_horz_angle`, `set_current_vert_angle` | `SetCurrentDirection` | `angle` |
| `set_stratified_current_velocity` | `SetStratifiedCurrentVelocity` | `layer`, `velocity`, `horizontal_angle`, `vertical_angle` |
| `set_stratified_current_horz_angle`, `set_stratified_current_vert_angle` | `SetStratifiedCurrentDirection` | `layer`, `angle` |

### 設定値の条件

- リクエストは全体を適用するか、変更せずに`success: false`を返します。
- 値は有限で、設定された範囲内である必要があります。速さの上下限は0以上、角度の範囲は水平±π以内、垂直±π/2以内です。
- モデル設定は全体と各深度層に適用されます。各層の平均値は維持するため、新しい範囲にもすべての平均値が収まる必要があります。
- `layer`は0から数えます。ROSの`noise`はSDFの`noiseAmp`に対応します。

## 潮流データを使う

`<tidal_oscillation>`にCSV、またはM2/S2/N2などの分潮、上げ潮・下げ潮の流向、開始時刻（GMT）を設定します。分潮の例では、振幅はm、位相は度、角速度は度/時です。海流サービスの角度はラジアンなので区別してください。

潮流モードでは、計算した分潮信号、またはCSVの速度を100で割った値に、水深プロファイルの第1列を掛けます。例の高さ振幅は数値係数として使われ、潮位を流速に変換するモデルではありません。目的の流速に合わせて信号とプロファイルの組み合わせを校正してください。

潮流CSVは見出し1行と`YYYY-MM-DD HH:MM,event,speed`形式のデータ行で構成します。

- イベントは`flood`、`ebb`、`slack`。速度は有限のcm/s値です。読み飛ばす`slack`行では`-`も使えます。
- 日付は有効で、厳密に時刻が増加する順に並べます。
- 正負が交互になるピークを少なくとも2つ保持します。時刻範囲外では最初または最後のピーク値を使います。
- ファイルの欠落、設定不足、不正なデータがあると、ワールドの海流プラグインを無効にします。

これらは実験の入力データです。単位、補間範囲、ワールドの原点、時刻の基準をそろえてください。同じ初期姿勢から流速と機体の動きを比較します。

## 海流と機体の応答

{{MEDIA:currents}}

図・動画：POSIM Notion Wiki。DAVEの出典は[引用とライセンス](citation.md)を参照してください。コマンドとトピック名はこのページの記載を使用します。
