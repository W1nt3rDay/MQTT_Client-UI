#pragma warning(disable : 4996)
#include <winsock2.h> 
#pragma comment(lib, "ws2_32.lib")
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <WS2tcpip.h>

#include "mqtt.h"



// -------------------- MQTT服务器的参数 --------------------
#define SERVER_IP   "124.70.218.131"
#define SERVER_PORT 1883

// -------------------- MQTT三元组 --------------------
#define CLIENTID	"6883826694a9a05c33772d12_dev_6ull_0_0_2025072513" //设备ID
#define USERNAME    "6883826694a9a05c33772d12_dev_6ull" //用户名
#define PASSWORD    "e28fd984809b1c41768994a2c8ebb24fbff9eea4534888dab00e7d347029e059"
// -------------------- 订阅、发布主题 --------------------
#define SET_TOPIC   "$oc/devices/6883826694a9a05c33772d12_dev_6ull/sys/messages/down"
#define POST_TOPIC  "$oc/devices/6883826694a9a05c33772d12_dev_6ull/sys/properties/report"


SOCKET connectSocket; //连接套接字
WSADATA wsaData;  // 结构体变量  Winsock数据结构

unsigned char mqtt_rxbuf[1024 * 1024];
unsigned char mqtt_txbuf[256];
unsigned int mqtt_rxlen;
unsigned int mqtt_txlen;

void MQTT_Init()
{
	//初始化MQTT相关参数
	mqtt_rxlen = sizeof(mqtt_rxbuf); //接收缓冲区长度
	mqtt_txlen = sizeof(mqtt_txbuf); //发送缓冲区长度
	memset(mqtt_rxbuf, 0, mqtt_rxlen); //清空接收缓冲区
	memset(mqtt_txbuf, 0, mqtt_txlen); //清空发送缓冲区
}

void MQTT_SendBuf(const unsigned char* buf, uint32_t len)
{
	Client_SendData(buf, len);
}


//连接MQTT
unsigned char MQTT_Connect(char* ClientID, char* Username, char* Password)
{
	int size = 0; //接收数据的大小
	int ClientIDLen = (int)strlen(ClientID);
	int UsernameLen = (int)strlen(Username);
	int PasswordLen = (int)strlen(Password);
	mqtt_txlen = 0;  //在mqtt_network.h中定义的全局变量
	unsigned int DataLen; //数据长度
	unsigned char buff[256]; //临时缓冲区
	
	DataLen = 10 + (ClientIDLen + 2) + (UsernameLen + 2) + (PasswordLen + 2); //计算剩余长度

	mqtt_txbuf[mqtt_txlen++] = 0x10;  //Packet fixed header固定包头
	//剩余长度
	do
	{
		unsigned char encodeByte = DataLen % 128; //计算剩余长度的编码字节
		DataLen /= 128; //除以128
		if (DataLen > 0)
		{
			encodeByte |= 0x80; //设置最高位为1
		}
		mqtt_txbuf[mqtt_txlen++] = encodeByte; //添加到缓冲区
	} while (DataLen > 0);

	//Protocol Name
	mqtt_txbuf[mqtt_txlen++] = 0; //协议名称长度高字节
	mqtt_txbuf[mqtt_txlen++] = 4; //协议名称长度低字节
	mqtt_txbuf[mqtt_txlen++] = 'M'; //协议名称'MQTT'
	mqtt_txbuf[mqtt_txlen++] = 'Q'; //协议名称'MQTT'
	mqtt_txbuf[mqtt_txlen++] = 'T'; //协议名称'MQTT'
	mqtt_txbuf[mqtt_txlen++] = 'T'; //协议名称'MQTT'

	//Protocol Level
	mqtt_txbuf[mqtt_txlen++] = MQTT_VERSION_3_1_1; //协议级别MQTT 3.1.1

	//Connect Flags
	mqtt_txbuf[mqtt_txlen++] = 0xc2; 

	//Keep Alive
	mqtt_txbuf[mqtt_txlen++] = 0; //保持连接时间高字节
	mqtt_txbuf[mqtt_txlen++] = 100; //保持连接时间低字节 (60秒)

	mqtt_txbuf[mqtt_txlen++] = BYTE1(ClientIDLen); //ClientID长度高字节
	mqtt_txbuf[mqtt_txlen++] = BYTE0(ClientIDLen); //ClientID长度低字节
	memcpy(&mqtt_txbuf[mqtt_txlen], ClientID, ClientIDLen); //复制ClientID到缓冲区
	mqtt_txlen += ClientIDLen; //更新缓冲区长度

	if (UsernameLen > 0)
	{
		mqtt_txbuf[mqtt_txlen++] = BYTE1(UsernameLen); //用户名长度高字节
		mqtt_txbuf[mqtt_txlen++] = BYTE0(UsernameLen); //用户名长度低字节
		memcpy(&mqtt_txbuf[mqtt_txlen], Username, UsernameLen); //复制用户名到缓冲区
		mqtt_txlen += UsernameLen; //更新缓冲区长度
	}

	if (PasswordLen > 0)
	{
		mqtt_txbuf[mqtt_txlen++] = BYTE1(PasswordLen); //密码长度高字节
		mqtt_txbuf[mqtt_txlen++] = BYTE0(PasswordLen); //密码长度低字节
		memcpy(&mqtt_txbuf[mqtt_txlen], Password, PasswordLen); //复制密码到缓冲区
		mqtt_txlen += PasswordLen; //更新缓冲区长度
	}

	//发送连接请求
	for (int i = 0; i < 5; i++)
	{
		memset(mqtt_rxbuf, 0, sizeof(mqtt_rxbuf)); //清空接收缓冲区
		MQTT_SendBuf(mqtt_txbuf, mqtt_txlen); //发送缓冲区内容
		size = Client_GetData(buff); //接收数据
		if (size <= 0)
		{
			continue; //如果接收失败，重试
		}
		//接受成功，开始判断接收到的数据
		//size = 4;
		memcpy(mqtt_rxbuf, buff, size); //复制接收到的数据到接收缓冲区

		printf("MQTT_Connect: Received %d bytes\n", size);
		for (int j = 0; j < size; j++)
		{
			printf("%02X ", mqtt_rxbuf[j]); //打印接收到的数据
		}
		printf("\n");

		if ( size < 2 ) //如果接收到的数据长度小于2，说明数据不完整
		{
			printf("MQTT_Connect: Received data is too short, retrying...\n");
			continue; //继续重试
		}

		else if ( mqtt_rxbuf[0] == packet_connectAck[0] && 
			 mqtt_rxbuf[1] == packet_connectAck[1] ) //检查是否为连接成功的响应
		{
			return 0; //连接成功
		}
		else
		{
			printf("MQTT_Connect: Connection failed, retrying...\n");
		}

	}
	return 1; // Ensure the function returns a value
}

