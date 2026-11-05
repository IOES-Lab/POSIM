# 球面座標

ROS プラグイン `SphericalCoords` はワールドの地理的な原点を管理し、ローカル直交座標と緯度・経度・高度を相互に変換します。

## 座標設定済みのワールドを起動する

[各インストール方法に合わせて準備したターミナル A](terminals.md)で起動します。

{{CODE:coordinates:0}}

準備済みのターミナル B で確認します。

{{CODE:coordinates:1}}

サンプル SDF の原点は緯度 **35.074823°**、経度 **129.084798°** です。実行中のワールドで使われている原点は応答で確認してください。

## SDF への組み込みと設定

両方のブロックを `<world>` の直下に置きます。

{{CODE:coordinates:2}}

プラグインはワールドの座標設定を読み取り、独自の SDF パラメーターは持ちません。4 つのサービスのパスは `/gz/` 以下で固定です。ワールド以外に取り付けた場合や地理座標の設定がない場合は、エラーを出してサービスを無効のままにします。

| ワールドの項目 | 意味 | 例 |
|---|---|---|
| `surface_model` | 基準楕円体 | `EARTH_WGS84` |
| `world_frame_orientation` | 軸の規約 | `ENU` |
| `latitude_deg`, `longitude_deg` | 原点、度 | `35.074823`, `129.084798` |
| `elevation` | 原点の高度、m | `0` |
| `heading_deg` | 方位のオフセット、度 | `0` |

原点は明示的に設定します。[プラグイン](https://github.com/IOES-Lab/POSIM/blob/main/gazebo/posim_ros_gz_plugins/src/SphericalCoords.cc)と[サンプルワールド](https://github.com/IOES-Lab/POSIM/blob/main/models/posim_worlds/worlds/posim_bimanual_example.world)も参照してください。

## サービス

型は `posim_interfaces/srv` にあります。角度は度、ローカル座標と高度は m です。

| `/gz/` 以下のサービス名 | 型 | リクエスト |
|---|---|---|
| `get_origin_spherical_coordinates` | `GetOriginSphericalCoord` | 空 |
| `set_origin_spherical_coordinates` | `SetOriginSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |
| `transform_to_spherical_coordinates` | `TransformToSphericalCoord` | `input: {x, y, z}` |
| `transform_from_spherical_coordinates` | `TransformFromSphericalCoord` | `latitude_deg`, `longitude_deg`, `altitude` |

値はすべて有限である必要があります。緯度は −90°～90°、経度は −180°～180° です。無効な原点更新は `success: false` を返し、原点を変更しません。変換失敗時の座標は `NaN` になります。制御に使う前に結果を確認してください。

## ローカル位置を変換する

{{CODE:coordinates:3}}

応答は緯度・経度・高度です。その値を逆変換のサービスに渡し、`output` を元のベクトルと比較すると、原点と軸の規約を確認できます。

## 原点を変更する

{{CODE:coordinates:4}}

変更するとローカル位置と地理座標の対応が変わります。地形、ワールドの向き、高度の基準、コントローラーの設定をそろえてください。変換は Gazebo に従うため、SDF で指定した方位と軸の向きを考慮します。
