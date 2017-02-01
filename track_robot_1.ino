/*
  Гусеничный робот на S4A с Bluetooth
  Created by Rostislav Varzar
*/
//#include <Servo.h>
//#include <SoftwareSerial.h>
#include <ServoTimer2.h>
#include <AltSoftSerial.h>
//#include <NewSoftSerial.h>

#define IR_SENSOR A0

#define LINESENSOR1 A1
#define LINESENSOR2 A2
#define LINESENSOR3 A3
#define LINESENSOR4 A4
#define LINESENSOR5 A5

#define CRASHSENSOR1 9
#define CRASHSENSOR2 12

#define US1_trigPin 3
#define US1_echoPin 4
#define minimumRange 0
#define maximumRange 400

#define PWMB 5
#define DIRB 10
#define PWMA 6
#define DIRA 11

// Параметры дистанций
#define DIST1 10

// Параметры моторов
#define MPWR 99

// Переменные для управления роботом
char command = 'S';
char prevCommand = 'A';
int velocity = 0;
unsigned long timer0 = 2000;
unsigned long timer1 = 0;

// Программный UART для Bluetooth
//SoftwareSerial BT(2, 0);
AltSoftSerial BT(8, 0);
//NewSoftSerial BT(2, 0);

// Сервомоторы
#define SERVO1_PWM 7 // Подъем
#define SERVO2_PWM 2 // Хват
//Servo servo_1;
//Servo servo_2;
ServoTimer2 servo_1;
ServoTimer2 servo_2;

int srv1 = 90;
int srv2 = 75;
int add1 = 0;
int add2 = 0;
int motor_mode = 0;

void setup()
{
  // Инициализация последовательного порта
  Serial.begin(9600);

  // Инициализация последовательного порта для Bluetooth
  BT.begin(9600);

  // Инициализация выводов для работы с УЗ датчиком
  pinMode(US1_trigPin, OUTPUT);
  pinMode(US1_echoPin, INPUT);

  // Инициализация выходов для управления моторами
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);
  pinMode(PWMA, OUTPUT);
  pinMode(PWMB, OUTPUT);

  // Инициализация портов для управления сервомоторами
  servo_1.attach(SERVO1_PWM);
  servo_2.attach(SERVO2_PWM);
  // Начальное положение сервомоторов
  servo_1.write(map(srv1, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH)); //15-125
  servo_2.write(map(srv2, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH));
}

