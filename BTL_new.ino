#include <Arduino.h>
#include <Servo.h>
#include <DHT.h>
#include<Wire.h>
#include <LiquidCrystal_I2C.h>

// màn hình
LiquidCrystal_I2C manhinh(0x27,16,2);



bool NhanDienNguoi = false;
// nút nguồn
static const int POWER_BUTTON_PIN = 2;
// các trạng thái nút nguồn
int lastButtonPower = HIGH;
unsigned long lastTimePower = 0;
static bool Nguon = false;
static bool Speed = false;
static bool Swing = false;
static bool NhietDo = false;
static bool BamNguoi = false;
static bool VaoHenGio = false;
static bool TangHenGio = false;



// khai báo các chân của MOTOR DRIVER (L298N)
static const int ENA_PIN = 5;
static const int IN1_PIN = 7;
static const int IN2_PIN = 8;
// nút speed
static const int SPEED_BUTTON_PIN = 4;
// các trạng thái nút speed
int lastButtonSpeed = HIGH;
unsigned long lastTimeSpeed = 0;
int speedLevel = 0;
int speedDuty[4] = {
  0,
  60,
  125,
  234
};


// khai báo chân nút Swing
static const int SWING_BUTTON_PIN = 3;
// khai báo biến trạng thái SWing BUTTON 
int lastButtonSwing = HIGH;
unsigned long lastTimeSwing = 0;
// khai báo chân servo
static const int SERVO_PIN = 9;
// khai báo các biến trạng thái của servo
Servo myServo;
static bool servoSwing = false;
static int servoAngle = 90;
static int servoDirection = 1; // cái này là mức tăng nha ae +1 hoặc -1 độ khi quay 
// =====================================================
// hàm chuyển trạng thái SERVO 
// =====================================================

void toggleServo() {

  // chỉ hoạt động khi nút này được phép hoạt động
  if (Swing == false) {
    return;
  }
  if (NhanDienNguoi == true) return;
  
  if(Swing == true){

    servoSwing = !servoSwing;// chuyển đổi trạng thái cho phép quay
    manhinh.setCursor(7,0);
    manhinh.print(servoSwing);
  }
  
}
// hàm quay sẽ được gọi liên tục ở trong loop
void QuaySwing() {

  // nếu servo đang tắt
  if (servoSwing == false) {
    return;
  }
  
    // tăng góc
  servoAngle += servoDirection;

  // chạm 180 độ -> quay ngược, tức là cộng với số âm
  if (servoAngle >= 180) {

    servoAngle = 180;

    servoDirection = -1;
  }

  // chạm 0 độ -> quay xuôi, tức là cộng với số dương
  if (servoAngle <= 0) {

    servoAngle = 0;

    servoDirection = 1;
  }

  myServo.write(servoAngle);

  // chỉnh tốc độ quay
  delay(10);
  
}


// hàm khi nhấn nút speed
void nextSpeed() {
  // chỉ hoạt động khi nút này ở trạng thái true; cho phép nút được hoạt động

  if(Speed == true){
    speedLevel = (speedLevel + 1) % 4;

    analogWrite(ENA_PIN, speedDuty[speedLevel]);
    manhinh.setCursor(2,0);
    manhinh.print(speedLevel);
  }
  
}

