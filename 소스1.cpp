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
//소켓 프로그래밍에 사용될 헤더파일 선언

#define BUF_LEN 128
//메시지 송수신에 사용될 버퍼 크기를 선언

int light_ex = 0;
int fan_ex = 0;
int pump_ex = 0;
int moter_ex = 0;
int tem_ex = 0;
int moter_step = 5;
int moisture_change = 1000;
float hum_change = 70.0;
//온습도 tmp
#define MAX_TIME 85  
#define DHT11PIN 4  //7
#define fan 17
int dht11_val[5] = { 0,0,0,0,0 };
float tem_value = 0;
float hum_value = 0;
float tmp_change = 24.0;
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

int main(void)
{
	char buffer[BUF_LEN];
	char buffer2[BUF_LEN];
	char buffer_snd[BUF_LEN];
	struct sockaddr_in server_addr, client_addr;
	char temp[20];
	int server_fd, client_fd;
	//server_fd, client_fd : 각 소켓 번호
	int len, msg_size;



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
		if (listen(server_fd, 5) < 0)
		{//소켓을 수동 대기모드로 설정
			printf("Server : Can't listening connect.\n");
			exit(0);
		}

		memset(buffer, 0x00, sizeof(buffer));
		printf("Server : wating connection request.\n");
		len = sizeof(client_addr);
		client_fd = accept(server_fd, (struct sockaddr*) & client_addr, &len);

		while (1)
		{
			if (client_fd < 0)
			{
				printf("Server: accept failed.\n");
				exit(0);
			}

			inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
			printf("Server : %s client connected.\n", temp);

			msg_size = read(client_fd, buffer, 1024);
			read(client_fd, buffer2, 1024);
			if (msg_size > 0)
			{
				if (strcmp(buffer, "information") == 0) {
					printf("receive:%s\n", buffer);
					printf("receive2:%s\n", buffer2);
					if (strcmp(buffer2, "led_q") == 0) {
						if (light_ex == 1) {
							sprintf(buffer_snd, "%s", "onn");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
						 if (light_ex == 0) {

							sprintf(buffer_snd, "%s", "off");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
					}
					 if (strcmp(buffer2, "fan_q") == 0) {
						if (fan_ex == 1) {
							sprintf(buffer_snd, "%s", "onn");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
						 if (fan_ex == 0) {
							sprintf(buffer_snd, "%s", "off");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
					}
					 if (strcmp(buffer2, "pump_q") == 0) {
						if (pump_ex == 1) {
							sprintf(buffer_snd, "%s", "onn");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
						 if (pump_ex == 0) {
							sprintf(buffer_snd, "%s", "off");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
					}
					 if (strcmp(buffer2, "moter_q") == 0) {
						if (moter_ex == 1) {
							sprintf(buffer_snd, "%s", "onn");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
						 if (moter_ex == 0) {
							sprintf(buffer_snd, "%s", "off");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
					}
					 if (strcmp(buffer2, "tem_q") == 0) {
						if (tem_ex == 1) {
							sprintf(buffer_snd, "%s", "onn");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
						 if (tem_ex == 0) {
							sprintf(buffer_snd, "%s", "off");
							printf("rq:%s\n", buffer_snd);
							write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
						}
					}
				}
				 if (strcmp(buffer, "auto") == 0) {
					printf("receive:%s\n", buffer);
					printf("receive2:%s\n", buffer2);
					if (strcmp(buffer2, "tmp") == 0) {
						sprintf(buffer_snd, "%.1f", tmp_change);
						printf("rq:%s\n", buffer_snd);
						write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
					}
					 if (strcmp(buffer2, "hum") == 0) {
						sprintf(buffer_snd, "%.1f", hum_change);
						printf("rq:%s\n", buffer_snd);
						write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
					}
					 if (strcmp(buffer2, "led") == 0) {
						sprintf(buffer_snd, "%d", led_value);
						printf("rq:%s\n", buffer_snd);
						write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
					}
					 if (strcmp(buffer2, "pump") == 0) {
						sprintf(buffer_snd, "%d", moisture_change);
						printf("rq:%s\n", buffer_snd);
						write(client_fd, buffer_snd, strlen(buffer_snd) + 1);
					}

				}
				 if (strcmp(buffer, "auto_tmp") == 0) {
					printf("receive:%s\n", buffer);
					printf("receive2:%s\n", buffer2);
				//	tmp_change = read(client_fd, buffer2, 1024);
				}
				 if (strcmp(buffer, "auto_hum") == 0) {
					printf("receive:%s\n", buffer);
					printf("receive2:%s\n", buffer2);
				//	hum_change = read(client_fd, buffer2, 1024);
				}
				 if (strcmp(buffer, "auto_led") == 0) {
					printf("receive:%s\n", buffer);
					printf("receive2:%s\n", buffer2);
				//	led_value = read(client_fd, buffer2, 1024);
				}
				 if (strcmp(buffer, "auto_mois") == 0) {
					printf("receive:%s\n", buffer);
					printf("receive2:%s\n", buffer2);
				//	moisture_change = read(client_fd, buffer2, 1024);
				}



				 if (strcmp(buffer, "sensor") == 0) {
					printf("receive:%s\n", buffer);
					printf("receive2:%s\n", buffer2);
					
					/*if (strcmp(buffer2, "fan_on") == 0) {

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
					else if (strcmp(buffer2, "moter_off") == 0) {

						moter_sensor(0);
					}
					else if (strcmp(buffer2, "th_on") == 0) {

						tmp_sensor(1);
					}
					else if (strcmp(buffer2, "th_off") == 0) {

						tmp_sensor(0);
					}*/
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
	close(server_fd);
	return 0;
}
