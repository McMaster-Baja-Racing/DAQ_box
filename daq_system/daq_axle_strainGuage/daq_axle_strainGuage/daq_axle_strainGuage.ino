#include "datastruct.h"
#include "daq_axle_strainGuage.h"


void openFile(fs::FS &fs, const char * fileName){
  Serial.printf("File Name: %s\n", fileName);  
  strcpy(fileDir,directory);
  strcat(fileDir,fileName);
  bajaData = fs.open(fileDir,FILE_WRITE); // Create file
  if(!bajaData){
      Serial.println("Failed to open file");
      return;
  }
  if(!bajaData.size()==0){
      Serial.println("New File");
      return;
  }
  else{
    Serial.println("Overwriting files");
    return;
  }

}


void sdSend(){
  if (savingBuff==&buff1){
    savingBuff=&buff2;
    sdBuff=&buff1;
  }
  else if (savingBuff==&buff2){
    savingBuff=&buff1;
    sdBuff=&buff2;
  }
  dataStruct sdTemp[8];
  int counter=0;
  unsigned long str=millis();
  while(!(*sdBuff).isEmpty()){
    if(millis()>(str+100)){
      Serial.println("SD LONG BOI");
      break;
    }
      
    if (counter>=7){
      bajaData.write((uint8_t *)&sdTemp,sizeof(sdTemp));
      counter=-1;
    }
    counter++;
  }
  if (counter!=0 && millis()<=(str+100)){
    bajaData.write((uint8_t *)&sdTemp,sizeof(sdTemp));
  }
  bajaData.flush();
}



void buffPush(int id, unsigned long tempData){
  if (!USE_SD && EN_SEROUT){
    Serial.print("UL* ID: ");
    Serial.print(DataTypeNames[id]);
    Serial.print(" Data: ");
    Serial.println(tempData);
    return;
  }
  temp.timeStamp_typ=(millis()<<6)|id;
  temp.data_long=tempData;
  if (!(*savingBuff).push(temp)){
    Serial.println("Lost Data");
    Serial.print("savingBuff Size = ");
    Serial.println((*savingBuff).size());

    Serial.print("sdBuff Size = ");
    Serial.println((*sdBuff).size());
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Setup Starting");
  
  pinMode(strainPin,INPUT);
  if (USE_SD) {
    // SD Card Setup
    if (!SD_MMC.begin()) {
      Serial.println("Card mount failed, or not present");
      // don't do anything more:
      USE_SD = false;
    }
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        return;
    }

    Serial.print("SD_MMC Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {  
        Serial.println("UNKNOWN");
    }
     
    openFile(SD_MMC,filename);
    delay(500);
    
  }
}

void strainData1(){
  buffPush(STRAIN5,(unsigned long)(analogRead(strainPin)));
}

void loop() {
  // put your main code here, to run repeatedly:


  if (SHOW_DEBUG &&(millis() - queueSizeTimer > (QUEUE_SIZE_INTERVAL - 1))){
      queueSizeTimer=millis();
      Serial.print("savingBuff Size = ");
      Serial.println((*savingBuff).size());

      Serial.print("sdBuff Size = ");
      Serial.println((*sdBuff).size());
   }

  if(EN_STRAIN && millis()-strainTimers>(STRAIN_FREQ-1)){
    strainTimers=millis();
    strainData1();
   }

  if(millis()-sdTimer>(SD_INTERVAL-1)){
    sdTimer=millis();
    sdSend();
   }
}
