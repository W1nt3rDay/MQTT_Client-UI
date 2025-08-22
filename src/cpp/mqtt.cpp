#include <QDebug>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "mqtt.h"



const quint8 parket_connetAck[] = {0x20,0x02,0x00,0x00};
const quint8 parket_disconnet[] = {0xe0,0x00};
const quint8 parket_heart[] = {0xc0,0x00};
const quint8 parket_heart_reply[] = {0xc0,0x00};
const quint8 parket_subAck[] = {0x90,0x03};

MQTT_WorkClass::~MQTT_WorkClass()
{
    qDebug() << "destruction---TCP";
}

void MQTT_WorkClass::run()
{
    qDebug() << "run!";

    if (timer)
    {
        delete timer;
        timer = nullptr;
    }
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(EndEvenLoop()));

    socket_type = 0;
    //connect server
    ConnectMqttServer(m_ip, m_port);
#if DEBUG_MODE
    qDebug() << "connect MQTT server begin";
#endif
    StartEvenLoop();

    //init MQTT protocol
    MQTT_Init();

    if (MQTT_Connect(m_MQTT_ClientID.toUtf8().data(),m_MQTT_UserName.toUtf8().data(),m_MQTT_PassWord.toUtf8().data()))
    {
        emit logSend("MQTT Server login failed T_T.\n");
    }
    else
    {
        emit logSend("MQTT login successful! TvT.\n");
    }
}

void MQTT_WorkClass::MQTT_Init(void)
{
    //缓冲区赋值
    mqtt_rxbuf = _mqtt_rxbuf;
    mqtt_rxlen = sizeof(_mqtt_rxbuf);
    mqtt_txbuf = _mqtt_txbuf;
    mqtt_txlen = sizeof(_mqtt_txbuf);
    memset(mqtt_rxbuf,0,mqtt_rxlen);
    memset(mqtt_txbuf,0,mqtt_txlen);
}

/*
 * @brief: connect to the server
 * retval: 0: success 1: fail
 */
quint8 MQTT_WorkClass::MQTT_Connect(char *ClientID,char *Username,char *Password)
{
    quint8 i,j;
    int ClientIDLen = strlen(ClientID);
    int UsernameLen = strlen(Username);
    int PasswordLen = strlen(Password);
    int DataLen;
    mqtt_txlen=0;
    //可变报头+Payload  每个字段包含两个字节的长度标识
    DataLen = 10 + (ClientIDLen+2) + (UsernameLen+2) + (PasswordLen+2);

    //固定报头
    //控制报文类型
    mqtt_txbuf[mqtt_txlen++] = 0x10;		//MQTT Message Type CONNECT
    //剩余长度(不包括固定头部)
    do
    {
        quint8 encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );

    //可变报头
    //协议名
    mqtt_txbuf[mqtt_txlen++] = 0;        	// Protocol Name Length MSB
    mqtt_txbuf[mqtt_txlen++] = 4;           // Protocol Name Length LSB
    mqtt_txbuf[mqtt_txlen++] = 'M';        	// ASCII Code for M
    mqtt_txbuf[mqtt_txlen++] = 'Q';        	// ASCII Code for Q
    mqtt_txbuf[mqtt_txlen++] = 'T';        	// ASCII Code for T
    mqtt_txbuf[mqtt_txlen++] = 'T';        	// ASCII Code for T
    //协议级别
    mqtt_txbuf[mqtt_txlen++] = 4;        		// MQTT Protocol version = 4   对于 3.1.1 版协议，协议级别字段的值是 4(0x04)
    //连接标志
    mqtt_txbuf[mqtt_txlen++] = 0xc2;        	// conn flags
    mqtt_txbuf[mqtt_txlen++] = 0;        		// Keep-alive Time Length MSB
    mqtt_txbuf[mqtt_txlen++] = 100;        	// Keep-alive Time Length LSB  100S心跳包    保活时间

    mqtt_txbuf[mqtt_txlen++] = BYTE1(ClientIDLen);// Client ID length MSB
    mqtt_txbuf[mqtt_txlen++] = BYTE0(ClientIDLen);// Client ID length LSB
    memcpy(&mqtt_txbuf[mqtt_txlen],ClientID,ClientIDLen);
    mqtt_txlen += ClientIDLen;

    if(UsernameLen > 0)
    {
        mqtt_txbuf[mqtt_txlen++] = BYTE1(UsernameLen);		//username length MSB
        mqtt_txbuf[mqtt_txlen++] = BYTE0(UsernameLen);    	//username length LSB
        memcpy(&mqtt_txbuf[mqtt_txlen],Username,UsernameLen);
        mqtt_txlen += UsernameLen;
    }

    if(PasswordLen > 0)
    {
        mqtt_txbuf[mqtt_txlen++] = BYTE1(PasswordLen);		//password length MSB
        mqtt_txbuf[mqtt_txlen++] = BYTE0(PasswordLen);    	//password length LSB
        memcpy(&mqtt_txbuf[mqtt_txlen],Password,PasswordLen);
        mqtt_txlen += PasswordLen;
    }

    //清空数据
    memset(mqtt_rxbuf,0,mqtt_rxlen);
    ReadData.clear();

    MQTT_SendBuf(mqtt_txbuf,mqtt_txlen);
    //开始事件循环
#if DEBUG_MODE
    qDebug() << "send connect package begin";
#endif
    StartEvenLoop();

    if(ReadData.length()==0)
    {
#if DEBUG_MODE
        qDebug() << "read connect package begin";
#endif
        //开始事件循环
        StartEvenLoop();
    }
    memcpy((char *)mqtt_rxbuf,ReadData.data(),ReadData.length());

    //CONNECT
    if(mqtt_rxbuf[0]==parket_connetAck[0] && mqtt_rxbuf[1]==parket_connetAck[1]) //连接成功
    {
        return 0;//连接成功
    }
    return 1;
}

