# USBL 測位

USBL（超短基線方式）は音響信号で水中機器の位置を求めます。この例は送受信機 1 台とトランスポンダー 2 台を接続します。全台への問い合わせと、ID を指定した個別問い合わせができます。

## サンプルワールドを起動する

[インストール方法に合わせて準備したターミナル A](terminals.md)で実行します。

{{CODE:usbl:0}}

{{MEDIA:usbl:0}}

3 台の機器はワールドに含まれています。準備済みのターミナル B で位置を受信します。

{{CODE:usbl:1}}

両方のメッセージに `transponder_id` がありますが、各フィールドの意味は異なります。

| 出力 | x | y | z |
|---|---|---|---|
| `transponder_location` | 水平方位角、度 | 距離、m | 仰角、度 |
| `transponder_location_cartesian` | 相対 x、m | 相対 y、m | 相対 z、m |

方向はワールド座標の位置差から求めます。水平方位角は +x 方向で 0°、+y 方向で 90°、正の仰角は +z 向きです。機体座標のデータと組み合わせる際は、この軸の規約を考慮してください。

## SDF に組み込む

送受信機を `box` モデル内に置きます。各 ID は `transponder_attached_object` の同じ順番のモデルに対応します。

{{CODE:usbl:2}}

次のトランスポンダーを `sphere` に置き、`sphere2` には `transponder_ID` が `2` のものを追加します。両端で送受信機の名前・ID・namespace を一致させます。

{{CODE:usbl:3}}

## パラメーター

| パラメーター | 対象 | 意味と既定値 |
|---|---|---|
| `namespace` | 両方 | 必須の namespace。例：`USBL` |
| `transceiver_device`, `transceiver_ID` | 両方 | パスに使う必須の送受信機名と ID |
| `transponder_device`, `transponder_ID` | 両方 | 必須の名前と ID。送受信機側はカンマ区切りの ID を指定可能 |
| `transponder_attached_object` | 送受信機 | 必須のモデル名。ID と同じ順にカンマ区切り |
| `transceiver_model` | トランスポンダー | 位置取得に使う送受信機のモデル名。必須 |
| `sound_speed` | 両方 | 10 °C、z = 0 での基準音速、m/s。`1540`。有限かつ正 |
| `enable_ping_scheduler` | 送受信機 | 必須の真偽値。`false` なら手動で ping を送信 |
| `ping_frequency` | 送受信機 | シミュレーション時間を基準とした要求頻度、Hz。`1`、例は `0.5`。有限かつ正 |
| `interrogation_mode` | 送受信機 | `common` または `individual`。既定は `common` |
| `mu`, `sigma` | トランスポンダー | 位置ノイズの平均・標準偏差、m。既定は `0` と `1`、例は `0` と `0`。有限、sigma ≥ 0 |

存在するモデル名、一意な ID、同じ長さの ID・モデル一覧を使います。ID は **0～2147483647** の 10 進整数で、符号や先頭の余分な 0 は使えません。各プラグインには link を含むモデルが必要です。[完全な SDF](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/usbl_tutorial.world)に形状と配置があります。

無効な設定ではプラグインは動作しません。未知のチャンネルやコマンド宛先は無視し、未知のチャンネルでは現在の選択を保ちます。初期チャンネルは一覧の最初のトランスポンダーです。定期 ping はシミュレーションとともに停止し、1 更新につき最大 1 回送信します。

## 音響伝搬時間

問い合わせには、要求がトランスポンダーに届くまでと、位置応答が送受信機へ戻るまでの 2 区間があります。各遅延は距離を対応する機器の音速で割った値です。

- 秒未満の遅延もシミュレーション時刻に従い、一時停止中は進みません。時刻が巻き戻ると破棄します。
- 各区間で保留できるのはトランスポンダーごとに 1 件で、追加の要求・応答は無視します。
- 全要求の応答を受けるには、予想往復時間より長い ping 間隔を設定します。

実効音速は `sound_speed + 0.017 × depth + 4 × (temperature − 10)` です。単位は m/s、z = 0 より下の深度 m、温度 °C です。水面上では深度項は 0、既定温度は 10 °C です。

温度は `/USBL/transceiver_manufacturer_168/temperature` または `/USBL/transponder_manufacturer_1/temperature` に `std_msgs/msg/Float64` で送ります。変更は次の伝搬区間から適用します。有限でない温度や、音速が非正・非有限になる変更は無視します。

この近似は [DOSITS の音速解説](https://dosits.org/decision-makers/tutorials/science/sound-speed/)に基づきます。距離は直線で、屈折、塩分、マルチパス、音響干渉は扱いません。復路は設定したノイズを含む受信位置を使います。

## 全トランスポンダーへ問い合わせる

{{CODE:usbl:4}}

`common` は定期 ping ごとに 1 回ブロードキャストし、同じトピックへ複数 ID の応答を出せます。応答ごとの ID を保持してください。送受信機はモードを保持し、後から参加するトランスポンダーにも伝えます。

## 1 台を選ぶ

{{CODE:usbl:5}}

チャンネル変更で `individual` モードになり、定期 ping はその ID に向かいます。全台を問い合わせる場合は `common` に戻します。

## 配置と問い合わせモード

{{MEDIA:usbl:1}}
{{MEDIA:usbl:2}}
{{MEDIA:usbl:3}}

画像の出典：POSIM Notion Wiki。DAVE の帰属は[引用とライセンス](citation.md)を参照してください。実行にはこのページのコマンドとトピック名を使います。