//发布主题
//topic: 主题名称，message: 消息内容，qos: 服务质量等级
unsigned char MQTT_PublishData(char* topic, char* message, unsigned char qos)
{
	mqtt_txlen = 0; //重置发送缓冲区长度
	unsigned int DataLen; //剩余长度
	unsigned short id = 0; //报文标识符
	unsigned int TopicLen = (int)strlen(topic); //主题长度
	unsigned int MessageLen = (int)strlen(message); //消息长度

	printf("上报JSON消息长度: %d\n", MessageLen);
	printf("message = %s\n", message);
	if (qos)   DataLen = (2 + TopicLen + 2 + MessageLen); //计算剩余长度
	else DataLen = (2 + TopicLen + MessageLen); //计算剩余长度

	//1. fixed header
	mqtt_txbuf[mqtt_txlen++] = 0x30; //发布主题的报文类型

	//2. 剩余长度
	do
	{
		unsigned char encodeByte = DataLen % 128; //计算剩余长度的编码字节
		DataLen /= 128; //除以128
		if (DataLen > 0)
		{
			encodeByte |= 0x80; //设置最高位为1
		}
		mqtt_txbuf[mqtt_txlen++] = encodeByte; //添加到缓冲区
	} while (DataLen > 0);

	//3. 可变报头
	mqtt_txbuf[mqtt_txlen++] = BYTE1(TopicLen); //主题长度高字节
	mqtt_txbuf[mqtt_txlen++] = BYTE0(TopicLen); //主题长度低字节
	memcpy(&mqtt_txbuf[mqtt_txlen], topic, TopicLen); //复制主题到缓冲区
	mqtt_txlen += TopicLen; //更新缓冲区长度

	//报文标识符
	if (qos)
	{

		mqtt_txbuf[mqtt_txlen++] = BYTE1(id);
		mqtt_txbuf[mqtt_txlen++] = BYTE0(id);
		id++; //增加报文标识符
	}
	memcpy(&mqtt_txbuf[mqtt_txlen], message, MessageLen); //复制消息到缓冲区
	mqtt_txlen += MessageLen; //更新缓冲区长度

	MQTT_SendBuf(mqtt_txbuf, mqtt_txlen); //发送缓冲区内容
	return mqtt_txlen;
}

