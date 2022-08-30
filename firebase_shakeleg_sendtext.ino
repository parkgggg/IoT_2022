#include "HX711.h" // 무게 측정 센서 HX711 헤더 포함.
#include <WiFi.h> // 와이파이를 사용하기 위해 Wifi 헤더파일 포함.
#include <Wire.h>
#include "time.h"
#include <AWS_IOT.h>
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
HX711 scale;

#include <Firebase_ESP_Client.h>
#include <Arduino.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define API_KEY "AIzaSyCxyq45rLEH57zZCVGrSxqgalooHNE7r0Q" //파이어베이스 연결을 위한 설정들
#define USER_EMAIL "ghp720@naver.com"
#define USER_PASSWORD "123456789"
#define DATABASE_URL "https://esp-firebase-pratice-log-default-rtdb.asia-southeast1.firebasedatabase.app"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String uid;
String databasePath;
String timePath = "/timestamp"; //파이어베이스 DB에 저장되는 json 키 경로들
String weightPath = "/sit";
String shakecountPath = "/shake";
String parentPath; 
FirebaseJson json;

int timestamp; //timestamp 변수

unsigned long sendDataPrevMillis = 0; //이전 푸시 시간 저장할 변수
unsigned long timerDelay = 30000; //파이어 베이스에 값을 푸시해줄 주기(기준 시간)(30초) => 사용자 정의


AWS_IOT sendtext; //AWS iot 사용을 위한 선언
char HOST_ADDRESS[] = "azr0rfcqbobyt-ats.iot.ap-southeast-1.amazonaws.com";
char CLIENT_ID[] = "amplify_test";
char pTOPIC_NAME[] = "esp32/shake"; // json데이터를 보낼 주제


const char* ssid = "Flip3"; // 와이파이 SSID
const char* password = "qus78522"; // 와이파이 Password
const char* ntpServer = "Kr.pool.ntp.org"; // ntp 한국으로 설정
const long gmtOffset_sec = 32400; //3200초는 1시간, 우리나라는 GMT+9 이므로 3600초x9 = 32400
const int daylightOffset_sec = 0;

// HX711 circuit wiring
#define LOADCELL_DOUT_PIN 16 //GPIO 16
#define LOADCELL_SCK_PIN 4 // GPIO 4

// scale - 10Kg loadcell : 226 / 5kg loadcell : 372
// ADC 모듈에서 측정된 결과값을 (loadcellValue)값 당 1g으로 변환해 줌
float loadcellValue = 372.0;
float Weight;

// SW18015P
const int leftVib = 17; // GPIO 17
const int rightVib = 5;
const int buz = 23;

// 수행 시간 측정을 위한 변수 선언
// 1초 단위면 millisTime = millis()/1000;
unsigned long timeVal = 0; //이전시간
unsigned long millisTime = 0; //현재시간
unsigned long countTime = 0; //카운트시작시간
unsigned long minutecount = 0;

//파이어베이스에 올려줄 값파이어베이스에 올려줄 값들
unsigned long sittingminuteperhour = 0; // 1.시간당 앉아있던 시간(분 단위) => 아직 밑에서 이대로 설정은 안 해줬음
unsigned long vibcountperhour = 0; // 2.시간당 다리 떤 횟수 (파이어베이스 푸시 후에 초기화 될 것)
unsigned long vibcountperminute = 0; // 분당  다리 떤 횟수(부저가 울리면 초기화 될 것)
unsigned long sittingsave = 0; //한 시간 동안 앉아만 있는 게 아니기 때문에 중간에 일어났다가 앉는 경우를 대비해 앉아있는 시간을 저장해줄 변수 (파이어베이스 푸시 x)


// 진동 센서 SW18015P 값을 받아오는 함수.
long left_TP() {
  delay(10);
  long Vibration = pulseIn (leftVib, HIGH);
  return Vibration;
}

long right_TP() {
  delay(10);
  long Vibration = pulseIn (rightVib, HIGH);
  return Vibration;
}

unsigned long getTime() { //현재 시간 (타임스탬프) 반환하는 함수
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return (0);
  }
  time(&now);
  return now;
}

