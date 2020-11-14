#include "AS608.h"
#include "beep.h"
#include "delay.h"
#include "exfuns.h"
#include "ff.h"
#include "fontupd.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "pwm.h"
#include "sdio_sdcard.h"
#include "sys.h"
#include "text.h"
#include "timer.h"
#include "usart.h"
#include "usart2.h"
#include "usart3.h"
#include "usmart.h"
#include "w25qxx.h"

#define usart2_baund 57600

SysPara AS608Para;
u16 ValidN;
u16 ID;
u16 Lock_state; //����״̬

void Add_FR(void);
void Del_FR(void);
void press_FR(void);
void ShowErrMessage(u8 ensure);
void OPEN(void);
void CLOSE(void);

int main(void) {
  u8 ensure;
  u8 key_num;
  char *str;
  u8 lcd_id[12];

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  delay_init(168);
  uart_init(115200);
  usart2_init(usart2_baund);
  usart3_init(115200);
  PS_StaGPIO_Init();
  BEEP_Init();
  KEY_Init();
  LCD_Init();
  W25QXX_Init();
  LCD_Init();
  PWM_Init();
  TIM_SetCompare3(TIM3, 192); //�޸ıȽ�ֵ���޸�ռ�ձȶ�Ӧ0�� Ĭ����Ϊ�ء�
  delay_ms(500);
  Lock_state = 0; //��Ĭ��Ϊ��

  sprintf((char *)lcd_id, "LCD ID:%04X", lcddev.id);
  usmart_dev.init(168);
  my_mem_init(SRAMIN);
  my_mem_init(SRAMCCM);
  exfuns_init();
  f_mount(fs[1], "1:", 1);
  POINT_COLOR = RED;
  while (font_init()) {
    LCD_ShowString(60, 50, 240, 16, 16, "Font Error!");
    delay_ms(200);
    LCD_Fill(60, 50, 240, 66, WHITE);
    delay_ms(200);
  }
  LCD_Clear(WHITE);
  POINT_COLOR = RED;
  Show_Str_Mid(0, 0, "AS608ָ��ʶ��ģ����Գ���", 16, 240);
  POINT_COLOR = BLUE;
  Show_Str_Mid(0, 40, "��AS608ģ������....", 16, 240);
  while (PS_HandShake(&AS608Addr)) {
    delay_ms(400);
    LCD_Fill(0, 40, 240, 80, WHITE);
    Show_Str_Mid(0, 40, "δ��⵽ģ��!!!", 16, 240);
    delay_ms(800);
    LCD_Fill(0, 40, 240, 80, WHITE);
    Show_Str_Mid(0, 40, "��������ģ��...", 16, 240);
  }
  LCD_Fill(30, 40, 240, 100, WHITE);
  Show_Str_Mid(0, 40, "ͨѶ�ɹ�!!!", 16, 240);
  str = mymalloc(SRAMIN, 30);
  sprintf(str, "������:%d   ��ַ:%x", usart2_baund, AS608Addr);
  Show_Str(0, 60, 240, 16, (u8 *)str, 16, 0);
  ensure = PS_ValidTempleteNum(&ValidN);
  if (ensure != 0x00)
    ShowErrMessage(ensure);
  ensure = PS_ReadSysPara(&AS608Para);
  if (ensure == 0x00) {
    mymemset(str, 0, 50);
    sprintf(str, "������:%d     �Աȵȼ�: %d", AS608Para.PS_max - ValidN,
            AS608Para.PS_level);
    Show_Str(0, 80, 240, 16, (u8 *)str, 16, 0);
  } else
    ShowErrMessage(ensure);
  myfree(SRAMIN, str);
  while (1) {
    key_num = KEY_Scan(0);

    if (key_num) {
      if (key_num == 1)
        Del_FR(); //ɾָ��
      if (key_num == 2)
        Add_FR(); //¼ָ��
      if (key_num == 3 && Lock_state == 1)
        CLOSE();
    }
    if (PS_Sta) {
      press_FR();
    }
  }
}

void ShowErrMessage(u8 ensure) {
  LCD_Fill(0, 120, lcddev.width, 160, WHITE);
  Show_Str_Mid(0, 120, (u8 *)EnsureMessage(ensure), 16, 240);
}