//订阅主题
unsigned char MQTT_SubscribeTopic(char* topic, unsigned char whether, unsigned char qos)
{
	unsigned char i, j;
	unsigned int size = 0;
	unsigned char buff[256]; //临时缓冲区
	unsigned int TopicLen = (int)strlen(topic); //主题长度
	unsigned int DataLen = 2 + (TopicLen + 2) + (whether ? 1 : 0); //计算剩余长度
	mqtt_txlen = 0; //重置发送缓冲区长度

	//1. fixed header
	if (whether) mqtt_txbuf[mqtt_txlen++] = 0x82; //订阅主题的报文类型
	else mqtt_txbuf[mqtt_txlen++] = 0xA2; //取消订阅主题的报文类型
	
	//2. 剩余长度
	do
	{
		unsigned char encodeByte = DataLen % 128; //计算剩余长度的编码字节
		DataLen /= 128; //除以128
		if (DataLen > 0)
		{
			encodeByte |= 0x80; //设置最高位为1
		}
		mqtt_txbuf[mqtt_txlen++] = encodeByte; //添加到缓冲区
	} while (DataLen > 0);

	//3. 可变报头
	mqtt_txbuf[mqtt_txlen++] = 0x00; //高字节
	mqtt_txbuf[mqtt_txlen++] = 0x01; //低字节，表示订阅的主题数量为1
	//4. 有效载荷
	mqtt_txbuf[mqtt_txlen++] = BYTE1(TopicLen); //主题长度高字节
	mqtt_txbuf[mqtt_txlen++] = BYTE0(TopicLen); //主题长度低字节
	memcpy(&mqtt_txbuf[mqtt_txlen], topic, TopicLen); //复制主题到缓冲区
	mqtt_txlen += TopicLen; //更新缓冲区长度
	if (whether) //如果是订阅主题
	{
		mqtt_txbuf[mqtt_txlen++] = qos; //QoS等级
	}
	for (i = 0; i < 10; i++)
	{
		memset(mqtt_rxbuf, 0, mqtt_rxlen); //清空接收缓冲区
		MQTT_SendBuf(mqtt_txbuf, mqtt_txlen); //发送缓冲区内容
		size = Client_GetData(buff); //接收数据
		if (size <= 0) 
		{
			continue; //如果接收失败，重试
		}
		memcpy(mqtt_rxbuf, buff, size); //复制接收到的数据到接收缓冲区

		printf("订阅应答MQTT_SubscribeTopic: Received %d bytes\n", size);
		for (j = 0; j < size; j++)
		{
			printf("%02X ", mqtt_rxbuf[j]); //打印接收到的数据
		}
		printf("\n");


		if (size < 2) //如果接收到的数据长度小于2，说明数据不完整
		{
			printf("订阅应答MQTT_SubscribeTopic: Received data is too short, retrying...\n");
			continue; //继续重试
		}
		else if (mqtt_rxbuf[0] == packet_subAck[0] &&
			mqtt_rxbuf[1] == packet_subAck[1] ) //检查是否为订阅成功的响应
		{
			return 0; //订阅成功
		}
		Sleep(1000); //等待1秒后重试
	}

	return 1;
}


char mqtt_message[1024]; //用于存储要发送的消息内容


//Thread function for receiving data
void Thread_ReceiveData(void)
{
	// 接收数据
	char buffer[1024];
	char request_id[100];
	char send_cmd[500];

	int recvSize;
	while (1)
	{
		//等待服务器下发消息
		recvSize = recv(connectSocket, buffer, 1024, 0);
		if (recvSize == SOCKET_ERROR)
		{
			std::cout << "网络断开连接: " << WSAGetLastError() << std::endl; //输出错误信息并退出程序
			return;
		}
		if (recvSize > 0)
		{
			printf("服务器下发消息:\r\n");
			//接收下发的数据
			for (int i = 0; i < recvSize; i++)
			{
				printf("%c", buffer[i]);
			}
			printf("\r\n");

			//下发指令请求回应给服务器(命令下发)
			if (strstr((char*)&buffer[5], "sys/commands/request_id="))
			{
				char* p = NULL;
				p = strstr((char*)&buffer[5], "request_id=");
				if (p)
				{
					//解析数据
					//$oc/devices/65697df3585c81787ad4da82_stm32/sys/commands/request_id=6e925cc1-4a8d-4eab-8d85-6c7f15d72189
					strncpy(request_id, p, 47);
				}

				//上报数据
				sprintf(mqtt_message, "{\"result_code\":0,\"response_name\":\"COMMAND_RESPONSE\",\"paras\":{\"result\":\"success\"}}");

				sprintf(send_cmd, "sys/commands/response/%s", request_id);

				//发布响应消息
				MQTT_PublishData(send_cmd, mqtt_message, 0);

				printf("(命令)发布主题:%s\r\n", send_cmd);
				printf("(命令)发布数据:%s\r\n", mqtt_message);
			}
		}
	}
}



