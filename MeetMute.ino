/*
 * RP2040-Zero Google Meet 双方向ミュート + ボリュームノブ (V5)
 * * 複合HIDデバイス (Telephony + Consumer Control)
 *
 * 機能:
 * 1. GP14 (Mute): Google Meetのマイクミュートをトグル (Telephony Mute)
 * 2. GP15 (Hook): Google Meetの通話状態をトグル (Telephony Hook)
 * 3. GP16 (LED): Google Meetのミュート状態と同期
 * 4. A0/GP26 (Pot): OSのマスターボリュームを制御 (Volume Up / Down)
 */

#include "Adafruit_TinyUSB.h"
#include <Bounce2.h>
#include <Adafruit_NeoPixel.h>

// =================================================================
// == 1. ピン定義 ==
// =================================================================

// --- Telephony ---
#define MUTE_BUTTON_PIN 14
#define HOOK_BUTTON_PIN 15
#define LED_PIN 16
#define NEOPIXEL_NUM 1

// --- Consumer Control ---
#define POT_PIN A0 // ポテンショメーター (GP26)

// =================================================================
// == 2. HIDレポートディスクリプタ ==
// =================================================================

// --- レポートID定義 (デバイス全体でユニーク) ---
#define REPORT_ID_TELEPHONY_IN  1
#define REPORT_ID_TELEPHONY_OUT 2
#define REPORT_ID_CONSUMER      3 // ★ 新規追加

// --- 2a. Telephony デバイス (Mute/Hook) ---
uint8_t const desc_telephony_report[]= {
  0x05, 0x0B,       // Usage Page (Telephony)
  0x09, 0x05,       // Usage (Headset)
  0xA1, 0x01,       // Collection (Application)
  // Input (Hook/Mute Buttons)
  0x85, REPORT_ID_TELEPHONY_IN, //   Report ID (1)
  0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x02,
  0x09, 0x20,       //   Usage (Hook Switch) [Bit 0]
  0x09, 0x2F,       //   Usage (Phone Mute)  [Bit 1]
  0x81, 0x02,       //   Input (Data,Var,Abs)
  0x75, 0x06, 0x95, 0x01, 0x81, 0x03, // Padding
  // Output (Mute LED)
  0x85, REPORT_ID_TELEPHONY_OUT, //   Report ID (2)
  0x05, 0x08,       //   Usage Page (LEDs)
  0x09, 0x09,       //   Usage (Mute)
  0x75, 0x01, 0x95, 0x01, 0x91, 0x02, // Output (Data,Var,Abs)
  0x75, 0x07, 0x95, 0x01, 0x91, 0x03, // Padding
  0xC0              // End Collection
};

// --- 2b. Consumer Control デバイス (Volume) ---
uint8_t const desc_consumer_report[] = {
  0x05, 0x0C,       // Usage Page (Consumer)
  0x09, 0x01,       // Usage (Consumer Control)
  0xA1, 0x01,       // Collection (Application)
  0x85, REPORT_ID_CONSUMER, // Report ID (3)
  0x15, 0x00,       // Logical Minimum (0)
  0x25, 0x01,       // Logical Maximum (1)
  0x75, 0x01,       // Report Size (1 bit)
  0x95, 0x02,       // Report Count (2)
  0x09, 0xE9,       //   Usage (Volume Increment) [Bit 0]
  0x09, 0xEA,       //   Usage (Volume Decrement) [Bit 1]
  0x81, 0x02,       //   Input (Data,Var,Abs)
  0x75, 0x06, 0x95, 0x01, 0x81, 0x03, // Padding
  0xC0              // End Collection
};


// =================================================================
// == 3. グローバルオブジェクトと変数 ==
// =================================================================

// --- 2つのHIDインターフェースを作成 ---
Adafruit_USBD_HID usb_telephony(desc_telephony_report, sizeof(desc_telephony_report), 
                               HID_ITF_PROTOCOL_NONE, 2, false);
                               
Adafruit_USBD_HID usb_consumer(desc_consumer_report, sizeof(desc_consumer_report), 
                              HID_ITF_PROTOCOL_NONE, 2, false);

// --- Telephony 関連 ---
Adafruit_NeoPixel pixel(NEOPIXEL_NUM, LED_PIN, NEO_GRB + NEO_KHZ800);
Bounce2::Button mute_button = Bounce2::Button();
Bounce2::Button hook_button = Bounce2::Button();
static bool current_mute_state = false;
static bool current_hook_state = false;

// --- Consumer Control 関連 (V6 - スムーズ化対応) ---
int lastPotValue = -1; // 前回のポテンショメーターの値 (0-1023)
unsigned long lastVolumeAdjustTime = 0; // 最後に音量を調整した時刻
const long VOLUME_ADJUST_INTERVAL = 10; // 10ms (チェック間隔を短くし、反応性を上げる)
const int POT_HYSTERESIS = 8;           // 反応閾値を小さくする (0-1023のうち8)
                                        // (ノイズが多い場合は 10 や 12 に増やしてください)

