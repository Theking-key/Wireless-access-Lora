/************************************��Ȩ����********************************************
**                             ���ݴ�ʹ��Ƽ����޹�˾
**                             http://www.gz-dc.com
**-----------------------------------�ļ���Ϣ--------------------------------------------
** �ļ�����:   hmi_driver.c
** �޸�ʱ��:   2018-05-18
** �ļ�˵��:   �û�MCU��������������
** ����֧�֣�  Tel: 020-82186683  Email: hmi@gz-dc.com Web:www.gz-dc.com
--------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------*/
#include "hmi_driver.h"

#define TX_8(P1) SEND_DATA((P1)&0xFF)                    //���͵����ֽ�
#define TX_8N(P,N) SendNU8((uint8 *)P,N)                 //����N���ֽ�
#define TX_16(P1) TX_8((P1)>>8);TX_8(P1)                 //����16λ����
#define TX_16N(P,N) SendNU16((uint16 *)P,N)              //����N��16λ����
#define TX_32(P1) TX_16((P1)>>16);TX_16((P1)&0xFFFF)     //����32λ����

#if(CRC16_ENABLE)

static uint16 _crc16 = 0xffff;
/*!
*  \brief ����CRC16У��
*  \param buffer ��У�������
*  \param n ���ݳ��ȣ�����CRC16
*  \param pcrc У����
*/
static void AddCRC16(uint8 *buffer,uint16 n,uint16 *pcrc)
{
    uint16 i,j,carry_flag,a;

    for (i=0; i<n; i++)
    {
        *pcrc=*pcrc^buffer[i];
        for (j=0; j<8; j++)
        {
            a=*pcrc;
            carry_flag=a&0x0001;
            *pcrc=*pcrc>>1;
            if (carry_flag==1)
                *pcrc=*pcrc^0xa001;
        }
    }
}
/*!
*  \brief  ��������Ƿ����CRC16У��
*  \param buffer ��У������ݣ�ĩβ�洢CRC16
*  \param n ���ݳ��ȣ�����CRC16
*  \return У��ͨ������1�����򷵻�0
*/
uint16 CheckCRC16(uint8 *buffer,uint16 n)
{
    uint16 crc0 = 0x0;
    uint16 crc1 = 0xffff;

    if(n>=2)
    {
        crc0 = ((buffer[n-2]<<8)|buffer[n-1]);
        AddCRC16(buffer,n-2,&crc1);
    }

    return (crc0==crc1);
}
/*!
*  \brief  ����һ���ֽ�
*  \param  c
*/
void SEND_DATA(uint8 c)
{
    AddCRC16(&c,1,&_crc16);
    SendChar(c);
}
/*!
*  \brief  ֡ͷ
*/
void BEGIN_CMD()
{
    TX_8(0XEE);
    _crc16 = 0XFFFF;                      //��ʼ����CRC16
}
/*!
*  \brief  ֡β
*/
void END_CMD()
{
    uint16 crc16 = _crc16;
    TX_16(crc16);                         //����CRC16
    TX_32(0XFFFCFFFF);
}

#else//NO CRC16

#define SEND_DATA(P) SendChar(P)          //����һ���ֽ�
#define BEGIN_CMD() TX_8(0XEE)            //֡ͷ
#define END_CMD() TX_32(0XFFFCFFFF)       //֡β

#endif
/*!
*  \brief  ��ʱ
*  \param  n ��ʱʱ��(���뵥λ)
*/
void DelayMS(unsigned int n)
{
    int i,j;
    for(i = n;i>0;i--)
        for(j=1000;j>0;j--) ;
}
/*!
*  \brief  ���ڷ������ַ���
*  \param  �ַ���
*/
void SendStrings(uchar *str)
{
    while(*str)
    {
        TX_8(*str);
        str++;
    }
}
/*!
*  \brief  ���ڷ�����N���ֽ�
*  \param  ����
*/
void SendNU8(uint8 *pData,uint16 nDataLen)
{
    uint16 i = 0;
    for (;i<nDataLen;++i)
    {
        TX_8(pData[i]);
    }
}
/*!
*  \brief  ���ڷ�����N��16λ������
*  \param  ����
*/
void SendNU16(uint16 *pData,uint16 nDataLen)
{
    uint16 i = 0;
    for (;i<nDataLen;++i)
    {
        TX_16(pData[i]);
    }
}
/*!
*  \brief  ������������
*/
void SetHandShake()
{
    BEGIN_CMD();
    TX_8(0x04);
    END_CMD();
}

