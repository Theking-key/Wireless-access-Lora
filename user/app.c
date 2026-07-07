#include "app.h"
#include "vMsgExec.h"
#include <string.h>
#define LOG_TAG          "APP"
#include "elog.h"
#include "bsp.h"
#include "key.h"
#include "dht11.h"
#include "main.h"
#include "cmd_queue.h"
#include "uart_interface.h"
#include "cmd_process.h"
#include"radio.h"
#include"dyn_mem.h"
#include"stdlib.h"
void app_uart_handle_effective_frame_callback(void* revobj, uint8_t *revbuff, uint16_t revlen);

void app_key_change_callback(key_action_t pressorrelase, uint8_t keyvalue);

void app_dlgcallback(VWM_MESSAGE * pMsg);

#define LORA_SYMBOL_TIMEOUT                         0         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

#define SID_MAINSCREEN      0x00
#define SID_CONFIGSCREEN    0x01

typedef struct
{
int TX_Output_Power;//��???��??
    int LORA_Preamble_Length;//ͬ���ֳ�??
    int CRCFlag;//CRC������־
    int LORA_BandWidth;//����Ƶ�ʴ��� // [0: 62.5kHz,
    //  1: 125 kHz,
    //  2: 250 kHz,
    //  3: 500 kHz,
    //  4: Reserved]
    int LORA_Spreading_Factor;//��Ƶ����, 6~12
    int LORA_CodingRate;//ǰ�þ���??
    // [1: 4/5,
    //  2: 4/6,
    //  3: 4/7,
    //  4: 4/8]
    int FreqHopOn;//LoRa��Ƶ������־
    int HopPeriod;//LoRa��Ƶ����
    int Mode;//ģʽ, 0 FSK, 1 Lora
    int RF_Frequency;//Ƶ��
    int AutoRX;//�Ƿ�??���Զ�����ģ??
}LoraParam_t;




static RadioEvents_t RadioEvents;

typedef struct Lora_network_config_t
{
    uint8_t network_id;
    uint8_t source_addr;
    
}Lora_network_config_t;

typedef struct app_t
{
   VWM_HWIN     main_hwnd;
   VWM_HTIMER   blink_timer;
   VWM_HTIMER   keyscan_timer;
   VWM_HTIMER   sensor_timer;

   volatile uint8_t  temp;
   volatile uint8_t  humi; 

   uart_interface_t lcd_uart;
   uint8            lcd_cmd_buffer[CMD_MAX_SIZE];                                                     //ָ���

   LoraParam_t WorkLoraParam;

   uint8_t     device_role;// 0�նˣ��ն˸����������� 1���أ���������
   uint8_t     sensor_collect_count;//�������ɼ�����
   Lora_network_config_t Lora_network_config;

   LoraParam_t setting_work_lora_param;
   Lora_network_config_t setting_lora_network_config;
   uint8_t     setting_device_role;
}app_t;

app_t g_app;


#define VWM_LORA_REV_DATA    (VWM_USER + 0x1002)

//ͨ��֡�ṹ
#pragma pack(1)
typedef struct lora_msg_frame_t
{
    uint8_t network_id;
    uint8_t source_addr;
    uint8_t dest_addr;
    uint8_t temp;
    uint8_t humi;

}lora_msg_frame_t;
#pragma pack()

#pragma pack(1)
typedef struct lora_rev_msg_t
{
    
     uint16_t size;
     int16_t rssi;
     int8_t snr;
     uint8_t payload[1];
}lora_rev_msg_t;
#pragma pack()
void OnTxDone( void )
{
	Radio.Sleep();
	Radio.Rx(0);
    log_i("Radio Tx done");
}


void OnTxTimeout( void )
{
	Radio.Sleep();
    
    log_i("Radio Tx timeout");
    if(g_app.device_role==1)
    {
        Radio.Rx(0);
    }
}



void OnRxTimeout( void )
{
	Radio.Sleep( );
    log_i("Radio Rx timeout");
}

void OnRxError( void )
{
	Radio.Sleep( );
    log_i("Radio Rx ERROR");
    
    
        Radio.Rx(0);
    
}

