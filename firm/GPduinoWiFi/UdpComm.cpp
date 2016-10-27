// UDP通信処理クラス
#include <string.h>
#include "UdpComm.h"

// デフォルトのポート番号
#define DEFAULT_LOCAL_PORT  0xC000
#define DEFAULT_REMOTE_PORT 0xC001

// 無効なIPアドレス
static IPAddress NULL_ADDR(0,0,0,0);

// コンストラクタ
UdpComm::UdpComm()
{
    this->localPort = DEFAULT_LOCAL_PORT;
    this->remotePort = DEFAULT_REMOTE_PORT;
    
    onReceive = NULL;
    remoteIP = NULL_ADDR;
}

// コンストラクタ(ポート番号指定)
UdpComm::UdpComm(int localPort, int remotePort)
{
    this->localPort = localPort;
    this->remotePort = remotePort;
    
    onReceive = NULL;
    remoteIP = NULL_ADDR;
}

// APモードで開始する
void UdpComm::beginAP(char* ssid, char* password)
{
    mode = UDP_COMM_MODE_AP;
    
    if(ssid == NULL){
        sprintf(this->mySSID, "gpduino-%06x", ESP.getChipId());
    }else{
        strncpy(mySSID, ssid, sizeof(mySSID)-1);
        mySSID[sizeof(mySSID)-1] = '\0';
    }
    
    strncpy(myPassword, password, sizeof(myPassword)-1);
    mySSID[sizeof(myPassword)-1] = '\0';
    
    // APの設定
    WiFi.mode(WIFI_AP);
    WiFi.softAP(mySSID, myPassword);
    localIP = WiFi.softAPIP();
    Serial.println();
    Serial.print("AP SSID: ");Serial.println(mySSID);
    Serial.print("AP IP address: ");Serial.println(localIP);
    remoteIP = NULL_ADDR;

    // UDPの開始
    udp.begin(localPort);
}

// STAモードで開始する
void UdpComm::beginSTA(char* ssid, char* password, char* hostName)
{
    mode = UDP_COMM_MODE_STA;
    isConnected = false;
    
    strncpy(hisSSID, ssid, sizeof(hisSSID)-1);
    hisSSID[sizeof(hisSSID)-1] = '\0';
    
    strncpy(hisPassword, password, sizeof(hisPassword)-1);
    hisPassword[sizeof(hisPassword)-1] = '\0';
    
    if(hostName == NULL){
        sprintf(this->hostName, "esp8266-%06x", ESP.getChipId());
    }else{
        strncpy(this->hostName, hostName, sizeof(this->hostName)-1);
        this->hostName[sizeof(this->hostName)-1] = '\0';
    }
    //Serial.print("HostName: ");  Serial.println(hostName);

    // STAの設定
    WiFi.mode(WIFI_STA);
    WiFi.begin(hisSSID, hisPassword);
}

// 周期処理 loop()で必ず呼び出す
void UdpComm::loop()
{
    if(mode == UDP_COMM_MODE_AP)
    {
        loopAP();
    }else{
        loopSTA();
    }
    // onReceive(buff);
}

// 周期処理 (APモード)
void UdpComm::loopAP()
{
    // パケット受信があればデータ取得
    int packetSize = udp.parsePacket();
    if (packetSize) {
        if(packetSize > (UDP_COMM_BUFF_SIZE-1)){
            packetSize = UDP_COMM_BUFF_SIZE-1;
        }
        int len = udp.read(buff, packetSize);
        if (len > 0) buff[len] = '\0';
        udp.flush();

        Serial.print("loopAP:");Serial.println(buff);
        
        // 相手のIPアドレス取得
        remoteIP = udp.remoteIP();
        
        // コールバック
        if(onReceive != NULL) onReceive(buff);
        
        //Serial.print(remoteIP);
        //Serial.print(" / ");
        //Serial.println(packetBuffer);
    }
}

// 周期処理 (STAモード)
void UdpComm::loopSTA()
{
    // APへの接続待ち
    if (WiFi.status() != WL_CONNECTED) {
      if(isConnected){
        isConnected = false;
        // UDPの停止
        udp.stop();
      }
      //delay(500);
      Serial.print(".");
      return;
    }
    
    // 接続時の処理
    if(!isConnected){
      isConnected = true;
      
      // IPアドレスの取得
      localIP = WiFi.localIP();
      Serial.println();
      Serial.print("Connected to "); Serial.println(hisSSID);
      Serial.print("STA IP address: "); Serial.println(localIP);
      remoteIP = NULL_ADDR;
      
      // UDPの開始
      udp.begin(localPort);
      if ( mdns.begin ( hostName, localIP ) ) {
        Serial.println ( "MDNS responder started" );
      }else{
        Serial.println("Error setting up MDNS responder!");
      }
    }
    
    // MDNSの更新
    mdns.update();
    
    // 以降はAPモードと同じ処理
    loopAP();

    //localIP = WiFi.localIP();
    //Serial.println();
    //Serial.print("AP SSID: ");Serial.println(mySSID);
    //Serial.print("AP IP address: ");Serial.println(localIP);
}

// データを送信する
void UdpComm::send(char* data)
{
    if(remoteIP != NULL_ADDR){
        udp.beginPacket(remoteIP, remotePort);
        udp.write(data);
        udp.endPacket();
        Serial.println(data);
    }
}

// 通信準備OKか？
bool UdpComm::isReady()
{
    if(mode == UDP_COMM_MODE_AP)
    {
        return true;
    }else{
        return isConnected;
    }
}
