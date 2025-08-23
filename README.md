# MQTT UI 

由于本人在学习嵌入式linux时，对mqtt协议了解一知半解，看了一些网上的教程还是只能停留在粗浅的表层理解，故想要自己动手实现简易的mqtt协议报文来加深理解。
一个基于 **Qt Quick** 的简易 MQTT 客户端，支持订阅、发布消息，适合学习和实际使用。 鄙人用来模拟真实的开发板与服务器通信，已在华为云IoT服务器测试!TvT!
	
---

## ✨ 功能特性

- ✅ 订阅和发布 MQTT 消息  
- ✅ 消息列表自动滚动  
- ✅ JSON 格式化展示消息   
- ✅ 自动向服务器发送心跳包   

---

## 📸 界面截图

> 
>   ![[Screenshot 2025-08-22 202740.png]]
> 

---

##  快速开始

### 1. 下载项目

下载release中压缩包，解压运行MQTT_UI.exe
### 2. 克隆项目
```bash
git clone https://github.com/W1nt3rDay/MQTT_Client-UI.git
cd mqtt_client
md build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
./MQTT_UI.exe
```

---

## Usage

### 1. 华为云Broker

1. 在华为云IoT平台创建一个免费的实例
![[Pasted image 20250823092144.png]]

2. 在该实例下创建产品![[Pasted image 20250823092310.png]]
3. 在该产品下定义模型![[Pasted image 20250823092726.png]]
4. 在该模型下注册设备：注意保存device_id和密钥![[Pasted image 20250823092501.png]]
5. device_id和secret![[Pasted image 20250823093157.png]]
6. 利用上面两个信息，在[Huaweicloud IoTDA Mqtt ClientId Generator](https://iot-tool.obs-website.cn-north-4.myhuaweicloud.com/)里生成三元组信息用来连接华为云服务器![[Pasted image 20250823093318.png]]


### 2. 客户端连接Client Connect

![[Pasted image 20250823093507.png]]

1. Server IP: 在创建的实例中可以找到![[Pasted image 20250823093658.png]]
   复制这个接入地址，打开cmd，执行： ping 4b79cd58d1.st1.iotda-device.cn-east-3.myhuaweicloud.com  这个服务器地址填你复制的接入地址即可,最后能得到ip地址![[Pasted image 20250823093850.png]]
2. Port 固定1883
3. ClientID、Username、Password三元组填你之前生成的即可
   ![[Pasted image 20250823094142.png]]连接成功
   ![[Pasted image 20250823094227.png]]可以看到华为云服务器上设备状态显示在线

### 3. 订阅和发布主题

1. 华为云服务器发布消息，客户端接收
   官方帮助文档地址：[平台消息下发_设备消息_设备侧MQTT/MQTTS接口参考_API参考_设备接入 IoTDA-华为云](https://support.huaweicloud.com/api-iothub/iot_06_v5_3017.html)
   ```
   Topic: $oc/devices/{device_id}/sys/messages/down
   {
	    "object_device_id": "{object_device_id}",
	    "name": "name",
	    "id": "id",
	    "content": "hello"
   }
   ```

要想接收到服务器下发的消息，在客户端只需订阅Topic即可![[Pasted image 20250823094918.png]]

我尝试在服务器下发消息![[Pasted image 20250823095056.png]]
可以看到客户端成功接收到了来自服务器的消息内容![[Pasted image 20250823095151.png]]

我在产品模型中创建了两个属性，Temp和Humi
现在我让客户端上传数据，模拟一个嵌入式设备中的温湿度传感器上报数据给服务器。
帮助文档：[设备属性上报_设备属性_设备侧MQTT/MQTTS接口参考_API参考_设备接入 IoTDA-华为云](https://support.huaweicloud.com/api-iothub/iot_06_v5_3010.html)
```
Topic: $oc/devices/{device_id}/sys/properties/report  
数据格式：  
{
    "services": [{
            "service_id": "imx6ull",
            "properties": {
                "Temp": 30,
                "Humi": 80
            }
        }]
}
```

上报的Topic中记得替换你的device_id与service_id

![[Pasted image 20250823095956.png]]

点击发送，可以看到服务器中的Temp和Humi成功更新![[Pasted image 20250823100122.png]]

到此就完成了服务器下发消息与客户端上传消息的实验操作


## 致谢

感谢Kious老师的精心拍摄图片， 感zzh老师的测试
感谢使用的第三方库、灵感来源 !TvT!
