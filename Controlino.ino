#include "DFRobotDFPlayerMini.h"  //biblioteca do módulo reprodutor de MP3
#include <U8g2lib.h>  //biblioteca da tela OLED
#include <DS3231.h>   //biblioteca do módulo relógio
#include <Wire.h>     //biblioteca para a comunicação I2C
#include <Adafruit_Sensor.h>  //bibliteca base para o sensor BME
#include <Adafruit_BME280.h>  //bibliotec apara osensor BME
#include <SD.h>       //biblioteca para o módulo de memória SD
#include <SPI.h>      //biblioteca para a comunicação SPI 

DFRobotDFPlayerMini Voz;  //instancia o módulo MP3
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //instancia o módulo de tela OLED
DS3231 Mod_relogio; //instancia o módulo relógio
RTClib RTC;         //instancia o módulo contador UNIXTIME do relógio
Adafruit_BME280 bme_0, bme_1, bme_2, bme_3, bme_4, bme_5, bme_6, bme_7; //instancia os sensores BME280
File myFile;        //instancia o módulo de memória SD

#define debug true  //mostra informações de debug na serial (PC)

String WifiID   = "xxxxxxxx";   //Nome da Rede
String WifiPass = "12345678";   //Senha da Rede

volatile byte falar = 0;  //variável alterada pelos botões do Controlino

byte vol_audio = 30; //ajuste do volume (0 a 30) do módulo MP3
byte hora, minuto, segundo, dia, mes, ano, semana;

const int gamma[] = { //https://cdn-learn.adafruit.com/downloads/pdf/led-tricks-gamma-correction.pdf
    0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,
    3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,
    6,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,
   10, 10, 10, 10, 11, 11, 11, 12, 12, 12, 13, 13, 14, 14, 14, 15,
   15, 16, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22,
   23, 23, 24, 24, 25, 25, 26, 27, 27, 28, 28, 29, 30, 30, 31, 32,
   32, 33, 34, 34, 35, 36, 36, 37, 38, 39, 39, 40, 41, 42, 43, 43,
   44, 45, 46, 47, 48, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
   59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71, 72, 73, 74, 75,
   76, 78, 79, 80, 81, 82, 84, 85, 86, 88, 89, 90, 92, 93, 94, 96,
   97, 98,100,101,103,104,106,107,109,110,112,113,115,116,118,120,
  121,123,125,126,128,130,131,133,135,136,138,140,142,144,145,147,
  149,151,153,155,157,159,161,163,165,167,169,171,173,175,177,179,
  181,183,185,187,190,192,194,196,198,201,203,205,208,210,212,215,
  217,219,222,224,227,229,232,234,237,239,242,244,247,250,252,255}; //intensidade PWM dos LEDS para acendimento crepuscular

void setup() {
  pinMode(2,INPUT_PULLUP);
  pinMode(3,INPUT_PULLUP);

  byte portas[] = {4, 5, 32, 33, 34, 47, 49}; //portas utilizadas pelos periféricos do Controlino

  for (byte i = 0; i < 7; i++){
    pinMode(portas[i], OUTPUT); //configura as portas como saídas digitais
    digitalWrite(portas[i], 1); //atribui nível lógico 1 para as portas
  }

  SDA_mux(0);                 //ativa a saída SDA0
  u8g2.begin();               //inicializa display
  u8g2.enableUTF8Print();     //permite uso de caracteres especiais no display
  console("   Iniciando  ", 1); //mostra informação na linha 1

  Serial.begin(57600);  //Comunicação serial com o PC
  Serial1.begin(38400); //WiFi
  Serial2.begin(9600);  //VOZ
  Serial3.begin(9600);  //Bluetooth

  attachInterrupt(digitalPinToInterrupt(2), interrupcao_bt_verde, FALLING); //chama a subrotina em caso de acionamento do botão verde
  attachInterrupt(digitalPinToInterrupt(3), interrupcao_bt_vermelho, FALLING);  //chama a subrotina em caso de acionamento do botão vermelho


  SDA_mux(0);
  if (bme_0.begin(0x76) || bme_0.begin(0x77)) console ("bme0 OK", 1);  //caso posua sensor conectado na porta SDA0 mostra esta mesagem
  else console ("bme0", 1); //caso não posua sensor conectado na porta SDA0 mostra esta mesagem
  SDA_mux(1);
  if (bme_1.begin(0x76) || bme_1.begin(0x77)) console ("bme1 OK", 2);
  else console ("bme1", 2);
  SDA_mux(2);
  if (bme_2.begin(0x76) || bme_2.begin(0x77)) console ("bme2 OK", 3);
  else console ("bme2", 3);
  SDA_mux(3);
  if (bme_3.begin(0x76) || bme_3.begin(0x77)) console ("bme3 OK", 4);
  else console ("bme3", 4);
  SDA_mux(4);
  if (bme_4.begin(0x76) || bme_4.begin(0x77)) console ("bme4 OK", 5);
  else console ("bme4", 5);
  SDA_mux(5);
  if (bme_5.begin(0x76) || bme_5.begin(0x77)) console ("bme5 OK", 6);
  else console ("bme5", 6);
  SDA_mux(6);
  if (bme_6.begin(0x76) || bme_6.begin(0x77)) console ("bme6 OK", 7);
  else console ("bme6", 7);
  SDA_mux(7);
  if (bme_7.begin(0x76) || bme_7.begin(0x77)) console ("bme7 OK", 8);
  else console ("bme7", 8);
  delay(2500);

  if (SD.begin(49)) {console("SD card ready"); delay(1000);} //verifica se o módulo SD foi iniciado e mostra mensagem
  else {
    console("SD card failed");  //se o módulo não foi iniciado mostra esta mensagem
    delay(1000);  //pausa para aparecer a mensagem na tela
  }

  iniciaVoz();
  iniciaWifi();

  console("Iniciado!");
  if (debug) Serial.println ("INICIADO!");

  logo();
}

