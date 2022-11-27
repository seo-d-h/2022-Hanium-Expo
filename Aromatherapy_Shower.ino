#include <SoftwareSerial.h>    // 블루투스 시리얼통신 헤더파일
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LiquidCrystal_I2C.h>    //LCD 헤더파일
#include <Adafruit_MLX90614.h>    // 온도센서 헤더파일
#include <DFPlayer_Mini_Mp3.h>    // mp3 헤더파일
#include <Servo.h>                // 수전제어 액추에이터

#define PIN_SERVO 12
#define TRIG 9 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 8 //ECHO 핀 설정 (초음파 받는 핀)
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

int Dir1Pin_A = 11;      // 제어신호 1핀
int Dir2Pin_A = 10;      // 제어신호 2핀

// 아로마 오일 펌프 핀세팅
int IN1 = 2;
int IN2 = 3;

int IN3 = 6;
int IN4 = 7;

int IN5 = 22;
int IN6 = 23;

int d = 1;
int delayMS = 50;

char boot_flag = '0';
char input_data = '0';
char recommended_oil = '0';

////////////////////////////////////////////
//              함수 선언부               //
///////////////////////////////////////////
int ultrasonic(int TR, int EC);
int lcd_height(long distance);
int lcd_temperature();
void actuator(long distance);
void lavendula();
void SetStrokePerc(float);
void SetStrokeMM(int ,int);

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);    // 블루투스
  Serial2.begin(9600);    // mp3  
  ///////////////////////////////////////////
  //                 초음파                 //
  ///////////////////////////////////////////
  pinMode(TRIG, OUTPUT);                   // 초음파 트리거
  pinMode(ECHO, INPUT);                    //초음파 에코
  
  ///////////////////////////////////////////
  //             액추에이터                  //
  ///////////////////////////////////////////
  pinMode(Dir1Pin_A, OUTPUT);             // 액추에이터 제어 1번핀 출력모드 설정
  pinMode(Dir2Pin_A, OUTPUT);             // 액추에이터 제어 2번핀 출력모드 설정

  ///////////////////////////////////////////
  //             OLED 디스플레이             //
  ///////////////////////////////////////////
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.setTextSize(2);
  display.setTextColor(WHITE);

  ///////////////////////////////////////////
  //             아로마 오일 펌프           //
  ///////////////////////////////////////////
  pinMode(IN1,OUTPUT);
  pinMode(IN2,OUTPUT);
  pinMode(IN3,OUTPUT);
  pinMode(IN4,OUTPUT);
  
  ///////////////////////////////////////////
  //             LCD디스플레이              //
  ///////////////////////////////////////////
  mlx.begin();
  lcd.begin (16,2);
  //lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.backlight(); //Lighting backlight
  lcd.home ();

  ///////////////////////////////////////////
  //                mp3 모듈               //
  ///////////////////////////////////////////  
  mp3_set_serial(Serial2);
  mp3_set_volume(30);
  ///////////////////////////////////////////
  //               수전 액추에이터          //
  ///////////////////////////////////////////  
  myServo.attach(PIN_SERVO);
}


void loop() {   // main 함수
  ///////////////////////////////////////////
  //            블루투스 시리얼통신            //
  ///////////////////////////////////////////
//  if (Serial1.available()) {     // 블루투스(앱) -> 아두이노 -> 시리얼 모니터
//    Serial.write(Serial1.read());
//  }
//  if (Serial.available()) {     // 시리얼 모니터 -> 아두이노 -> 블루투스(앱)
//    Serial1.println(Serial.read());
//    //Serial.write(Serial.read());
//  } 
//  while(1){
//    if(Serial1.available()){
//    Serial.println(Serial1.read());
//    if(Serial1.read() == 'a'){
//      break;
//      }
//    }
//  }
  
  ///////////////////////////////////////////
  //             액추에이터                  //
  ///////////////////////////////////////////
  long distance;    // 초음파센서로 측정한 거리
 
  while(1){
    mp3_play(1);      // "키측정을 시작합니다"

  delay(5000);
    distance = ultrasonic(TRIG, ECHO);
    Serial.println(distance);
    lcd_height(distance);
    if((distance >= 3) && (distance <= 30)) {
      actuator(distance);  
      Serial1.println('1');
      break;
      }
    }

  mp3_play(2);      // "샤워기 높이설정이 완료되었습니다."
  delay(5000);
  Serial1.println('1');
  
  mp3_play(3);      // "표정인식을 시작합니다."
  delay(5000);

  ///////////////////////////////////////////
  //               표정인식                 //
  ///////////////////////////////////////////
  while(input_data == '0'){             // input_data가 0이 아니면 탈출
//    while(Serial.available()){
//        input_data = Serial.read();   // 1 : natural, 2 : angry, 3 : happy, 4 : sad
//        }
    if(Serial.available()){                 
      input_data = Serial.read();       // jetsom -> arduino
      if(input_data == '1'){
        Serial.println("1 : happy");    // 아두이노 터미널창 출력
        Serial1.println('1');           // arduino -> app
       mp3_play(14);     // "라벤더"
       delay(5000);        
      }
      else if(input_data == '2'){
        Serial.println("2 : angry");
        Serial1.println('2');    
       mp3_play(15);     // "로즈마리"
       delay(5000);        
        }
      else if(input_data == '3'){
        Serial.println("3 : sad");
        Serial1.println('3');
       mp3_play(16);     // "만다린"
       delay(5000);        
        }
      else{
        Serial.println("I can't detect your face");
        }
      }
    }

  //////////////////////////////////////////
  //  앱에서 추천받은 아로마 오일 받아오기 //
  /////////////////////////////////////////
 while(recommended_oil == '0'){
   if(Serial1.available()){
     recommended_oil = Serial1.read();    // app -> arduino or Serial1.read()
   }
 }
  
  mp3_play(4);      // "체온측정을 시작합니다."
  delay(5000);
  lcd_temperature();
  
  mp3_play(5);      // "물 배출을 시작합니다."
  delay(5000);
  Serial1.println("1");

  //Servo myServo; 
  
  for ( int i = 1; i <45; i += d ){   //  물 틀기
    SetStrokePerc(i);
    delay(delayMS);
  }

  ///////////////아로마 오일 주입 코드////////////////////
  switch(input_data){
    case '1':
      for(int i = 0;i<3;i++){
        lavendula();                      // 라벤더
        }
      break;

    case '2':
      for(int i = 0;i<3;i++){
        rosmarinus();                      // 로즈마리
        }
      break;

    case '3':
      for(int i = 0;i<3;i++){
        mandarin();                      // 만다린
        }
      break;
  }
  
  mp3_play(6);      // "1분후 샤워가 종료됩니다."
  delay(5000);
  
  for ( int i = 45; i > 1;  i -= d ){  //  물 끄기
    SetStrokePerc(i);
    delay(delayMS);
  }

  mp3_play(7);      // "샤워가 종료되었습니다. 만족도 측정을 해주세요"
  delay(5000);
  
  Serial1.println('1');
  digitalWrite(Dir1Pin_A, HIGH);          //액추에이터 하강
  digitalWrite(Dir2Pin_A, LOW);

}