/*!
*  \brief  ����ǰ��ɫ
*  \param  color ǰ��ɫ
*/
void SetFcolor(uint16 color)
{
    BEGIN_CMD();
    TX_8(0x41);
    TX_16(color);
    END_CMD();
}
/*!
*  \brief  ���ñ���ɫ
*  \param  color ����ɫ
*/
void SetBcolor(uint16 color)
{
    BEGIN_CMD();
    TX_8(0x42);
    TX_16(color);
    END_CMD();
}
/*!
*  \brief ��ȡ
*  \param  color ����ɫ
*/
void ColorPicker(uint8 mode, uint16 x,uint16 y)
{
    BEGIN_CMD();
    TX_8(0xA3);
    TX_8(mode);
    TX_16(x);
    TX_16(y);
    END_CMD();
}
/*!
*  \brief  �������
*/
void GUI_CleanScreen()
{
    BEGIN_CMD();
    TX_8(0x01);
    END_CMD();
}
/*!
*  \brief  �������ּ��
*  \param  x_w ������
*  \param  y_w ������
*/
void SetTextSpace(uint8 x_w, uint8 y_w)
{
    BEGIN_CMD();
    TX_8(0x43);
    TX_8(x_w);
    TX_8(y_w);
    END_CMD();
}
/*!
*  \brief  ����������ʾ����
*  \param  enable �Ƿ���������
*  \param  width ����
*  \param  height �߶�
*/
void SetFont_Region(uint8 enable,uint16 width,uint16 height)
{
    BEGIN_CMD();
    TX_8(0x45);
    TX_8(enable);
    TX_16(width);
    TX_16(height);
    END_CMD();
}
/*!
*  \brief  ���ù���ɫ
*  \param  fillcolor_dwon ��ɫ�½�
*  \param  fillcolor_up ��ɫ�Ͻ�
*/
void SetFilterColor(uint16 fillcolor_dwon, uint16 fillcolor_up)
{
    BEGIN_CMD();
    TX_8(0x44);
    TX_16(fillcolor_dwon);
    TX_16(fillcolor_up);
    END_CMD();
}

/*!
*  \brief  ���ù���ɫ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  back ��ɫ�Ͻ�
*  \param  font ����
*  \param  strings �ַ�������
*/
void DisText(uint16 x, uint16 y,uint8 back,uint8 font,uchar *strings )
{
    BEGIN_CMD();
    TX_8(0x20);
    TX_16(x);
    TX_16(y);
    TX_8(back);
    TX_8(font);
    SendStrings(strings);
    END_CMD();
}
/*!
*  \brief    ��ʾ���
*  \param  enable �Ƿ���ʾ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  width ����
*  \param  height �߶�
*/
void DisCursor(uint8 enable,uint16 x, uint16 y,uint8 width,uint8 height )
{
    BEGIN_CMD();
    TX_8(0x21);
    TX_8(enable);
    TX_16(x);
    TX_16(y);
    TX_8(width);
    TX_8(height);
    END_CMD();
}
/*!
*  \brief      ��ʾȫ��ͼƬ
*  \param  image_id ͼƬ����
*  \param  masken �Ƿ�����͸������
*/
void DisFull_Image(uint16 image_id,uint8 masken)
{
    BEGIN_CMD();
    TX_8(0x31);
    TX_16(image_id);
    TX_8(masken);
    END_CMD();
}
/*!
*  \brief      ָ��λ����ʾͼƬ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  image_id ͼƬ����
*  \param  masken �Ƿ�����͸������
*/
void DisArea_Image(uint16 x,uint16 y,uint16 image_id,uint8 masken)
{
    BEGIN_CMD();
    TX_8(0x32);
    TX_16(x);
    TX_16(y);
    TX_16(image_id);
    TX_8(masken);
    END_CMD();
}
/*!
*  \brief      ��ʾ�ü�ͼƬ
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  image_id ͼƬ����
*  \param  image_x ͼƬ�ü�λ��X����
*  \param  image_y ͼƬ�ü�λ��Y����
*  \param  image_l ͼƬ�ü�����
*  \param  image_w ͼƬ�ü��߶�
*  \param  masken �Ƿ�����͸������
*/
void DisCut_Image(uint16 x,uint16 y,uint16 image_id,uint16 image_x,uint16 image_y,uint16 image_l, uint16 image_w,uint8 masken)
{
    BEGIN_CMD();
    TX_8(0x33);
    TX_16(x);
    TX_16(y);
    TX_16(image_id);
    TX_16(image_x);
    TX_16(image_y);
    TX_16(image_l);
    TX_16(image_w);
    TX_8(masken);
    END_CMD();
}
/*!
*  \brief      ��ʾGIF����
*  \param  x λ��X����
*  \param  y λ��Y����
*  \param  flashimage_id ͼƬ����
*  \param  enable �Ƿ���ʾ
*  \param  playnum ���Ŵ���
*/
void DisFlashImage(uint16 x,uint16 y,uint16 flashimage_id,uint8 enable,uint8 playnum)
{
    BEGIN_CMD();
    TX_8(0x80);
    TX_16(x);
    TX_16(y);
    TX_16(flashimage_id);
    TX_8(enable);
    TX_8(playnum);
    END_CMD();
}
/*!
*  \brief      ����
*  \param  x λ��X����
*  \param  y λ��Y����
*/
void GUI_Dot(uint16 x,uint16 y)
{
    BEGIN_CMD();
    TX_8(0x50);
    TX_16(x);
    TX_16(y);
    END_CMD();
}
/*!
*  \brief      ����
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*/
void GUI_Line(uint16 x0, uint16 y0, uint16 x1, uint16 y1)
{
    BEGIN_CMD();
    TX_8(0x51);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}

/*!
*  \brief      ������
*  \param  mode ģʽ
*  \param  dot ���ݵ�
*  \param  dot_cnt ����
*/
void GUI_ConDots(uint8 mode,uint16 *dot,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x63);
    TX_8(mode);
    TX_16N(dot,dot_cnt*2);
    END_CMD();
}