void loop() {
  float temp;
  relogio();
  blink();
  if (falar == 1) {
    fala_hora();
    fala_semana();
    falar = 3;
  }
  else if (falar == 2) {
    fala (9999);
    falar = 3;
  }
  else if (falar == 3) {
    mostra_temp();
  }
}

bool iniciaWifi() {
  bool WiFiOn = true;
  String dados_acesso = "AT+CWJAP=\"" + WifiID + "\",\"" + WifiPass + "\"\r\n";
  
  console("Wifi OK?");
  Serial1.print("AT\r\n");
  if (espera ("OK")) console("            OK"); 
  else {
    console("RESET por fio");
    digitalWrite(47, LOW);
    delay (250);
    digitalWrite(47, HIGH);
    delay (2000);
  }
  
  console("Wifi OK?");
  Serial1.print("AT\r\n");
  if (espera ("OK")) console("            OK"); 
  else return false;
  
  console("IniciandoWifi");
  Serial1.print("AT+RST\r\n");
  if (espera ("OK")) console("            OK"); else return false;
  
  console("Resetando");
  if (espera ("ready")) console("            OK"); else return false;
  
  console("ConfigurandoAP");
  Serial1.print("AT+CWMODE_CUR=1\r\n"); // Station
  if (espera ("OK")) console("            OK"); else return false;
  
  console("ConfConexUnica");
  Serial1.print("AT+CIPMUX=0\r\n"); // configure for multiple connections
  if (espera ("OK")) console("            OK"); else return false;
  
  if (WifiID.length() > 0) {
    console("ConectandoRede");
    Serial1.print(dados_acesso);
    delay(1000);
    if (espera ("OK")) console("            OK");
    else {
      console ("falhaConecRede");
      delay (500);
      WiFiOn = false;
    }
  }
  return WiFiOn;
}


void serialEvent3() {     //Bluetooth

char incomingByte;
String frase_recebida;

while (1){
  if (Serial3.available()){
  incomingByte = Serial3.read();
  frase_recebida += incomingByte;         
  if (incomingByte == '\n') break;
  }
}
console (frase_recebida);
}

void console (String mensagem, byte linha) {

  byte mux_anterior = SDA_mux();
  byte coluna = 0;
  
  SDA_mux(0);
  
  if (linha == 1) u8g2.clearBuffer();
  
  if (mensagem.indexOf('\n') <  0) mensagem += '\n';
  
  if (linha > 5) {
    linha -= 5;
    coluna = 64;
  }
  
  u8g2.setFont(u8g2_font_courR10_tf);
  if (linha == 5 || linha == 10) u8g2.setCursor(coluna, 64);
  else u8g2.setCursor(coluna, 13 * linha);
  u8g2.print(mensagem);
  u8g2.sendBuffer();
  
  SDA_mux(mux_anterior);

}

void console (String mensagem) {

byte mux_anterior = SDA_mux();

SDA_mux(0);

if (mensagem.indexOf('\n') <  0) mensagem += '\n';

u8g2.clearBuffer();
u8g2.setFont(u8g2_font_courR10_tf);
u8g2.setCursor(0, 13);
u8g2.print(mensagem);
u8g2.sendBuffer();

SDA_mux(mux_anterior);

}

void fala (int texto) {
  console ("Fala:");
  console (String(texto));
  Voz.playMp3Folder (texto);
  
  while (!Voz.available());
  Voz._isAvailable = false;
  while (!Voz.available());
  Voz._isAvailable = false;
}