// 网络相关函数实现...


int Client_SendData(const unsigned char* data, uint32_t len)
{
	if (connectSocket == INVALID_SOCKET)
	{
		printf("Socket is not connected.\n");
		return -1;
	}
	int result = send(connectSocket, (const char*)data, len, 0);
	if (result == SOCKET_ERROR)
	{
		printf("send failed with error: %d\n", WSAGetLastError());
		return -1;
	}
	return 0;
}

//获取服务器下发的数据
int Client_GetData(unsigned char* buf)
{
	if (connectSocket == INVALID_SOCKET)
	{
		printf("Socket is not connected.\n");
		return -1;
	}
	int result = recv(connectSocket, (char*)buf, 200, 0);
	if (result == SOCKET_ERROR)
	{
		printf("recv failed with error: %d\n", WSAGetLastError());
		return -1;
	}
	return result;
}


int main()
{

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData); //初始化Winsock
	if (result != 0) 
	{
		printf("WSAStartup failed with error: %d\n", result);
		return 1; //初始化失败
	}

	connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); //创建套接字
	if (connectSocket == INVALID_SOCKET) 
	{
		printf("Socket creation failed with error: %d\n", WSAGetLastError());
		WSACleanup(); //清理Winsock
		return 1; //套接字创建失败
	}

	// 设置服务器地址和端口
	struct addrinfo hints, * result_list, * ptr;
	int sock_result;

	// 初始化hints结构
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;      // IPv4
	hints.ai_socktype = SOCK_STREAM; // TCP流
	hints.ai_protocol = IPPROTO_TCP; // TCP协议

	// 解析域名或IP地址
	sock_result = getaddrinfo(SERVER_IP, NULL, &hints, &result_list);
	if (sock_result != 0) {
		printf("getaddrinfo failed with error: %d\n", sock_result);
		return 1;
	}

	// 遍历结果链表，尝试连接每个地址
	for (ptr = result_list; ptr != NULL; ptr = ptr->ai_next) {
		// 创建套接字
		connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if (connectSocket == INVALID_SOCKET) {
			printf("socket failed with error: %ld\n", WSAGetLastError());
			continue;
		}

		// 设置端口号
		((struct sockaddr_in*)ptr->ai_addr)->sin_port = htons(SERVER_PORT);

		// 连接到服务器
		sock_result = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (sock_result == SOCKET_ERROR) {
			printf("Connection failed with error: %d\n", WSAGetLastError());
			closesocket(connectSocket);
			connectSocket = INVALID_SOCKET;
			continue;
		}
		break; // 连接成功，跳出循环
	}

	// 释放地址信息
	freeaddrinfo(result_list);

	// 检查是否成功连接
	if (connectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}

	printf("Connected to MQTT server at %s:%d\n", SERVER_IP, SERVER_PORT);

	/*连接成功，开始MQTT连接*/

	MQTT_Init();

	while (true)
	{
		/*登陆服务器*/
		if ( 0 == MQTT_Connect((char*)CLIENTID, (char*)USERNAME, (char*)PASSWORD) )
		{
			printf("登陆成功！MQTT_Connect: Login successful\n");
			break; //登录成功，跳出循环
		}
		else
		{
			printf("服务器登陆校验中...\n");
			Sleep(1000); //等待1秒后重试
		}
	}


	//订阅物联网平台数据
	int stat = MQTT_SubscribeTopic((char*)SET_TOPIC, 1, 1); //订阅主题
	if (stat)
	{
		printf("MQTT_SubscribeTopic: Subscription failed\n");
		closesocket(connectSocket); //关闭套接字
		WSACleanup(); //清理Winsock
		return 1; //订阅失败
	}
	printf("MQTT_SubscribeTopic: Subscription successful\n");


	//Create thread to receive data
	HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_ReceiveData, NULL, 0, NULL);
	if (hThread == NULL)
	{
		printf("Create Thread failed with error: %d\n", GetLastError());
		return 1; //线程创建失败
	}

	//发布消息到物联网平台
	while (1)
	{
		sprintf(mqtt_message, "{ \"services\": [{\"service_id\": \"imx6ull\", \"properties\":{\"Temp\":30, \"Humi\":60}}] }" );

		//发布主题
		MQTT_PublishData((char*)POST_TOPIC, mqtt_message, 0); //发布消息
		printf("消息发布成功！MQTT_PublishData: Message published successfully\n");
		Sleep(5000); //等待5秒后继续发布消息
	}
}