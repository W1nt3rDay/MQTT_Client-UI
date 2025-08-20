#ifndef MQTT_H
#define MQTT_H

#define DEBUG_MODE 1

#include <QObject>
#include <QTimer>
#include <QTcpServer>


#define BYTE0(dwTemp)       (*(char *)(&dwTemp))
#define BYTE1(dwTemp)       (*((char *)(&dwTemp) + 1))
#define BYTE2(dwTemp)       (*((char *)(&dwTemp) + 2))
#define BYTE3(dwTemp)       (*((char *)(&dwTemp) + 3))

typedef enum
{
    //名字 	    值 			报文流动方向 	描述
    M_RESERVED1	=0	,	//	禁止	保留
    M_CONNECT		,	//	客户端到服务端	客户端请求连接服务端
    M_CONNACK		,	//	服务端到客户端	连接报文确认
    M_PUBLISH		,	//	两个方向都允许	发布消息
    M_PUBACK		,	//	两个方向都允许	QoS 1消息发布收到确认
    M_PUBREC		,	//	两个方向都允许	发布收到（保证交付第一步）
    M_PUBREL		,	//	两个方向都允许	发布释放（保证交付第二步）
    M_PUBCOMP		,	//	两个方向都允许	QoS 2消息发布完成（保证交互第三步）
    M_SUBSCRIBE		,	//	客户端到服务端	客户端订阅请求
    M_SUBACK		,	//	服务端到客户端	订阅请求报文确认
    M_UNSUBSCRIBE	,	//	客户端到服务端	客户端取消订阅请求
    M_UNSUBACK		,	//	服务端到客户端	取消订阅报文确认
    M_PINGREQ		,	//	客户端到服务端	心跳请求
    M_PINGRESP		,	//	服务端到客户端	心跳响应
    M_DISCONNECT	,	//	客户端到服务端	客户端断开连接
    M_RESERVED2		,	//	禁止	保留
}_typdef_mqtt_message;


class MQTT_WorkClass: public QObject
{
    Q_OBJECT
public:
    QTimer* timer = nullptr;
    MQTT_WorkClass(QObject* parent=nullptr) : QObject(parent) {}
    ~MQTT_WorkClass();

    //username initialize
    void LoginInit(char* ProductKey, char* DeviceName, char* DeviceSecret);

    //MQTT protocol function declaraion
    Q_INVOKABLE quint8 MQTT_PublishData(const QString& topic, const QString& message, quint8 qos);
    Q_INVOKABLE quint8 MQTT_SubscribeTopic(const QString& topic,quint8 qos,quint8 whether);
    Q_INVOKABLE void MQTT_Init(void);
    Q_INVOKABLE quint8 MQTT_Connect(char *ClientID,char *Username,char *Password);
    Q_INVOKABLE void MQTT_SentHeart(void);
    Q_INVOKABLE void MQTT_Disconnect(void);
    Q_INVOKABLE void MQTT_SendBuf(quint8 *buf,quint16 len);
    Q_INVOKABLE void ConnectMqttServer(QString ip,quint16 port);
    Q_INVOKABLE void Set_MQTT_Addr(QString ip,quint16 port,QString MQTT_ClientID,QString MQTT_UserName,QString MQTT_PassWord);
    Q_INVOKABLE void StartEvenLoop();
public slots:
    void EndEvenLoop();
    void run();
    void LocalTcpClientConnectedSlot();
    void LocalTcpClientDisconnectedSlot();
    void LocalTcpClientReadDataSlot();
    void LocalTcpClientBytesWrittenSlot(qint64 byte);
    //subscribe topic
    Q_INVOKABLE void slot_SubscribeTopic(QString topic);
    //publish message
    Q_INVOKABLE void slot_PublishData(QString topic, QString message);
    //disconnect
    Q_INVOKABLE void slot_tcp_close();
signals:
    void logSend(QString text);
    void MQTT_ConnectState(bool state);
private:
    quint8 *mqtt_rxbuf;
    quint8 *mqtt_txbuf;
    quint16 mqtt_rxlen;
    quint16 mqtt_txlen;
    quint8 _mqtt_txbuf[256];//发送数据缓存区
    quint8 _mqtt_rxbuf[256];//接收数据缓存区

    QTcpSocket* LocalTcpClientSocket = nullptr;
    QString m_ip;
    quint16 m_port;

    bool socket_type = 0;

    QString m_MQTT_ClientID;
    QString m_MQTT_UserName;
    QString m_MQTT_PassWord;

    QEventLoop loop;

    QByteArray  ReadData;
};

#endif
