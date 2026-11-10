# オブジェクトと作業シーン

POSIM には、マニピュレーション、点検、接触実験に使えるモデルがあります。`posim_object_models` がモデル記述、メッシュ、設定をインストールします。WSL、Docker、RDP を含め、[各環境の POSIM ターミナル](terminals.md)で実行してください。

## ローカルのモデル一覧

以下は `models/posim_object_models/description` にあるディレクトリです。SDF、リンク、ジョイント、リソースを確認できます。サンプルワールドでは Gazebo Fuel のオブジェクトも使用します。

{{OBJECT_CATALOG}}

## 用意されたシーンを起動する

{{CODE:objects:0}}

ほかに `posim_electrical_mating` と `posim_bimanual_example` があります。これらは Fuel の噴出防止装置パネル、オス・メスのコネクター、水中の花瓶などを使います。ロボットの配置や接触・関節制御の実験を始める前に、SDF 内の URI と配置を確認してください。

## ワールドにオブジェクトを追加する

URI を使用するモデルの実際のリソース名に置き換えます。

{{CODE:objects:1}}

位置の単位は m、角度は rad です。各インスタンスには異なる名前を付けます。点検用の固定対象と接触で動かす対象では力学的な扱いが異なるため、用途に応じて `static` を選びます。

表示形状は見た目を、衝突形状は接触を決めます。縮尺、姿勢、質量、慣性、関節の可動範囲、摩擦を確認してください。まず小さな制御された動作で接触を試し、[ROS 2](ros.md)で状態を記録します。

## オブジェクトを単独で起動する

環境を読み込んだターミナルで `mossy_cinder_block` を起動します。

{{CODE:objects:2}}

この起動ファイルは独自のワールドを開始します。別の例と組み合わせる場合は、1 つのワールドに `<include>` で配置してください。[ロボットを追加する](custom-robots.md)と[ワールド一覧](worlds.md)も参照できます。

## Fuel のモデルをローカルで使う

[Fuel カタログ](https://app.gazebosim.org/fuel/models)からモデルをダウンロードして展開します。たとえば `~/posim_models/my_object/` に `model.sdf`、`model.config`、`meshes/`、`materials/` をまとめます。Gazebo を起動するターミナルで親ディレクトリを登録します。

{{CODE:objects:3}}

Gazebo の **Resource Spawner** でローカルモデルを選択して配置します。SDF では `<uri>model://my_object</uri>` で再利用できます。Fuel が示す SDF 内の HTTPS URI も利用でき、その場合は初回にダウンロードします。オフラインで使う場合はファイルとライセンスを保存してください。

## Fuel に公開する

[Gazebo Fuel](https://app.gazebosim.org/home)にサインインしてモデルを作成します。名前、説明、ライセンスを入力し、`model.sdf`、`model.config`、参照するメッシュと材質をアップロードします。公開後にダウンロードし、外部のローカルパスに依存していないことを確認してください。必要に応じてコレクションに追加できます。

プラグインのライブラリ名とクラス名には、インストールした Jetty で利用できるものを使います。浮力、流体力学、推進器、センサーの接続は[ロボットを追加する](custom-robots.md)を参照してください。

{{MEDIA:objects}}

画像の出典：POSIM Notion Wiki。DAVE ドキュメントの帰属は[引用とライセンス](citation.md)に記載しています。実行コマンドとトピック名には、このページのコードブロックを使ってください。
