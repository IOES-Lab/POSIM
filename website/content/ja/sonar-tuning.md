# ソナーの設定と性能調整

計算バックエンドを選び、観測範囲、細かさ、更新周波数を比較します。[マルチビームソナー](sonar.md)には、クイックスタート、SDF、パラメーター表があります。

## 実行環境を選ぶ

[準備済みの POSIM ターミナル](terminals.md)を使います。編集、ビルド、実行は、POSIM をインストールした同じ Ubuntu、WSL、Docker 環境内で行ってください。`POSIM_WS` はそのワークスペースのパスです。どの計算バックエンドでも Gazebo の描画センサーが必要です。

| 環境 | 計算方法と条件 |
| --- | --- |
| Linux ネイティブ | CPU、利用可能な Vulkan アダプターによる WGPU、対応 NVIDIA GPU とツールキットによる CUDA |
| WSL 2 または Linux コンテナ | CPU。GPU 計算には、その環境の**内部から**アダプターとドライバーを利用できることが必要 |
| macOS の Docker/RDP | CPU から始めます。Linux コンテナは Mac の Metal デバイスを公開しません。 |
| macOS ネイティブ | 別途用意したネイティブ ROS/Gazebo 環境で WGPU の Metal を利用可能。[macOS インストールガイド](install-macos.md)は Docker/RDP 方式です。 |