/*!
*  \brief   x����Ⱦ�ʹ��ǰ��ɫ����
*  \param  x ������
*  \param  x_space ����
*  \param  dot_y  һ����������
*  \param  dot_cnt  ���������
*/
void GUI_ConSpaceDots(uint16 x,uint16 x_space,uint16 *dot_y,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x59);
    TX_16(x);
    TX_16(x_space);
    TX_16N(dot_y,dot_cnt);
    END_CMD();
}
/*!
*  \brief   ��������ƫ������ǰ��ɫ����
*  \param  x ������
*  \param  y �ݾ���
*  \param  dot_offset  ƫ����
*  \param  dot_cnt  ƫ��������
*/
void GUI_FcolorConOffsetDots(uint16 x,uint16 y,uint16 *dot_offset,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x75);
    TX_16(x);
    TX_16(y);
    TX_16N(dot_offset,dot_cnt);
    END_CMD();
}
/*!
*  \brief   ��������ƫ�����ñ���ɫ����
*  \param  x ������
*  \param  y �ݾ���
*  \param  dot_offset  ƫ����
*  \param  dot_cnt  ƫ��������
*/
void GUI_BcolorConOffsetDots(uint16 x,uint16 y,uint8 *dot_offset,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x76);
    TX_16(x);
    TX_16(y);
    TX_16N(dot_offset,dot_cnt);
    END_CMD();
}
/*!
*  \brief  �Զ����ڱ�������
*  \param  enable ʹ��
*  \param  bl_off_level ��������
*  \param  bl_on_level  ��������
*  \param  bl_on_time  ƫ��������
*/
void SetPowerSaving(uint8 enable, uint8 bl_off_level, uint8 bl_on_level, uint8  bl_on_time)
{
    BEGIN_CMD();
    TX_8(0x77);
    TX_8(enable);
    TX_8(bl_off_level);
    TX_8(bl_on_level);
    TX_8(bl_on_time);
    END_CMD();
}
/*!
*  \brief  ���ƶ��Ķ���������ǰ��ɫ��������
*  \param  dot  �����
*  \param  dot_cnt  ƫ��������
*/
void GUI_FcolorConDots(uint16 *dot,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x68);
    TX_16N(dot,dot_cnt*2);
    END_CMD();
}
/*!
*  \brief  ���ƶ��Ķ��������ñ���ɫ��������
*  \param  dot  �����
*  \param  dot_cnt  ƫ��������
*/
void GUI_BcolorConDots(uint16 *dot,uint16 dot_cnt)
{
    BEGIN_CMD();
    TX_8(0x69);
    TX_16N(dot,dot_cnt*2);
    END_CMD();
}
/*!
*  \brief     ������Բ
*  \param  x0 Բ��λ��X����
*  \param  y0 Բ��λ��Y����
*  \param  r �뾶
*/
void GUI_Circle(uint16 x, uint16 y, uint16 r)
{
    BEGIN_CMD();
    TX_8(0x52);
    TX_16(x);
    TX_16(y);
    TX_16(r);
    END_CMD();
}
/*!
*  \brief      ��ʵ��Բ
*  \param  x0 Բ��λ��X����
*  \param  y0 Բ��λ��Y����
*  \param  r �뾶
*/
void GUI_CircleFill(uint16 x, uint16 y, uint16 r)
{
    BEGIN_CMD();
    TX_8(0x53);
    TX_16(x);
    TX_16(y);
    TX_16(r);
    END_CMD();
}
/*!
*  \brief      ������
*  \param  x0 Բ��λ��X����
*  \param  y0 Բ��λ��Y����
*  \param  r �뾶
*  \param  sa ��ʼ�Ƕ�
*  \param  ea ��ֹ�Ƕ�
*/
void GUI_Arc(uint16 x,uint16 y, uint16 r,uint16 sa, uint16 ea)
{
    BEGIN_CMD();
    TX_8(0x67);
    TX_16(x);
    TX_16(y);
    TX_16(r);
    TX_16(sa);
    TX_16(ea);
    END_CMD();
}
/*!
*  \brief      �����ľ���
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*/
void GUI_Rectangle(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x54);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
*  \brief      ��ʵ�ľ���
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*/
void GUI_RectangleFill(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x55);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
*  \brief      ��������Բ
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*/
void GUI_Ellipse(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x56);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
*  \brief      ��ʵ����Բ
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*/
void GUI_EllipseFill(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x57);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
*  \brief      ����
*  \param  x0 ��ʼλ��X����
*  \param  y0 ��ʼλ��Y����
*  \param  x1 ����λ��X����
*  \param  y1 ����λ��Y����
*/
void SetBackLight(uint8 light_level)
{
    BEGIN_CMD();
    TX_8(0x60);
    TX_8(light_level);
    END_CMD();
}

/*!
*  \brief   ����������
*  \time  time ����ʱ��(���뵥λ)
*/
void SetBuzzer(uint8 time)
{
    BEGIN_CMD();
    TX_8(0x61);
    TX_8(time);
    END_CMD();
}

