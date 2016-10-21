#ifndef    _UDP_COMM_H_
#define    _UDP_COMM_H_

#include <ESP8266WiFi.h> // WiFi
#include <ESP8266mDNS.h> // mDNS
#include <WiFiUDP.h>     // UDP

// 受信バッファサイズ
#define UDP_COMM_BUFF_SIZE 64
// モード
#define UDP_COMM_MODE_AP    0
#define UDP_COMM_MODE_STA   1

// UDP通信処理クラス
class UdpComm
{
public:
    UdpComm();
    UdpComm(int localPort, int remotePort);
    // APモードで開始する
    void beginAP(char* ssid, char* password);
    // STAモードで開始する
    void beginSTA(char* ssid, char* password, char* hostName);
    // 周期処理 loop()で必ず呼び出す
    void loop();
    // データを送信する
    void send(char* data);
    // 通信準備OKか？
    bool isReady();
    
    // 受信コールバック
    void (*onReceive)(char* data);
    
    // 自分のSSIDとパスワード(APモード)
    char mySSID[33];
    char myPassword[64];
    // APのSSIDとパスワード(STAモード)
    char hisSSID[33];
    char hisPassword[64];
    // 自分のIPアドレスとポート番号
    IPAddress localIP;
    int localPort;
    // 相手のIPアドレスとポート番号
    IPAddress remoteIP;
    int remotePort;
    // ホスト名
    char hostName[32];
    
private:
    // 周期処理 (APモード)
    void loopAP();
    // 周期処理 (STAモード)
    void loopSTA();
    
    // APモード or STAモード
    int mode;
    // 受信データのバッファ
    char buff[UDP_COMM_BUFF_SIZE];
    // UDPオブジェクト
    WiFiUDP udp;
    // mDNSオブジェクト
    MDNSResponder mdns;
    // APに接続しているか？(STAモード時)
    bool isConnected;
};

#endif
