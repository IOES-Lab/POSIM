# ROV と BlueROV2

`namespace` 引数で、インストール済みモデルのディレクトリを選びます。ROS トピックはそのモデルの設定に従います。

## 利用できるモデル

| モデル | 用途 |
|---|---|
| `rexrov` | Gazebo スラスター、流体力学、オドメトリ |
| `bluerov2` | ArduSub SITL と MAVROS を組み合わせた BlueROV2 |
| `bluerov2_heavy` | Heavy 構成 |
| `bluerov2_heavy_multibeam_sonar` | CUDA ソナー付き Heavy 構成 |

インストール方法に合った[準備済みターミナル](terminals.md)を使います。REXROV は[最初のシミュレーション](quickstart.md)から始めてください。

## BlueROV2 を起動する {#start-bluerov2}

{{CODE:rovs:0}}

Heavy を選ぶには `namespace:=bluerov2_heavy` を指定します。起動出力と `/mavros/state` でオートパイロットの接続を確認します。ソナー付きモデルには [CUDA の準備](sonar-tuning.md)が必要です。

## キーボードで操作する

デスクトップの対話型ターミナルで `gui:=true headless:=false use_teleop:=true` を指定します。

| キー | 操作 |
|---|---|
| `c` / `x` | アーム／ディスアーム |
| `w` / `s` | 前進／後退 |
| `a` / `d` | 左旋回／右旋回 |
| `r` / `f` | 上昇／下降 |
| `h` / `j` | ALT_HOLD／STABILIZE |
| スペース | 中立指令 |

## ブラウザーのジョイスティックを使う

別の準備済みターミナルで、インストール済みのページを配信します。

{{CODE:rovs:1}}

遠隔操作を有効にして BlueROV2 を起動します。

{{CODE:rovs:2}}

WebSocket ブリッジは遠隔操作の起動処理に含まれるため、`use_teleop:=true` を維持します。ブラウザーが開かない場合は、同じ Linux デスクトップで URL を開いてください。Docker イメージには Firefox と QGC が含まれます。QGC の導入、仮想ジョイスティック、映像受信は [QGroundControl](qgroundcontrol.md)で説明しています。

自分のプログラムからの購読と指令は [ROS](ros.md)を参照してください。

## ロボットの動作を見る

{{MEDIA:rovs}}

図と動画：POSIM Notion Wiki。DAVE の帰属表示は[引用とライセンス](citation.md)にあります。本ページのコマンドとトピック名を使ってください。
