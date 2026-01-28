/**
	************************************************************
	************************************************************
	************************************************************
	*	文件名： 	onenet.c
	*
	*	作者： 		张继瑞
	*
	*	日期： 		2017-05-08
	*
	*	版本： 		V1.1
	*
	*	说明： 		与onenet平台的数据交互接口层
	*
	*	修改记录：	V1.0：协议封装、返回判断都在同一个文件，并且不同协议接口不同。
	*				V1.1：提供统一接口供应用层使用，根据不同协议文件来封装协议相关的内容。
	************************************************************
	************************************************************
	************************************************************
**/
#include "onenet.h"

/* 产品 ID */
#define PROID	"product-id"

// 鉴权 Token
#define TOKEN	"version=2018-10-31&res=products%2FyHtMN9icjM%2Fdevices%2Ftest&et=1782126077&method=md5&sign=pj16A1L5hxgFaeONZtSFSA%3D%3D"

// 设备名称
#define DEVID	"device-name"

/*发布主题  product-id是产品ID，device-name是设备名称*/
const char devPubTopic[] = "$sys/product-id/device-name/thing/property/post";
/*订阅主题  product-id是产品ID，device-name是设备名称*/
const char *devSubTopic[] = {"$sys/product-id/device-name/thing/property/set"};
/*==============================================================
 *  函数名称：	UsartPrintf
 *  函数功能：	格式化打印到调试串口
 *  输入参数：	fmt  格式串
 *  返回参数：	无
 *  说明：		仅内部使用
 *============================================================*/
static void UsartPrintf(const char *fmt, ...)
{
	static char buf[256];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	uart_write_string(PRINT_UART, buf);
}

/*==============================================================
 *  函数名称：	OneNet_DevLink
 *  函数功能：	与 OneNet 建立 MQTT 连接
 *  输入参数：	无
 *  返回参数：	0-成功  1-失败
 *  说明：		调用 MQTT_PacketConnect 组包，ESP8266 发送
 *============================================================*/
_Bool OneNet_DevLink(void)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};	// 协议包
	unsigned char *dataPtr;
	_Bool status = 1;

	UsartPrintf("OneNet_DevLink\r\nPROID: %s, DEVID: %s\r\n", PROID, DEVID);

	if (MQTT_PacketConnect(PROID, TOKEN, DEVID, 256, 1, MQTT_QOS_LEVEL0,
	                       NULL, NULL, 0, &mqttPacket) == 0)
	{
		esp8266_senddata(mqttPacket._data, mqttPacket._len);	// 上传平台
		dataPtr = esp8266_getipd(250);							// 等待平台响应

		if (dataPtr != NULL)
		{
			if (MQTT_UnPacketRecv(dataPtr) == MQTT_PKT_CONNACK)
			{
				switch (MQTT_UnPacketConnectAck(dataPtr))
				{
				case 0: UsartPrintf("Tips:	连接成功\r\n"); status = 0; break;
				case 1: UsartPrintf("WARN:	连接失败，协议版本\r\n"); break;
				case 2: UsartPrintf("WARN:	连接失败，非法 clientid\r\n"); break;
				case 3: UsartPrintf("WARN:	连接失败，服务不可用\r\n"); break;
				case 4: UsartPrintf("WARN:	连接失败，用户名/密码错误\r\n"); break;
				case 5: UsartPrintf("WARN:	连接失败，鉴权失败\r\n"); break;
				default:UsartPrintf("ERR:	连接失败，未知原因\r\n"); break;
				}
			}
		}
		MQTT_DeleteBuffer(&mqttPacket);		// 释放
	}
	else
	{
		UsartPrintf("WARN:	MQTT_PacketConnect Failed\r\n");
	}

	return status;
}

/*==============================================================
 *  函数名称：	OneNet_Subscribe
 *  函数功能：	订阅主题
 *  输入参数：	topics-主题数组  topic_cnt-主题个数
 *  返回参数：	无
 *  说明：		
 *============================================================*/
void OneNet_Subscribe(const char *topics[], unsigned char topic_cnt)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
	for (unsigned char i = 0; i < topic_cnt; i++)
		UsartPrintf("Subscribe Topic: %s\r\n", topics[i]);

	if (MQTT_PacketSubscribe(MQTT_SUBSCRIBE_ID, MQTT_QOS_LEVEL0,
	                         topics, topic_cnt, &mqttPacket) == 0)
	{
		esp8266_senddata(mqttPacket._data, mqttPacket._len);	// 发送订阅包
		MQTT_DeleteBuffer(&mqttPacket);						// 释放
	}
}

/*==============================================================
 *  函数名称：	OneNet_Publish
 *  函数功能：	发布消息
 *  输入参数：	topic-主题  msg-消息内容
 *  返回参数：	无
 *  说明：		
 *============================================================*/
void OneNet_Publish(const char *topic, const char *msg)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};
	UsartPrintf("Publish Topic: %s, Msg: %s\r\n", topic, msg);

	if (MQTT_PacketPublish(MQTT_PUBLISH_ID, topic, msg, strlen(msg),
	                       MQTT_QOS_LEVEL0, 0, 1, &mqttPacket) == 0)
	{
		esp8266_senddata(mqttPacket._data, mqttPacket._len);	// 发送发布包
		MQTT_DeleteBuffer(&mqttPacket);							// 释放
	}
}

/*==============================================================
 *  函数名称：	OneNet_RevPro
 *  函数功能：	平台下行数据统一处理
 *  输入参数：	cmd-平台下行数据指针
 *  返回参数：	无
 *  说明：		解析 MQTT 报文并做对应处理
 *============================================================*/