// #pragma pack(1)
// typedef struct {
// 	uint32_t sourceaddr; //Դ��ַ
// 	uint32_t destinationaddr; //Ŀ�ĵ�ַ
// 	uint8_t temp;//�¶�
// 	uint8_t humi;//ʪ��
// }temphumi_msg_t;
// #pragma pack()

// char charbuffer[128];

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	// temphumi_msg_t *pmsg;
	// uint8_t slen;
    lora_rev_msg_t *msg;
	Radio.Sleep();
    msg=(lora_rev_msg_t*) dm_alloc (sizeof(lora_rev_msg_t)-1+size);
    msg->size=size;
    msg->rssi=rssi;
    msg->snr=snr;
    memcpy(msg->payload,payload,size);
    VWM_SendMessageP(VWM_LORA_REV_DATA,g_app.main_hwnd,g_app.main_hwnd,msg,1);
	// LORA_RX = 0;
	// //��LED�Ƶ���������ָʾ���յ���LoRa����
	// HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin,GPIO_PIN_RESET);

	// //HAL_UART_Transmit(&huart1, (uint8_t*)payload, size, size*10);

	// pmsg = (temphumi_msg_t*)payload;
	
	// //1. ���ǲ���Ŀ�Ľڵ�
	// if (pmsg->destinationaddr != node_address) {
	// 	return;
	// }

	// sprintf(charbuffer, "Rev RemoteID : %d, Temp:%d, Humi:%d\r\n", pmsg->sourceaddr, pmsg->temp, pmsg->humi);

	// slen = strlen(charbuffer);

	// HAL_UART_Transmit(&huart1, (uint8_t*)charbuffer, slen, slen*10);
	
}


void app_update_default_channel(void)
{
    g_app.WorkLoraParam.FreqHopOn = 0;
    g_app.WorkLoraParam.AutoRX = 1;
    g_app.WorkLoraParam.CRCFlag = 1;
    g_app.WorkLoraParam.LORA_BandWidth = 1;//125Khz// = 1;//125Khz
    g_app.WorkLoraParam.LORA_CodingRate = 1;//4/5
    g_app.WorkLoraParam.LORA_Preamble_Length= 10;
    g_app.WorkLoraParam.LORA_Spreading_Factor = 12;
    g_app.WorkLoraParam.Mode = 1;//lora
    g_app.WorkLoraParam.RF_Frequency = 435000000;//424600000;//478000000;
    g_app.WorkLoraParam.TX_Output_Power = 18;
    g_app.WorkLoraParam.HopPeriod = 0;


    Radio.SetChannel(g_app.WorkLoraParam.RF_Frequency);
    Radio.SetMaxPayloadLength(MODEM_LORA, 255);


    Radio.SetTxConfig( MODEM_LORA, g_app.WorkLoraParam.TX_Output_Power, 0, g_app.WorkLoraParam.LORA_BandWidth,
                       g_app.WorkLoraParam.LORA_Spreading_Factor, g_app.WorkLoraParam.LORA_CodingRate,
                       g_app.WorkLoraParam.LORA_Preamble_Length, LORA_FIX_LENGTH_PAYLOAD_ON,
                       g_app.WorkLoraParam.CRCFlag, g_app.WorkLoraParam.FreqHopOn, g_app.WorkLoraParam.HopPeriod, LORA_IQ_INVERSION_ON, 6000 );
    Radio.SetRxConfig( MODEM_LORA, g_app.WorkLoraParam.LORA_BandWidth, g_app.WorkLoraParam.LORA_Spreading_Factor,
                       g_app.WorkLoraParam.LORA_CodingRate, 0, g_app.WorkLoraParam.LORA_Preamble_Length,
                       LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                       0, g_app.WorkLoraParam.CRCFlag, g_app.WorkLoraParam.FreqHopOn, g_app.WorkLoraParam.HopPeriod, LORA_IQ_INVERSION_ON, true );

}