void GUI_AreaInycolor(uint16 x0, uint16 y0, uint16 x1,uint16 y1 )
{
    BEGIN_CMD();
    TX_8(0x65);
    TX_16(x0);
    TX_16(y0);
    TX_16(x1);
    TX_16(y1);
    END_CMD();
}
/*!
*  \brief   ����������
*  \param enable ����ʹ��
*  \param beep_on ����������
*  \param work_mode ��������ģʽ��0���¾��ϴ���1�ɿ����ϴ���2�����ϴ�����ֵ��3���º��ɿ����ϴ�����
*  \param press_calibration �������������20��У׼��������0���ã�1����
*/
void SetTouchPaneOption(uint8 enbale,uint8 beep_on,uint8 work_mode,uint8 press_calibration)
{
    uint8 options = 0;

    if(enbale)
        options |= 0x01;
    if(beep_on)
        options |= 0x02;
    if(work_mode)
        options |= (work_mode<<2);
    if(press_calibration)
        options |= (press_calibration<<5);

    BEGIN_CMD();
    TX_8(0x70);
    TX_8(options);
    END_CMD();
}
/*!
*  \brief   У׼������
*/
void CalibrateTouchPane()
{
    BEGIN_CMD();
    TX_8(0x72);
    END_CMD();
}
/*!
*  \brief  ����������
*/
void TestTouchPane()
{
    BEGIN_CMD();
    TX_8(0x73);
    END_CMD();
}

/*!
*  \brief  �����豸���ã�����֮����Ҫ�����������޸Ĳ����ʡ���������������������ʽ
*/
void UnlockDeviceConfig(void)
{
    BEGIN_CMD();
    TX_8(0x09);
    TX_8(0xDE);
    TX_8(0xED);
    TX_8(0x13);
    TX_8(0x31);
    END_CMD();
}

/*!
*  \brief  �����豸����
*/
void LockDeviceConfig(void)
{
    BEGIN_CMD();
    TX_8(0x08);
    TX_8(0xA5);
    TX_8(0x5A);
    TX_8(0x5F);
    TX_8(0xF5);
    END_CMD();
}
/*!
*  \brief    �޸Ĵ������Ĳ�����
*  \details  ������ѡ�Χ[0~14]����Ӧʵ�ʲ�����
{1200,2400,4800,9600,19200,38400,57600,115200,1000000,2000000,218750,437500,875000,921800,2500000}
*  \param  option ������ѡ��
*/
void SetCommBps(uint8 option)
{
    BEGIN_CMD();
    TX_8(0xA0);
    TX_8(option);
    END_CMD();
}
/*!
*  \brief      ���õ�ǰд��ͼ��
*  \details  һ������ʵ��˫����Ч��(��ͼʱ������˸)��
*  \details  uint8 layer = 0;
*  \details  WriteLayer(layer);   ����д���
*  \details  ClearLayer(layer);   ʹͼ���͸��
*  \details  ����һϵ�л�ͼָ��
*  \details  DisText(100,100,0,4,"hello hmi!!!");
*  \details  DisplyLayer(layer);  �л���ʾ��
*  \details  layer = (layer+1)%2; ˫�����л�
*  \see DisplyLayer
*  \see ClearLayer
*  \param  layer ͼ����
*/
void WriteLayer(uint8 layer)
{
    BEGIN_CMD();
    TX_8(0xA1);
    TX_8(layer);
    END_CMD();
}
/*!
*  \brief      ���õ�ǰ��ʾͼ��
*  \param  layer ͼ����
*/
void DisplyLayer(uint8 layer)
{
    BEGIN_CMD();
    TX_8(0xA2);
    TX_8(layer);
    END_CMD();
}
/*!
*  \brief      ����ͼ��
*  \param  src_layer ԭʼͼ��
*  \param  dest_layer Ŀ��ͼ��
*/
void CopyLayer(uint8 src_layer,uint8 dest_layer)
{
    BEGIN_CMD();
    TX_8(0xA4);
    TX_8(src_layer);
    TX_8(dest_layer);
    END_CMD();
}
/*!
*  \brief      ���ͼ�㣬ʹͼ����͸��
*  \param  layer ͼ����
*/
void ClearLayer(uint8 layer)
{
    BEGIN_CMD();
    TX_8(0x05);
    TX_8(layer);
    END_CMD();
}