//khai báo chân nút Nhiệt độ
static const int NhietDo_BUTTON_PIN = 11;
// khai báo biến trạng thái NhietDo BUTTON 
int lastButtonNhietDo = HIGH;
unsigned long lastTimeNhietDo = 0;
// khai báo con cảm biến nhiệt độ
#define DHTPIN 10      // chân OUT nối D10
#define DHTTYPE DHT22 // loại cảm biến
DHT dht(DHTPIN, DHTTYPE);
unsigned long lanCuoiDocNhietDo = 0; // cái này để mình tính mấy giây mới cho đọc nhiệt độ 1 lần
static bool doNhietDo = false;
// hàm bật đo nhiệt độ
void toggleNhietDo(){
  if (NhietDo == false) {
    return;
  }

  doNhietDo = ! doNhietDo;
  if(doNhietDo == true){
    manhinh.setCursor(2,0);
    manhinh.print(" ");
    Speed = false;
  }
  else{
    Speed = true;
    analogWrite(ENA_PIN, speedDuty[speedLevel]);
    manhinh.setCursor(2,0);
    manhinh.print(speedLevel);

    manhinh.setCursor(2,1);
    manhinh.print("    ");
  }
}
// hàm này để trong vòng loop nếu mà doNhietDo có trạng thái true thì trong loop hàm này sẽ hoạt động (cứ mấy giây nó sẽ báo nhiệt độ về 1 lần)
void quayTheoNhietDo(){

  if(doNhietDo == false){
    return;
  }
// chỗ này để 8s sau mới đọc nhiệt độ và cho quạt quay với nhiệt độ đó để tránh cập nhật liên tục nhiệt độ dẫn đến quạt quay ko ổn định
  if(millis() - lanCuoiDocNhietDo > 2000){

    float t = dht.readTemperature();

    if(isnan(t)){
      Serial.println("DHT loi");
      lanCuoiDocNhietDo = millis();
      return;
    }

    manhinh.setCursor(2,1);
    manhinh.print("    "); // xóa số cũ

    manhinh.setCursor(2,1);
    manhinh.print(t,1);
    // giới hạn nhiệt độ
    if(t < 10){
     t = 10;
    }
    if(t > 60){
     t = 60;
    }
    // chuyển nhiệt độ -> PWM
    int pwm = (t - 10) * 255 / 50;
    analogWrite(ENA_PIN, pwm);
    Serial.print("Nhiet do: ");
    Serial.println(t);
    lanCuoiDocNhietDo = millis();

  }
}
bool nguoiDangVangMat = false;
// HÀM CHO NGƯỜI QUAY LẠI
void NguoiQuayLai() {

    if (nguoiDangVangMat == false) {
        return;
    }

    nguoiDangVangMat = false;

    // nếu đang dùng speed
    if (doNhietDo == false) {

        analogWrite(ENA_PIN, speedDuty[speedLevel]);
    }

    // nếu đang dùng nhiệt độ
    else {

        lanCuoiDocNhietDo = millis() - 3000;
    }

}
// khai báo nút bám người
static const int BamNguoi_BUTTON_PIN = A2;
//khai báo các biến trạng thái của nút bấm bám người
int lastButtonBamNguoi = HIGH;
unsigned long lastTimeBamNguoi = 0;
// KHAI BÁO CÁC CHÂN CẢM BIẾN NGỪOI
// HC-SR04 LEFT
#define TRIG_LEFT 6
#define ECHO_LEFT 12

// MID
#define TRIG_MID 13
#define ECHO_MID A3

// RIGHT
#define TRIG_RIGHT A0
#define ECHO_RIGHT A1
// KHAI BÁO CÁC BIẾN ĐỂ LƯU KHOẢNG CÁCH
long distLeft = 0;
long distMid = 0;
long distRight = 0;
// khai báo biến để lưu trạng thái đang hoạt động của cảm biến

// hàm chuyển đổi trạng thái bám người
void toggleBamNguoi() {

  if (BamNguoi == false) return;

  NhanDienNguoi = !NhanDienNguoi;
  manhinh.setCursor(11,0);
  manhinh.print(NhanDienNguoi);

  if (NhanDienNguoi == true) {

    Swing = false;
    servoSwing = false;
    manhinh.setCursor(7,0);
    manhinh.print(0);

  } else {

    Swing = true;
    manhinh.setCursor(7,0);
    manhinh.print(servoSwing);

  }
}
//HÀM TÍNH KHOẢNG CÁCH TỪ VẬT ĐẾN CẢM BIẾN , TÍ CHỈ VIỆC THAY CÁC CHÂN VÀO
long DoKhoangCach(int trigPin, int echoPin) {

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); 
  // timeout 30ms tránh treo

  long distance = duration * 0.034 / 2;

  if (distance == 0) return 999; // không đo được thì coi như xa

  return distance;
}

