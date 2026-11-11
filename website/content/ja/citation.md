# 引用とライセンス

POSIMはIOES-Labが保守しています。利用・配布する際は、各コンポーネントの著者表示とライセンス通知を保持してください。

## シミュレーションの基盤を引用する

水中シミュレーション環境については、DAVEプロジェクトの論文を参照してください。

Zhangほか、*DAVE Aquatic Virtual Environment: Toward a General Underwater Robotics Simulator*、IEEE/OES Autonomous Underwater Vehicles Symposium、2022年。[DOI: 10.1109/AUV53081.2022.9965808](https://doi.org/10.1109/AUV53081.2022.9965808)。

マルチビームソナーのモデルについて：

Choiほか、*Physics-Based Modelling and Simulation of Multibeam Echosounder Perception for Autonomous Underwater Manipulation*、Frontiers in Robotics and AI、2021年。[DOI: 10.3389/frobt.2021.706646](https://doi.org/10.3389/frobt.2021.706646)。

実験を再現できるよう、POSIMのリビジョン、ロボットとワールドの設定、関連する依存関係も記録してください。著者一覧などの完全な書誌情報は原論文の記録を使用します。

## コンポーネントの出典

| コンポーネント | 出典・通知 |
| --- | --- |
| ApacheライセンスのPOSIM独自コードと継承コード | [LICENSE](https://github.com/IOES-Lab/POSIM/blob/main/LICENSE)と各ファイルの通知 |
| DAVEの基盤 | [Project DAVE](https://github.com/Field-Robotics-Lab/dave)、著者・出典表示 |
| 外部Wave Sim | [asv_wave_sim](https://github.com/srmainwaring/asv_wave_sim)、元プロジェクトのGPLv3ライセンス |
| 外部WAM-V/VRXアセット | 取得した依存関係に付属する通知 |
| Gazebo Fuelモデル | 各アセットの出典とライセンス |

波のビルドでは、元のソースと変更内容を別に保持します。依存関係と配布の説明は[extras/surface/README.md](https://github.com/IOES-Lab/POSIM/blob/main/extras/surface/README.md)を参照してください。リポジトリを分けても、各コンポーネントのライセンス条件に従う必要があります。

## ドキュメントの出典

[元のROS 2プロジェクト文書](https://dave-ros2.notion.site/)は、モデル、環境、センサーの解説の参考資料です。このサイトのコマンドはPOSIMのパッケージ名と、インストールガイドに記載したROS/Gazebo構成を使用します。

[IOES-Lab POSIM Notion Wiki](https://caring-dibble-be5.notion.site/d24c9419989882cfa6498152ad4c840c?v=b42c941998988252a89588ad58631495&pvs=74)と各ページの出典リンクも参照できます。

## 保守と連絡先

韓国海洋大学校（KMOU）の[IOES Lab.](https://lab.wschoi.com)がPOSIMを保守しています。

研究室へのお問い合わせは[woensug.choi@gmail.com](mailto:woensug.choi@gmail.com)へ、ソフトウェアの不具合報告は[GitHub Issues](https://github.com/IOES-Lab/POSIM/issues)へお送りください。