/*
 * @brief: subscribe & unsubscribe
 * paras: topic
 *        qos
 *        whether  1: subscribe 0: unsubscribe
 * retval: 0: success 1: fail
 */
quint8 MQTT_WorkClass::MQTT_SubscribeTopic(const QString& topic, quint8 qos, quint8 whether)
{
    QByteArray utf8Topic = topic.toUtf8();
    const char* cTopic = utf8Topic.constData();

    quint8 i;
    quint8 j;
    mqtt_txlen = 0;
    int topiclen = strlen(cTopic);

    int DataLen = 2 + (topiclen+2) + (whether?1:0);//可变报头的长度（2字节）加上有效载荷的长度
    //固定报头
    //控制报文类型
    if(whether)mqtt_txbuf[mqtt_txlen++] = 0x82; //消息类型和标志订阅
    else	mqtt_txbuf[mqtt_txlen++] = 0xA2;    //取消订阅

    //剩余长度
    do
    {
        quint8 encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );

    //可变报头
    mqtt_txbuf[mqtt_txlen++] = 0;			//消息标识符 MSB
    mqtt_txbuf[mqtt_txlen++] = 0x0A;        //消息标识符 LSB
    //有效载荷
    mqtt_txbuf[mqtt_txlen++] = BYTE1(topiclen);//主题长度 MSB
    mqtt_txbuf[mqtt_txlen++] = BYTE0(topiclen);//主题长度 LSB
    memcpy(&mqtt_txbuf[mqtt_txlen],cTopic,topiclen);
    mqtt_txlen += topiclen;

    if(whether)
    {
        mqtt_txbuf[mqtt_txlen++] = qos;//QoS级别
    }

    ReadData.clear();
    MQTT_SendBuf(mqtt_txbuf,mqtt_txlen);

    //开始事件循环
    StartEvenLoop();

    if(ReadData.length()==0)
    {
        //开始事件循环
        StartEvenLoop();
    }
    memcpy((char *)mqtt_rxbuf,ReadData.data(),ReadData.length());

    if(mqtt_rxbuf[0]==parket_subAck[0] && mqtt_rxbuf[1]==parket_subAck[1]) //订阅成功
    {
        return 0;//订阅成功
    }

    return 1; //失败
}

/*
 * @brief: publish function
 * paras: topic
 *        message
 *        qos
 */
quint8 MQTT_WorkClass::MQTT_PublishData(const QString& topic, const QString& message, quint8 qos)
{

    QByteArray utf8Topic = topic.toUtf8();
    const char* cTopic = utf8Topic.constData();
    QByteArray utf8Message = message.toUtf8();
    const char* cMessage = utf8Message.constData();
    int topicLength = strlen(cTopic);
    int messageLength = strlen(cMessage);
    static quint16 id=0;
    int DataLen;
    mqtt_txlen=0;
    //有效载荷的长度这样计算：用固定报头中的剩余长度字段的值减去可变报头的长度
    //QOS为0时没有标识符
    //数据长度             主题名   报文标识符   有效载荷
    if(qos)	DataLen = (2+topicLength) + 2 + messageLength;
    else	DataLen = (2+topicLength) + messageLength;

    //固定报头
    //控制报文类型
    mqtt_txbuf[mqtt_txlen++] = 0x30;    // MQTT Message Type PUBLISH

    //剩余长度
    do
    {
        quint8 encodedByte = DataLen % 128;
        DataLen = DataLen / 128;
        // if there are more data to encode, set the top bit of this byte
        if ( DataLen > 0 )
            encodedByte = encodedByte | 128;
        mqtt_txbuf[mqtt_txlen++] = encodedByte;
    }while ( DataLen > 0 );

    mqtt_txbuf[mqtt_txlen++] = BYTE1(topicLength);//主题长度MSB
    mqtt_txbuf[mqtt_txlen++] = BYTE0(topicLength);//主题长度LSB
    memcpy(&mqtt_txbuf[mqtt_txlen],cTopic,topicLength);//拷贝主题
    mqtt_txlen += topicLength;

    //报文标识符
    if(qos)
    {
        mqtt_txbuf[mqtt_txlen++] = BYTE1(id);
        mqtt_txbuf[mqtt_txlen++] = BYTE0(id);
        id++;
    }
    memcpy(&mqtt_txbuf[mqtt_txlen],cMessage,messageLength);
    mqtt_txlen += messageLength;

    ReadData.clear();
    MQTT_SendBuf(mqtt_txbuf,mqtt_txlen);

    //开始事件循环
    StartEvenLoop();
    return mqtt_txlen;
}