int targetAngle = 90;
unsigned long lastSensorRead = 0; // khai báo 1 biến nhớ thời gian lần đọc gần nhất
// hàm này để mình tính khoảng cách nhé tức là thay các chân vào
// tính khoảng cách của từng cảm biến, nếu có vật sẽ hiển thị số khoảng cách còn ko có thì ko hiển thị
int soLanKhongThayNguoi = 0;
void CapNhatSenSor() {

  if(NhanDienNguoi == false){
    return;
  }

  if (millis() - lastSensorRead < 6000) {
    return;
  }
  distLeft = DoKhoangCach(TRIG_LEFT, ECHO_LEFT);

  distMid = DoKhoangCach(TRIG_MID, ECHO_MID);

  distRight = DoKhoangCach(TRIG_RIGHT, ECHO_RIGHT);

    // ưu tiên vật trong khoảng 0 - 30cm
  bool leftDetected  = (distLeft > 0 && distLeft <= 30);
  bool midDetected   = (distMid > 0 && distMid <= 30);
  bool rightDetected = (distRight > 0 && distRight <= 30);

  if (leftDetected && !midDetected && !rightDetected) {


    NguoiQuayLai();
    servoSwing = false;
    targetAngle = 50;
    soLanKhongThayNguoi = 0;


  }
  else if (!leftDetected && midDetected && !rightDetected) {
    NguoiQuayLai();
    servoSwing = false;
    targetAngle = 90;
    soLanKhongThayNguoi = 0;
    

  }
  else if (!leftDetected && !midDetected && rightDetected) {
    NguoiQuayLai();
    servoSwing = false;
    targetAngle = 130;
    soLanKhongThayNguoi = 0;
    

  }
  else if ( (leftDetected && midDetected) || (leftDetected && rightDetected) || (midDetected && rightDetected) ) {
    NguoiQuayLai();
    servoSwing = true;
    soLanKhongThayNguoi = 0;
    

  }
  else {

    soLanKhongThayNguoi++;
    if(soLanKhongThayNguoi >= 2){
      nguoiDangVangMat = true;
      servoSwing = false;
      analogWrite(ENA_PIN, 0);
      targetAngle = servoAngle;
    }

  }


  Serial.print("L: ");
  Serial.print(distLeft);

  Serial.print(" | M: ");
  Serial.print(distMid);

  Serial.print(" | R: ");
  Serial.println(distRight);

  lastSensorRead = millis();

  
}


  // hàm quay
  void moveServoToTarget() {
  if (NhanDienNguoi == false) return;
  if (servoSwing == true) return;

  if (servoAngle < targetAngle) {

    servoAngle++;

    myServo.write(servoAngle);

  }
  else if (servoAngle > targetAngle) {

    servoAngle--;

    myServo.write(servoAngle);

  }

  }
// những hàm trên chỉ phát hiện xem có người không thôi
// ví dụ mà có người ở trong phạm vi thì sẽ sử dụng hàm này để quay theo người 

// nút vào hẹn giờ
static const int VaoHenGio_BUTTON_PIN = 0;
int lastButtonVaoHenGio = HIGH;
unsigned long lastTimeVaoHenGio = 0;
// nút tăng hẹn giờ
static const int TangHenGio_BUTTON_PIN = 1;
int lastButtonTangHenGio = HIGH;
unsigned long lastTimeTangHenGio = 0;