void loop()
{
  //  motorA_setpower(100, true);
  //  delay(5000);
  //  motorA_setpower(-100, true);
  //  delay(5000);
  //  motorA_setpower(0, true);
  //  delay(5000);
  //  motorB_setpower(100, false);
  //  delay(5000);
  //  motorB_setpower(-100, false);
  //  delay(5000);
  //  motorB_setpower(0, false);
  //  delay(5000);

  // Ожидание нажатия кнопки и тестирование датчиков
  while (digitalRead(CRASHSENSOR1) && digitalRead(CRASHSENSOR2))
  {
    Serial.print(digitalRead(CRASHSENSOR1));
    Serial.print("\t");
    Serial.print(digitalRead(CRASHSENSOR2));
    Serial.print("\t");
    Serial.print(digitalRead(LINESENSOR1));
    Serial.print("\t");
    Serial.print(digitalRead(LINESENSOR2));
    Serial.print("\t");
    Serial.print(digitalRead(LINESENSOR3));
    Serial.print("\t");
    Serial.print(digitalRead(LINESENSOR4));
    Serial.print("\t");
    Serial.print(digitalRead(LINESENSOR5));
    Serial.print("\t");
    Serial.print(analogRead(IR_SENSOR));
    Serial.print("\t");
    Serial.print(readUS1_distance());
    Serial.print("\t");
    delay(25);
    Serial.println("");
  }
  delay(128);

  if (!digitalRead(CRASHSENSOR1))
  {
    Serial.println("Started line going program");
    // Езда по датчикам линии
    motorA_setpower(0, true);
    motorB_setpower(0, false);
    while (true)
    {
      float left2 = digitalRead(LINESENSOR5);
      float left1 = digitalRead(LINESENSOR4);
      float center = digitalRead(LINESENSOR3);
      float right1 = digitalRead(LINESENSOR2);
      float right2 = digitalRead(LINESENSOR1);
      float dist1 = readUS1_distance();
      delay(10);
      // Проверка на столкновение
      if ((dist1 != (-1)) && (dist1 < DIST1))
      {
        motorA_setpower(0, true);
        motorB_setpower(0, false);
      }
      // Проверка линии
      if (!left1)
      {
        motorA_setpower(MPWR * 1.00, true);
        motorB_setpower(MPWR * 0.00, false);
      }
      if (!left2)
      {
        motorA_setpower(MPWR * 1.00, true);
        motorB_setpower(-MPWR * 0.50, false);
      }
      if (!right1)
      {
        motorA_setpower(MPWR * 0.00, true);
        motorB_setpower(MPWR * 1.00, false);
      }
      if (!right2)
      {
        motorA_setpower(-MPWR * 0.50, true);
        motorB_setpower(MPWR * 1.00, false);
      }
      if (left2 && left1 && right1 && right2)
      {
        // Уход с линии - уменьшаем скорость
        if (center)
        {
          motorA_setpower(MPWR * 0.75, true);
          motorB_setpower(MPWR * 0.75, false);
        }
        else
        {
          motorA_setpower(MPWR * 1.00, true);
          motorB_setpower(MPWR * 1.00, false);
        }
      }
      //motorA_setpower(MPWR * right1, true);
      //motorB_setpower(MPWR * left1, false);
      Serial.print(left1);
      Serial.print("\t");
      Serial.print(right1);
      Serial.print("\t");
      Serial.println("");
    }
  }
  else if (!digitalRead(CRASHSENSOR2))
  {
    Serial.println("Started BT going program");
    while (true)
    {
      // Bluetooth
      if (BT.available() > 0) {
        timer1 = millis();
        prevCommand = command;
        command = BT.read();
        if (command != prevCommand) {
          switch (command) {
            case 'F':
              if (motor_mode == 0) {
                motorA_setpower(velocity, true);
                motorB_setpower(velocity, false);
              } else {
                add1 = 1;
              }
              break;
            case 'B':
              if (motor_mode == 0) {
                motorA_setpower(-velocity, true);
                motorB_setpower(-velocity, false);
              } else {
                add1 = -1;
              }
              break;
            case 'L':
              if (motor_mode == 0) {
                motorA_setpower(-velocity, true);
                motorB_setpower(velocity, false);
              } else {
                add2 = 1;
              }
              break;
            case 'R':
              if (motor_mode == 0) {
                motorA_setpower(velocity, true);
                motorB_setpower(-velocity, false);
              } else {
                add2 = -1;
              }
              break;
            case 'S':
              add1 = 0;
              add2 = 0;
              motorA_setpower(0, true);
              motorB_setpower(0, false);
              break;
            case 'I':
              if (motor_mode == 0) {
                motorA_setpower(velocity, true);
                motorB_setpower(velocity / 2, false);
              } else {
                add1 = 1;
                add2 = -1;
              }
              break;
            case 'J':
              if (motor_mode == 0) {
                motorA_setpower(-velocity, true);
                motorB_setpower(-velocity / 2, false);
              } else {
                add1 = -1;
                add2 = -1;
              }
              break;
            case 'G':
              if (motor_mode == 0) {
                motorA_setpower(velocity / 2, true);
                motorB_setpower(velocity, false);
              } else {
                add1 = 1;
                add2 = 1;
              }
              break;
            case 'H':
              if (motor_mode == 0) {
                motorA_setpower(-velocity / 2, true);
                motorB_setpower(-velocity, false);
              } else {
                add1 = -1;
                add2 = 1;
              }
              break;
            case 'W':
              motor_mode = 1;
              break;
            case 'w':
              motor_mode = 0;
              break;
            case 'D':
              add1 = 0;
              add2 = 0;
              motorA_setpower(0, true);
              motorB_setpower(0, false);
              break;
            default:  //Get velocity
              if (command == 'q')
              {
                velocity = 100;
              }
              else {
                if ((command >= 48) && (command <= 57)) {
                  velocity = (command - 48) * 10;
                  Serial.println(velocity);
                }
              }
          }
        }
        // Вращение сервомоторов
        srv1 = srv1 + add1;
        if (srv1 <= 15) {
          srv1 = 15;
        }
        if (srv1 >= 125) {
          srv1 = 125;
        }
        servo_1.write(map(srv1, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH));
        // Вращение сервомоторов
        srv2 = srv2 + add2;
        if (srv2 <= 0) {
          srv2 = 0;
        }
        if (srv2 >= 180) {
          srv2 = 180;
        }
        servo_2.write(map(srv2, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH));
      }
      else {
        timer0 = millis();
        if ((timer0 - timer1) > 500) {
          motorA_setpower(0, true);
          motorB_setpower(0, false);
          add1 = 0;
          add2 = 0;
        }
      }
    }
  }
}

// УЗ датчик 1
float readUS1_distance()
{
  float duration = 0;
  float distance = 0;
  digitalWrite(US1_trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(US1_trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(US1_trigPin, LOW);
  duration = pulseIn(US1_echoPin, HIGH, 50000);
  distance = duration / 58.2;
  if (distance >= maximumRange || distance <= minimumRange) {
    distance = -1;
  }
  return distance;
}

// Мощность мотора "A" от -100% до +100% (от знака зависит направление вращения)
void motorA_setpower(float pwr, bool invert)
{
  // Проверка, инвертирован ли мотор
  if (invert)
  {
    pwr = -pwr;
  }
  // Проверка диапазонов
  if (pwr < -100)
  {
    pwr = -100;
  }
  if (pwr > 100)
  {
    pwr = 100;
  }
  // Установка направления
  if (pwr < 0)
  {
    digitalWrite(DIRA, LOW);
  }
  else
  {
    digitalWrite(DIRA, HIGH);
  }
  // Установка мощности
  float pwmvalue = fabs(pwr) * 2.55;
  analogWrite(PWMA, pwmvalue);
}

// Мощность мотора "B" от -100% до +100% (от знака зависит направление вращения)
void motorB_setpower(float pwr, bool invert)
{
  // Проверка, инвертирован ли мотор
  if (invert)
  {
    pwr = -pwr;
  }
  // Проверка диапазонов
  if (pwr < -100)
  {
    pwr = -100;
  }
  if (pwr > 100)
  {
    pwr = 100;
  }
  // Установка направления
  if (pwr < 0)
  {
    digitalWrite(DIRB, LOW);
  }
  else
  {
    digitalWrite(DIRB, HIGH);
  }
  // Установка мощности
  float pwmvalue = fabs(pwr) * 2.55;
  analogWrite(PWMB, pwmvalue);
}

