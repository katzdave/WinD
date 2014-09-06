#define BUFFERSIZE 100
#define MESSAGESIZE 5

int num;
int flipper;
int sensorVal;
int avgCounter;
int bitCounter;
int charCounter;
int buffer[BUFFERSIZE];
int avgs[BUFFERSIZE];

double lowThresh;
double highThresh;

char inputMessage[MESSAGESIZE];
int inputBits[MESSAGESIZE * 8];
int recvdBits[MESSAGESIZE * 8];
char recvdMessage[MESSAGESIZE];

void setup(){
  Serial.begin(9600);
  pinMode(3, OUTPUT);
  pinMode(4, INPUT);
  num = 0;
  sensorVal = 0;
  flipper = 0;
  avgCounter = 0;
  bitCounter = 0;
  charCounter = 0;
  inputMessage[0] = 'H';
  inputMessage[1] = 'e';
  inputMessage[2] = 'l';
  inputMessage[3] = 'l';
  inputMessage[4] = 'o';
  inputMessage[5] = '\0';
  
  digitalWrite(3,HIGH);  
  delay(1000);
  for(int i=0; i<BUFFERSIZE; i++){
    buffer[i] = digitalRead(4);
    delay(1);
  }
  lowThresh = getAverageOnes();
  
  digitalWrite(3,LOW);
  delay(780);
  for(int i=0; i<BUFFERSIZE; i++){
    buffer[i] = digitalRead(4);
    delay(1);
  }
  highThresh = getAverageOnes();
  
  //Serial.println(lowThresh);
  //Serial.println(highThresh);
  
  digitalWrite(3,HIGH);
  delay(1500);
  populateSendBuffer();
  
  Serial.print("Transmitting message: ");
  Serial.println(inputMessage);
}

void loop(){
  retransmit:  
  num = (num + 1)%4500;
  if(num == 1){
    digitalWrite(3, LOW);
    avgCounter = 0;
  }
  if(num == 1500){
    if(inputBits[bitCounter] == 1){
      Serial.print("1 Transmitted: ");
      digitalWrite(3, HIGH);
    }else Serial.print("0 Transmitted: ");
  }
  if(num == 3000){
    digitalWrite(3, HIGH);
    int tot = 0;
    for(int i=0; i<avgCounter; i++){
      tot = tot + avgs[i];
    }
    double avg = ((double) tot / (double) avgCounter);
    
    //Serial.print("avg is: ");
    //Serial.print(avg);
    //Serial.print("\n");
    if(abs(avg - lowThresh) > abs(avg - highThresh)){
      recvdBits[bitCounter] = 0;
      Serial.print(0);
      Serial.print(" Received\n"); 
    }
    else{
      recvdBits[bitCounter] = 1;
      Serial.print(1);
      Serial.print(" Received\n"); 
    }
    
    if(abs(avg - (lowThresh+highThresh)/2) < .01){
       Serial.println("Possible error, retransmitting last line");
       delay(1500);
       num = 0;
       goto retransmit;
    }
    
    if((bitCounter % 8) == 7){
      char rcvd = 0;
      for(int i=0; i<8; i++){
        //rcvd = rcvd + (recvdBits[bitCounter/8+i])*pow(2,i);
        rcvd = rcvd + recvdBits[charCounter*8+i]*(1<<(7-i));
        //Serial.print(recvdBits[charCounter*8+i]);
        //Serial.print(recvdBits[i]);
      }
      Serial.print("Received: ");
      Serial.write(rcvd);
      Serial.print("\n");
      recvdMessage[charCounter] = rcvd;
      
      charCounter = charCounter + 1;
      if(charCounter == MESSAGESIZE){
        Serial.print("Complete message received: ");
        for(int i=0; i<MESSAGESIZE; i++){
          Serial.write(recvdMessage[i]);
        }
        delay(1000000);
      }
      //Serial.print("\n");
    }
    
    bitCounter = bitCounter + 1;
  }
  if((num % BUFFERSIZE) == 0){
    avgs[avgCounter] = getAverageOnes();
    avgCounter = avgCounter + 1;
    //Serial.print(getAverageOnes());
    //Serial.print("\n");
  }
  
  sensorVal = digitalRead(4);
  buffer[num%BUFFERSIZE] = sensorVal;
  
  //Serial.print(sensorVal);
  //Serial.print("\n"); 
  delay(1);
}

double getAverageOnes(){
  int currChainLen = 0;
  int chainNum = 0;
  int oneCount = 0;
  
  int hitZero = 0;
    
  for(int i=0; i<BUFFERSIZE; i++){
    if(!hitZero && (buffer[i] == 0)){
      hitZero = 1; 
    }
    
    if((hitZero == 0) && (buffer[i] == 1)){
      continue;
    }
    else if(buffer[i] == 1){
      currChainLen = currChainLen + 1; 
    }
    else if((buffer[i] == 0) && (currChainLen > 0)){
      oneCount = oneCount + currChainLen;
      currChainLen = 0;
      chainNum = chainNum + 1;
    }    
  }
  
  return ((double) oneCount) / ((double) chainNum);
}

void populateSendBuffer(){
  for(int i=0; i<MESSAGESIZE; i++){
     char currChar = inputMessage[i];
     int *curBin = (int *)malloc(8*sizeof(int));
     convertToBinary(currChar, 2, curBin);
     for(int j=0; j<8; j++){
       inputBits[8*i+j] = curBin[j]; 
     }
     //Serial.print("\n");
  }
} 

int convertToBinary(int input, int base, int *output){
  int remainder;
  char digitsArray[3] = "01";
  for (int i=8; i > 0; i--){
    remainder = input % base;
    input = input / base;
    output[i-1] = digitsArray[remainder] - '0';
    //Serial.print(output[i-1]);
  }
  
}