bool caiDatTime = false;
bool dangDemTime = false;
int thoiGianDemNguoc = 0;
unsigned long lastTime = 0;
// hàm vào hẹn giờ
void HamVaoHenGio(){

  if(dangDemTime == true){
    dangDemTime = false;
    caiDatTime = false;
    thoiGianDemNguoc = 0;
    manhinh.setCursor(11,1);
    manhinh.print("    ");
    manhinh.setCursor(11,1);
    manhinh.print("0");
    return;
  }

  if(caiDatTime == false){
    caiDatTime = true;
  }
  else{

    caiDatTime = false;
    if (thoiGianDemNguoc > 0)
        {
            dangDemTime = true;
            lastTime = millis();
        }

  }

}
 void HamTangHenGio(){
  if(caiDatTime == false){
    return;
  }
  if(caiDatTime == true){
    thoiGianDemNguoc += 15;
    manhinh.setCursor(11,1);
    manhinh.print("    ");

    manhinh.setCursor(11,1);
    manhinh.print(thoiGianDemNguoc);
  }
}




// hàm khi nhấn nút nguồn
void toggleFan() {

  Nguon = !Nguon;

  // nếu tắt nguồn tổng
  if (Nguon == false) {

    // tắt nút speed
    Speed = false;

    // tắt nút swing
    Swing = false;
    // chuyển trạng thái servo
    servoSwing = false;
    // đưa servo về 90 độ
    myServo.write(90);
    servoAngle = 90;


    //tắt nút nhiệt độ
    NhietDo = false;
    // tắt trạng thái đo nhiệt độ
    doNhietDo = false;

    //tắt nút nhiệt độ
    BamNguoi = false;
    //reset hết thời gian
    caiDatTime = false;
    dangDemTime = false;
    thoiGianDemNguoc = 0;
    //tắt luôn biến người
    NhanDienNguoi = false;
    nguoiDangVangMat = false;

    // tắt luôn quạt
    digitalWrite(IN1_PIN, LOW);
    digitalWrite(IN2_PIN, LOW);
    analogWrite(ENA_PIN, 0);
    manhinh.clear();
    manhinh.noBacklight();
  }
  else if(Nguon == true){

    // bật nút speed
    Speed = true;

    // bật nút swing
    Swing = true;

    //bật nút nhiệt độ
    NhietDo = true;

    //bật nút nhiệt độ
    BamNguoi = true;

    //  bật motor ở mức 1
    speedLevel = 1;
    digitalWrite(IN1_PIN, HIGH);
    digitalWrite(IN2_PIN, LOW);

    analogWrite(ENA_PIN, speedDuty[speedLevel]);
    manhinh.backlight();

    manhinh.setCursor(0,0);
    manhinh.print("S:1 SW:0 H:0");

    manhinh.setCursor(0,1);
    manhinh.print("T:    SET:0");
  }

}
// hàm đếm ngược
void DemNguoc()
{
    if (!dangDemTime)
    {
        return;
    }

    if (millis() - lastTime >= 1000)
    {
        lastTime = millis();

        thoiGianDemNguoc--;
        manhinh.setCursor(11,1);
        manhinh.print("    ");

        manhinh.setCursor(11,1);
        manhinh.print(thoiGianDemNguoc);

        if (thoiGianDemNguoc <= 0)
        {
            thoiGianDemNguoc = 0;
            dangDemTime = false;
            caiDatTime = false;

            // tắt quạt
            if(Nguon == true){
              toggleFan();
            }
        }
    }
}