////////////////////////////////////////////
//             함수 정의부                //
///////////////////////////////////////////
int lcd_height(long distance){
   lcd.setCursor(0,0);
   lcd.print("height:");
   lcd.print(180-distance);
   lcd.print("cm");
}

int lcd_temperature(){                // 디스플레이 함수
   lcd.setCursor(0,1);
   lcd.print("Target  ");
   lcd.print(mlx.readObjectTempC());
   lcd.print(" C");
   Serial1.println(mlx.readObjectTempC()-20);

  delay(1000);
}

int ultrasonic(int TR, int EC){     // 초음파 함수
  long duration, distance;
  digitalWrite(TR,LOW);
  delayMicroseconds(2);
  digitalWrite(TR, HIGH);
  delayMicroseconds(10);
  digitalWrite(TR, LOW);
  duration = pulseIn(EC, HIGH);
  distance = duration * 17 / 1000;     //초음파센서의 거리값이 위 계산값과 동일하게 Cm로 환산되는 계산공식. 수식이 간단해지도록 적용
  return distance;
}

void actuator(long distance){
  Serial.print("\nDIstance : ");
  Serial.print(distance); //측정된 물체로부터 거리값(cm값)을 보여줌
  Serial.println(" Cm");

  if(distance != 0){
  //500mm 액추에이터 기준, 1초당 10mm동작
    if ((15 <= distance) && (distance < 20)){
      Serial1.println(180-distance);
       digitalWrite(Dir1Pin_A, LOW);            //액추에이터 상승
       digitalWrite(Dir2Pin_A, HIGH);
       delay(10000);
       digitalWrite(Dir1Pin_A, LOW);          //액추에이터 멈춤
       digitalWrite(Dir2Pin_A, LOW);
       pinMode(TRIG, OUTPUT);                   
       pinMode(ECHO, OUTPUT);                   
    }
      
    else if ((10 <= distance) && (distance < 15)){
      Serial1.println(180-distance);
       digitalWrite(Dir1Pin_A, LOW);         // 액추에이터 하강
       digitalWrite(Dir2Pin_A, HIGH);
       delay(20000);
       digitalWrite(Dir1Pin_A, LOW);         //모터가 멈춤
       digitalWrite(Dir2Pin_A, LOW);
       pinMode(TRIG, OUTPUT);                
       pinMode(ECHO, OUTPUT);
       //만약 150 < distance < 160이후 이어서 동작 시킬려면  1500으로 수정
      }
      
    else if ((0 <= distance) && (distance < 10)){
      Serial1.println(180-distance);
       digitalWrite(Dir1Pin_A, LOW);           //액추에이터 상승
       digitalWrite(Dir2Pin_A, HIGH);
       delay(30000);
       digitalWrite(Dir1Pin_A, LOW);         //액추에이터 멈춤
       digitalWrite(Dir2Pin_A, LOW);
       pinMode(TRIG, OUTPUT);                 
       pinMode(ECHO, OUTPUT);
      }
//      digitalWrite(Dir1Pin_A, HIGH);          //액추에이터 하강
//      digitalWrite(Dir2Pin_A, LOW);
   }
   return distance;
  }

void lavendula(){       // 5초 on / off
  digitalWrite(IN1,HIGH);        
  digitalWrite(IN2,LOW);
  delay(5000);
  digitalWrite(IN1,LOW);
  digitalWrite(IN2,LOW);
  delay(5000);
}

void rosmarinus(){
  digitalWrite(IN3,HIGH);        
  digitalWrite(IN4,LOW);
  delay(5000);
  digitalWrite(IN3,LOW);
  digitalWrite(IN4,LOW);
  delay(5000);
}

void mandarin(){
  digitalWrite(IN5,HIGH);        
  digitalWrite(IN6,LOW);
  delay(5000);
  digitalWrite(IN5,LOW);
  digitalWrite(IN6,LOW);
  delay(5000);
}

void SetStrokePerc(float strokePercentage)
{
  if ( strokePercentage >= 1.0 && strokePercentage <= 99.0 )
  {
    int usec = 1000 + strokePercentage * ( 2000 - 1000 ) / 100.0 ;
    myServo.writeMicroseconds( usec );
  }
}

void SetStrokeMM(int strokeReq,int strokeMax)
{
  SetStrokePerc( ((float)strokeReq) / strokeMax );
} 
