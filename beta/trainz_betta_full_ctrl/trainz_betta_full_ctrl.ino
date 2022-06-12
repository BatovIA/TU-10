esoeez#include <GyverButton.h>

#define USBEMU  1     // Режим эмуляции клавиатуры
#define DEBUG 1     // Режим отладки (рекомендуется к использованию при тестирование обарудования)

#include <Servo.h>
#include <Wire.h>
#include <PCF8574.h>
#include <EasyHID.h>

#define pSTART 0003                              // Звук пуска
#define pSIG 0003                                // Звук тифона/свистка
#define pSTOP 0003                               // Звук остановки
#define pERRORE 0003                             // Звук ошибки
#define CTRL A0                                  // Управление
#define DIAG 11                                  // Лампа диагностики
#define ELC 10                                   // Лампа ЭСДУ

GButton SAND (8, HIGH_PULL, NORM_OPEN);          // Песок
GButton SIG (9, HIGH_PULL, NORM_OPEN);          // Сигнал
GButton START (6, HIGH_PULL, NORM_OPEN);          // Сигнал
GButton STOP (7, HIGH_PULL, NORM_OPEN);          // Сигнал

PCF8574 bus(0x27);
Servo man;                                       // Созджание пременной манометра

int val, val1;                                   // Переменные для работы плавного перехода между значениями манометра
int Gear,LastGear;                              // Положение ручки газа
boolean flag_g0;                                // Флаг газ в 0
boolean flag_q_press;                           // Флаг Q освободить тормоз была нажата
boolean flag_z_press;                           // Флаг Z BRAKE LAP была нажата
boolean flag_e_press;                           // Флаг E INDEPENDENT BRAKE была нажата
boolean flag_o_press;                           // Флаг O-map-PAUSE экстренный была нажата
boolean flag_c_press;                           // Флаг была нажата

void(* resetFunc) (void) = 0;                   // Инициализируем програмный сброс

void setup() {
if (USBEMU) {HID.begin();}
  bus.begin();

  #if (DEBUG == 1)
  Serial.begin(115200);
  Serial.println("Запущен режим отладки");
  #endif

  pinMode(CTRL, INPUT);
  pinMode(DIAG, OUTPUT);
  pinMode(ELC, OUTPUT);
  pinMode(5, OUTPUT);

  man.attach(3);                                 // Инициализируем манометр
  man.write(180);
  val = 180;
  man.write(val);

  digitalWrite(5, 0);
  digitalWrite(ELC, HIGH);

while (!START.isPress()){
  START.tick();
  digitalWrite(DIAG, LOW);
  delay(200);
  digitalWrite(DIAG, HIGH);
  delay(1);
}

if (USBEMU) {
  digitalWrite(ELC, LOW);
  digitalWrite(5, 0);
   delay(1000);
  digitalWrite(5, 1);
}
  digitalWrite(DIAG, LOW);
  digitalWrite(ELC, HIGH);

  Keyboard.click(KEY_2);
   delay(2000);

  Keyboard.click(KEY_1);
 // Keyboard.click(KEY_F5);
  Keyboard.click(KEY_J);
  Keyboard.click(KEY_ARROW_DOWN);
  Keyboard.click(KEY_ARROW_DOWN);
  Keyboard.click(KEY_ARROW_DOWN);
  Keyboard.click(KEY_ARROW_RIGHT);
  Keyboard.click(KEY_ARROW_RIGHT);
  Keyboard.click(KEY_ARROW_RIGHT);
  Keyboard.click(KEY_ARROW_RIGHT);

}


