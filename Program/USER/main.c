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
u16 Lock_state; //门锁状态

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
  TIM_SetCompare3(TIM3, 192); //修改比较值，修改占空比对应0度 默认门为关。
  delay_ms(500);
  Lock_state = 0; //门默认为关

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
  Show_Str_Mid(0, 0, "AS608指纹识别模块测试程序", 16, 240);
  POINT_COLOR = BLUE;
  Show_Str_Mid(0, 40, "与AS608模块握手....", 16, 240);
  while (PS_HandShake(&AS608Addr)) {
    delay_ms(400);
    LCD_Fill(0, 40, 240, 80, WHITE);
    Show_Str_Mid(0, 40, "未检测到模块!!!", 16, 240);
    delay_ms(800);
    LCD_Fill(0, 40, 240, 80, WHITE);
    Show_Str_Mid(0, 40, "尝试连接模块...", 16, 240);
  }
  LCD_Fill(30, 40, 240, 100, WHITE);
  Show_Str_Mid(0, 40, "通讯成功!!!", 16, 240);
  str = mymalloc(SRAMIN, 30);
  sprintf(str, "波特率:%d   地址:%x", usart2_baund, AS608Addr);
  Show_Str(0, 60, 240, 16, (u8 *)str, 16, 0);
  ensure = PS_ValidTempleteNum(&ValidN);
  if (ensure != 0x00)
    ShowErrMessage(ensure);
  ensure = PS_ReadSysPara(&AS608Para);
  if (ensure == 0x00) {
    mymemset(str, 0, 50);
    sprintf(str, "库容量:%d     对比等级: %d", AS608Para.PS_max - ValidN,
            AS608Para.PS_level);
    Show_Str(0, 80, 240, 16, (u8 *)str, 16, 0);
  } else
    ShowErrMessage(ensure);
  myfree(SRAMIN, str);
  while (1) {
    key_num = KEY_Scan(0);

    if (key_num) {
      if (key_num == 1)
        Del_FR(); //删指纹
      if (key_num == 2)
        Add_FR(); //录指纹
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

//添加指纹
void Add_FR(void) {
  u8 i, ensure, processnum = 0;

  while (1) {
    switch (processnum) {
    case 0:
      i++;
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 100, "请按指纹", 16, 240);
      ensure = PS_GetImage();
      if (ensure == 0x00) {
        BEEP = 1;
        ensure = PS_GenChar(CharBuffer1); //生成特征
        BEEP = 0;
        if (ensure == 0x00) {
          LCD_Fill(0, 120, lcddev.width, 160, WHITE);
          Show_Str_Mid(0, 120, "指纹正常", 16, 240);
          i = 0;
          processnum = 1; //跳到第二步
        }
      }
      break;

    case 1:
      i++;
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 100, "请按再按一次指纹", 16, 240);
      ensure = PS_GetImage();
      if (ensure == 0x00) {
        BEEP = 1;
        ensure = PS_GenChar(CharBuffer2); //生成特征
        BEEP = 0;
        if (ensure == 0x00) {
          LCD_Fill(0, 120, lcddev.width, 160, WHITE);
          Show_Str_Mid(0, 120, "指纹正常", 16, 240);
          i = 0;
          processnum = 2; //跳到第三步
        }
      }
      break;

    case 2:
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 100, "对比两次指纹", 16, 240);
      ensure = PS_Match();
      if (ensure == 0x00) {
        LCD_Fill(0, 120, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 120, "对比成功,两次指纹一样", 16, 240);
        processnum = 3; //跳到第四步
      } else {
        LCD_Fill(0, 100, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 100, "对比失败，请重新录入指纹", 16, 240);
        ShowErrMessage(ensure);
        i = 0;
        processnum = 0; //跳回第一步
      }
      delay_ms(1200);
      break;

    case 3:
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 100, "生成指纹模板", 16, 240);
      ensure = PS_RegModel();
      if (ensure == 0x00) {
        LCD_Fill(0, 120, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 120, "生成指纹模板成功", 16, 240);
        processnum = 4; //跳到第五步
      } else {
        processnum = 0;
      }
      delay_ms(1200);
      break;

    case 4:
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      PS_ValidTempleteNum(&ValidN);
      ID = ValidN + 1;
      ensure = PS_StoreChar(CharBuffer2, ID); //储存模板
      if (ensure == 0x00) {
        LCD_Fill(0, 100, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 120, "录入指纹成功", 16, 240);
        PS_ValidTempleteNum(&ValidN); //读库指纹个数
        LCD_ShowNum(56, 80, AS608Para.PS_max - ValidN, 3,
                    16); //显示剩余指纹容量
        delay_ms(1500);
        LCD_Fill(0, 100, 240, 160, WHITE);
        return;
      } else {
        processnum = 0;
      }
      break;
    }
    delay_ms(400);
    if (i == 5) { //超过5次没有按手指则退出
      LCD_Fill(0, 100, lcddev.width, 160, WHITE);
      break;
    }
  }
}

//刷指纹
void press_FR(void) {
  SearchResult seach;
  u8 delay_time = 0;
  u8 ensure;
  char *str;
  ensure = PS_GetImage();
  if (ensure == 0x00) { //获取图像成功
    BEEP = 1;           //打开蜂鸣器
    ensure = PS_GenChar(CharBuffer1);
    if (ensure == 0x00) { //生成特征成功
      BEEP = 0;           //关闭蜂鸣器
      ensure = PS_HighSpeedSearch(CharBuffer1, 0, AS608Para.PS_max, &seach);
      if (ensure == 0x00) { //搜索成功
        LCD_Fill(0, 100, lcddev.width, 160, WHITE);
        Show_Str_Mid(0, 100, "刷指纹成功", 16, 240);
        str = mymalloc(SRAMIN, 2000);
        sprintf(str, "确有此人,ID:%d  匹配得分:%d", seach.pageID,
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

    BEEP = 0; //关闭蜂鸣器
    delay_ms(600);
    LCD_Fill(0, 100, lcddev.width, 160, WHITE);
  }
}

//删除指纹
void Del_FR(void) {
  u8 ensure;
  u16 num = 0;
  while (1) {
    LCD_Fill(0, 100, lcddev.width, 160, WHITE);
    Show_Str_Mid(0, 100, "删除指纹", 16, 240);
    Show_Str_Mid(0, 120, "KEY2:删除,KEY_UP:返回", 16, 240);
    delay_ms(100);
    num = KEY_Scan(0);
    if (num == 3)
      goto MENU; //返回主页面
    else if (num == 2)
      ensure = PS_Empty(); //清空指纹库
    if (ensure == 0) {
      LCD_Fill(0, 120, lcddev.width, 160, WHITE);
      Show_Str_Mid(0, 140, "删除指纹成功", 16, 240);
      delay_ms(200);
      break;
    }
  }
MENU:
  PS_ValidTempleteNum(&ValidN);                          //读库指纹个数
  LCD_ShowNum(56, 80, AS608Para.PS_max - ValidN, 3, 16); //显示剩余指纹容量
  LCD_Fill(0, 100, lcddev.width, 160, WHITE);
  delay_ms(50);
}

void OPEN(void) {
  TIM_SetCompare3(TIM3, 185); //修改比较值 修改占空比为对应90度 门为开。
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