void app_update_channel(void)
{
    Radio.Sleep();
    Radio.SetChannel(g_app.WorkLoraParam.RF_Frequency);

    Radio.SetTxConfig( MODEM_LORA, g_app.WorkLoraParam.TX_Output_Power, 0, g_app.WorkLoraParam.LORA_BandWidth,
                       g_app.WorkLoraParam.LORA_Spreading_Factor, g_app.WorkLoraParam.LORA_CodingRate,
                       g_app.WorkLoraParam.LORA_Preamble_Length, LORA_FIX_LENGTH_PAYLOAD_ON,
                       g_app.WorkLoraParam.CRCFlag, g_app.WorkLoraParam.FreqHopOn, g_app.WorkLoraParam.HopPeriod, LORA_IQ_INVERSION_ON, 6000 );
    Radio.SetRxConfig( MODEM_LORA, g_app.WorkLoraParam.LORA_BandWidth, g_app.WorkLoraParam.LORA_Spreading_Factor,
                       g_app.WorkLoraParam.LORA_CodingRate, 0, g_app.WorkLoraParam.LORA_Preamble_Length,
                       LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                       0, g_app.WorkLoraParam.CRCFlag, g_app.WorkLoraParam.FreqHopOn, g_app.WorkLoraParam.HopPeriod, LORA_IQ_INVERSION_ON, true );

}

void Lora_Config()
{
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    RadioEvents.CadDone = NULL;
    Radio.Init(&RadioEvents);
    Radio.Sleep;

    app_update_default_channel();
}

void app_init()
{
    memset(&g_app, 0, sizeof(app_t));

    

    g_app.main_hwnd = MsgExec_CreateModule(app_dlgcallback, 0);

    g_app.device_role = 0;//0�ն� 1����

    if (g_app.main_hwnd == NULL) {
        log_e("app_init MsgExec_CreateModule error!");
    } else {
        log_i("app_init MsgExec_CreateModule ok!");
    }

    key_init(app_key_change_callback);

    queue_reset();

    uart_interface__init(&g_app.lcd_uart, 
        &huart2, 
        g_app.main_hwnd, 
        app_uart_handle_effective_frame_callback, 
        0, 
        NULL,
        0,
        NULL);
    
    uart_interface__start_receive(&g_app.lcd_uart);


    g_app.Lora_network_config.network_id=0x10;
    if(g_app.device_role == 1)
    {
        g_app.Lora_network_config.source_addr=0x00;
    }
    else
    {
        g_app.Lora_network_config.source_addr=0x01;
    }


    Lora_Config();
    //��ʼ������ģ��

    if(g_app.device_role == 1)
    {
        Radio.Rx(0);
    }

    //����һ�½�����?
    if(g_app.device_role == 1)
    {
        SetTextValue(0, 6, "gateway");
    }
    else
    {
        SetTextValue(0, 6, "device");
    }

    SetTextValue(0, 4, "-");
    SetTextValue(0, 5, "-");
    SetTextValue(0, 2, "-");
    SetTextValue(0, 3, "-");
    SetScreen(SID_MAINSCREEN);
}