void setup() {
  Serial.begin(9600);
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPEED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SWING_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NhietDo_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BamNguoi_BUTTON_PIN, INPUT_PULLUP);
  pinMode(VaoHenGio_BUTTON_PIN, INPUT_PULLUP);
  pinMode(TangHenGio_BUTTON_PIN, INPUT_PULLUP);

  // set up LCD
  manhinh.init();
  manhinh.backlight();

  // dòng 1
  manhinh.setCursor(0,0);
  manhinh.print("S:0 SW:0 H:0");

  // dòng 2
  manhinh.setCursor(0,1);
  manhinh.print("T:    SET:0");

  // setup chân L298N
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(ENA_PIN, OUTPUT);

  // set up Servo
  myServo.attach(SERVO_PIN);
  myServo.write(90);

  //setup DHT22
  dht.begin();

  //set up HC-SR04

  pinMode(TRIG_LEFT, OUTPUT);
  pinMode(ECHO_LEFT, INPUT);

  pinMode(TRIG_MID, OUTPUT);
  pinMode(ECHO_MID, INPUT);

  pinMode(TRIG_RIGHT, OUTPUT);
  pinMode(ECHO_RIGHT, INPUT);


  Serial.println("READY");
}
void loop() {

  // khi nhấn nút nguồn
   int currentButtonPower = digitalRead(POWER_BUTTON_PIN);
if(millis() - lastTimePower > 50){
  if(lastButtonPower == HIGH && currentButtonPower == LOW){
    toggleFan();
    lastTimePower = millis();
    Serial.println("da nhan nguon");
  }
  lastButtonPower = currentButtonPower;
}
  // khi nhấn nút speed
 int currentButtonSpeed = digitalRead(SPEED_BUTTON_PIN);

if(millis() - lastTimeSpeed > 50){
    if(lastButtonSpeed == HIGH && currentButtonSpeed == LOW){
        nextSpeed();
        lastTimeSpeed = millis();
        Serial.println("da nhan speed");
    }
    lastButtonSpeed = currentButtonSpeed;
}

// khi nhấn nút Swing
 int currentButtonSwing = digitalRead(SWING_BUTTON_PIN);
if(millis() - lastTimeSwing > 50){
    if(lastButtonSwing == HIGH && currentButtonSwing == LOW){
        toggleServo();
        lastTimeSwing = millis();
        Serial.println("da nhan swings");
    }
    lastButtonSwing = currentButtonSwing;
}

// khi nhấn nút quay theo nhiệt độ
  int currentButtonNhietDo = digitalRead(NhietDo_BUTTON_PIN);
if(millis() - lastTimeNhietDo > 50){
    if(lastButtonNhietDo == HIGH && currentButtonNhietDo == LOW){
        toggleNhietDo();
        lastTimeNhietDo = millis();
        Serial.println("da nhan quay theo nhiệt độ");
    }
    lastButtonNhietDo = currentButtonNhietDo;
}

// khi nhấn nút bám người

int currentButtonBamNguoi = digitalRead(BamNguoi_BUTTON_PIN);

if (millis() - lastTimeBamNguoi > 50) {
  if (lastButtonBamNguoi == HIGH && currentButtonBamNguoi == LOW) {
    toggleBamNguoi();
    lastTimeBamNguoi = millis();
    Serial.println("da nhan quay theo người");
  }
  lastButtonBamNguoi= currentButtonBamNguoi;
}

// khi nhấn nút để vào hẹn giờ
int currentButtonVaoHenGio = digitalRead(VaoHenGio_BUTTON_PIN);

if (millis() - lastTimeVaoHenGio > 50) {
  if (lastButtonVaoHenGio == HIGH && currentButtonVaoHenGio == LOW) {
    HamVaoHenGio();
    lastTimeVaoHenGio = millis();
    Serial.println("da nhan quay vào hẹn giờ");
  }
  lastButtonVaoHenGio= currentButtonVaoHenGio;
}
// khi nhấn nút để tăng hẹn giờ
int currentButtonTangHenGio = digitalRead(TangHenGio_BUTTON_PIN);

if (millis() - lastTimeTangHenGio > 50) {
  if (lastButtonTangHenGio == HIGH && currentButtonTangHenGio == LOW) {
    HamTangHenGio();
    lastTimeTangHenGio = millis();
    Serial.println("da nhan quay tang hẹn giờ");
  }
  lastButtonTangHenGio= currentButtonTangHenGio;
}

CapNhatSenSor();
QuaySwing();
moveServoToTarget();
quayTheoNhietDo();
DemNguoc();
}