void iniciaVoz() {
  byte tentativa_voz = 1;
  console("Iniciando Voz");
  while (!Voz.begin(Serial2)) {      //Iniciando módulo de Voz
    tentativa_voz++;
    console("tentativa " + String(tentativa_voz++));
    if (tentativa_voz >= 6) {
      console("Voz ?");
      delay(1000);
      break;
    }
  }
  if (tentativa_voz < 6) {
    console ("OK!");
    Voz.volume(vol_audio);       //Set volume value (0~30).
    Voz.EQ(DFPLAYER_EQ_ROCK);
    Voz.outputDevice(DFPLAYER_DEVICE_SD);
  }
}

void blink(){
   for (int i=0; i<255; i++){
      analogWrite(4, gamma[i]);
      analogWrite(5, gamma[254-i]);
      delayMicroseconds(1000);
  }
  for (int i=0; i<255; i++){
      analogWrite(4, gamma[254-i]);
      analogWrite(5, gamma[i]);      
      delayMicroseconds(1000);
  }
}


void interrupcao_bt_verde(){
  falar = 1;
}

void interrupcao_bt_vermelho(){
  falar = 2;
}

void relogio() {

  bool h12, PM, Century = false;

  SDA_mux(0);
  
  segundo = Mod_relogio.getSecond(); //Guarda os segundos
  minuto = Mod_relogio.getMinute(); //Guarda os minutos
  hora = Mod_relogio.getHour(h12, PM); //Guarda as horas
  dia = Mod_relogio.getDate(); //Guarda o dia do mês
  mes = Mod_relogio.getMonth(Century); //Guarda o mês
  ano = Mod_relogio.getYear(); // Guarda o ano
  semana = Mod_relogio.getDoW(); //Guarda o dia da semana 

}

void fala_hora() {

  fala (1100 + hora);
  if (minuto > 0) fala (4100 + minuto);
  
}


void fala_semana() {

  fala (semana + 9200);  //Dia da semana
  if (dia == 1) fala (306); 
  else fala (dia);
  fala (9100 + mes);
  fala (2000 + ano);
}

bool espera (String str) {

  int str_len = str.length() + 1; 
  char oque[str_len];
  str.toCharArray(oque, str_len);
  
  bool tudo_OK = true;
  byte tentativa = 1;
  
  while (!Serial1.find(oque)) {
    if (tentativa >= 7) {
      tudo_OK = false;
      break;
    }
    tentativa++;
    delay (100 * tentativa);
    console("   tentativa " + String(tentativa));
  }
  delay(25);
  return tudo_OK;
}

void teste_SD(){
    // Create/Open file 
  myFile = SD.open("test.txt", FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println("Writing to file...");
    // Write to file
    myFile.println("Testing text 1, 2 ,3...");
    myFile.close(); // close the file
    Serial.println("Done.");
  }
  // if the file didn't open, print an error:
  else {
    Serial.println("error opening test.txt");
  }
  // Reading the file
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("Read:");
    // Reading the whole file
    while (myFile.available()) {
      Serial.write(myFile.read());
   }
    myFile.close();
  }
  else {
    Serial.println("error opening test.txt");
  }
}

void SDA_mux (byte mux) {

  if (mux & 1) digitalWrite(32, 1);
  else digitalWrite(32, 0);
  if (mux & 2) digitalWrite(33, 1);
  else digitalWrite(33, 0);
  if (mux & 4) digitalWrite(34, 1);
  else digitalWrite(34, 0);
}

byte SDA_mux (void) {

  byte porta_acionada = 0;

  bool a, b, c;
  a = bitRead(PORTC,5);  // porta 32 está ativada?
  b = bitRead(PORTC,4);  // porta 33 está ativada?
  c = bitRead(PORTC,3);  // porta 34 está ativada?

  if (a) porta_acionada = 1;
  if (b) porta_acionada += 2;
  if (c) porta_acionada += 4;

  return (porta_acionada);
}

void logo(){
  SDA_mux(0);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courR10_tf);
  u8g2.setCursor(0, 13);
  u8g2.print("  C O N T R O  ");
  u8g2.setFont(u8g2_font_fub35_tf ); 
  u8g2.setCursor(10, 55);
  u8g2.print("LINO");
  u8g2.sendBuffer();
}

void mostra_temp(){
  SDA_mux(0);
  console (String(bme_0.readTemperature()), 1);
  SDA_mux(1);
  console (String(bme_1.readTemperature()), 2);
  SDA_mux(2);
  console (String(bme_2.readTemperature()), 3);
  SDA_mux(3);
  console (String(bme_3.readTemperature()), 4);
  SDA_mux(4);
  console (String(bme_4.readTemperature()), 5);
  SDA_mux(5);
  console (String(bme_5.readTemperature()), 6);
  SDA_mux(6);
  console (String(bme_6.readTemperature()), 7);
  SDA_mux(7);
  console (String(bme_7.readTemperature()), 8);
}