void app_rev_lora_msg(lora_rev_msg_t *msg)
{
    lora_msg_frame_t* revframe;
    lora_msg_frame_t sendframe;
    uint8_t nowsourceaddr;
    if(msg == NULL) return;
    log_i("REV Lora Message,size=%d,rssi=%d,snr=%d",msg->size,msg->rssi,msg->snr);
    revframe=(lora_msg_frame_t*)msg->payload;
    log_i("package field = network_id:%d,source_addr:%d,dest_addr:%d,temp:%d,humi:%d",revframe->network_id,revframe->source_addr,revframe->dest_addr,revframe->temp,revframe->humi);
    
    if(g_app.device_role == 1)
        {
            nowsourceaddr = 0;
        }
        else
        {
            nowsourceaddr = g_app.Lora_network_config.source_addr;
        }
    if(revframe->dest_addr==nowsourceaddr && revframe->network_id==g_app.Lora_network_config.network_id)
    {
        //��ʾԶ����ʪ��  
        SetTextInt32(0, 5, revframe->temp, 0, 2);
        SetTextInt32(0, 4, revframe->humi, 0, 2);

        //����Լ������أ������Լ������?��
        if(g_app.device_role == 1)  
            {
                        sendframe.network_id=g_app.Lora_network_config.network_id; 
                        sendframe.source_addr=g_app.Lora_network_config.source_addr;
                        sendframe.dest_addr=revframe->source_addr;
                        sendframe.temp=g_app.temp;
                        sendframe.humi=g_app.humi;
                        Radio.Send((uint8_t*)&sendframe, sizeof(lora_msg_frame_t));

                        // sendframe.network_id=16; 
                        // sendframe.source_addr=0;
                        // sendframe.dest_addr=1;
                        // sendframe.temp=10;
                        // sendframe.humi=g_app.humi;
                        // Radio.Send((uint8_t*)&sendframe, sizeof(lora_msg_frame_t));
                    
            }
    } 
    else
    {
        log_e("dest_addr is not me,me addr is %d,me network id:%d",nowsourceaddr,g_app.Lora_network_config.network_id);
        if(g_app.device_role==1)
        {
            Radio.Rx(0);

        }
    }

}
void app_dlgcallback(VWM_MESSAGE * pMsg)
{
    int16_t ledshowdelay;
    lora_msg_frame_t sendframe;

    switch (pMsg->MsgId)
    {
    case VWM_INIT_DIALOG:
        g_app.blink_timer = VWM_CreateTimer(pMsg->hWin, 0, 500, 1);
        if (g_app.blink_timer == NULL) {
            log_e("app_init create blink_timer error!");
        } else {
            log_i("app_init create blink_timer ok!");
            VWM_StartTimer(g_app.blink_timer);
        }

        g_app.keyscan_timer = VWM_CreateTimer(pMsg->hWin, 0, 10, 1);
        if (g_app.keyscan_timer == NULL) {
            log_e("app_init create keyscan_timer error!");
        } else {
            log_i("app_init create keyscan_timer ok!");
            VWM_StartTimer(g_app.keyscan_timer);
        }

        g_app.sensor_timer = VWM_CreateTimer(pMsg->hWin, 0, 1000, 1);
        if (g_app.sensor_timer == NULL) {
            log_e("app_init create sensor_timer error!");
        } else {
            log_i("app_init create sensor_timer ok!");
            VWM_StartTimer(g_app.sensor_timer);
        }

        break;

    case VWM_TIMER:
        if (pMsg->Data.v == g_app.blink_timer) {
            if (HAL_GPIO_ReadPin(LED_GPIO_Port, LED_Pin) == GPIO_PIN_SET) {
                if (g_app.humi >= 90) {
                    ledshowdelay = 100;
                } else if (g_app.humi <= 50) {
                    ledshowdelay = 500;
                } else {
                    ledshowdelay = (500 - (g_app.humi - 50) *(500-100)/(90-50));
                    if (ledshowdelay < 0) {
                        ledshowdelay = 100;
                    }
                }
                VWM_RestartTimer(g_app.blink_timer, ledshowdelay);
                log_i("LED Timer:%d", ledshowdelay);
            }
            led_toggole();
        } else if (pMsg->Data.v == g_app.keyscan_timer) {
            key_scan();
        } else if (pMsg->Data.v == g_app.sensor_timer) {
            if (DHT11_Read_Data((uint8_t*)&g_app.temp, (uint8_t*)&g_app.humi) != 0) {
                log_e("DHT11_Read_Data error!");
            } else{
                log_i("Temp:%d; Humi:%d", g_app.temp, g_app.humi);
                SetTextInt32(0, 2, g_app.temp, 0, 2);
                SetTextInt32(0, 3, g_app.humi, 0, 2);
                g_app.sensor_collect_count++;
                //����豸ʱ�նˣ��������?�ѱ��ص���ʪ�ȷ��͸�����
                if(g_app.device_role == 0)
                {
                    //������Ϣ������
                    if(g_app.sensor_collect_count %2 == 0)
                    {
                        sendframe.network_id=g_app.Lora_network_config.network_id; 
                        sendframe.source_addr=g_app.Lora_network_config.source_addr;
                        sendframe.dest_addr=0;
                        sendframe.temp=g_app.temp;
                        sendframe.humi=g_app.humi;
                        Radio.Send((uint8_t*)&sendframe, sizeof(lora_msg_frame_t));

                        // sendframe.network_id=16; 
                        // sendframe.source_addr=1;
                        // sendframe.dest_addr=0;
                        // sendframe.temp=30;
                        // sendframe.humi=50;
                        // Radio.Send((uint8_t*)&sendframe, sizeof(lora_msg_frame_t));
                    }
                }
            }
        }
        break;

    case VWM_UART_REV_DATA:
		
        uart_interface__handle_data_from_uart((uart_revmsg_t*)pMsg->Data.p);
	
		break;
    case VWM_LORA_REV_DATA:
        app_rev_lora_msg(pMsg->Data.p);
        break;
    
    default:
        break;
    }
}