void MQTT_WorkClass::MQTT_SentHeart(void)
{
    MQTT_SendBuf((quint8 *)parket_heart,sizeof(parket_heart));
}

void MQTT_WorkClass::MQTT_Disconnect(void)
{
    MQTT_SendBuf((quint8 *)parket_disconnet,sizeof(parket_disconnet));
}

void MQTT_WorkClass::MQTT_SendBuf(quint8 *buf,quint16 len)
{
    if(socket_type)
    {
               qDebug()<<"len:"<<len;
               for(int i=0;i<len;i++)
               {
                   qDebug("%#x ",buf[i]);
               }
        LocalTcpClientSocket->write((const char *)buf,len);
    }
}


//客户端模式：创建客户端
void MQTT_WorkClass::ConnectMqttServer(QString ip,quint16 port)
{
    if(LocalTcpClientSocket)
    {
        LocalTcpClientSocket->close();
        delete  LocalTcpClientSocket;
        LocalTcpClientSocket=nullptr;
    }
    /*1. 创建本地客户端TCP套接字*/
    LocalTcpClientSocket = new QTcpSocket;
    /*2. 设置服务器IP地址*/
    QHostAddress FarServerAddr(ip);
    /*3. 连接客户端的信号槽*/
    connect(LocalTcpClientSocket,SIGNAL(connected()),this,SLOT(LocalTcpClientConnectedSlot()));
    connect(LocalTcpClientSocket,SIGNAL(disconnected()),this,SLOT(LocalTcpClientDisconnectedSlot()));
    connect(LocalTcpClientSocket,SIGNAL(readyRead()),this,SLOT(LocalTcpClientReadDataSlot()));
    connect(LocalTcpClientSocket,SIGNAL(bytesWritten(qint64)),this,SLOT(LocalTcpClientBytesWrittenSlot(qint64)));

    /*4. 尝试连接服务器主机*/
    LocalTcpClientSocket->connectToHost(FarServerAddr,port);
}

void MQTT_WorkClass::Set_MQTT_Addr(QString ip,quint16 port,QString MQTT_ClientID,QString MQTT_UserName,QString MQTT_PassWord)
{
    m_ip=ip;
    m_port=port;
    m_MQTT_ClientID=MQTT_ClientID;
    m_MQTT_UserName=MQTT_UserName;
    m_MQTT_PassWord=MQTT_PassWord;
}

//客户端模式：响应连接上服务器之后的操作
void MQTT_WorkClass::LocalTcpClientConnectedSlot()
{
    socket_type=1;
    //通知外部
    emit MQTT_ConnectState(socket_type);
    //结束事件循环
    EndEvenLoop();
}

//客户端模式：断开服务器
void MQTT_WorkClass::LocalTcpClientDisconnectedSlot()
{
    socket_type=0;
    //通知外部
    emit MQTT_ConnectState(socket_type);
    emit logSend(QString("Client Disconnect\n"));
}

