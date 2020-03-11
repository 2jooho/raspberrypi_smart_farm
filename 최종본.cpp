#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdlib.h>  
#include <stdint.h>  
#include <softPwm.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

//soket
#define BUF_LEN 128
//char buffer[BUF_LEN];
//char buffer2[BUF_LEN];
//char buffer_snd[BUF_LEN];
//struct sockaddr_in server_addr, client_addr;
//char temp[20];
//int server_fd, client_fd;
////server_fd, client_fd : 각 소켓 번호
//int len, msg_size;



//
char change[5];
int light_ex = 0;
int fan_ex = 0;
int pump_ex = 0;
int moter_ex = 0;
int tem_ex = 0;
int moter_step = 5;
int moisture_change = 1000;
int hum_change = 70;
//온습도 tmp
#define MAX_TIME 85  
#define DHT11PIN 4  //7
#define fan 17
int dht11_val[5] = { 0,0,0,0,0 };
float tem_value = 0;
float hum_value = 0;
int tmp_change = 22;
int led_value = 99;
// 조도와  mcp
#define CS_MCP3208 8
#define RED 18
#define PUMP 21
#define SPI_CHANNEL 0 
#define SPI_SPEED 1000000 




  //------moter
#define OUT1 6      //BCN_GPIO 10
#define OUT2 13      //BCM_GPIO 9
#define OUT3 19       //BCM_GPIO 23
#define OUT4 26      //BCM_GPIO 24

void setsteps(int w1, int w2, int w3, int w4)
{
   pinMode(OUT1, OUTPUT);
   digitalWrite(OUT1, w1);
   pinMode(OUT2, OUTPUT);
   digitalWrite(OUT2, w2);
   pinMode(OUT3, OUTPUT);
   digitalWrite(OUT3, w3);
   pinMode(OUT4, OUTPUT);
   digitalWrite(OUT4, w4);
}

void forward(int del, int steps)
{
   int i;
   for (i = 0; i <= steps; i++)
   {
      setsteps(1, 1, 0, 0);
      delay(del);
      setsteps(0, 1, 1, 0);
      delay(del);
      setsteps(0, 0, 1, 1);
      delay(del);
      setsteps(1, 0, 0, 1);
      delay(del);
   }
}

void backward(int del, int steps)
{
   int k;
   for (k = 0; k <= steps; k++)
   {
      setsteps(1, 0, 0, 1);
      delay(del);
      setsteps(0, 0, 1, 1);
      delay(del);
      setsteps(0, 1, 1, 0);
      delay(del);
      setsteps(1, 1, 0, 0);
      delay(del);
   }
}