void test_lcd()
{
  uint8_t sendbuff[] = {0xEE, 0xB1, 0x10, 0x00, 0x00, 0x00, 0x02, 0x32, 0x38, 0xFF, 0xFC, 0xFF, 0xFF};
  HAL_UART_Transmit(&huart2,(uint8_t *)sendbuff, sizeof(sendbuff), sizeof(sendbuff)*10 +100);
}

void app_key_change_callback(key_action_t pressorrelase, uint8_t keyvalue)
{
    if (pressorrelase == KEY_RELEASED) {
        log_i("app_key_release callback %d", keyvalue);
        if (keyvalue == KEY_1) {
            // test_lcd();
            // log_i("Temp:%d; Humi:%d", g_app.temp, g_app.humi);
            
            //  SetTextInt32(0, 2, g_app.temp, 0, 2);
            //SetTextValue(0, 2, "28");
        }
    }
}

void gui_frefresh_manager_dlg()
{
    if(g_app.WorkLoraParam.LORA_BandWidth == 1)
        {
            SetTextValue(SID_CONFIGSCREEN,6,"125KHz");
        }
        else if(g_app.WorkLoraParam.LORA_BandWidth == 2)
        {
            SetTextValue(SID_CONFIGSCREEN,6,"250KHz");
        }
        else if(g_app.WorkLoraParam.LORA_BandWidth == 3)
        {
            SetTextValue(SID_CONFIGSCREEN,6,"500KHz");
        }

        SetTextInt32(SID_CONFIGSCREEN,1,g_app.WorkLoraParam.LORA_Preamble_Length,0,2);
        SetTextInt32(SID_CONFIGSCREEN,2,g_app.WorkLoraParam.LORA_Spreading_Factor,0,2);
        SetTextInt32(SID_CONFIGSCREEN,3,g_app.WorkLoraParam.RF_Frequency,0,9);
        SetTextInt32(SID_CONFIGSCREEN,4,g_app.WorkLoraParam.TX_Output_Power,0,2);
        SetTextInt32(SID_CONFIGSCREEN,13,g_app.Lora_network_config.network_id,0,3);
        SetTextInt32(SID_CONFIGSCREEN,12,g_app.Lora_network_config.source_addr,0,3);

        if(g_app.device_role == 0)
        {
            SetButtonValue(SID_CONFIGSCREEN,8,0);
            SetButtonValue(SID_CONFIGSCREEN,9,1);
        }
        else
        {
            SetButtonValue(SID_CONFIGSCREEN,8,1);
            SetButtonValue(SID_CONFIGSCREEN,9,0);
        }
}


void NotifyButton(uint16 screen_id, uint16 control_id, uint8  state)
{
    if (state == 1) {
        log_i("Button Pressed, ScreenID:%d; ControlID:%d", screen_id, control_id);
    } else {
        log_i("Button Released, ScreenID:%d; ControlID:%d", screen_id, control_id);
    }
    if(screen_id == SID_MAINSCREEN)
    {
        if(control_id == 0x01)
        {
            gui_frefresh_manager_dlg();
            g_app.setting_work_lora_param = g_app.WorkLoraParam;
            g_app.setting_lora_network_config = g_app.Lora_network_config;
            g_app.setting_device_role = g_app.device_role;
            SetScreen(SID_CONFIGSCREEN);
        }
    }
    else if(screen_id == SID_CONFIGSCREEN)
    {
        if(state == 1)
        {
            if(control_id == 8)
            {
            
                    SetButtonValue(SID_CONFIGSCREEN,9,0);

                    g_app.setting_device_role = 1;
            }
            

            else if(control_id == 9)
            {
                
                SetButtonValue(SID_CONFIGSCREEN,8,0);

                g_app.setting_device_role = 0;

            }
            else if(control_id == 10)
            {
                g_app.WorkLoraParam = g_app.setting_work_lora_param ;
                g_app.Lora_network_config= g_app.setting_lora_network_config;
                g_app.device_role =  g_app.setting_device_role;

                  if(g_app.device_role == 1)
                {
                    SetTextValue(0, 6, "gateway");
                }
                 else
                {
                    SetTextValue(0, 6, "device");
                }
                app_update_channel();
                if(g_app.device_role == 1)
                {
                    Radio.Rx(0);

                }
                SetScreen(SID_MAINSCREEN);
            }
        }
        else
        {
            if(control_id == 8)
            {
                if(g_app.device_role == 1)
                {   
                    SetButtonValue(SID_CONFIGSCREEN,8,1);
                }
            }
            else if(control_id == 9)
            {
                if(g_app.device_role == 0)
                {   
                    SetButtonValue(SID_CONFIGSCREEN,9,1);
                }
            }
        }
    }
}