void GUI_DispRTC(uint8 enable,uint8 mode,uint8 font,uint16 color,uint16 x,uint16 y)
{
    BEGIN_CMD();
    TX_8(0x85);
    TX_8(enable);
    TX_8(mode);
    TX_8(font);
    TX_16(color);
    TX_16(x);
    TX_16(y);
    END_CMD();
}
/*!
*  \brief  д���ݵ��������û��洢��
*  \param  startAddress ��ʼ��ַ
*  \param  length �ֽ���
*  \param  _data ��д�������
*/
void WriteUserFlash(uint32 startAddress,uint16 length,uint8 *_data)
{
    BEGIN_CMD();
    TX_8(0x87);
    TX_32(startAddress);
    TX_8N(_data,length);
    END_CMD();
}
/*!
*  \brief  �Ӵ������û��洢����ȡ����
*  \param  startAddress ��ʼ��ַ
*  \param  length �ֽ���
*/
void ReadUserFlash(uint32 startAddress,uint16 length)
{
    BEGIN_CMD();
    TX_8(0x88);
    TX_32(startAddress);
    TX_16(length);
    END_CMD();
}
/*!
*  \brief      ��ȡ��ǰ����
*/
void GetScreen(uint16 screen_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x01);
    END_CMD();
}
/*!
*  \brief      ���õ�ǰ����
*  \param  screen_id ����ID
*/
void SetScreen(uint16 screen_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x00);
    TX_16(screen_id);
    END_CMD();
}
/*!
*  \brief     ����\���û������
*  \details ����\����һ��ɶ�ʹ�ã����ڱ�����˸�����ˢ���ٶ�
*  \details �÷���
*	\details SetScreenUpdateEnable(0);//��ֹ����
*	\details һϵ�и��»����ָ��
*	\details SetScreenUpdateEnable(1);//��������
*  \param  enable 0���ã�1����
*/
void SetScreenUpdateEnable(uint8 enable)
{
    BEGIN_CMD();
    TX_8(0xB3);
    TX_8(enable);
    END_CMD();
}
/*!
*  \brief     ���ÿؼ����뽹��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  focus �Ƿ�������뽹��
*/
void SetControlFocus(uint16 screen_id,uint16 control_id,uint8 focus)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x02);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(focus);
    END_CMD();
}
/*!
*  \brief     ��ʾ\���ؿؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  visible �Ƿ���ʾ
*/
void SetControlVisiable(uint16 screen_id,uint16 control_id,uint8 visible)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x03);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(visible);
    END_CMD();
}
/*!
*  \brief     ���ô����ؼ�ʹ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  enable �ؼ��Ƿ�ʹ��
*/
void SetControlEnable(uint16 screen_id,uint16 control_id,uint8 enable)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x04);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(enable);
    END_CMD();
}
/*!
*  \brief     ���ð�ť״̬
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ť״̬
*/
void SetButtonValue(uint16 screen_id,uint16 control_id,uchar state)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(state);
    END_CMD();
}
/*!
*  \brief     �����ı�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  str �ı�ֵ
*/
void SetTextValue(uint16 screen_id,uint16 control_id,uchar *str)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    SendStrings(str);
    END_CMD();
}

#if FIRMWARE_VER>=908
/*!
*  \brief     �����ı�Ϊ����ֵ��Ҫ��FIRMWARE_VER>=908
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �ı���ֵ
*  \param  sign 0-�޷��ţ�1-�з���
*  \param  fill_zero ����λ��������ʱ��ಹ��
*/
void SetTextInt32(uint16 screen_id,uint16 control_id,uint32 value,uint8 sign,uint8 fill_zero)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x07);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(sign?0X01:0X00);
    TX_8((fill_zero&0x0f)|0x80);
    TX_32(value);
    END_CMD();
}
/*!
*  \brief     �����ı������ȸ���ֵ��Ҫ��FIRMWARE_VER>=908
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �ı���ֵ
*  \param  precision С��λ��
*  \param  show_zeros Ϊ1ʱ����ʾĩβ0
*/
void SetTextFloat(uint16 screen_id,uint16 control_id,float value,uint8 precision,uint8 show_zeros)
{
	uint8 i = 0;

	BEGIN_CMD();
	TX_8(0xB1);
	TX_8(0x07);
	TX_16(screen_id);
	TX_16(control_id);
	TX_8(0x02);
	TX_8((precision&0x0f)|(show_zeros?0x80:0x00));

	for (i=0;i<4;++i)
	{
	 //��Ҫ���ִ�С��
#if(0)
		TX_8(((uint8 *)&value)[i]);
#else
		TX_8(((uint8 *)&value)[3-i]);
#endif
	}
	END_CMD();
}
#endif
/*!
*  \brief      ���ý���ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*/
void SetProgressValue(uint16 screen_id,uint16 control_id,uint32 value)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_32(value);
    END_CMD();
}
/*!
*  \brief     �����Ǳ�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*/
void SetMeterValue(uint16 screen_id,uint16 control_id,uint32 value)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_32(value);
    END_CMD();
}
/*!
*  \brief     �����Ǳ�ֵ
*  \param  screen_id ����ID
*  \param  control_id ͼƬ�ؼ�ID
*  \param  value ��ֵ
*/
void Set_picMeterValue(uint16 screen_id,uint16 control_id,uint16 value)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(value);
    END_CMD();
}
/*!
*  \brief      ���û�����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*/