void setup()
{
  Serial.begin(115200);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" CONNECTED");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  ////파이어베이스 연결
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  }
  //파이어베이스 연결 성공하면 User ID 출력
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.println(uid);

  // 실제 값들이 저장될 파이어베이스 db 경로
  databasePath = "/UsersData/" + uid + "/readings";

  //AWS 연결
  if (sendtext.connect(HOST_ADDRESS, CLIENT_ID) == 0) {
    Serial.println("Connected to AWS");
    delay(1000);
  }
  else {
    Serial.println("AWS connection failed, Check the HOST Address");
    while (1);
  }

  pinMode(leftVib, INPUT); //센서핀 입력
  pinMode(rightVib, INPUT);
  pinMode(buz, OUTPUT);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);// 로드셀 HX711 보드 pin 설정
  delay(2000);// 부팅 후 잠시 대기 (2초)
  scale.set_scale(loadcellValue); // 스케일 설정
  scale.tare(10); // 오프셋 설정(10회 측정 후 평균값 적용) - 저울 위에 아무것도 없는 상태를 0g으로 정하는 기준점 설정
  Serial.println("———————————Start————————————");

}


void loop()
{
  // 오프셋 및 스케일이 적용된 측정값 출력 (5회 측정 평균값)
  Serial.print("Weight value :\t");
  Weight = fabs(scale.get_units(5));
  // 5회 측정 평균값, 소수점 아래 2자리 출력
  Serial.print(Weight);
  Serial.println(" g");

  ///파이어베이스 연결되어있고, DB에 푸시한 지 timerDelay만큼 시간이 지났으면(현재 시간 - 지난 번 푸시시간) 조건문 실행
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay)) {
    ////이번 푸시하는 시간 저장
    sendDataPrevMillis = millis();
    countTime = millis();
    
    timestamp = getTime(); //타임스탬프 값 가져옴

    parentPath = databasePath + "/" + String(timestamp); //DB 루트 경로에
    sittingminuteperhour += sittingsave;
    json.set(weightPath, String((sittingminuteperhour)));
    json.set(shakecountPath, String(vibcountperhour));
    json.set(timePath, String(timestamp));
    Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
    sittingminuteperhour = 0;
    sittingsave = 0;
    vibcountperhour = 0;
  }


  if (Weight > 2000) // 2kg을 기준으로 시간을 측정한다.
  {
    Serial.println("무게가 기준 2kg을 넘었습니다. / 타이머를 측정합니다.");

    if (millis() - timeVal >= 1000) { //1초단위로 출력
      timeVal = millis();
      millisTime = (millis() - countTime) / 1000; //한 번 착석 후 앉아있었던 시간
      sittingminuteperhour = millisTime;//앉아있는 경우는 계속해서 저장해줌

      long Vibrationleft = left_TP();//진동 측정
      long Vibrationright = right_TP();
      Serial.print("leftVib measurement : ");
      Serial.println(Vibrationleft);
      Serial.print("rightVib measurement : ");
      Serial.println(Vibrationright);

      if (Vibrationleft > 5 || Vibrationleft > 5)//임계값(5)보다 큰 경우 다리떠는 상황으로 판단
      {
        vibcountperhour += 1; 
        vibcountperminute += 1.3;
      }

      if (timeVal - minutecount >= 25000) //25초 동안 다리떨기 카운트가 임계값 이상일 때 부저울림(아니면 카운트 초기화)
      {
        if (vibcountperminute >= 5)
        {
          digitalWrite(buz, HIGH);
        }
        minutecount = timeVal;
        vibcountperminute = 0;
      }
      digitalWrite(buz, LOW);

      // 한 번에 앉아있는 시간이 설정된 값(지속적으로 앉아있을 시 1분마다)이 되면 Aws sns를 사용해 문자로 사용자에게 알린다.
      if (millisTime % 60000 == 0) {
        Serial.println("1분이 지났습니다.");
        ///Json형식으로 aws iot에 페이로드를 날림
        StaticJsonDocument<48> doc;
        doc["Message"] = "One hour of sitting in the chair has passed. Get up and go for a 5 minute walk!!";
        char jsonBuffer[512];
        serializeJson(doc, jsonBuffer);      // print to client
        if (sendtext.publish(pTOPIC_NAME, jsonBuffer) == 0) {
          Serial.print("Publish Message:");
          Serial.println(jsonBuffer);
        }
        else
          Serial.println("Publish failed");
      }
    }
  }
  else {
    // 측정된 시간을 다시 초기화한다.
    sittingsave += sittingminuteperhour;
    sittingminuteperhour = 0;
    countTime = millis();
    timeVal = countTime;
  }
}