//온습도 tmp
void dht11_read_val()
{
   uint8_t lststate = HIGH;
   uint8_t counter = 0;
   uint8_t j = 0, i;
   float farenheit;
   for (i = 0; i < 5; i++)
      dht11_val[i] = 0;
   pinMode(DHT11PIN, OUTPUT);
   digitalWrite(DHT11PIN, LOW);
   delay(18);
   digitalWrite(DHT11PIN, HIGH);
   delayMicroseconds(40);
   pinMode(DHT11PIN, INPUT);
   for (i = 0; i < MAX_TIME; i++)
   {
      counter = 0;
      while (digitalRead(DHT11PIN) == lststate) {
         counter++;
         delayMicroseconds(1);
         if (counter == 255)
            break;
      }
      lststate = digitalRead(DHT11PIN);
      if (counter == 255)
         break;
      // top 3 transistions are ignored  
      if ((i >= 4) && (i % 2 == 0)) {
         dht11_val[j / 8] <<= 1; // 비트를 왼쪽으로 1번 이동한 후 할당
         if (counter > 50)
            dht11_val[j / 8] |= 1;  // 1(0000 0001) OR 연산 후 할당
         j++;
      }
   }
   // verify cheksum and print the verified data  
   pinMode(fan, OUTPUT);


   //softPwmCreate(MOTOR_RIGHT_ROTATE_PIN, 0, 100);
  //   softPwmCreate(MOTOR_LEFT_ROTATE_PIN, 0, 100);
   if ((j >= 40) && (dht11_val[4] == ((dht11_val[0] + dht11_val[1] + dht11_val[2] + dht11_val[3]) & 0xFF)))
   {
      farenheit = dht11_val[2] * 9. / 5. + 32;
      printf("Humidity = %d.%d %% Temperature = %d.%d *C (%.1f *F)\n", dht11_val[0], dht11_val[1], dht11_val[2], dht11_val[3], farenheit);
      tem_value = dht11_val[2] + (0.1 * dht11_val[3]);
      hum_value = dht11_val[0] + (0.1 * dht11_val[1]);
      tem_ex = 1;
   }
   else {
      printf("Invalid Data!!\n");
   }
   /*if (value >= 26) {
      digitalWrite(fan, HIGH);
      forward(2, 5);
   }
   else {
      digitalWrite(fan, LOW);
      backward(2, 5);
   }*/
}
void light_sensor(int control) {


   if (control == 1) {
      digitalWrite(RED, HIGH);
      //softPwmCreate(RED, 0, 100);
      //softPwmWrite(RED, led_value);
      light_ex = 1;
   }
   else if (control == 0)
   {
      digitalWrite(RED, LOW);
      light_ex = 0;
   }
}
void fan_sensor(int control) {

   if (control == 1) {
      digitalWrite(fan, HIGH);
      fan_ex = 1;
   }
   else if (control == 0)
   {
      digitalWrite(fan, LOW);
      fan_ex = 0;
   }
}
void pump_sensor(int control) {

   if (control == 1) {
      digitalWrite(PUMP, 1);
      pump_ex = 1;
   }
   else if (control == 0)
   {
      digitalWrite(PUMP, 0);
      pump_ex = 0;
   }
}
void moter_sensor(int control) {

   if (control == 1) {
      for (int i = 0; i < 20; i++)
         forward(5, 10);
      moter_ex = 1;
   }
   else if (control == 0)
   {
      for (int i = 0; i < 20; i++)
         backward(5, 10);
      moter_ex = 0;
   }
}
void tmp_sensor(int control) {

   if (control == 1) {
      digitalWrite(DHT11PIN, HIGH);
      tem_ex = 1;
   }
   else if (control == 0)
   {
      digitalWrite(DHT11PIN, LOW);
      tem_ex = 0;
   }
}
//조도와 mcp
int read_mcp3208_adc(unsigned char adcChannel)
{
   unsigned char buff[3];
   int adcValue = 0;
   buff[0] = 0x06 | ((adcChannel & 0x07) >> 2);
   buff[1] = ((adcChannel & 0x07) << 6);
   buff[2] = 0x00;
   digitalWrite(CS_MCP3208, 0);
   wiringPiSPIDataRW(SPI_CHANNEL, buff, 3);
   buff[1] = 0x0f & buff[1]; adcValue = (buff[1] << 8) | buff[2];
   digitalWrite(CS_MCP3208, 1);
   return adcValue;
}