void SetSliderValue(uint16 screen_id,uint16 control_id,uint32 value)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_32(value);
    END_CMD();
}
/*!
*  \brief      ����ѡ��ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  item ��ǰѡ��
*/
void SetSelectorValue(uint16 screen_id,uint16 control_id,uint8 item)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x10);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(item);
    END_CMD();
}
/*!
*  \brief     ��ȡ�ؼ�ֵ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void GetControlValue(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x11);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}

/*!
*  \brief      ��ʼ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void AnimationStart(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x20);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}

/*!
*  \brief      ֹͣ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void AnimationStop(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x21);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief      ��ͣ���Ŷ���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void AnimationPause(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x22);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief     �����ƶ�֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  frame_id ֡ID
*/
void AnimationPlayFrame(uint16 screen_id,uint16 control_id,uint8 frame_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x23);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(frame_id);
    END_CMD();
}
/*!
*  \brief     ������һ֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void AnimationPlayPrev(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x24);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief     ������һ֡
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void AnimationPlayNext(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x25);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief     ���߿ؼ�-����ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  color ��ɫ
*/
void GraphChannelAdd(uint16 screen_id,uint16 control_id,uint8 channel,uint16 color)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x30);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    TX_16(color);
    END_CMD();
}
/*!
*  \brief     ���߿ؼ�-ɾ��ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*/
void GraphChannelDel(uint16 screen_id,uint16 control_id,uint8 channel)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x31);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    END_CMD();
}
/*!
*  \brief     ���߿ؼ�-��������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*  \param  pData ��������
*  \param  nDataLen ���ݸ���
*/
void GraphChannelDataAdd(uint16 screen_id,uint16 control_id,uint8 channel,uint8 *pData,uint16 nDataLen)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x32);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    TX_16(nDataLen);
    TX_8N(pData,nDataLen);
    END_CMD();
}
/*!
*  \brief     ���߿ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ����
*/
void GraphChannelDataClear(uint16 screen_id,uint16 control_id,uint8 channel)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x33);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    END_CMD();
}
/*!
*  \brief     ���߿ؼ�-������ͼ����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  x_offset ˮƽƫ��
*  \param  x_mul ˮƽ����ϵ��
*  \param  y_offset ��ֱƫ��
*  \param  y_mul ��ֱ����ϵ��
*/
void GraphSetViewport(uint16 screen_id,uint16 control_id,int16 x_offset,uint16 x_mul,int16 y_offset,uint16 y_mul)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x34);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(x_offset);
    TX_16(x_mul);
    TX_16(y_offset);
    TX_16(y_mul);
    END_CMD();
}
/*!
*  \brief     ��ʼ��������
*  \param  screen_id ����ID
*/
void BatchBegin(uint16 screen_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x12);
    TX_16(screen_id);
}
/*!
*  \brief     �������°�ť�ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*/
void BatchSetButtonValue(uint16 control_id,uint8 state)
{
    TX_16(control_id);
    TX_16(1);
    TX_8(state);
}
/*!
*  \brief     �������½������ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*/
void BatchSetProgressValue(uint16 control_id,uint32 value)
{
    TX_16(control_id);
    TX_16(4);
    TX_32(value);
}

/*!
*  \brief     �������»������ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*/
void BatchSetSliderValue(uint16 control_id,uint32 value)
{
    TX_16(control_id);
    TX_16(4);
    TX_32(value);
}
/*!
*  \brief     ���������Ǳ��ؼ�
*  \param  control_id �ؼ�ID
*  \param  value ��ֵ
*/
void BatchSetMeterValue(uint16 control_id,uint32 value)
{
    TX_16(control_id);
    TX_16(4);
    TX_32(value);
}
/*!
*  \brief      �����ַ�������
*/
uint32 GetStringLen(uchar *str)
{
    uchar *p = str;
    while(*str)
    {
        str++;
    }

    return (str-p);
}
/*!
*  \brief     ���������ı��ؼ�
*  \param  control_id �ؼ�ID
*  \param  strings �ַ���
*/
void BatchSetText(uint16 control_id,uchar *strings)
{
    TX_16(control_id);
    TX_16(GetStringLen(strings));
    SendStrings(strings);
}
/*!
*  \brief     �������¶���\ͼ��ؼ�
*  \param  control_id �ؼ�ID
*  \param  frame_id ֡ID
*/
void BatchSetFrame(uint16 control_id,uint16 frame_id)
{
    TX_16(control_id);
    TX_16(2);
    TX_16(frame_id);
}

#if FIRMWARE_VER>=908

/*!
*  \brief     �������ÿؼ��ɼ�
*  \param  control_id �ؼ�ID
*  \param  visible ֡ID
*/
void BatchSetVisible(uint16 control_id,uint8 visible)
{
    TX_16(control_id);
    TX_8(1);
    TX_8(visible);
}
/*!
*  \brief     �������ÿؼ�ʹ��
*  \param  control_id �ؼ�ID
*  \param  enable ֡ID
*/
void BatchSetEnable(uint16 control_id,uint8 enable)
{
    TX_16(control_id);
    TX_8(2);
    TX_8(enable);
}

#endif
/*!
*  \brief    ������������
*/
void BatchEnd()
{
    END_CMD();
}
/*!
*  \brief     ���õ���ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  timeout ����ʱ(��)
*/
void SeTimer(uint16 screen_id,uint16 control_id,uint32 timeout)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x40);
    TX_16(screen_id);
    TX_16(control_id);
    TX_32(timeout);
    END_CMD();
}
/*!
*  \brief     ��������ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void StartTimer(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x41);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief     ֹͣ����ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void StopTimer(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x42);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief     ��ͣ����ʱ�ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void PauseTimer(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x44);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief     ���ÿؼ�����ɫ
*  \details  ֧�ֿؼ������������ı�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  color ����ɫ
*/
void SetControlBackColor(uint16 screen_id,uint16 control_id,uint16 color)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x18);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(color);
    END_CMD();
}
/*!
*  \brief     ���ÿؼ�ǰ��ɫ
* \details  ֧�ֿؼ���������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  color ǰ��ɫ
*/
void SetControlForeColor(uint16 screen_id,uint16 control_id,uint16 color)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x19);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(color);
    END_CMD();
}
/*!
*  \brief     ��ʾ\���ص����˵��ؼ�
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  show �Ƿ���ʾ��Ϊ0ʱfocus_control_id��Ч
*  \param  focus_control_id �������ı��ؼ�(�˵��ؼ�������������ı��ؼ�)
*/
void ShowPopupMenu(uint16 screen_id,uint16 control_id,uint8 show,uint16 focus_control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x13);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(show);
    TX_16(focus_control_id);
    END_CMD();
}
/*!
*  \brief     ��ʾ\����ϵͳ����
*  \param  show 0���أ�1��ʾ
*  \param  x ������ʾλ��X����
*  \param  y ������ʾλ��Y����
*  \param  type 0С���̣�1ȫ����
*  \param  option 0�����ַ���1���룬2ʱ������
*  \param  max_len ����¼���ַ���������
*/
void ShowKeyboard(uint8 show,uint16 x,uint16 y,uint8 type,uint8 option,uint8 max_len)
{
    BEGIN_CMD();
    TX_8(0x86);
    TX_8(show);
    TX_16(x);
    TX_16(y);
    TX_8(type);
    TX_8(option);
    TX_8(max_len);
    END_CMD();
}

