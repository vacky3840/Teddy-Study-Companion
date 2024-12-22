#include <LiquidCrystal_I2C.h> 
#include <WiFi.h> 
#include <WiFiClientSecure.h> 
#include <UniversalTelegramBot.h> 
 
// WiFi credentials 
const char* ssid = "SUTD_Guest"; 
const char* password = ""; 
 
// Telegram Bot Token and Chat ID 
#define BOTtoken "7714557177:AAFr_FfivFwg8oUQL42H4ya6YhcPUIUGg-E" 
#define CHAT_ID "5276957908" 
#define PARENT_CHAT_ID "717607066"
 
// LCD and health state 
const int trig_pin = 41; 
const int echo_pin = 40; 
const int MAX_HEALTH = 100; 
const int SAD_HEALTH = 50;
const int SICK_HEALTH = 20;
const int MIN_HEALTH = 0; 
int health = MAX_HEALTH; 
char mood = 'A'; // To avoid sending repeated messages, A = neutral, B = sad, C = sick, D = dead

// Initialize objects 
LiquidCrystal_I2C lcd(0x27, 16, 2); 
WiFiClientSecure client; 
UniversalTelegramBot bot(BOTtoken, client);  
 
float readDistanceCM() { 
  digitalWrite(trig_pin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trig_pin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trig_pin, LOW); 
  int duration = pulseIn(echo_pin, HIGH); 
  return duration * 0.034 / 2; 
} 
 
void faceSmiley() { 
  lcd.setCursor(6, 0); 
  lcd.print("^  ^"); 
  lcd.setCursor(7, 1); 
  lcd.print("--"); 
} 
 
void faceNeutral() { 
  lcd.setCursor(6, 0); 
  lcd.print("O  O"); 
  lcd.setCursor(7, 1); 
  lcd.print("__"); 
} 
 
void faceSad() { 
  lcd.setCursor(6, 0); 
  lcd.print(">  <"); 
  lcd.setCursor(7, 1); 
  lcd.print("__"); 
} 

void faceSick() { 
  lcd.setCursor(6, 0); 
  lcd.print("@  @"); 
  lcd.setCursor(7, 1); 
  lcd.print(" ."); 
} 

void faceDead() { 
  lcd.setCursor(6, 0); 
  lcd.print("X  X"); 
  lcd.setCursor(7, 1); 
  lcd.print("~~"); 
} 

void losingHealth() {
  // Update mood when health is lost 
  if (health == MIN_HEALTH) {  
    faceDead(); 
    if (mood == 'C') {
      bot.sendMessage(CHAT_ID, "TEDDY has fainted. Study now to resuscitate him!");
      bot.sendMessage(PARENT_CHAT_ID, "TEDDY has fainted. Check in on your child!");
      mood = 'D';
    }
  } else if (health <= SICK_HEALTH){
    faceSick();
    if (mood == 'B') {
      bot.sendMessage(CHAT_ID, "TEDDY is feeling sick... Study soon so he can recover!");
      mood = 'C';
    }
  } else if (health <= SAD_HEALTH){
    faceSad();
    if (mood == 'A') {
      bot.sendMessage(CHAT_ID, "TEDDY is feeling sad... Study to make him happier!");
      mood = 'B';
    }
  } else {
    faceNeutral();
  }
}

void gainingHealth() {
  // Update mood when health is gained 
  if (health > SAD_HEALTH) {  
    mood = 'A'; 
  } else if (health > SICK_HEALTH){
    mood = 'B';
  } else if (health > MIN_HEALTH){
    mood = 'C';
  }
}

void setup() { 
  // Initialize LCD 
  lcd.init(); 
  lcd.backlight(); 
  faceSmiley(); 
 
  // Initialize serial monitor 
  Serial.begin(115200); 
 
  // Initialize ultrasonic sensor pins 
  pinMode(trig_pin, OUTPUT); 
  pinMode(echo_pin, INPUT); 
 
  // Connect to WiFi 
  WiFi.mode(WIFI_STA); 
  WiFi.begin(ssid, password); 
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Use the built-in root certificate 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  } 
  Serial.println("\nWiFi connected"); 
 
  // Send initial Telegram message 
  bot.sendMessage(CHAT_ID, "Nice to meet you! My name is TEDDY and I become happier as you study (and sadder if you don't). Hope to study lots with you! ^-^"); 
} 
 
void loop() { 
  // Read the distance and update health 
  float distance = readDistanceCM(); 
  if (distance > 0 && distance < 4 && health < MAX_HEALTH) { 
    health++; 
    faceSmiley(); 
    gainingHealth();
  } else if (health > MIN_HEALTH) { 
    health = max(health - 1, MIN_HEALTH); 
    losingHealth();
  } 
 
  // Debugging and update LCD 
  Serial.println(health);  
  delay(1000); 
}