void MQTT_WorkClass::LocalTcpClientReadDataSlot()
{
    ReadData = LocalTcpClientSocket->readAll();
    if (ReadData.length() > 2 )
    {
    emit logSend(QString("Read data sent by the server: %1\n").arg(ReadData.length()));
    qDebug() << "Read data sent by the server:" << ReadData.length();
    qDebug() << "As hex:" << ReadData.toHex(' ');
    }

    // -------------------------- 第一步：剥离MQTT协议头，定位到JSON payload --------------------------
    int dataPos = 0;
    // 1. 跳过MQTT控制字节（1字节，PUBLISH消息固定为0x30）
    if (dataPos >= ReadData.length()) {
        emit logSend("MQTT data empty\n");
        EndEvenLoop();
        return;
    }
    dataPos += 1;

    // 2. 解码MQTT剩余长度（变长编码，1-4字节）
    quint32 remainingLength = 0;
    quint8 remainingLengthBytes = 0;
    quint8 encodedByte;
    do {
        if (dataPos >= ReadData.length() || remainingLengthBytes > 4) {
            emit logSend("MQTT remaining length error\n");
            EndEvenLoop();
            return;
        }
        encodedByte = ReadData.at(dataPos++);
        remainingLength |= (encodedByte & 0x7F) << (7 * remainingLengthBytes);
        remainingLengthBytes++;
    } while ((encodedByte & 0x80) != 0);

    // 3. 跳过MQTT可变头（Topic：2字节长度 + Topic内容）
    if (dataPos + 2 > ReadData.length()) {
        //emit logSend("MQTT Topic length missing\n");
        EndEvenLoop();
        return;
    }
    quint16 topicLength = (static_cast<quint8>(ReadData.at(dataPos)) << 8) | static_cast<quint8>(ReadData.at(dataPos + 1));
    dataPos += 2 + topicLength;  // 跳过“Topic长度”和“Topic内容”

    // 4. 提取JSON payload（此时dataPos指向payload起始位置）
    if (dataPos >= ReadData.length()) {
        //emit logSend("MQTT payload missing\n");
        EndEvenLoop();
        return;
    }
    QByteArray jsonPayload = ReadData.mid(dataPos);  // 从dataPos到末尾都是JSON
    emit logSend(QString("Extracted JSON payload: %1\n").arg(QString(jsonPayload)));

    // -------------------------- 第二步：解析JSON，提取所有content消息 --------------------------
    // QJsonParseError jsonErr;
    // QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonPayload, &jsonErr);
    // if (jsonErr.error != QJsonParseError::NoError) {
    //     emit logSend(QString("JSON parse error: %1\n").arg(jsonErr.errorString()));
    //     EndEvenLoop();
    //     return;
    // }

    // // 5. 提取root对象中的content字段
    // QJsonObject rootObj = jsonDoc.object();
    // if (!rootObj.contains("content")) {  // 检查是否有content字段
    //     emit logSend("JSON has no 'content' field\n");
    //     EndEvenLoop();
    //     return;
    // }
    // QJsonValue contentVal = rootObj["content"];
    // if (!contentVal.isObject()) {  // 确保content是对象（而非数组/字符串）
    //     emit logSend("JSON 'content' is not an object\n");
    //     EndEvenLoop();
    //     return;
    // }
    // QJsonObject contentObj = contentVal.toObject();  // 转换为content对象

    // // 6. 提取content中的所有键值对（无论有多少字段，都能全部提取）
    // QString allContentMsg = "All content messages:\n";
    // for (auto iter = contentObj.begin(); iter != contentObj.end(); iter++) {
    //     QString key = iter.key();          // 字段名（如"Temp"）
    //     QString value = iter.value().toString();  // 字段值（如"111"）
    //     allContentMsg += QString("  %1: %2\n").arg(key).arg(value);
    // }

    // // 输出结果（日志+调试信息）
    // emit logSend(allContentMsg);
    // qDebug() << allContentMsg;

    EndEvenLoop();
}

//客户端模式：数据发送成功
void MQTT_WorkClass::LocalTcpClientBytesWrittenSlot(qint64 byte)
{
    if (byte > 2)
    {
    emit logSend(QString("send data len: %1\n").arg(byte));
    }
    EndEvenLoop(); //退出事件循环
}

//订阅主题
void MQTT_WorkClass::slot_SubscribeTopic(QString topic)
{
    if(MQTT_SubscribeTopic(topic,0,1))
    {
        emit logSend(QString("Topic subscribe failed! T_T.\n"));
    }
    else
    {
        emit logSend(QString("Topic subscribe successful! TvT.\n"));
    }
}

//发布消息
void MQTT_WorkClass::slot_PublishData(QString topic,QString message)
{
    MQTT_PublishData(topic, message, 1);
}

void MQTT_WorkClass::EndEvenLoop()
{
    //停止定时器
    timer->stop();
    //先退出事件循环
    loop.exit();
    qDebug()<<"退出事件循环";
}


//开始事件循环
void MQTT_WorkClass::StartEvenLoop()
{
    qDebug()<<"开始事件循环";
    timer->start(5000);
    loop.exec();
}

//断开连接
void MQTT_WorkClass::slot_tcp_close()
{
    if(socket_type)
    {
        timer->stop();
        loop.exit();
        LocalTcpClientSocket->close();
    }
}