#if FIRMWARE_VER>=921
/*!
*  \brief     ����������
*  \param  ui_lang �û���������0~9
*  \param  sys_lang ϵͳ��������-0���ģ�1Ӣ��
*/
void SetLanguage(uint8 ui_lang,uint8 sys_lang)
{
    uint8 lang = ui_lang;
    if(sys_lang)   lang |= 0x80;

    BEGIN_CMD();
    TX_8(0xC1);
    TX_8(lang);
    TX_8(0xC1+lang);//У�飬��ֹ�����޸�����
    END_CMD();
}
#endif


#if FIRMWARE_VER>=921
/*!
*  \brief     ��ʼ����ؼ���ֵ��FLASH
*  \param  version ���ݰ汾�ţ�������ָ������16λΪ���汾�ţ���16λΪ�ΰ汾��
*  \param  address �������û��洢���Ĵ�ŵ�ַ��ע���ֹ��ַ�ص�����ͻ
*/
void FlashBeginSaveControl(uint32 version,uint32 address)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0xAA);
    TX_32(version);
    TX_32(address);
}

/*!
*  \brief     ����ĳ���ؼ�����ֵ��FLASH
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void FlashSaveControl(uint16 screen_id,uint16 control_id)
{
    TX_16(screen_id);
    TX_16(control_id);
}
/*!
*  \brief     ����ĳ���ؼ�����ֵ��FLASH
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void FlashEndSaveControl()
{
    END_CMD();
}
/*!
*  \brief     ��FLASH�лָ��ؼ�����
*  \param  version ���ݰ汾�ţ����汾�ű�����洢ʱһ�£���������ʧ��
*  \param  address �������û��洢���Ĵ�ŵ�ַ
*/
void FlashRestoreControl(uint32 version,uint32 address)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0xAB);
    TX_32(version);
    TX_32(address);
    END_CMD();
}

#endif

#if FIRMWARE_VER>=921
/*!
*  \brief     ������ʷ���߲�������ֵ(���ֽڣ�uint8��int8)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*/
void HistoryGraph_SetValueInt8(uint16 screen_id,uint16 control_id,uint8 *value,uint8 channel)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x60);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8N(value,channel);
    END_CMD();
}
/*!
*  \brief     ������ʷ���߲�������ֵ(˫�ֽڣ�uint16��int16)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*/
void HistoryGraph_SetValueInt16(uint16 screen_id,uint16 control_id,uint16 *value,uint8 channel)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x60);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16N(value,channel);
    END_CMD();
}
/*!
*  \brief     ������ʷ���߲�������ֵ(���ֽڣ�uint32��int32)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*/
void HistoryGraph_SetValueInt32(uint16 screen_id,uint16 control_id,uint32 *value,uint8 channel)
{
    uint8 i = 0;

    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x60);
    TX_16(screen_id);
    TX_16(control_id);

    for (;i<channel;++i)
    {
        TX_32(value[i]);
    }

    END_CMD();
}
/*!
*  \brief     ������ʷ���߲�������ֵ(�����ȸ�����)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value ����������
*  \param  channel ͨ����
*/
void HistoryGraph_SetValueFloat(uint16 screen_id,uint16 control_id,float *value,uint8 channel)
{
    uint8 i = 0;
    uint32 tmp = 0;

    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x60);
    TX_16(screen_id);
    TX_16(control_id);

    for (;i<channel;++i)
    {
        tmp = *(uint32 *)(value+i);
        TX_32(tmp);
    }

    END_CMD();
}
/*!
*  \brief     �������ֹ��ʷ���߲���
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  enable 0-��ֹ��1-����
*/
void HistoryGraph_EnableSampling(uint16 screen_id,uint16 control_id,uint8 enable)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x61);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(enable);
    END_CMD();
}
/*!
*  \brief     ��ʾ��������ʷ����ͨ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  channel ͨ�����
*  \param  show 0-���أ�1-��ʾ
*/
void HistoryGraph_ShowChannel(uint16 screen_id,uint16 control_id,uint8 channel,uint8 show)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x62);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(channel);
    TX_8(show);
    END_CMD();
}
/*!
*  \brief     ������ʷ����ʱ�䳤��(����������)
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  sample_count һ����ʾ�Ĳ�������
*/
void HistoryGraph_SetTimeLength(uint16 screen_id,uint16 control_id,uint16 sample_count)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x63);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(0x00);
    TX_16(sample_count);
    END_CMD();
}