void NotifyScreen(uint16 screen_id)
{
    if(screen_id == SID_CONFIGSCREEN)
    {
        
    }
}

void NotifyText(uint16 screen_id, uint16 control_id, uint8 *str)
{
    if(screen_id == SID_CONFIGSCREEN)
    {
        if(control_id == 0x01)
        {
            if(strlen(str)<1)
            {
                SetTextInt32(SID_CONFIGSCREEN,1,g_app.setting_work_lora_param.LORA_Preamble_Length,0,2);
            }
            else
            {
                g_app.setting_work_lora_param.LORA_Preamble_Length = atoi(str);
            }
        }
        else if(control_id == 0x02)
        {
            if(strlen(str)<1)
            {
                SetTextInt32(SID_CONFIGSCREEN,2,g_app.setting_work_lora_param.LORA_Spreading_Factor,0,2);
            }
            else
            {
                g_app.setting_work_lora_param.LORA_Spreading_Factor = atoi(str);
            }
        }
        else if(control_id == 0x03)
        {
            if(strlen(str)<1)
            {
                SetTextInt32(SID_CONFIGSCREEN,3,g_app.setting_work_lora_param.RF_Frequency,0,2);
            }
            else
            {
                g_app.setting_work_lora_param.RF_Frequency = atoi(str);
            }
        }
         else if(control_id == 0x04)
        {
            if(strlen(str)<1)
            {
                SetTextInt32(SID_CONFIGSCREEN,4,g_app.setting_work_lora_param.TX_Output_Power,0,2);
            }
            else
            {
                g_app.setting_work_lora_param.TX_Output_Power = atoi(str);
            }
        }
        else if(control_id == 13)
        {
            if(strlen(str)<1)
            {
                SetTextInt32(SID_CONFIGSCREEN,13,g_app.setting_lora_network_config.network_id,0,3);
            }
            else
            {
                g_app.setting_lora_network_config.network_id = atoi(str);
            }
        }
        else if(control_id == 12)
        {
            if(strlen(str)<1)
            {
                SetTextInt32(SID_CONFIGSCREEN,12,g_app.setting_lora_network_config.source_addr,0,3);
            }
            else
            {
                g_app.setting_lora_network_config.source_addr = atoi(str);
            }
          
        }

    }

}

void NotifyMenu(uint16 screen_id, uint16 control_id, uint8  item, uint8  state)
{
    if(screen_id == SID_CONFIGSCREEN)
    {
        if(state == 0)
        {    
            if(control_id == 5)
            {
                g_app.setting_work_lora_param.LORA_BandWidth = item+1;
            }
        }
    }
            
}

/*! 
*  \brief  ����֪ͨ
*/
void NOTIFYHandShake()
{
   SetButtonValue(3,2,1);
}