void loop() {

  SIG.tick();
  SAND.tick();
  STOP.tick();
  START.tick(); 
  START.tick();

  if (bus.read(0) == 1){
if ((USBEMU)&(!flag_q_press)) {Keyboard.click(KEY_Q);}

    flag_q_press = true;
    flag_z_press = false;
    flag_e_press = false;
    flag_o_press = false;
   flag_c_press = false;

    val1 = val;
    val = 180;
    man_sweep();
  }
  if (bus.read(1) == 1){
if ((USBEMU)&(!flag_z_press)){Keyboard.click(KEY_Z);}

    flag_z_press = true;
    flag_q_press = false;
    flag_e_press = false;
    flag_o_press = false;
   flag_c_press = false;

    val1 = val;
    val = 160;
    man_sweep();
    }

  if (bus.read(2) == 1){

//if ((USBEMU)&(!flag_c_press)){Keyboard.click(KEY_C);}

if ((USBEMU)&(flag_e_press)){Keyboard.click(KEY_E);}

  flag_z_press = false;
  flag_q_press = false;
  flag_e_press = false;
  flag_o_press = false;
  flag_c_press = true;

    val1 = val;
    val = 140;
    man_sweep();
  }

  if (bus.read(3) == 1){
if ((USBEMU)&(!flag_e_press)){Keyboard.click(KEY_E);}

   flag_e_press = true;
   flag_z_press = false;
   flag_q_press = false;
   flag_o_press = false;
   flag_c_press = false;

    val1 = val;
    val = 120;
    man_sweep();
}

if (bus.read(1) == 1 && START.isPress() && STOP.isPress()){
  pizda();
}

  if (bus.read(4) == 1){      //экстренное
if ((USBEMU)&(!flag_o_press)){
    Keyboard.click(KEY_S);
//    Keyboard.click(KEY_E);
    Keyboard.click(KEY_O);   //экстренное через KEYMAP - O - PAUSE!BREAK
    Keyboard.releaseAll();

}

  flag_q_press = false;
  flag_z_press = false;
  flag_e_press = false;
  flag_o_press = true;

  val1 = val;
    val = 100;
    man_sweep();
    }

  if ((bus.read(5) == 1)&(bus.read(6) == 0)) {      //реверс вперед
 if (USBEMU){
  Keyboard.click(KEY_F);
  Keyboard.press(KEY_RIGHT_ALT);
  Keyboard.click(KEY_C);
  delay(300);
  Keyboard.click(KEY_RIGHT_ALT);
  }
    }

  if ((bus.read(5) == 0)&(bus.read(6) == 1)) {      //реверс назад
if (USBEMU){
  Keyboard.click(KEY_R);
  Keyboard.press(KEY_RIGHT_ALT);
  Keyboard.click(KEY_C);
  delay(300);
 }
    }
if (USBEMU){
  if (SIG.isPress()) Keyboard.click(KEY_H);
  if (SAND.isPress()) Keyboard.click(KEY_V);
}

  int maps = 0;
  for (int i = 1; i< 6;i++){
   maps = maps + map(analogRead(A0), 470, 590, 8, 0);
  delay (50);
  }
    Gear = maps/5;
    if (Gear>8) Gear = 8;
    if (Gear<0) Gear = 0;

        if (Gear>0){
     if (LastGear != Gear) {
        if (Gear > LastGear){
          for (int i = LastGear;i < Gear;i++){
if (USBEMU){Keyboard.click(KEY_W);}
             Serial.print("+");
            }
            LastGear=Gear;
            Serial.println("");
            flag_g0 = true;
        }

        if (Gear<LastGear){
          for (int i=Gear;i < LastGear;i++){
if (USBEMU){Keyboard.click(KEY_X);}
             Serial.print("-");
          }
            LastGear=Gear;
            flag_g0 = true;
        }}} else {

         if (flag_g0){
if (USBEMU==1){Keyboard.click(KEY_S);}
           flag_g0 = false;
         }}
if (Gear>8) Serial.println("НЕИСПРАВНОСТЬ! Проверьте настройки ручки газа!");

   Serial.println(Gear);
//   Serial.println(analogRead(A0));
//   Serial.println(usl);

    int usl = 300;

  static uint32_t timer = millis();     // Миллис - таймер (НЕ DELAY!!!)
  if (millis() - timer >= usl) {       // Каждые 3000 мс
    timer = millis();
}

  if (STOP.isPress()) {
    dbflash();
  resetFunc();
  }

}

void man_sweep() {                          // Плавная работа манометра

  if (val > val1) {
    for (int i = val1; i <= val; i++) {
      man.write(i);
      delay(30);
    }
  }
  if (val < val1) {
    for (int i = val1; i >= val; i--) {
      man.write(i);
      delay(10);
    }
  }
}

void dbflash()
{
    delay(50);
   digitalWrite(ELC, LOW);
   delay(50);
   digitalWrite(ELC, HIGH);
}

void pizda() {
  static uint32_t pizex = millis();
  if (millis() - pizex >= 300) {
    pizex = millis();
    Keyboard.clickSystemKey(KEY_LEFT_WIN);
    Keyboard.click(KEY_E);
  }

}