// =================================================================
// == 4. セットアップ関数 ==
// =================================================================
void setup() {
  Serial.begin(115200);

  // --- Telephony (Mute/Hook/LED) の設定 ---
  mute_button.attach(MUTE_BUTTON_PIN, INPUT_PULLUP);
  mute_button.interval(25);
  hook_button.attach(HOOK_BUTTON_PIN, INPUT_PULLUP);
  hook_button.interval(25);
  pixel.begin();
  pixel.setBrightness(40);
  pixel.setPixelColor(0, pixel.Color(0, 255, 0)); // 初期色は緑
  pixel.show();

  // --- ポテンショメーターのピン設定 ---
  // analogRead() を使うので pinMode は不要

  // --- 複合HIDの開始 ---
  // 1. Telephony のコールバック登録
  usb_telephony.setReportCallback(NULL, hid_set_report_callback);
  // 2. Telephony デバイスを開始
  usb_telephony.begin();
  // 3. Consumer Control デバイスを開始
  usb_consumer.begin();

  Serial.println("RP2040 Mute/Hook/Volume Knob V5 - 起動しました");
  
  while(!TinyUSBDevice.mounted()) delay(1); 
  
  Serial.println("USB HID (Composite) 接続完了");

  // 起動時に現在の初期状態をPCに送信
  sendTelephonyReport();
}

// =================================================================
// == 5. メインループ ==
// =================================================================
void loop() {
  // HIDの準備ができていない場合は待機
  if (!usb_telephony.ready() || !usb_consumer.ready()) { 
    return; 
  }

  // 1. Mute/Hook ボタンの処理
  handleButtons();

  // 2. ボリュームノブ (ポテンショメーター) の処理
  handleVolumePot();
}

// =================================================================
// == 6. サブ関数 (Telephony) ==
// =================================================================

// --- Mute/Hook ボタン処理 ---
void handleButtons() {
  mute_button.update();
  hook_button.update();
  bool report_needed = false;

  if (mute_button.fell()) {
    Serial.println("Mute (GP14) 押下");
    current_mute_state = !current_mute_state;
    set_led_color(current_mute_state);
    report_needed = true;
  }

  if (hook_button.fell()) {
    Serial.println("Hook (GP15) 押下");
    current_hook_state = !current_hook_state;
    report_needed = true;
  }

  if (report_needed) {
    sendTelephonyReport();
  }
}

// --- Telephony レポート送信 ---
void sendTelephonyReport() {
  if (!usb_telephony.ready()) return;
  uint8_t report_data = 0;
  if (current_hook_state) report_data |= 0b00000001; // Bit 0
  if (current_mute_state) report_data |= 0b00000010; // Bit 1
  
  usb_telephony.sendReport(REPORT_ID_TELEPHONY_IN, &report_data, 1);
  
  Serial.print("PCへ送信 (ID 1): ");
  Serial.print("Hook = "); Serial.print(current_hook_state ? "ON" : "OFF");
  Serial.print(", Mute = "); Serial.println(current_mute_state ? "ON" : "OFF");
}

// --- PCからのMute状態受信コールバック ---
void hid_set_report_callback(uint8_t report_id, hid_report_type_t report_type, 
                             uint8_t const* buffer, uint16_t bufsize) 
{
  if (report_id == REPORT_ID_TELEPHONY_OUT && report_type == HID_REPORT_TYPE_OUTPUT && bufsize >= 1) {
    Serial.println("PCからMute状態受信 (ID 2)");
    bool host_mute_status = (buffer[0] & 0b00000001); 
    set_led_color(host_mute_status);
    current_mute_state = host_mute_status; 
  }
}

// --- LED制御 ---
void set_led_color(bool is_muted) {
  if (is_muted) {
    pixel.setPixelColor(0, pixel.Color(255, 0, 0)); // 赤
  } else {
    pixel.setPixelColor(0, pixel.Color(0, 255, 0)); // 緑
  }
  pixel.show();
}

// =================================================================
// == 7. サブ関数 (Consumer Control) ==
// =================================================================

// --- ボリュームノブ処理 (V6 - スムーズ化対応) ---
void handleVolumePot() {
  // 処理が頻繁すぎないようにスロットリング
  unsigned long currentTime = millis();
  if (currentTime - lastVolumeAdjustTime < VOLUME_ADJUST_INTERVAL) {
    return; // 10ms 待つ
  }
  lastVolumeAdjustTime = currentTime; // チェック時刻を更新

  // ポテンショメーターの値を読み取り (0-1023)
  int potValue = analogRead(POT_PIN);
  uint8_t report_data = 0; // 0 = 何もしない

  // 最初の読み取り時 (初期化)
  if (lastPotValue == -1) {
    lastPotValue = potValue;
    return;
  }
  
  // 閾値(8)を超えて増加したか？
  if (potValue > lastPotValue + POT_HYSTERESIS) {
    report_data = 0b00000001; // Volume Up
    Serial.println("Volume Up ->");
    lastPotValue = potValue; // ★送信した時だけ、その値で更新
  }
  // 閾値(8)を超えて減少したか？
  else if (potValue < lastPotValue - POT_HYSTERESIS) {
    report_data = 0b00000010; // Volume Down
    Serial.println("<- Volume Down");
    lastPotValue = potValue; // ★送信した時だけ、その値で更新
  }
  // (それ以外は何もしない。lastPotValueも更新しない)
  // (ノブが停止していれば、差分が出ないので送信も停止する)

  // 変化があった場合 (0以外) のみレポートを送信
  if (report_data != 0) {
    // Consumer Control (ID 3) でレポートを送信
    usb_consumer.sendReport(REPORT_ID_CONSUMER, &report_data, 1);
    
    // ★重要: 押したらすぐに離す（キーリリースのレポート）
    // リリースまでの遅延を短くする
    delay(5); 
    report_data = 0; // すべてのボタンを離す
    usb_consumer.sendReport(REPORT_ID_CONSUMER, &report_data, 1);
  }
}
