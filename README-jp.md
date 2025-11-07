# Google Meet HIDテレフォニーデバイス

RP2040-Zeroを使用した、真のHIDテレフォニープロトコル（Usage Page 0x0B）による双方向通信と視覚的フィードバックを実装したGoogle Meet用ハードウェアミュートボタンコントローラー。

## 概要

このプロジェクトは、単純なキーボードエミュレーションを超えたGoogle Meet専用のハードウェアミュートボタンを実装しています。HIDテレフォニー仕様を使用することで、適切なテレフォニーデバイスを作成し、WebHID API経由でGoogle Meetウェブアプリケーションとコマンド送信とステータス更新受信の両方が可能になります。

## 特徴

- **真のHIDテレフォニー実装** - キーボードエミュレーションではなくUsage Page 0x0Bを使用
- **双方向通信** - ミュートコマンド送信とステータス更新受信
- **視覚的フィードバック** - NeoPixel LEDが現在のミュート状態を表示
- **複合USBデバイス** - テレフォニーとコンシューマーコントロールインターフェースの組み合わせ
- **WebHID API統合** - Google Meetウェブアプリケーションとの直接通信
- **3Dプリント可能な筐体** - ハードウェア用のカスタム設計ケース

## ハードウェア要件

- **マイクロコントローラー**: RP2040-Zero (Waveshare)
- **コンポーネント**:
  - 内蔵BOOTボタン（ミュートボタンとして使用）
  - 内蔵NeoPixel LED (WS2812)
  - USB-Cケーブル（接続用）
- **オプション**: 3Dプリント筐体（STLファイル付属）

## ソフトウェア依存関係

### ファームウェア
- Arduino IDE (1.8.x または 2.x)
- Raspberry Pi Pico/RP2040ボードパッケージ（Earle Philhower版）
- Adafruit TinyUSBライブラリ
- Adafruit NeoPixelライブラリ
- Bounce2ライブラリ（ボタンデバウンス処理用）

### ホストアプリケーション
- Google Chrome（バージョン89以上、WebHIDサポート付き）
- Google Meetウェブアプリケーション

## インストール

### 1. Arduino IDEセットアップ

1. [arduino.cc](https://www.arduino.cc/)からArduino IDEをインストール
2. RP2040ボードサポートを追加:
   - 基本設定 → 追加のボードマネージャーURL
   - 追加: `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`
3. ボードパッケージをインストール:
   - ツール → ボード → ボードマネージャー
   - 検索して「Raspberry Pi Pico/RP2040」をインストール
4. 必要なライブラリをインストール:
   - Adafruit TinyUSBライブラリ
   - Adafruit NeoPixel
   - Bounce2

### 2. ファームウェアアップロード

1. BOOTボタンを押しながらRP2040-Zeroを接続
2. ボード選択: ツール → ボード → Raspberry Pi Pico
3. USBスタック選択: ツール → USBスタック → Adafruit TinyUSB
4. スケッチをアップロード

### 3. Google Meetセットアップ

1. ChromeでGoogle Meetを開く
2. プロンプトが表示されたらWebHIDデバイスの許可を承認
3. 会議に参加すると自動的にデバイスが接続されます

## 使用方法

- **ボタンを押す**: Google Meetのミュート/ミュート解除を切り替え
- **LEDインジケーター**:
  - 赤: ミュート
  - 緑: ミュート解除
  - 青: 接続中/初期化中

## 技術アーキテクチャ

### USBディスクリプタ構造
```
デバイス
├── 構成
│   ├── インターフェース 0: HIDテレフォニー
│   │   └── エンドポイント IN（割り込み）
│   │   └── エンドポイント OUT（割り込み）
│   └── インターフェース 1: コンシューマーコントロール
│       └── エンドポイント IN（割り込み）
```

### HIDレポート形式
- **入力レポート** (1バイト): フックスイッチ状態（ビット5）
- **出力レポート** (1バイト): ホストからのLED状態

### 主要な設計判断

1. **RP2040をATmega32U4/SAMD21より選択した理由**: ネイティブUSBサポート、デュアルコア処理、コスト効率
2. **TinyUSBをHID-Projectより選択した理由**: より良い複合デバイスサポート、より柔軟なディスクリプタ管理
3. **HIDテレフォニーをキーボードエミュレーションより選択した理由**: 適切なセマンティックの意味、キーストロークの競合なし、双方向通信
4. **WebHIDをネイティブアプリケーションより選択した理由**: クロスプラットフォーム互換性、ドライバーインストール不要


## 将来の拡張

- **拡張コントロール**: 音量調整、ビデオ切り替え、画面共有
- **複数のUsage ID**: 異なる会議プラットフォームのサポート
- **設定インターフェース**: WebUSBベースの設定管理
- **ESP32-S3への移行**: リモート制御用のWi-Fi統合
- **機能レポート**: HID機能レポート経由の高度な設定
- **PIO最適化**: 高度なLED効果のためのRP2040のPIO使用

## トラブルシューティング

### デバイスが認識されない
- ChromeにWebHIDサポートが有効になっていることを確認
- USBケーブルがデータ転送をサポートしていることを確認
- 別のUSBポートを試す

### LEDが更新されない
- Google Meetにデバイスの権限があることを確認
- Meetの設定でミュート状態の同期が有効になっていることを確認

### ボタンが動作しない
- ChromeのHIDデバイスリストでデバイスが表示されていることを確認
- デバッグ出力のためのシリアルモニターを確認
- 適切なUSBディスクリプタの列挙を確保

## 貢献

貢献を歓迎します！バグや機能リクエストのために、プルリクエストを送信したり、イシューを開いたりしてください。

## 参考文献

- [USB HID使用テーブル 1.12](https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf)
- [WebHID API仕様](https://wicg.github.io/webhid/)
- [RP2040データシート](https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf)
- [TinyUSBドキュメント](https://docs.tinyusb.org/)
- [Bounce2ライブラリ](https://github.com/thomasfredericks/Bounce2)

## ライセンス

MIT License

Copyright (c) 2025 TOKIDA

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


## 謝辞

- AdafruitのTinyUSBライブラリと優れたドキュメント
- Raspberry Pi FoundationのRP2040開発ツール
- Google ChromeチームのWebHID API実装
- Thomas O FredericksのBounce2ライブラリ
- 様々なコード例とインスピレーションを提供したオープンソースコミュニティ