`auto` は CUDA → WGPU → CPU の順で選択します。比較実験では `cpu`、`wgpu`、`cuda` を明示してください。指定したバックエンドが使えなければ、センサーの初期化は失敗します。[選択表](sonar.md#compute-backend)を参照してください。

## ソナーパッケージをビルドする {#build-the-sonar-packages}

[ソースからのインストール](install.md)と依存関係の準備を済ませてください。`wgpu_vendor` は CPU を使う場合も Rust ライブラリをビルドします。Rust と Cargo がなければ[公式の導入手順](https://www.rust-lang.org/tools/install)に従います。

{{CODE:sonar-tuning:0}}

環境を読み込んだワークスペースで、ライブラリ、センサー、ワールドシステム、例をビルドします。

{{CODE:sonar-tuning:1}}

CUDA は任意です。インストール済みの CUDA を使わず CPU/WGPU 用にビルドする場合は、`--cmake-args -DCMAKE_DISABLE_FIND_PACKAGE_CUDAToolkit=ON` を追加します。Rust の依存関係は `Cargo.lock` で固定され、初回のダウンロードにはインターネット接続が必要です。

{{CODE:sonar-tuning:2}}

4 パッケージを確認し、[ソナーの例](sonar.md#run-a-sonar-example)で画像と生データを受信してください。パッケージ一覧だけではセンサーの読み込みを確認できません。コンテナ内でパッケージを確認し、なければソースからビルドします。イメージタグだけで同梱を判断しないでください。

## WGPU アダプターを選ぶ

Linux では、Gazebo を実行する環境内で `vulkaninfo --summary` を確認します。システムに合った Vulkan ドライバーとツールが必要です。llvmpipe などのソフトウェアアダプターは Vulkan を実行できますが、ハードウェアによる高速化は行いません。WGPU の起動出力に選択されたアダプターが表示されます。

{{CODE:sonar-tuning:3}}

| 環境変数 | 用途 |
| --- | --- |
| `WGPU_BACKEND` | Linux は `vulkan`、macOS ネイティブプロセスは `metal`。省略時はプラットフォームの既定値 |
| `WGPU_ADAPTER_NAME` | 名前に指定文字列を含む最初のアダプター。大文字と小文字は区別しません。ドライバーや起動出力の名前を使用 |
| `WGPU_POWER_PREF` | `high`、`low`、`none`。名前の指定がなければ既定値は `high` |

変数は起動前に設定します。初期化はプロセス内で共有されるため、変更後はシミュレーションを終了して再起動してください。API は [wgpu プロジェクト](https://github.com/gfx-rs/wgpu)で説明されています。初期化やリソース確認に失敗した場合は、表示されたアダプターと上限を確認し、`cpu` を選ぶかセンサーの規模を縮小します。

## CUDA を使う場合

NVIDIA ドライバー、CUDA コンパイラーとランタイム、cuFFT、cuBLAS が必要です。実行環境で確認します。

{{CODE:sonar-tuning:4}}

Linux コンテナではホストに [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html)を設定します。Windows は [CUDA on WSL](https://docs.nvidia.com/cuda/wsl-user-guide/index.html)と、必要に応じて [Docker Desktop の GPU 設定](https://docs.docker.com/desktop/features/gpu/)に従います。ディスプレイドライバーは Windows に導入し、WSL 内に Linux 用を入れないでください。コンテナ内にも CUDA ライブラリが必要です。

GPU とツールキットが対応するアーキテクチャを選びます。以下の SM 75 の例では、`75` を自分の GPU の値に置き換えてください。`OFF` は CPU/WGPU 専用ビルドで無効にしたツールキット検出を再び有効にします。

{{CODE:sonar-tuning:5}}

`nvcc --list-gpu-code` で対応ターゲットを確認できます。指定がなければコンパイラーの既定値を使います。複数のツールキットがある場合は `-DCUDAToolkit_ROOT=/path/to/cuda` で選び、その `bin` を `PATH` に追加します。`compute_backend:=cuda` で例を起動してください。

## 基準を測定する

シーン、対象までの距離、センサー姿勢を固定します。ターミナル B で画像の受信周波数を測ります。

{{CODE:sonar-tuning:6}}

Ctrl+C で止め、シミュレーション速度を確認します。

{{CODE:sonar-tuning:7}}

ワールドのエンティティ名は `default` です。受信周波数と `real_time_factor` を記録します。シミュレーション時間で 30 Hz を要求しても、実時間の 1 秒に 30 枚届くとは限りません。Ctrl+C で終了します。

## サンプリングと距離を調整する

[SDF](sonar.md#sensor-configuration) のコピーで 1 項目ずつ変更します。再ビルドして環境を読み込み、同じ姿勢で起動します。

| 実験 | 比較 | 確認する点 |
| --- | --- | --- |
| 角度方向の細かさ | 視野角を固定し、ビーム 512 本と 256 本 | 近接対象の分離と受信周波数 |
| 垂直サンプリング | レイ 300 本と 150 本。別に `raySkips` 10 と 5 | 対象の連続性と計算時間 |
| 観測範囲 | レイ範囲と `maxDistance` を同時に変更 | 最も遠い対象と距離軸 |
| 更新周波数 | 30 Hz と 10 Hz | 受信周波数と実時間係数 |

距離間隔は `soundSpeed / (2 × bandwidth)`、セル数は `ceil(maxDistance / 間隔) + 1` です。1500 m/s、29.9 kHz、15 m では 599 セルになります。最後の座標は `maxDistance` を少し超える場合がありますが、設定範囲外の反射は除外されます。実際の距離配列とビーム数は生メッセージまたは CSV から読み取ってください。

WGPU は 2 の累乗以外でも要求した変換長を維持し、FFT の段階を自動選択します。距離と帯域幅を増やすとメモリーと計算量も増えます。既定の格子方式では、CPU は反射を最も近い距離セルに割り当て、GPU は周波数領域で処理します。バックエンドを比較するときは振幅と距離応答もそれぞれ測定してください。

## 再現可能な比較を記録する

描画が安定してから測定します。性能測定中は `writeLog` と `debugFlag` を無効にし、[CSV](sonar.md) や [ROS](visualization.md) の記録は別に行います。`blazingSonarImage=false` は乱数シードを 1234 に固定します。

SDF、POSIM リビジョン、バックエンドとアダプター、ドライバーとツールキットの版、フレーム時間、受信周波数、実時間係数、メモリー使用量を保存します。画面だけでなく、生の強度・距離データも比較してください。研究モデルの出典は[引用](citation.md)にあります。

## GPU の処理フロー

{{MEDIA:sonar-tuning}}

レイの加算、窓関数・補正、FFT の各段階があります。計算時間とメモリー転送時間を分けて測定します。図：POSIM Notion Wiki。
