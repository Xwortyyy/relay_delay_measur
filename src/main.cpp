#include <Arduino.h>

#define read_pin 4
#define tranz_base_pin 2


constexpr int max_iter = 10;
int current_iter = 0;
uint32_t results[max_iter];
volatile bool contact_closed = false;
volatile uint32_t stop_time = 0;
uint32_t start_time =0;

void IRAM_ATTR onContactClosed(){
   stop_time = micros();
  contact_closed = true;
}


void setup() {
  Serial.begin(115200);
  digitalWrite(tranz_base_pin, LOW);
  pinMode(tranz_base_pin, OUTPUT);
  pinMode(read_pin, INPUT_PULLUP);
}

void loop() {
  if(current_iter < max_iter){
    contact_closed = false; //скидаємо прапорець
    attachInterrupt(digitalPinToInterrupt(read_pin), onContactClosed, FALLING);  //слухаємо 
    start_time = micros();
    digitalWrite(tranz_base_pin, HIGH); //даєм напругу на реле

    uint32_t waintStart = millis();
    while(!contact_closed && (millis() - waintStart < 100)){
      //чекаємо поки контакт замкнеться або пройде 100 мс
    }

    detachInterrupt(digitalPinToInterrupt(read_pin)); //виключаємо переривання, тим самим ігноруємо брязкіт 

    if(contact_closed){
      uint32_t duration = stop_time - start_time;
      results[current_iter] = duration;
      Serial.printf("Ітерація %d: контакт замкнувся за %u мкс, (%.2f мс)\n", current_iter+1, duration, duration/1000.0f);
    }
    else{
      Serial.printf("Ітерація %d: помилка! Контакт не замкнувся протягом 100 мс\n", current_iter+1);
    }

    digitalWrite(tranz_base_pin, LOW);//знімаємо напругу з реле
    delay(100); 
    current_iter++;
    
    if(current_iter == max_iter){
      Serial.println("Результати вимірювань:");
      uint64_t sum = 0;
      for(int i = 0; i < max_iter; i++){
        sum += results[i];
      }
      float average = static_cast<float>(sum) / max_iter;
      Serial.printf("Середній час: %.2f мкс, (%.2f мс)\n", average, average/1000.0f);
    }
  }
}