void OneNet_RevPro(unsigned char *cmd)
{
	MQTT_PACKET_STRUCTURE mqttPacket = {NULL, 0, 0, 0};

	char *req_payload = NULL;
	char *cmdid_topic = NULL;

	uint16 topic_len = 0;		/* 原 unsigned short → uint16 消除警告 */
	uint16 req_len   = 0;		/* 原 unsigned short → uint16 消除警告 */

	unsigned char type = 0;
	unsigned char qos  = 0;
	static uint16 pkt_id = 0;	/* 原 unsigned short → uint16 消除警告 */

	short result = 0;
	cJSON *json, *params_json, *led_json,*Alarm_json, *Temp_json;//json数据
	
	type = MQTT_UnPacketRecv(cmd);
	switch (type)
	{
	case MQTT_PKT_CMD:										// 命令下发
		result = MQTT_UnPacketCmd(cmd, &cmdid_topic, &req_payload, &req_len);
		if (result == 0)
		{
			UsartPrintf("cmdid: %s, req: %s, req_len: %d\r\n", cmdid_topic, req_payload, req_len);
			if (MQTT_PacketCmdResp(cmdid_topic, req_payload, &mqttPacket) == 0)
			{
				UsartPrintf("Tips:	Send CmdResp\r\n");
				esp8266_senddata(mqttPacket._data, mqttPacket._len);	// 回复命令
				MQTT_DeleteBuffer(&mqttPacket);							// 释放
			}
		}
		break;

		case MQTT_PKT_PUBLISH:														//接收的Publish消息
		
			result = MQTT_UnPacketPublish(cmd, &cmdid_topic, &topic_len, &req_payload, &req_len, &qos, &pkt_id);
			if(result == 0)
			{
				UsartPrintf("topic: %s, topic_len: %d, payload: %s, payload_len: %d\r\n",
																	cmdid_topic, topic_len, req_payload, req_len);
				
				// 对数据包req_payload进行JSON格式解析
				json = cJSON_Parse(req_payload);
				params_json = cJSON_GetObjectItem(json,"params");//找到params后把params的数据赋值给params_json
				led_json = cJSON_GetObjectItem(params_json,"LED");//找到LED后把LED的数据赋值给led_json
				Alarm_json = cJSON_GetObjectItem(params_json,"Alarm");//找到Alarm后把Alarm的数据赋值给Alarm_json
				Temp_json = cJSON_GetObjectItem(params_json,"Temp");
				if(Temp_json != NULL)
				{
					//Temp_value = Temp_json->valueint;			
				}
				if(led_json != NULL)//LED控制
				{
					if(led_json->type == cJSON_True) //如果是开灯
					{
							//开灯
					}
					else 
					{
							//关灯
					}
				}	
				if(Alarm_json != NULL)//蜂鸣器控制
				{
					if(Alarm_json->type == cJSON_True) 
					{
						//	Alarm_flag = 1;
							UsartPrintf("Alarm_flag = 1\r\n");									
					}
					else 
					{
							//Alarm_flag = 0;
							UsartPrintf("Alarm_flag = 0\r\n");					
					}
				}	
				cJSON_Delete(json);
			}
		break;

	case MQTT_PKT_PUBACK:									// Publish ACK
		if (MQTT_UnPacketPublishAck(cmd) == 0)
			UsartPrintf("Tips:	MQTT Publish ACK\r\n");
		break;

	case MQTT_PKT_PUBREC:									// Publish REC
		if (MQTT_UnPacketPublishRec(cmd) == 0)
		{
			UsartPrintf("Tips:	Rev PublishRec\r\n");
			if (MQTT_PacketPublishRel(MQTT_PUBLISH_ID, &mqttPacket) == 0)
			{
				UsartPrintf("Tips:	Send PublishRel\r\n");
				esp8266_senddata(mqttPacket._data, mqttPacket._len);
				MQTT_DeleteBuffer(&mqttPacket);
			}
		}
		break;

	case MQTT_PKT_PUBREL:									// Publish REL
		if (MQTT_UnPacketPublishRel(cmd, pkt_id) == 0)
		{
			UsartPrintf("Tips:	Rev PublishRel\r\n");
			if (MQTT_PacketPublishComp(MQTT_PUBLISH_ID, &mqttPacket) == 0)
			{
				UsartPrintf("Tips:	Send PublishComp\r\n");
				esp8266_senddata(mqttPacket._data, mqttPacket._len);
				MQTT_DeleteBuffer(&mqttPacket);
			}
		}
		break;

	case MQTT_PKT_PUBCOMP:									// Publish COMP
		if (MQTT_UnPacketPublishComp(cmd) == 0)
			UsartPrintf("Tips:	Rev PublishComp\r\n");
		break;

	case MQTT_PKT_SUBACK:									// Subscribe ACK
		if (MQTT_UnPacketSubscribe(cmd) == 0)
			UsartPrintf("Tips:	MQTT Subscribe OK\r\n");
		else
			UsartPrintf("Tips:	MQTT Subscribe Err\r\n");
		break;

	case MQTT_PKT_UNSUBACK:									// UnSubscribe ACK
		if (MQTT_UnPacketUnSubscribe(cmd) == 0)
			UsartPrintf("Tips:	MQTT UnSubscribe OK\r\n");
		else
			UsartPrintf("Tips:	MQTT UnSubscribe Err\r\n");
		break;

	default:
		result = -1;
		break;
	}

	esp8266_clear();	/* 清空 8266 接收缓冲区 */

	if (result == -1)
		return;

	/* 释放 MQTT 解析过程申请的堆内存 */
	if (type == MQTT_PKT_CMD || type == MQTT_PKT_PUBLISH)
	{
		if (cmdid_topic) MQTT_FreeBuffer(cmdid_topic);
		if (req_payload) MQTT_FreeBuffer(req_payload);
	}
}

