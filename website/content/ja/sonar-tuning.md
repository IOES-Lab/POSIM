# ソナーの設定と性能調整

実験に合わせて観測範囲、細かさ、更新周波数を選びます。[マルチビームソナー](sonar.md)にはクイックスタート、SDF、パラメーターがあります。このガイドでは設定の比較と CUDA の準備を説明します。

## 必要な実行環境

NVIDIA GPU を搭載した Ubuntu または対応する WSL 2、あるいは GPU にアクセスできるコンテナで、[準備済み Linux ターミナル](terminals.md)を使います。編集とビルドもそこで行い、`POSIM_WS` にそのワークスペースを指定します。macOS の Docker/RDP 構成では CUDA GPU を利用できません。このソナーは NVIDIA GPU のある Linux または WSL 2 ホストで実行してください。

## 基準となる計測を行う

[BlueView P900 の例](sonar.md#run-a-sonar-example)を起動し、シーン、対象までの距離、センサー姿勢を固定します。画像と生データを受信してから性能を測ります。CUDA がない場合は先に実行環境を準備してください。

別の準備済みターミナルで受信画像の周波数を測ります。

{{CODE:sonar-tuning:0}}

安定した値を得たら Ctrl+C で止めます。シミュレーションの進行速度は別に確認します。

{{CODE:sonar-tuning:1}}

ワールドの内部名は `default` です。`real_time_factor` はシミュレーション時間と実時間の比なので、画像の受信周波数と一緒に記録します。30 Hz を要求しても、実時間の 1 秒に 30 枚届くとは限りません。Ctrl+C で表示を終了します。

## 1 つの設定を比較する

[センサー定義](sonar.md#sensor-configuration)のコピーで 1 項目を変更し、再ビルドして環境を読み込み、同じ姿勢で起動します。

| 実験 | 比較 | 観察する点 |
|---|---|---|
| 角度方向の細かさ | 視野角を固定し、水平ビーム 512 本と 256 本 | 近接した対象の分離と受信周波数 |
| 垂直方向のサンプリング | レイ 300 本と 150 本 | 対象の連続性と計算時間 |
| 観測範囲 | レイの範囲と `maxDistance` を同時に変更 | 最遠の対象と距離軸の目盛り |
| 更新周波数 | 30 Hz と 10 Hz | 受信周波数と `real_time_factor` |
| レイの間引き | `raySkips` の 10 と 5 | 垂直方向の細かさと計算量 |

比較結果は実測してください。性能測定中は `writeLog` と `debugFlag` を無効にし、生データや処理時間を調べるときに別途有効にします。SDF、POSIM のリビジョン、GPU/CUDA のバージョン、観測結果を保存します。トピックの記録は [ROS 2](ros.md#visualization-and-recording)を参照してください。

## ツールチェーンを確認する

[プラグイン](sonar.md)には NVIDIA ドライバー、CUDA コンパイラーとランタイム、cuFFT、cuBLAS、Gazebo の描画が必要です。ソナーを実行する環境で確認します。

{{CODE:sonar-tuning:2}}

GPU へのアクセスとコンパイラーの有無は別々に確認します。Linux の Docker Engine ではホストに [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)を設定します。Windows では [Docker Desktop/WSL 2 の GPU 設定](https://docs.docker.com/desktop/features/gpu/)に従います。イメージ内にも CUDA ライブラリが必要です。

## CUDA アーキテクチャを選ぶ

`CMAKE_CUDA_ARCHITECTURES` には GPU とツールキットが対応する SM を指定します。以下は SM 75 の例です。`75` を対象 GPU の値に変え、準備済みワークスペースで実行します。

{{CODE:sonar-tuning:3}}

`nvcc --list-gpu-code` で対応ターゲットを確認できます。未指定の場合は CMake がコンパイラーの既定値を使います。CUDA が複数ある場合は `-DCUDAToolkit_ROOT=/path/to/cuda` で選び、その `bin` ディレクトリを `PATH` に含めます。

## インストール結果を確認する

{{CODE:sonar-tuning:4}}

CUDA のビルドを省略してもパッケージは登録される場合があります。ライブラリを確認します。

{{CODE:sonar-tuning:5}}

各コマンドの終了コードは 0 になる必要があります。省略されたターゲットやロードエラーを確認し、[マルチビームソナー](sonar.md)の画像と生データを受信してください。

## 計測結果を読み解く

測定前に描画を安定させます。同じシーンと姿勢で、フレーム処理時間、受信周波数、`real_time_factor`、CPU/GPU 使用率、メモリーを記録します。物理、描画、CUDA 計算、配信、ログの負荷を分けて考えます。

`blazingSonarImage` は変化する乱数シードと固定シード `1234` を切り替えるため、再現性を比較できます。数値カーネルを変える際は基準データを残し、画像に加えて強度と距離を比較してください。研究の出典は[引用](citation.md)にあります。

## GPU の処理の流れ

{{MEDIA:sonar-tuning}}

処理にはレイの加算、窓関数・補正、FFT が含まれます。プロファイリングでは各段階と GPU メモリー転送の時間を分けます。図の出典：POSIM Notion Wiki。
