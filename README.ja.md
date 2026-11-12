# POSIM — Platform for Ocean Simulation

[English](README.md) · [Español](README.es.md) · [Français](README.fr.md) · [Deutsch](README.de.md) · [Italiano](README.it.md) · [한국어](README.ko.md) · [日本語](README.ja.md) · [中文](README.zh.md)

[![POSIM Dockerイメージのビルドと公開](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml/badge.svg)](https://github.com/IOES-Lab/POSIM/actions/workflows/docker-amd64.yml)

POSIMは、[韓国海洋大学校（KMOU）のIOES Lab.](https://lab.wschoi.com)が保守する海洋ロボット向けのオープンソースシミュレーションライブラリです。
Gazebo Simで海洋環境、水中・水上ロボット、センサーモデル、ROS 2の制御インターフェースを組み合わせて利用できます。

![GraphicalAbstract](docs/release/POSIM_Graphical_Abstract_Release1.0.0_FF.png)

## ドキュメントと使い始め方

**[日本語ドキュメントを読む](https://ioes-lab.github.io/POSIM/ja/)**

[変更履歴](CHANGELOG.md)

POSIMはROS 2 LyricalとGazebo Sim Jettyを使用します。お使いのOSに合ったインストール方法を選んでください。

- **Ubuntu 26.04：** [ネイティブインストール](https://ioes-lab.github.io/POSIM/ja/install.html)を推奨します。Linuxでは[Dockerを使う方法](https://ioes-lab.github.io/POSIM/ja/install-ubuntu-docker.html)も選べます。
- **Windows：** [WSL](https://ioes-lab.github.io/POSIM/ja/install-windows.html)または[WSL + Docker](https://ioes-lab.github.io/POSIM/ja/install-windows-docker.html)。
- **macOS：** [Docker + RDP](https://ioes-lab.github.io/POSIM/ja/install-macos.html)。

[実行環境の選択](https://ioes-lab.github.io/POSIM/ja/requirements.html)で各方法を比較したら、
[ターミナルを準備](https://ioes-lab.github.io/POSIM/ja/terminals.html)して
[初めてのシミュレーション](https://ioes-lab.github.io/POSIM/ja/quickstart.html)に進んでください。

[機能とサンプル](https://ioes-lab.github.io/POSIM/ja/features.html)から、ロボット、ワールド、センサー、制御の各ガイドを参照できます。
[水中の描画効果とマリンスノー](https://ioes-lab.github.io/POSIM/ja/underwater-rendering.html)や
[海洋環境でのGNSS](https://ioes-lab.github.io/POSIM/ja/gnss.html)も紹介しています。

## Dockerイメージ

現在のイメージをダウンロードします。

```bash
docker pull ioeslab/posim:latest
```

Dockerがホストに合うAMD64版またはARM64版を選択します。同じバージョンで実験を再現する場合は、
[Docker Hub](https://hub.docker.com/r/ioeslab/posim/tags)で、コミットIDの先頭12文字を使った
`sha-<12-character-commit>`形式のタグを選んでください。
コンテナを作成する前に、[実行環境の選択](https://ioes-lab.github.io/POSIM/ja/requirements.html)から
Linux、Windows WSL、macOSそれぞれの画面表示の設定を確認してください。

## 関連アプリケーション

[WWW-POSIM](https://www-posim.vercel.app/)は、POSIMに地理的な地形、経路計画、機体制御のライブラリを組み合わせ、
GUIとWeb・デスクトップアプリケーションを提供します。アプリの操作方法はWWW-POSIMのドキュメントで説明しています。

## 開発への参加

不具合の報告や変更の提案は[IOES-Lab/POSIM](https://github.com/IOES-Lab/POSIM)で受け付けています。
開発時の確認事項とドキュメントの更新方法は、[コントリビューションガイド](https://ioes-lab.github.io/POSIM/ja/contributing.html)をご覧ください。

## ライセンスとクレジット

POSIMには[Project DAVE](https://github.com/Field-Robotics-Lab/dave)のソースコードが含まれており、著作者と著作権の表示を保持しています。
POSIMで作成したコードには[Apache License 2.0](LICENSE)が適用されます。第三者のコードやリソースには、それぞれの元の条件が適用されます。
外部ライブラリのWave Simは上流のリポジトリからビルドし、GPLの条件に従って利用します。
詳しくは[Wave Simのライセンスと配布に関する説明](extras/surface/README.md#licensing)をご覧ください。