//���ָ��
void Add_FR(void) {
  u8 i, ensure, processnum = 0;

  while (1) {
    switch (processnum) {
    case 0:
      i++;
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 100, "�밴ָ��", 16, 240);
      ensure = PS_GetImage();
      if (ensure == 0x00) {
        BEEP = 1;
        ensure = PS_GenChar(CharBuffer1); //��������
        BEEP = 0;
        if (ensure == 0x00) {
          LCD_Fill(0, 120, lcddev.width, 160, WHITE);
          Show_Str_Mid(0, 120, "ָ������", 16, 240);
          i = 0;
          processnum = 1; //�����ڶ���
        }
      }
      break;

    case 1:
      i++;
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 100, "�밴�ٰ�һ��ָ��", 16, 240);
      ensure = PS_GetImage();
      if (ensure == 0x00) {
        BEEP = 1;
        ensure = PS_GenChar(CharBuffer2); //��������
        BEEP = 0;
        if (ensure == 0x00) {
          LCD_Fill(0, 120, lcddev.width, 160, WHITE);
          Show_Str_Mid(0, 120, "ָ������", 16, 240);
          i = 0;
          processnum = 2; //����������
        }
      }
      break;

    case 2:
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 100, "�Ա�����ָ��", 16, 240);
      ensure = PS_Match();
      if (ensure == 0x00) {
        LCD_Fill(0, 120, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 120, "�Աȳɹ�,����ָ��һ��", 16, 240);
        processnum = 3; //�������Ĳ�
      } else {
        LCD_Fill(0, 100, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 100, "�Ա�ʧ�ܣ�������¼��ָ��", 16, 240);
        ShowErrMessage(ensure);
        i = 0;
        processnum = 0; //���ص�һ��
      }
      delay_ms(1200);
      break;

    case 3:
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 100, "����ָ��ģ��", 16, 240);
      ensure = PS_RegModel();
      if (ensure == 0x00) {
        LCD_Fill(0, 120, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 120, "����ָ��ģ��ɹ�", 16, 240);
        processnum = 4; //�������岽
      } else {
        processnum = 0;
      }
      delay_ms(1200);
      break;

    case 4:
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      PS_ValidTempleteNum(&ValidN);
      ID = ValidN + 1;
      ensure = PS_StoreChar(CharBuffer2, ID); //����ģ��
      if (ensure == 0x00) {
        LCD_Fill(0, 100, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 120, "¼��ָ�Ƴɹ�", 16, 240);
        PS_ValidTempleteNum(&ValidN); //����ָ�Ƹ���
        LCD_ShowNum(56, 80, AS608Para.PS_max - ValidN, 3,
                    16); //��ʾʣ��ָ������
        delay_ms(1500);
        LCD_Fill(0, 100, 240, 160, WHITE);
        return;
      } else {
        processnum = 0;
      }
      break;
    }
    delay_ms(400);
    if (i == 5) { //����5��û�а���ָ���˳�
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      break;
    }
  }
}

//ˢָ��
void press_FR(void) {
  SearchResult seach;
  u8 delay_time = 0;
  u8 ensure;
  char *str;
  ensure = PS_GetImage();
  if (ensure == 0x00) { //��ȡͼ��ɹ�
    BEEP = 1;           //�򿪷�����
    ensure = PS_GenChar(CharBuffer1);
    if (ensure == 0x00) { //���������ɹ�
      BEEP = 0;           //�رշ�����
      ensure = PS_HighSpeedSearch(CharBuffer1, 0, AS608Para.PS_max, &seach);
      if (ensure == 0x00) { //�����ɹ�
        LCD_Fill(0, 100, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 100, "ˢָ�Ƴɹ�", 16, 240);
        str = mymalloc(SRAMIN, 2000);
        sprintf(str, "ȷ�д���,ID:%d  ƥ��÷�:%d", seach.pageID,
                seach.mathscore);
        Show_Str_Mid(0, 140, (u8 *)str, 16, 240);
        myfree(SRAMIN, str);
        if (Lock_state == 0) {
          Lock_state = 1;
          OPEN();
        }
      } else {
        // TamperAlarm
        /// sys/a1ERx0fKxTG/${deviceName}/thing/event/${tsl.event.identifier}/post
        // u3_printf("{\"method\":\"thing.event.property.post\",\"params\":{\"LockState\":0},\"topic\":\"/sys/a1ERx0fKxTG/fingerprintlock/thing/event/property/post\",\"version\":\"1.0\"}");
        for (delay_time = 0; delay_time < 5; delay_time++) {
          BEEP = 1;
          delay_ms(500);
          BEEP = 0;
          delay_ms(500);
        }
      }
    }

    BEEP = 0; //�رշ�����
    delay_ms(600);
    LCD_Fill(0, 100, lcddev.width, 160, WHITE);
  }
}

//ɾ��ָ��
void Del_FR(void) {
  u8 ensure;
  u16 num = 0;
  while (1) {
    LCD_Fill(0, 100, lcddev.width, 160, WHITE);
    Show_Str_Mid(0, 100, "ɾ��ָ��", 16, 240);
    Show_Str_Mid(0, 120, "KEY2:ɾ��,KEY_UP:����", 16, 240);
    delay_ms(100);
    num = KEY_Scan(0);
    if (num == 3)
      goto MENU; //������ҳ��
    else if (num == 2)
      ensure = PS_Empty(); //���ָ�ƿ�
    if (ensure == 0) {
      LCD_Fill(0, 120, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 140, "ɾ��ָ�Ƴɹ�", 16, 240);
      delay_ms(200);
      break;
    }
  }
MENU:
  PS_ValidTempleteNum(&ValidN);                          //����ָ�Ƹ���
  LCD_ShowNum(56, 80, AS608Para.PS_max - ValidN, 3, 16); //��ʾʣ��ָ������
  LCD_Fill(0, 100, lcddev.width, 160, WHITE);
  delay_ms(50);
}

void OPEN(void) {
  TIM_SetCompare3(TIM3, 185); //�޸ıȽ�ֵ �޸�ռ�ձ�Ϊ��Ӧ90�� ��Ϊ����
  delay_ms(500);
  u3_printf("{\"method\":\"thing.event.property.post\",\"params\":{"
            "\"LockState\":1},\"topic\":\"/sys/a1ERx0fKxTG/fingerprintlock/"
            "thing/event/property/post\",\"version\":\"1.0\"}");
  Lock_state = 1;
}

void CLOSE(void) {
  TIM_SetCompare3(TIM3, 192);
  delay_ms(500);
  u3_printf("{\"method\":\"thing.event.property.post\",\"params\":{"
            "\"LockState\":0},\"topic\":\"/sys/a1ERx0fKxTG/fingerprintlock/"
            "thing/event/property/post\",\"version\":\"1.0\"}");
  Lock_state = 0;
}