int main(void) {
   unsigned char adcChannel_light = 0;
   int adcValue_light = 0;

   int moisture = 0;
   int adcValue[8] = { 0 };
   //소켓
   char buffer[BUF_LEN];
   char buffer2[BUF_LEN];
   char buffer_snd[BUF_LEN];
   struct sockaddr_in server_addr;
   struct sockaddr_in client_addr;
   char temp[20];
   int server_fd;
   int  client_fd;
   //server_fd, client_fd : 각 소켓 번호
   int len;
   int msg_size;

   printf("start");
   if (wiringPiSetupGpio() == -1)
   {
      fprintf(stdout, "Unable to start wiringPi :%s\n", strerror(errno));
      return 1;
   }
   if (wiringPiSPISetup(SPI_CHANNEL, SPI_SPEED) == -1)
   {
      fprintf(stdout, "wiringPiSPISetup Failed :%s\n", strerror(errno));
      return 1;
   }


   //

   if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {// 소켓 생성
      printf("Server : Can't open stream socket\n");
      exit(0);
   }
   memset(&server_addr, 0x00, sizeof(server_addr));
   //server_Addr 을 NULL로 초기화

   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
   server_addr.sin_port = htons(9900);
   //server_addr 셋팅

   if (bind(server_fd, (struct sockaddr*) & server_addr, sizeof(server_addr)) < 0)
   {//bind() 호출
      printf("Server : Can't bind local address.\n");
      exit(0);
   }
   while (1) {


      if (listen(server_fd, BUF_LEN) < 0)
      {//소켓을 수동 대기모드로 설정
         printf("Server : Can't listening connect.\n");
         //exit(0);
      }
      memset(buffer, 0x00, sizeof(buffer));
      memset(buffer2, 0x00, sizeof(buffer2));
      printf("Server : wating connection request.\n");


      //

      while (1) {
         //for (int k = 0; k < 3; k++)
       //   {
         int i = 0;
         //   pinMode(DHT11PIN, OUTPUT);
         //   pinMode(DHT11PIN, INPUT);
         pinMode(CS_MCP3208, OUTPUT);
         pinMode(RED, OUTPUT);
         pinMode(PUMP, OUTPUT);
         pinMode(DHT11PIN, OUTPUT);
         pinMode(DHT11PIN, INPUT);
         pinMode(fan, OUTPUT);


         adcValue_light = read_mcp3208_adc(adcChannel_light);
         printf("light sensor = %u\n", adcValue_light);

         /*   if (adcValue_light < 2000) {
              digitalWrite(RED, HIGH);
            }
            else {
              digitalWrite(RED, LOW);
            }*/

         dht11_read_val();
         if (tem_value >= tmp_change || hum_value >= hum_change) {
            fan_sensor(1);
            //digitalWrite(fan, HIGH);
            //forward(5, moter_step);
         }
         else {
            fan_sensor(0);
            //digitalWrite(fan, LOW);
            //   backward(5, moter_step);
         }
         moisture = read_mcp3208_adc(1);
         printf("Moisture = %5d\n", moisture);
         if (adcValue[1] < moisture_change) {
            digitalWrite(PUMP, 1);
            delay(7000);
         }
         else {
            digitalWrite(PUMP, 0);
            delay(7000);
         }

         //}
         // 소켓


         ///
         /*state = select(server_fd, null, 0, 0, &tv);
         if (state == 0) {
            break;
         }
         else {*/
         client_fd = accept(server_fd, (struct sockaddr*) & client_addr, &len);

         //while (1)
     //      {

         if (client_fd < 0)
         {
            printf("Server: accept failed.\n");
            //exit(0);
         }
         inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
         printf("Server : %s client connected.\n", temp);

         msg_size = read(client_fd, buffer, BUF_LEN);
         read(client_fd, buffer2, BUF_LEN);
         


switch(strcmp(buffer, "information")){
case 0 : printf("receive:%s\n", buffer);
                  printf("receive2:%s\n", buffer2);
	

                  if (strcmp(buffer2, "led_q") == 0) {
                     if (light_ex == 1) {
                        sprintf(buffer_snd, "%s", "onn");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                     else {

                        sprintf(buffer_snd, "%s", "off");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                  }

                  else if (strcmp(buffer2, "fan_q") == 0) {
                     if (fan_ex == 1) {
                        sprintf(buffer_snd, "%s", "onn");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                     else {
                        sprintf(buffer_snd, "%s", "off");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                  }

                  else if (strcmp(buffer2, "pump_q") == 0) {
                     if (pump_ex == 1) {
                        sprintf(buffer_snd, "%s", "onn");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                     else {
                        sprintf(buffer_snd, "%s", "off");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                  }

                  else if (strcmp(buffer2, "moter_q") == 0) {
                     if (moter_ex == 1) {
                        sprintf(buffer_snd, "%s", "onn");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                     else {
                        sprintf(buffer_snd, "%s", "off");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                  }

                  else if (strcmp(buffer2, "tem_q") == 0) {
                     if (tem_ex == 1) {
                        sprintf(buffer_snd, "%s", "onn");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                     else {
                        sprintf(buffer_snd, "%s", "off");
                        printf("rq:%s\n", buffer_snd);
                        write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                     }
                  }
                  else {
                     close(client_fd);
                     printf("Server-err : %s client closed.\n", temp);
                                       }
		break;
default : 
                  
                     close(client_fd);
                     printf("Server-err : %s client closed.\n", temp);
		break;

}



            if (msg_size > 0) {
               if (strcmp(buffer, "auto") == 0) {
                  printf("receive:%s\n", buffer);
                  printf("receive2:%s\n", buffer2);
                  if (strcmp(buffer2, "tmp") == 0) {
                     sprintf(buffer_snd, "%d", tmp_change);
                printf("rq:%s\n", buffer_snd);
                     write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                  }
                  else if (strcmp(buffer2, "hum") == 0) {
                     sprintf(buffer_snd, "%d", hum_change);
                     printf("rq:%s\n", buffer_snd);
                     write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                  }
                  else if (strcmp(buffer2, "led") == 0) {
                     sprintf(buffer_snd, "%d", led_value);
                     write(client_fd, buffer_snd, strlen(buffer_snd) + 1);

                  }
                  else if (strcmp(buffer2, "pump") == 0) {
                     sprintf(buffer_snd, "%d", moisture_change);
                     write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
                  }
                  else {
                     close(client_fd);
                     printf("Server-err : %s client closed.\n", temp);
                     break;
                  }
               }
               else if (strcmp(buffer, "auto_tmp") == 0) {
                  printf("receive:%s\n", buffer);
                  printf("receive2:%s\n", buffer2);
char s[2]={0} ;
read(client_fd, buffer2, 1024);
strcpy(s, buffer2);
                  tmp_change = atoi(s);
printf("receive3:%d\n", tmp_change);
                  //tmp_change = atof(change);
               }
               else if (strcmp(buffer, "auto_hum") == 0) {
                  printf("receive:%s\n", buffer);
                  printf("receive2:%s\n", buffer2);
                  read(client_fd, buffer2, 1024);
                  hum_change = atoi(buffer2);
                  //   change = read(client_fd, buffer2, 1024);
                  //   hum_change = atof(change);
               }
               else if (strcmp(buffer, "auto_led") == 0) {
                  printf("receive:%s\n", buffer);
                  printf("receive2:%s\n", buffer2);
                  led_value = read(client_fd, buffer2, 1024);
                  //change = read(client_fd, buffer2, 1024);
                  //led_value = atoi(change);
               }
               else if (strcmp(buffer, "auto_mois") == 0) {
                  printf("receive:%s\n", buffer);
                  printf("receive2:%s\n", buffer2);
                  //   change = read(client_fd, buffer2, 1024);
                     //moisture_change = atoi(change);
                  moisture_change = read(client_fd, buffer2, 1024);
               }
               else if (strcmp(buffer, "sensor") == 0) {
                  printf("receive:%s\n", buffer);
                  printf("receive2:%s\n", buffer2);
                  if (strcmp(buffer2, "fan_on") == 0) {

                     fan_sensor(1);
                  }
                  else if (strcmp(buffer2, "fan_off") == 0) {

                     fan_sensor(0);
                  }
                  else if (strcmp(buffer2, "led_on") == 0) {

                     light_sensor(1);
                  }
                  else if (strcmp(buffer2, "led_off") == 0) {

                     light_sensor(0);
                  }
                  else if (strcmp(buffer2, "pump_on") == 0) {

                     pump_sensor(1);
                  }
                  else if (strcmp(buffer2, "pump_off") == 0) {

                     pump_sensor(0);
                  }
                  else if (strcmp(buffer2, "moter_on") == 0) {

                     moter_sensor(1);
                  }
                  else  if (strcmp(buffer2, "moter_off") == 0) {

                     moter_sensor(0);
                  }
                  else  if (strcmp(buffer2, "th_on") == 0) {
                     printf("receive:%s\n", buffer);
                     printf("receive2:%s\n", buffer2);
                     tmp_sensor(1);
                  }
                  else  if (strcmp(buffer2, "th_off") == 0) {
                     printf("receive:%s\n", buffer);
                     printf("receive2:%s\n", buffer2);
                     tmp_sensor(0);
                  }
                  else {
                     close(client_fd);
                     printf("Server-err : %s client closed.\n", temp);
                     break;
                  }
               }

               else
               {
                  close(client_fd);
                  printf("Server : %s client closed.\n", temp);
                  break;
               }


            }
         

         else
         {
            close(client_fd);
            printf("Server : %s client closed.\n", temp);
            break;
         }


      }


   }

   printf("Server :server_fd closed.\n");
   close(server_fd);
   return 0;

}