/*!
*  \brief     ��ʷ�������ŵ�ȫ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void HistoryGraph_SetTimeFullScreen(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x63);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(0x01);
    END_CMD();
}
/*!
*  \brief     ������ʷ�������ű���ϵ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  zoom ���Űٷֱ�(zoom>100%ʱˮƽ������С�������Ŵ�)
*  \param  max_zoom �������ƣ�һ�������ʾ��������
*  \param  min_zoom �������ƣ�һ��������ʾ��������
*/
void HistoryGraph_SetTimeZoom(uint16 screen_id,uint16 control_id,uint16 zoom,uint16 max_zoom,uint16 min_zoom)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x63);
    TX_16(screen_id);
    TX_16(control_id);
    TX_8(0x02);
    TX_16(zoom);
    TX_16(max_zoom);
    TX_16(min_zoom);
    END_CMD();
}

#endif

#if SD_FILE_EN
/*!
*  \brief     ���SD���Ƿ����
*/
void SD_IsInsert(void)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x01);
    END_CMD();
}
/*!
*  \brief     �򿪻򴴽��ļ�
*  \param  filename �ļ�����(��ASCII����)
*  \param  mode ģʽ����ѡ���ģʽ����FA_XXXX
*/
void SD_CreateFile(uint8 *filename,uint8 mode)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x05);
    TX_8(mode);
    SendStrings(filename);
    END_CMD();
}
/*!
*  \brief     �Ե�ǰʱ�䴴���ļ�������:20161015083000.txt
*  \param  ext �ļ���׺������ txt
*/
void SD_CreateFileByTime(uint8 *ext)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x02);
    SendStrings(ext);
    END_CMD();
}
/*!
*  \brief     �ڵ�ǰ�ļ�ĩβд������
*  \param  buffer ����
*  \param  dlc ���ݳ���
*/
void SD_WriteFile(uint8 *buffer,uint16 dlc)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x03);
    TX_16(dlc);
    TX_8N(buffer,dlc);
    END_CMD();
}
/*!
*  \brief     ��ȡ��ǰ�ļ�
*  \param  offset �ļ�λ��ƫ��
*  \param  dlc ���ݳ���
*/
void SD_ReadFile(uint32 offset,uint16 dlc)
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x07);
    TX_32(offset);
    TX_16(dlc);
    END_CMD();
}

/*!
*  \brief     ��ȡ��ǰ�ļ�����
*/
void SD_GetFileSize()
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x06);
    END_CMD();
}
/*!
*  \brief     �رյ�ǰ�ļ�
*/
void SD_CloseFile()
{
    BEGIN_CMD();
    TX_8(0x36);
    TX_8(0x04);
    END_CMD();
}

#endif//SD_FILE_EN
/*!
*  \brief     ��¼�ؼ�-��������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �澯ֵ
*  \param  time �澯������ʱ�䣬Ϊ0ʱʹ����Ļ�ڲ�ʱ��
*/
void Record_SetEvent(uint16 screen_id,uint16 control_id,uint16 value,uint8 *time)
{
    uint8 i  = 0;

    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x50);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(value);

    if(time)
    {
        for(i=0;i<7;++i)
            TX_8(time[i]);
    }

    END_CMD();
}
/*!
*  \brief     ��¼�ؼ�-�������
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  value �澯ֵ
*  \param  time �澯�����ʱ�䣬Ϊ0ʱʹ����Ļ�ڲ�ʱ��
*/
void Record_ResetEvent(uint16 screen_id,uint16 control_id,uint16 value,uint8 *time)
{
    uint8 i  = 0;

    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x51);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(value);

    if(time)
    {
        for(i=0;i<7;++i)
            TX_8(time[i]);
    }

    END_CMD();
}
/*!
*  \brief    ��¼�ؼ�- ���ӳ����¼
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  record һ����¼(�ַ���)������ͨ���ֺŸ��������磺��һ��;�ڶ���;������;
*/
void Record_Add(uint16 screen_id,uint16 control_id,uint8 *record)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x52);
    TX_16(screen_id);
    TX_16(control_id);

    SendStrings(record);

    END_CMD();
}
/*!
*  \brief     ��¼�ؼ�-�����¼����
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void Record_Clear(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x53);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief     ��¼�ؼ�-���ü�¼��ʾƫ��
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*  \param  offset ��ʾƫ�ƣ�������λ��
*/
void Record_SetOffset(uint16 screen_id,uint16 control_id,uint16 offset)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x54);
    TX_16(screen_id);
    TX_16(control_id);
    TX_16(offset);
    END_CMD();
}
/*!
*  \brief     ��¼�ؼ�-��ȡ��ǰ��¼��Ŀ
*  \param  screen_id ����ID
*  \param  control_id �ؼ�ID
*/
void Record_GetCount(uint16 screen_id,uint16 control_id)
{
    BEGIN_CMD();
    TX_8(0xB1);
    TX_8(0x55);
    TX_16(screen_id);
    TX_16(control_id);
    END_CMD();
}
/*!
*  \brief     ��ȡ��ĻRTCʱ��
*/
void ReadRTC(void)
{
    BEGIN_CMD();
    TX_8(0x82);
    END_CMD();
}

/*!
*  \brief   ��������
*  \param   buffer ʮ�����Ƶ�����·��������
*/
void PlayMusic(uint8 *buffer)
{
    uint8 i  = 0;

    BEGIN_CMD();
    if(buffer)
    {
        for(i=0;i<19;++i)
            TX_8(buffer[i]);
    }
    END_CMD();
}