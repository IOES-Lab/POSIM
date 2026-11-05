# ハイトマップ地形

ハイトマップは、規則的な格子の各点に高さを与えて地形を表します。カメラに見える海底と接触面が一致するよう、表示形状と衝突形状を一緒に設定します。

## 作業環境

Ubuntu ネイティブ、WSL、Docker の[準備済み Linux ターミナル](terminals.md)を使います。macOS では RDP デスクトップ内で作業します。編集とビルドもその環境で行い、`POSIM_WS` にワークスペースのパスを使います。

## 参考ワールド

[Santorini](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_Santorini.world) は Fuel の `Santorini Scaled` を使った地理座標のシーンです。[段状の海底](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_graded_seabed.world)もワールド構成の参考になります。

ハイトマップの形状自体は [Gazebo のチュートリアル](https://gazebosim.org/api/sim/10/heightmap_dem.html)と [SDFormat の形状リファレンス](http://sdformat.org/spec?ver=1.12&elem=geometry)を参照してください。

## リソースをパッケージに保存する

ワールドは `models/posim_worlds/worlds`、データは `models/posim_worlds/media` に置きます。パッケージの CMake インストール規則とリソース検索設定を確認し、持ち運べるインストール済みリソース URI を使います。

表示形状または衝突形状内の基本構造は次のとおりです。

{{CODE:heightmaps:0}}

URI は自分のデータに置き換えてください。描画エンジンと物理エンジンに対応するラスター形式とサンプリングを選びます。高さの符号化を確認してから、縦方向の大きさとオフセットを決めます。

## 座標と縮尺を維持する

`size` は水平の広がりと高さの倍率を m で表し、`pos` はオフセットを与えます。正規化画像だけでは地理参照は付きません。元の範囲、高さの範囲、海面基準、再投影やリサンプリングの内容を記録してください。

地理座標を使う場合はワールドに `<spherical_coordinates>` を設定します。[球面座標](coordinates.md)で緯度・経度とローカル座標の変換を説明しています。

## ビルドして確認する

リソース追加後に再ビルドして環境を読み込みます。リポジトリの外から起動し、次を確認してください。

- 特徴的な地形の高さと、表示・衝突形状の一致。
- センサーの深度。
- 条件を固定した落下実験での接触。
- 海面基準とロボットの初期高さ。

Jetty では、ゼロ以外の `<heightmap><pos>` が表示地形だけを動かし、衝突面に同じ移動が反映されない場合があります。モデルの姿勢に移動量を設定しても、ずれが残ることがあります。使用する描画・物理エンジンで両方を確認してください。一致しない場合は、接触と表示の確認を通る形状またはエンジンを選んでから航法実験に進みます。
