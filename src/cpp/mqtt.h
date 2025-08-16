#pragma once
#ifndef MQTT_H
#define MQTT_H

/*===============================================================
 * File:        mqtt.h
 * Description: MQTT 协议客户端接口头文件
 * Author:      wint3r
 * Created:     2025-07-19
 * Last Update: 2025-08-14
 *================================================================*/

 /*==============================
  * 依赖头文件
  *==============================*/
#include <winsock2.h>                                      // SOCKET, WSADATA
#include <stdint.h>                                               // uint32_t
#include <QObject>
  /*==============================
   * 宏定义（字节操作）
   *==============================*/
   // 用于多字节数据处理的宏
#define BYTE0(dwTemp)   (*(char *)(&dwTemp))
#define BYTE1(dwTemp)   (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)   (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)   (*((char *)(&dwTemp) + 3))

/*==============================
 * 协议固定报文（常量数组）
 *==============================*/
static const unsigned char packet_connectAck[] = { 0x20, 0x02, 0x00, 0x00 };
static const unsigned char packet_subAck[] = { 0x90, 0x03 };

/*==============================
 * 全局变量声明
 *==============================*/
extern SOCKET   connectSocket;                                   // 连接套接字
extern WSADATA  wsaData;                                   // Winsock 数据结构

/*==============================
 * MQTT 控制报文类型枚举
 *==============================*/
typedef enum {
    M_RESERVED1 = 0,                                                   // 保留
    M_CONNECT = 1,                                       // 客户端请求连接服务器
    M_CONNACK = 2,                                                 // 连接确认
    M_PUBLISH = 3,                                                 // 发布消息
    M_PUBACK = 4,                                             // QoS1 发布确认
    M_PUBREC = 5,                                     // QoS2 发布收到（第一步）
    M_PUBREL = 6,                                     // QoS2 发布释放（第二步）
    M_PUBCOMP = 7,                                    // QoS2 发布完成（第三步）
    M_SUBSCRIBE = 8,                                               // 订阅请求
    M_SUBACK = 9,                                                  // 订阅确认
    M_UNSUBSCRIBE = 10,                                         // 取消订阅请求
    M_UNSUBACK = 11,                                            // 取消订阅确认
    M_PINGREQ = 12,                                                // 心跳请求
    M_PINGRESP = 13,                                               // 心跳响应
    M_DISCONNECT = 14,                                             // 断开连接
    M_AUTH = 15                                         // 认证交换（MQTT 5.0）
} MQTT_MessageType;

/*==============================
 * MQTT 版本枚举
 *==============================*/
typedef enum {
    MQTT_VERSION_3_1 = 3,
    MQTT_VERSION_3_1_1 = 4,
    MQTT_VERSION_5_0 = 5
} MQTT_Version;

/*==============================
 * 函数声明
 *==============================*/

 /**
  * @brief 连接到 MQTT 服务器
  * @param ClientID  客户端 ID
  * @param Username  用户名
  * @param Password  密码
  * @return 0 成功, 1 失败
  */
unsigned char MQTT_Connect(const char* ClientID, const char* Username, const char* Password);

/**
 * @brief 发送数据到服务器
 * @param buf  数据缓冲区
 * @param len  数据长度
 */
void MQTT_SendBuf(const unsigned char* buf, uint32_t len);

/**
 * @brief 发布消息到指定主题
 * @param topic    主题
 * @param message  消息
 * @param qos      服务质量等级
 * @return 0 成功, 1 失败
 */
unsigned char MQTT_PublishData(char* topic, char* message, unsigned char qos);

/**
 * @brief 订阅或取消订阅主题
 * @param topic    主题
 * @param whether  0 订阅, 1 取消订阅
 * @param qos      QoS 等级
 * @return 0 成功, 1 失败
 */
unsigned char MQTT_SubscribeTopic(char* topic, unsigned char whether,
                                  unsigned char qos);

/**
 * @brief 接收数据线程
 */
void Thread_ReceiveData(void);

/**
 * @brief 网络层接口 - 发送数据
 */
int Client_SendData(const unsigned char* data, uint32_t len);

/**
 * @brief 网络层接口 - 接收数据
 */
int Client_GetData(unsigned char* buf);



/**
 * @brief QT通信接口 - setServerIP
 */
class MqttClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString serverIP READ serverIP WRITE setServerIP NOTIFY serverIPChanged)
    Q_PROPERTY(int serverPort READ serverPort WRITE setServerPort NOTIFY serverPortChanged)
    Q_PROPERTY(QString clientID READ clientID WRITE setClientID NOTIFY clientIDChanged)
    Q_PROPERTY(QString userName READ userName WRITE setUserName NOTIFY userNameChanged)
    Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
public:
    explicit MqttClient(QObject *parent = nullptr);
    QString serverIP() const;
    int serverPort() const;
    QString clientID() const;
    QString userName() const;
    QString password() const;
public slots:
    Q_INVOKABLE void setServerIP(const QString &ip);
    Q_INVOKABLE void setServerPort(const int port);
    Q_INVOKABLE void setClientID(const QString &clientid);
    Q_INVOKABLE void setUserName(const QString &username);
    Q_INVOKABLE void setPassword(const QString &pswd);
    Q_INVOKABLE int start();

signals:
    void serverIPChanged();
    void serverPortChanged();
    void clientIDChanged();
    void userNameChanged();
    void passwordChanged();
private:
    QString m_serverIP = "124.70.218.131";
    int m_serverPort = 1883;
    QString m_clientID = "6883826694a9a05c33772d12_dev_6ull_0_0_2025072513";
    QString m_userName = "6883826694a9a05c33772d12_dev_6ull";
    QString m_password = "e28fd984809b1c41768994a2c8ebb24fbff9eea4534888dab00e7d347029e059";
};



#endif // MQTT_H