/*! 
*  \brief  ��Ϣ��������
*  \param msg ��������Ϣ
*  \param size ��Ϣ����
*/
void ProcessMessage( PCTRL_MSG msg, uint16 size )
{
    uint8 cmd_type = msg->cmd_type;                                                  //ָ������
    uint8 ctrl_msg = msg->ctrl_msg;                                                  //��Ϣ������
    uint8 control_type = msg->control_type;                                          //�ؼ�����
    uint16 screen_id = PTR2U16(&msg->screen_id);                                     //����ID
    uint16 control_id = PTR2U16(&msg->control_id);                                   //�ؼ�ID
    uint32 value = PTR2U32(msg->param);                                              //��ֵ


    switch(cmd_type)
    {  
    case NOTIFY_TOUCH_PRESS:                                                        //����������
    case NOTIFY_TOUCH_RELEASE:                                                      //�������ɿ�
        //NotifyTouchXY(cmd_buffer[1],PTR2U16(cmd_buffer+2),PTR2U16(cmd_buffer+4)); 
        break;                                                                    
    case NOTIFY_WRITE_FLASH_OK:                                                     //дFLASH�ɹ�
        //NotifyWriteFlash(1);                                                      
        break;                                                                    
    case NOTIFY_WRITE_FLASH_FAILD:                                                  //дFLASHʧ��
        //NotifyWriteFlash(0);                                                      
        break;                                                                    
    case NOTIFY_READ_FLASH_OK:                                                      //��ȡFLASH�ɹ�
        //NotifyReadFlash(1,cmd_buffer+2,size-6);                                     //ȥ��֡ͷ֡β
        break;                                                                    
    case NOTIFY_READ_FLASH_FAILD:                                                   //��ȡFLASHʧ��
        //NotifyReadFlash(0,0,0);                                                   
        break;                                                                    
    case NOTIFY_READ_RTC:                                                           //��ȡRTCʱ��
        //NotifyReadRTC(cmd_buffer[2],cmd_buffer[3],cmd_buffer[4],cmd_buffer[5],cmd_buffer[6],cmd_buffer[7],cmd_buffer[8]);
        break;
    case NOTIFY_CONTROL:
        {
            if(ctrl_msg==MSG_GET_CURRENT_SCREEN)                                    //����ID�仯֪ͨ
            {
                NotifyScreen(screen_id);                                            //�����л������ĺ���
            }
            else
            {
                switch(control_type)
                {
                case kCtrlButton:                                                   //��ť�ؼ�
                    NotifyButton(screen_id,control_id,msg->param[1]);                  
                    break;                                                             
                case kCtrlText:                                                     //�ı��ؼ�
                    NotifyText(screen_id,control_id,msg->param);                       
                    break;                                                             
                case kCtrlProgress:                                                 //�������ؼ�
                    //NotifyProgress(screen_id,control_id,value);                        
                    break;                                                             
                case kCtrlSlider:                                                   //�������ؼ�
                    //NotifySlider(screen_id,control_id,value);                          
                    break;                                                             
                case kCtrlMeter:                                                    //�Ǳ��ؼ�
                    //NotifyMeter(screen_id,control_id,value);                           
                    break;                                                             
                case kCtrlMenu:                                                     //�˵��ؼ�
                    NotifyMenu(screen_id,control_id,msg->param[0],msg->param[1]);      
                    break;                                                              
                case kCtrlSelector:                                                 //ѡ��ؼ�?
                    //NotifySelector(screen_id,control_id,msg->param[0]);                
                    break;                                                              
                case kCtrlRTC:                                                      //����ʱ�ؼ�
                    //NotifyTimer(screen_id,control_id);
                    break;
                default:
                    break;
                }
            } 
            break;  
        } 
    case NOTIFY_HandShake:                                                          //����֪ͨ                                                     
        NOTIFYHandShake();
        break;
    default:
        break;
    }
}

void app_uart_handle_effective_frame_callback(void* revobj, uint8_t *revbuff, uint16_t revlen)
{
    uint16_t i;
    qsize  size = 0;       
    for (i = 0; i < revlen; i++) {
        queue_push(revbuff[i]);


        size = queue_find_cmd(g_app.lcd_cmd_buffer, CMD_MAX_SIZE);                              //�ӻ������л�ȡһ��ָ��         

        if(size>0 && g_app.lcd_cmd_buffer[1] != 0x07)              //���յ�ָ�� �����ж��Ƿ�Ϊ������ʾ
        {                                                                           
            ProcessMessage((PCTRL_MSG)g_app.lcd_cmd_buffer, size); //ָ���  
        }                                                                           
        else if(size>0 && g_app.lcd_cmd_buffer[1]==0x07)    //����?ָ��0x07��������STM32  
        {                                                                           
            __disable_fault_irq();                                                   
            NVIC_SystemReset();                                                                                                                                          
        }                                                                            

    }
   
}


void HAL_UART_IDLECallBack(UART_HandleTypeDef *huart)
{
	if (huart == &huart2) {
		uart_interface__rev_data(&g_app.lcd_uart, false);
	}
}


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart == &huart2) {
		uart_interface__rev_data(&g_app.lcd_uart, true);
	}
}
