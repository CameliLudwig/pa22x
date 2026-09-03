#ifndef  _ART_DAM3000M_SERIAL_
#define _ART_DAM3000M_SERIAL_

#include <windows.h>

//########################## 通用宏定义 ###################################

// 串口号(以此类推) 供DAM3000M_CreateDevice使用，可根据自身需要扩充
#define DAM3000M_COM1									0x01	// COM1
#define DAM3000M_COM2									0x02	// COM2
#define DAM3000M_COM3									0x03	// COM3
#define DAM3000M_COM4									0x04	// COM4
#define DAM3000M_COM5									0x05	// COM5

// 波特率选择 供DAM3000M_SetDeviceInfo和DAM3000M_GetDeviceInfo中的PDAM3000M_DEVICE_INFO使用
#define DAM3000M_BAUD_1200								0x00	// 波特率1200
#define DAM3000M_BAUD_2400								0x01	// 波特率2400
#define DAM3000M_BAUD_4800								0x02	// 波特率4800
#define DAM3000M_BAUD_9600								0x03	// 波特率9600
#define DAM3000M_BAUD_19200								0x04	// 波特率19200
#define DAM3000M_BAUD_38400								0x05	// 波特率38400
#define DAM3000M_BAUD_57600								0x06	// 波特率57600
#define DAM3000M_BAUD_115200							0x07	// 波特率115200
#define DAM3000M_BAUD_128000							0x08	// 波特率128000
#define DAM3000M_BAUD_153600							0x09	// 波特率153600
#define DAM3000M_BAUD_230400							0x0A	// 波特率230400
#define DAM3000M_BAUD_256000							0x0B	// 波特率256000
#define DAM3000M_BAUD_460800							0x0C	// 波特率460800
#define DAM3000M_BAUD_500000							0x0D	// 波特率500000
#define DAM3000M_BAUD_512000							0x0E	// 波特率512000
#define DAM3000M_BAUD_600000							0x0F	// 波特率600000
#define DAM3000M_BAUD_750000							0x10	// 波特率750000
#define DAM3000M_BAUD_921600							0x11	// 波特率921600
#define DAM3000M_BAUD_1000000							0x12	// 波特率1000000
#define DAM3000M_BAUD_150000							0x13	// 波特率150000
#define DAM3000M_BAUD_2000000							0x14	// 波特率2000000


// 波特率选择 供DAM3000M_SetDeviceInfo和DAM3000M_GetDeviceInfo中的PDAM3000M_DEVICE_INFO(bParity)使用
#define DAM3000M_PARITY_NONE							0x00	// 无校验
#define DAM3000M_PARITY_EVEN							0x01	// 偶校验
#define DAM3000M_PARITY_ODD								0x02	// 奇校验

// 流控制选择 供DAM3000M_InitDeviceEx使用
#define DAM3000M_FLOWCONTROL_NONE						0x00	// 默认不开启
#define DAM3000M_FLOWCONTROL_SOFTWARE					0x01	// 软件
#define DAM3000M_FLOWCONTROL_HARDWARERTSCTS				0x02	// 硬件RTS/CTS
#define DAM3000M_FLOWCONTROL_HARDWAREDSRDTR				0x03	// 硬件DSR/DTR

// 超时时间 供DAM3000M_InitDeviceEx使用
#define DAM3000M_DEFAULT_TIMEOUT						-1		// 默认超时时间

//重新启动电路板 供DAM3000M_Restart函数中的lpBuffer参数使用
#define DAM3000M_DEV_NORMAL								0x00	// 重新启动电路板,0:正常工作模式
#define DAM3000M_DEV_RESTART							0x01	// 重新启动电路板,1:重启电路板

//看门狗超时工作模式 供DAM3000M_WdgSetMode和DAM3000M_WdgSetMode函数中的lpBuffer参数使用
#define DAM3000M_WDG_MODE_RESET							0x00	// 看门狗超时工作模式,0:系统复位
#define DAM3000M_WDG_MODE_SAFE							0x01	// 看门狗超时工作模式,1:进入安全模式

//########################## AI ###################################
//AI传输方式 供DAM3000M_AISetTransformMode和DAM3000M_AIGetTransformMode函数中的lpBuffer参数使用 
#define DAM3000M_AI_TRANSMISSION_LINE					0x00	// AI传输方式,0:线性传输
#define DAM3000M_AI_TRANSMISSION_DIRECT					0x01	// AI传输方式,1:数据直传

//########################## AO ###################################
// 模块量输出斜率类型
#define DAM3000M_SLOPE_IMMEDIATE						0x00 // Immediate
#define DAM3000M_SLOPE_POINT125							0x01 // 0.125 mA/S
#define DAM3000M_SLOPE_POINT25							0x02 // 0.25  mA/S
#define DAM3000M_SLOPE_POINT5							0x03 // 0.5  mA/S
#define DAM3000M_SLOPE_1								0x04 // 1.0  mA/S
#define DAM3000M_SLOPE_2								0x05 // 2.0  mA/S
#define DAM3000M_SLOPE_4								0x06 // 4.0  mA/S
#define DAM3000M_SLOPE_8								0x07 // 8.0  mA/S
#define DAM3000M_SLOPE_16								0x08 // 16.0  mA/S
#define DAM3000M_SLOPE_32								0x09 // 32.0  mA/S
#define DAM3000M_SLOPE_64								0x0A // 64.0  mA/S
#define DAM3000M_SLOPE_128								0x0B // 128.0  mA/S
#define DAM3000M_SLOPE_256								0x0C // 256.0  mA/S
#define DAM3000M_SLOPE_512								0x0D // 512.0  mA/S
#define DAM3000M_SLOPE_1024								0x0E // 1024.0  mA/S
#define DAM3000M_SLOPE_2048								0x0F // 2048.0  mA/S

//########################## 模拟量输入/输出类型(量程) ###################################
// 电压 
#define DAM3000M_VOLT_N15_P15							0x01 //  -15～+15mV
#define DAM3000M_VOLT_N50_P50							0x02 //  -50～+50mV
#define DAM3000M_VOLT_N100_P100							0x03 // -100～+100mV
#define DAM3000M_VOLT_N150_P150							0x04 // -150～+150mV
#define DAM3000M_VOLT_N500_P500							0x05 // -500～+500mV
#define DAM3000M_VOLT_N1_P1								0x06 //   -1～+1V
#define DAM3000M_VOLT_N25_P25							0x07 // -2.5～+2.5V
#define DAM3000M_VOLT_N5_P5								0x08 //   -5～+5V
#define DAM3000M_VOLT_N10_P10							0x09 //  -10～+10V
#define DAM3000M_VOLT_N0_P5								0x0D //    0～+5V
#define DAM3000M_VOLT_N0_P10							0x0E //    0～+10V
#define DAM3000M_VOLT_N0_P25							0x0F //    0～+2.5V
#define DAM3000M_VOLT_N0_P15							0x55 //	  0～+15V
#define DAM3000M_VOLT_N0_P50MV							0x68 //	  0～50mV
#define DAM3000M_VOLT_N0_P100MV							0x69 //    0～100mV
#define DAM3000M_VOLT_N0_P150MV							0x6A //    0～150mV
#define DAM3000M_VOLT_N0_P500MV							0x6B //    0～500mV
#define DAM3000M_VOLT_N35_P0							0x7E //  -35～0V
#define DAM3000M_VOLT_N35_P35							0x7F //  -35～35V
#define DAM3000M_VOLT_N0_P35							0x72 //    0～35V
#define DAM3000M_VOLT_N0_P50							0x65 //    0～50V
#define DAM3000M_VOLT_N50_P0							0x66 //   -50～0V
#define DAM3000M_VOLT_N50V_P50V							0x67 //    -50～50V
#define DAM3000M_VOLT_N0V_P60V							0x5C //    0～60V
#define DAM3000M_VOLT_N60V_P0V							0x5D //   -60～0V
#define DAM3000M_VOLT_N60V_P60V							0x5E //    -60～60V
#define DAM3000M_VOLT_N0_P400MV							0x76 //    0～+400mV
#define DAM3000M_VOLT_N20_P20							0x81 //  -20～+20mV
#define DAM3000M_VOLT_N1_P5								0x82 //    1～+5V
#define DAM3000M_VOLT_N30_P30V							0x83 //  -30～+30V
#define DAM3000M_VOLT_N0_P30V							0x84 //    0～+30V
#define DAM3000M_VOLT_N0_P1								0x89 //    0～+1V
#define DAM3000M_VOLT_N500_P500V						0x8A // -500～+500V
#define DAM3000M_VOLT_N30_P30							0x8B //  -30～+30mV
#define DAM3000M_VOLT_N0_P12							0x8C //    0～12V
#define DAM3000M_VOLT_N0_P20							0x8D //    0～20V
#define DAM3000M_VOLT_N20_P0V							0x8E //  -20～0V
#define DAM3000M_VOLT_N20_P20V							0x8F //  -20～20V
#define DAM3000M_VOLT_N0_P300V							0x62 //  0～300V
#define DAM3000M_VOLT_N300_P0V							0x63 //  -300～0V
#define DAM3000M_VOLT_N300_P300V						0x64 //  -300～300V
#define DAM3000M_VOLT_N12_P12V							0x92 //  -12～12V
#define DAM3000M_ACVOLT_N0_P35V							0x32 //  交流0~35V
#define DAM3000M_ACVOLT_N0_P50V							0x33 //  交流0~50V
#define DAM3000M_ACVOLT_N0_P220V						0x34 //  交流0~220V
#define DAM3000M_ACVOLT_N0_P400V						0x35 //  交流0~400V
#define DAM3000M_ACVOLT_N0_P500MV						0x36 //  交流0~500mV
#define DAM3000M_VOLT_N10_P0V							0x5F //  -10～0V
#define DAM3000M_VOLT_N0V_P100V						    0x56// 0～+100V
#define DAM3000M_VOLT_N100V_P0V						    0x57// -100～+0V
#define DAM3000M_VOLT_N100V_P100V						0x58 // -100～+100V
#define DAM3000M_VOLT_N400_P400MV						0x4d // -400～+400mV
#define DAM3000M_VOLT_N0_P30MV							0x4c// 0～+30mV
#define DAM3000M_VOLT_N0_P20MV							0x4a// 0～+20mV
#define DAM3000M_VOLT_N0_P15MV							0x4b// 0～+15mV

// 电流 
#define DAM3000M_CUR_N0_P10								0x00 //    0～10mA
#define DAM3000M_CUR_N20_P20							0x0A //  -20～+20mA
#define DAM3000M_CUR_N0_P20								0x0B //    0～20mA
#define DAM3000M_CUR_N4_P20								0x0C //    4～20mA
#define DAM3000M_CUR_N0_P5A								0x59 //    0～5A
#define DAM3000M_CUR_N5_P0A								0x5A //    -5～0A
#define DAM3000M_CUR_N5_P5A								0x5B //    -5～5A
#define DAM3000M_CUR_N0_P1000				    		0x6C //    0～1000mA
#define DAM3000M_CUR_N1000_P0				    		0x6D //-1000～0mA
#define DAM3000M_CUR_N1000_P1000						0x6E //-1000～1000mA
#define DAM3000M_CUR_N10_P0A							0x6F //  -10～+0A
#define DAM3000M_CUR_N0_P200							0x73 //    0～200mA
#define DAM3000M_CUR_N0_P500							0x74 //    0～500mA
#define DAM3000M_CUR_N0_P1500							0x75 //    0～1500mA(0~1.5A)
#define DAM3000M_CUR_N0_P400UA							0x77 //    0～+400uA
#define DAM3000M_CUR_N1500_P0							0x78 //-1500～0mA
#define DAM3000M_CUR_N1500_P1500						0x79 //-1500～1500mA
#define DAM3000M_CUR_N500_P0							0x7A // -500～0mA
#define DAM3000M_CUR_N500_P500							0x7B // -500～500mA
#define DAM3000M_CUR_N200_P0							0x7C // -200～0mA
#define DAM3000M_CUR_N200_P200							0x7D // -200～200mA
#define DAM3000M_CUR_N0_P22								0x80 //    0～22mA
#define DAM3000M_CUR_N10_P10A							0x85 //  -10～+10A
#define DAM3000M_CUR_N0_P10A							0x86 //    0～+10A
#define DAM3000M_CUR_N50_P50A							0x87 //  -50～+50A
#define DAM3000M_CUR_N0_P50A							0x88 //    0～+50A
#define DAM3000M_CUR_N0_P250							0x50 //    0～250mA
#define DAM3000M_CUR_N250_P0							0x51 //    -250～0mA
#define DAM3000M_CUR_N250_P250							0x52 //    -250～250mA
#define DAM3000M_CUR_N0_P3A								0x93 //    0～3A
#define DAM3000M_CUR_N3_P0A								0x94 //    -3～0A
#define DAM3000M_CUR_N3_P3A								0x95 //    -3～3A
#define DAM3000M_CUR_N0_P50								0x61 //    0～50mA
#define DAM3000M_CUR_N50_P50							0x4f //    -50～50mA
#define DAM3000M_CUR_N10_P10							0x4e //    -10～10mA



// 热电偶 
#define DAM3000M_TMC_J									0x10 // J型热电偶	 0～1200℃
#define DAM3000M_TMC_K									0x11 // K型热电偶    0～1300℃
#define DAM3000M_TMC_T									0x12 // T型热电偶 -200～400℃
#define DAM3000M_TMC_E									0x13 // E型热电偶    0～1000℃
#define DAM3000M_TMC_R									0x14 // R型热电偶    0～1700℃
#define DAM3000M_TMC_S									0x15 // S型热电偶    0～1768℃
#define DAM3000M_TMC_B									0x16 // B型热电偶    0～1800℃
#define DAM3000M_TMC_N									0x17 // N型热电偶    0～1300℃
#define DAM3000M_TMC_C									0x18 // C型热电偶    0～2090℃
#define DAM3000M_TMC_WRE								0x19 // 钨铼5-钨铼26 0～2310℃
#define DAM3000M_TMC_K_EX								0x70 // K型热电偶  -40～1300℃
#define DAM3000M_TMC_B_N								0x71 // B型热电偶  250～1800℃
#define DAM3000M_NTC_10K								0x90 // NTC10K     0～100℃
#define DAM3000M_NTC_100K								0x91 // NTC100K    0～100℃
#define DAM3000M_RN0_10K								0x99 // 0~10kΩ
#define DAM3000M_RN0_50K								0x9A // 0~50kΩ
#define DAM3000M_RN0_200								0x9B // 0~200Ω
#define DAM3000M_RN0_250K								0x9C // 0~250kΩ




// 热电阻(电阻)
#define DAM3000M_RTD_PT100_385_N200_P600				0x20 // Pt100(385)热电阻 -200℃～600℃
#define DAM3000M_RTD_PT100_385_N100_P100				0x21 // Pt100(385)热电阻 -100℃～100℃
#define DAM3000M_RTD_PT100_385_N0_P100					0x22 // Pt100(385)热电阻    0℃～100℃
#define DAM3000M_RTD_PT100_385_N0_P200					0x23 // Pt100(385)热电阻    0℃～200℃
#define DAM3000M_RTD_PT100_385_N0_P600					0x24 // Pt100(385)热电阻    0℃～600℃
#define DAM3000M_RTD_PT100_3916_N200_P600				0x25 // Pt100(3916)热电阻-200℃～600℃
#define DAM3000M_RTD_PT100_3916_N100_P100				0x26 // Pt100(3916)热电阻-100℃～100℃
#define DAM3000M_RTD_PT100_3916_N0_P100					0x27 // Pt100(3916)热电阻   0℃～100℃
#define DAM3000M_RTD_PT100_3916_N0_P200					0x28 // Pt100(3916)热电阻   0℃～200℃
#define DAM3000M_RTD_PT100_3916_N0_P600					0x29 // Pt100(3916)热电阻   0℃～600℃
#define DAM3000M_RTD_PT1000								0x30 // Pt1000热电阻     -200℃～600℃
#define DAM3000M_RTD_PT1000_N200_P850					0x31 // Pt1000热电阻     -200℃～850℃
#define DAM3000M_RTD_CU50								0x40 // Cu50热电阻        -50℃～150℃
#define DAM3000M_RTD_CU100								0x41 // Cu100热电阻       -50℃～150℃
#define DAM3000M_RTD_BA1								0x42 // BA1热电阻        -200℃～650℃
#define DAM3000M_RTD_BA2								0x43 // BA2热电阻        -200℃～650℃
#define DAM3000M_RTD_G53								0x44 // G53热电阻         -50℃～150℃
#define DAM3000M_RTD_Ni50								0x45 // Ni50热电阻        100℃
#define DAM3000M_RTD_Ni508								0x46 // Ni508热电阻         0℃～100℃
#define DAM3000M_RTD_Ni1000								0x47 // Ni1000热电阻      -60℃～160℃
#define DAM3000M_RTD_103AT								0x60 // 103AT电阻		  -50℃～110℃

#define DAM3000M_R_N0_R400Ω								0x88 // 远传压力表电阻   0～400Ω

// YH协议电压
#define DAM3000M_VOLT_YH_N25_P25						0x05 // YH协议下	-2.5～+2.5V
#define DAM3000M_VOLT_YH_N10_P10						0x08 // YH协议下	 -10～+10V
#define DAM3000M_VOLT_YH_N5_P5							0x09 // YH协议下	  -5～+5V
#define DAM3000M_VOLT_YH_N1_P1							0x0A // YH协议下	  -1～+1V
#define DAM3000M_VOLT_YH_N0_P500V						0x4B // YH协议下	0～+500mV
#define DAM3000M_VOLT_YH_N500_P500V						0x0B // YH协议下	-500～+500mV
#define DAM3000M_VOLT_YH_N150_P150V						0x0C // YH协议下	-150～+150mV
#define DAM3000M_VOLT_YH_N0_P150V						0x4C // YH协议下	-0～+150mV
#define DAM3000M_VOLT_YH_N15_P15						0x15 // YH协议下	 -15～+15V
#define DAM3000M_VOLT_YH_N0_P25							0x45 // YH协议下	   0～+2.5V
#define DAM3000M_VOLT_YH_N0_P10							0x48 // YH协议下	   0～+10V
#define DAM3000M_VOLT_YH_N0_P5							0x49 // YH协议下	   0～+5V
#define DAM3000M_VOLT_YH_N0_P1							0x4A // YH协议下	   0～+1V
#define DAM3000M_VOLT_YH_N0_P15							0x55 // YH协议下	   0～+15V

// YH协议电流
#define DAM3000M_CUR_YH_N4_P20A							0x07 // YH协议下	  4～20mA
#define DAM3000M_CUR_YH_N20_P20A						0x0D // YH协议下	  -20～20mA
#define DAM3000M_CUR_YH_N0_P20A							0x4D // YH协议下	  0～20mA






//########################## DI ###################################
// DI工作模式 供DAM3000M_DISetMode和DAM3000M_DIGetMode函数中的lpBuffer参数使用
#define DAM3000M_DI_MODE_INPUT							0x00	// DI工作模式,0:DI输入
#define DAM3000M_DI_MODE_CNT							0x01	// DI工作模式,1:计数
#define DAM3000M_DI_MODE_LATCH_L_H						0x02	// DI工作模式,2:低到高锁存
#define DAM3000M_DI_MODE_LATCH_H_L						0x03	// DI工作模式,3:高到低锁存
#define DAM3000M_DI_MODE_FREQ							0x04	// DI工作模式,4:频率工作模式

// DI计数方式 供DAM3000M_SetDeviceMode函数中的lEdgeMode参数使用
#define DAM3000M_DIR_FALLING							0x00	// 下降沿
#define DAM3000M_DIR_RISING								0x01	// 上升沿

//##########################DO###################################
// DO工作模式 供DAM3000M_DOSetMode和DAM3000M_DOGetMode函数中的lMode参数使用
#define DAM3000M_DO_MODE_OUTPUT_IMMED					0x00	// DO工作模式,0:立即输出模式
#define DAM3000M_DO_MODE_OUTPUT_DELAY_L_H				0x01	// DO工作模式,1:低到高延时输出
#define DAM3000M_DO_MODE_OUTPUT_DELAY_H_L               0x02	// DO工作模式,2:高到低延时输出
#define DAM3000M_DO_MODE_OUTPUT_PULSE					0x03	// DO工作模式,3:连续脉冲输出
#define DAM3000M_DO_MODE_OUTPUT_NON_LOCK				0x05	// DO工作模式,5:本机非锁联动
#define DAM3000M_DO_MODE_OUTPUT_SELF_LOCK				0x06	// DO工作模式,6:本机自锁联动
#define DAM3000M_DO_MODE_OUTPUT_INTERLOCK				0x04	// DO工作模式,7:互锁联动
#define DAM3000M_DO_MODE_OUTPUT_NON_LOCK_DOUBLE			0x08	// DO工作模式,8:双机非锁联动
#define DAM3000M_DO_MODE_OUTPUT_SELF_LOCK_DOUBLE		0x09	// DO工作模式,9:双机自锁联动
#define DAM3000M_DO_MODE_OUTPUT_CLICK_DELAY				0x0A	// DO工作模式,10:点动延时
#define DAM3000M_DO_MODE_OUTPUT_PULSE_FLASH_OFF			0x0B	// DO工作模式,11:闪闭模式
#define DAM3000M_DO_MODE_OUTPUT_PULSE_FLASH_BREAK		0x0C	// DO工作模式,12:闪断模式

// DO脉冲输出模式 供DAM3000M_DOSetPulseMode和DAM3000M_DOGetPulseMode函数中的lpBuffer参数使用
#define DAM3000M_DO_PULSE_MODE_CONTINUE                 0x00	//DO脉冲冲输出模式,0:继续输出
#define DAM3000M_DO_PULSE_MODE_NONCONTINUE              0x01	//DO脉冲冲输出模式,1:非继续输出

//##########################计数器###################################
// 模块的工作模式 供DAM3000M_SetDevWorkMode函数中的lMode参数使用
#define DAM3000M_WORKMODE_CNT							0x00	// 计数器
#define DAM3000M_WORKMODE_FREQ							0x01	// 频率器

// 计数器/频率的输入方式 供DAM3000M_PARA_CNT结构体中的lInputMode参数使用
#define DAM3000M_UNISOLATED								0x00	// 非隔离
#define DAM3000M_ISOLATED								0x01	// 隔离

// 门槛值状态 供DAM3000M_PARA_CNT结构体中的GateSts参数使用
#define DAM3000M_GATE_LOW								0x00	// 门槛值为低电平
#define DAM3000M_GATE_HIGH								0x01	// 门槛值为高电平
#define DAM3000M_GATE_NULL								0x02	// 门槛值无效

// 报警方式 供DAM3000M_CNT_ALARM结构体中的AlarmMode参数使用
#define DAM3000M_CNT_ALARM_MODE0						0x00	// 报警方式0	0通道-1通道上限
#define DAM3000M_CNT_ALARM_MODE1						0x01	// 报警方式1	0通道上限 / 上上限

// 报警方式0使能 供DAM3000M_CNT_ALARM结构体中的EnableAlarm0 和 EnableAlarm1参数使用
#define DAM3000M_CNT_ALAMODE0_DISABLE					0x00	// 报警方式0禁止报警
#define DAM3000M_CNT_ALAMODE0_ENABLE					0x01	// 报警方式0允许报警

// 报警方式1使能 供DAM3000M_CNT_ALARM结构体中的EnableAlarm0参数使用
#define DAM3000M_CNT_ALAMODE1_DISABLE					0x00	// 报警方式1 计数器0 禁止报警
#define DAM3000M_CNT_ALAMODE1_INSTANT					0x01	// 报警方式1 计数器0 瞬间报警允许
#define DAM3000M_CNT_ALAMODE1_LATCH						0x02	// 报警方式1 计数器0 闭锁报警允许

// 滤波状态使能 供DAM3000M_PARA_FILTER结构体中的bEnableFilter参数使用
#define DAM3000M_FILTER_DISABLE							0x00	// 禁止滤波
#define DAM3000M_FILTER_ENABLE							0x01	// 允许滤波

//########################## 电量 ###################################
// 获得电量值 供DAM3000M_GetEnergyVal中的lAanlogType参数使用
#define DAM3000M_GET_I_RMS								0x00	// 获得电流有效值
#define DAM3000M_GET_V_RMS								0x01	// 获得电压有效值
#define DAM3000M_GET_PHVOLTAGE							0x01	// 获得电压有效值、相电压
#define DAM3000M_GET_POWER								0x02	// 获得有功功率
#define DAM3000M_GET_VAR								0x03	// 获得无功功率
#define DAM3000M_GET_VA									0x04	// 获得视在功率
#define DAM3000M_GET_WATTHR								0x05	// 获得正相有功电度
#define DAM3000M_GET_RWATTHR							0x06	// 获得反相有功电度
#define DAM3000M_GET_VARHR								0x07	// 获得正相无功电度
#define DAM3000M_GET_RVARHR								0x08	// 获得反相无功电度
#define DAM3000M_GET_PF									0x09	// 获得功率因数
#define DAM3000M_GET_FREQ								0x0A	// 获得输入信号频率
#define DAM3000M_GET_VAWATTHR							0x0B	// 获得电度
#define DAM3000M_GET_LINEVOLTAGE						0x0C	// 获得线电压

//########################## ADC采样速率 ###################################
#define DAM3000M_ADCRATE_500							0x00 // 500Hz
#define DAM3000M_ADCRATE_1K								0x01 // 1KHz
#define DAM3000M_ADCRATE_2K								0x02 // 2KHz
#define DAM3000M_ADCRATE_10								0x04 // 10Hz
#define DAM3000M_ADCRATE_100							0x05 // 100Hz



//###########################################################################
// ****************** 设备基本信息的结构体 ******************************
typedef struct _DAM3000M_DEVICE_INFO
{
	LONG    DeviceType;		// 模块类型 
	LONG    TypeSuffix;		// 类型后缀
	LONG	ModusType;		// M
	LONG	VesionID;		// 版本号(2字节)
	LONG	DeviceID;		// 模块ID号(SetDeviceInfo时，为设备的新ID)
	LONG	BaudRate;		// 波特率
	LONG	bParity;		// 0:无校验 1:偶校验 2:奇校验(只有这3个值才能表示该模块有可能支持此功能，设置时此值不为0 1 2表示不设置此参数)
} DAM3000M_DEVICE_INFO, *PDAM3000M_DEVICE_INFO;


// 驱动函数接口
#ifndef DEFINING
#define DEVAPI __declspec(dllimport)
#else
#define DEVAPI //__declspec(dllexport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	//###########################################################################################
	//####################### 设备对象管理函数 ##################################################
	//###########################################################################################

	//****************************************************************************	
	// 说明：创建设备对象
	// 参数：返回句柄，供与模块通信使用
	//****************************************************************************	
	HANDLE DEVAPI FAR PASCAL DAM3000M_CreateDevice(
								LONG lPortNum);			// 串口号

	//****************************************************************************	
	// 说明：初始化串口（初始与模块之间的通信参数）
	// 参数：波特率、奇偶校验、超时时间
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_InitDevice(
											HANDLE hDevice,										// 设备对象句柄
											LONG lBaud,											// 波特率
											LONG lDataBits,										// 数据位
											LONG lStopBits,										// 停止位
											LONG lParity,										// 校验方式
											LONG lTimeOut = DAM3000M_DEFAULT_TIMEOUT,			// 超时时间，主要用于接收数据，如果为-1 则使用默认超时时间
											LONG lFlowControl = DAM3000M_FLOWCONTROL_NONE);		// 流控制 默认不开启

	//****************************************************************************	
	// 说明：释放设备对象（串口），执行后无法操作串口
	// 参数：句柄
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReleaseDevice(HANDLE hDevice);		 

	//****************************************************************************	
	// 说明：复位设置（恢复出厂设置）
	// 功能码：0x10
	// 地址：0x207 
	//****************************************************************************	
	BOOL DEVAPI FAR PASCAL DAM3000M_ResetDevice(						 
									HANDLE	hDevice,	// 设备对象句柄
									LONG	lDeviceID,	// 设备地址
									BOOL	bReset);	// 复位设备标志 =TRUE 复位 =FALSE 无操作

	//****************************************************************************	
	// 使模块进入(退出)校准模式（保留功能，不推荐用户使用）
	// 功能码：0x10
	// 地址：0x208 
	//****************************************************************************	
	BOOL DEVAPI FAR PASCAL DAM3000M_EnterCalibration( 
									HANDLE	hDevice,	// 设备对象句柄
									LONG	lDeviceID,	// 设备地址
									BOOL	bCalibrate);// 进入校准标志  =TRUE 进入校准模式 =FALSE 不操作

	//###########################################################################################
	//################################### dll版本获得 #################################
	//###########################################################################################
	//######################################################################
	// 获取DLL版本号
	BOOL DEVAPI	FAR	PASCAL DAM3000M_GetDllVersion(int &iVersion);

	//###########################################################################################
	//################################### 模块信息取得/修改函数 #################################
	//###########################################################################################
	
	//****************************************************************************	
	// 读取模块信息(类型、地址、波特率、校验模式等)
	// 功能码：0x03
	// 地址：0x80 
	//****************************************************************************	
	BOOL DEVAPI FAR PASCAL	DAM3000M_GetDeviceInfo(				
									HANDLE	hDevice,				// 设备对象句柄
									LONG	lDeviceID,				// 设备地址
									PDAM3000M_DEVICE_INFO pInfo);	// 设备信息

	//****************************************************************************	
	// 修改模块信息(地址、波特率、校验)
	// 功能码：0x10
	// 地址：0x80 
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_SetDeviceInfo(						 
									HANDLE	hDevice,				// 设备对象句柄
									LONG	lDeviceID,				// 设备地址
									DAM3000M_DEVICE_INFO& Info);	// 设备信息


	//###########################################################################################
	//####################################### Modus 功能操作函数 ################################
	//#如函数无特别说明，默认的数据通信方式为：寄存器外小端，寄存器内大端########################
	//#即：寄存器内数据先发送高位；当两个寄存器组成一个32位数据时，低16位在寄存器小地址##########
	//#具体寄存器地址，请遵照模块用户手册寄存器定义##############################################

	//###########################################################################################

	// ## 01功能码 ######
	// ## 01功能码 ######
	//****************************************************************************	
	// 读继电器状态
	// 功能码：0x01
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN]设备句柄，由CreateDevice创建
	// 		lDeviceID [IN]设备ID号
	// 		addr [IN]通信地址，写入地址减1
	// 		len [IN]读取寄存器个数
	// 		flag [OUT]返回读到的寄存器的数据（0或1）
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadCoils(							 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									BYTE	flag[]);


	// ## 02功能码 ######
	// ## 02功能码 ######
	//****************************************************************************	
	// 读开关量输入
	// 功能码：0x02
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN]设备句柄，由CreateDevice创建
	// 		lDeviceID [IN]设备ID号
	// 		addr [IN]通信地址，写入地址减1
	// 		len [IN]读取寄存器个数
	// 		state [OUT]返回读到的寄存器的数据（0或1）
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadDiscretes(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									BYTE	state[]);

	// ## 03功能码 ######
	// ## 03功能码 ######
	//****************************************************************************	
	// 读保持寄存器，返回数据8位无符号整型
	// 功能码：0x03
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取寄存器个数
	// 		buf [OUT] 返回读到的寄存器的数据,，每个寄存器占用2个元素。
	//				  buf[0]表示第一个寄存器的高位，buf[1]表示第一个寄存器的低位，以此类推。
	// 				  如，第一个寄存器数据内容为0x1234，则buf[0]=0x12 buf[1]=0x34。 	
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadMultiRegs(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									BYTE	buf[]);

	//****************************************************************************	
	// 读保持寄存器，返回数据16位无符号整型
	// 功能码：0x03
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [OUT] 返回读到的寄存器的数据，每个寄存器占用一个元素。
	//				  buf[0]表示第一个地址对应寄存器的值，以此类推。
	//				  如，第一个寄存器的数据内容是0x1234，则返回的buf[0]=0x1234
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadMultiRegsUInt16(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									USHORT	buf[]);


	//****************************************************************************	
	// 读保持寄存器，返回数据32位无符号整形
	// 功能码：0x03
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [OUT] 返回读到的寄存器的数据，每两个寄存器值组成一个元素值。
	//				  buf[0]表示第一个地址和第二个地址对应寄存器的值，以此类推。
	//				  如，第一个寄存器的数据内容是0x1234，第二个寄存器的数据内容是0x5678，则返回的buf[0]=0x56781234
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadMultiRegsUInt32(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									ULONG	buf[]);

	//****************************************************************************	
	// 读保持寄存器，返回数据32位浮点数
	// 功能码：0x03
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [OUT] 返回读到的寄存器的数据，每两个寄存器值组成一个元素值。
	//				  buf[0]表示第一个地址和第二个地址对应寄存器的值，以此类推。
	//				  如，第一个寄存器的数据内容是0x1234，第二个寄存器的数据内容是0x5678，则返回的buf[0]=0x56781234
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadMultiRegsFloat32(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									float	buf[]);

	//****************************************************************************	
	// 读保持寄存器，返回数据32位无符号整型，寄存器外为大端发送
	// 功能码：0x03
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [OUT] 返回读到的寄存器的数据，每两个寄存器值组成一个元素值。
	//				  buf[0]表示第一个地址和第二个地址对应寄存器的值，以此类推。
	//				  如，第一个寄存器的数据内容是0x1234，第二个寄存器的数据内容是0x5678，则返回的buf[0]=0x12345678
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadMultiRegsUInt32_BigEndian(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									ULONG	buf[]);


	// ## 04功能码 ######
	// ## 04功能码 ######
	//****************************************************************************	
	// 读输入寄存器，返回数据8位无符号整型
	// 功能码：0x04
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN]设备句柄，由CreateDevice创建
	// 		lDeviceID [IN]设备ID号
	// 		addr [IN]通信地址，写入地址减1
	// 		len [IN]读取数据个数
	// 		buf [OUT] 返回读到的寄存器的数据,，每个寄存器占用2个元素。
	//				  buf[0]表示第一个寄存器的高位，buf[1]表示第一个寄存器的低位，以此类推。
	// 				  如，第一个寄存器数据内容为0x1234，则buf[0]=0x12 buf[1]=0x34。 	
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadInputRegs(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									BYTE	buf[]);

	//****************************************************************************	
	// 读输入寄存器，返回数据16位无符号整型
	// 功能码：0x04
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [OUT] 返回读到的寄存器的数据，每个寄存器占用一个元素。
	//				  buf[0]表示第一个地址对应寄存器的值，以此类推。
	//				  如，第一个寄存器的数据内容是0x1234，则返回的buf[0]=0x1234
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadInputRegsUInt16(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									USHORT	buf[]);


	//****************************************************************************	
	// 读输入寄存器，返回数据32位无符号整型
	// 功能码：0x04
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [OUT] 返回读到的寄存器的数据，每两个寄存器占用一个元素。
	//				  buf[0]表示第一个地址和第二个地址对应寄存器的值，以此类推。
	//				  如，第一个寄存器的数据内容是0x1234，第二个寄存器的数据内容是0x5678，则返回的buf[0]=0x56781234
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadInputRegsUInt32(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									ULONG	buf[]);

	//****************************************************************************	
	// 读输入寄存器,返回数据32位浮点数
	// 功能码：0x04
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [OUT] 返回读到的寄存器的数据，每两个寄存器占用一个元素。
	//				  buf[0]表示第一个地址和第二个地址对应寄存器的值，以此类推。
	//				  如，第一个寄存器的数据内容是0x1234，第二个寄存器的数据内容是0x5678，则返回的buf[0]=56781234
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ReadInputRegsFloat32(						 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									float	buf[]);


	// ## 05功能码 ######
	// ## 05功能码 ######
	//****************************************************************************
	// 设置单个继电器
	// 功能码：0x05
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		status [IN] 写入寄存器的值，0或1
	//****************************************************************************
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteCoil(
									HANDLE	hDevice,
									LONG	lDeviceID,
									int		addr,
									BYTE	status);


	// ## 0F功能码 ######
	// ## 0F功能码 ######
	//****************************************************************************	
	// 设置多个继电器
	// 功能码：0x0F
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		status [IN] 写入寄存器的值，0或1
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_ForceMultiCoils(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									BYTE	status[]);


	// ## 10功能码 ######
	// ## 10功能码 ######
	//****************************************************************************	
	// 设置单个保持寄存器
	// 功能码：0x10
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		val [IN] 写入寄存器的数据
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteSingleReg(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									USHORT	val);


	//****************************************************************************	
	// 设置多个保持寄存器
	// 功能码：0x10
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	//				 数据赋值示例：写入第一个寄存器的值为0x1234，那么buf[0]=0x12 buf[1]=0x34
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteMultiRegs(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									BYTE	buf[]);

	//****************************************************************************	
	// 设置多个保持寄存器
	// 功能码：0x10
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入寄存器的值为0x1234，那么buf[0]=0x12 buf[1]=0x34
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteMultiRegsUInt16(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									USHORT	buf[]);

	//****************************************************************************	
	// 设置多个保持寄存器
	// 功能码：0x10
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入第一个寄存器的值为0x1234，第二个寄存器的值为0x5678，那么buf[0]=0x56781234
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteMultiRegsUInt32(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									ULONG	buf[]);

	//****************************************************************************	
	// 设置多个保持寄存器，寄存器外为大端发送
	// 功能码：0x10
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入第一个寄存器的值为0x1234，第二个寄存器的值为0x5678，那么buf[0]=0x12345678
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteMultiRegsUInt32_BigEndian(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									ULONG	buf[]);

	//****************************************************************************	
	// 设置多个保持寄存器
	// 功能码：0x10
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入的浮点值为3.14159，那么buf[0]=3.14159
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteMultiRegsFloat32(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									float	buf[]);

	//****************************************************************************	
	// 设置多个输出寄存器
	// 功能码：0x06
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入的浮点值为3.14159，那么buf[0]=3.14159
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL DAM3000M_WriteOutPutRegs(
									HANDLE hDevice, 
									LONG lDeviceID, 
									int addr,
									int len, 
									byte buf[]);
	
	//****************************************************************************	
	// 设置多个输出寄存器
	// 功能码：0x06
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入的浮点值为3.14159，那么buf[0]=3.14159
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteOutPutRegsFloat32(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									float	buf[]);

	//****************************************************************************	
	// 设置多个输出寄存器
	// 功能码：0x06
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入的浮点值为3.14159，那么buf[0]=3.14159
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteOutPutRegsUInt16(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									USHORT	buf[]);

	//****************************************************************************	
	// 设置多个输出寄存器
	// 功能码：0x06
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入的浮点值为3.14159，那么buf[0]=3.14159
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteOutPutRegsUInt32(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									ULONG	buf[]);

	//****************************************************************************	
	// 设置多个输出寄存器
	// 功能码：0x06
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		len [IN] 读取数据个数
	// 		buf [IN] 写入寄存器的数据
	// 				数据赋值示例：写入的浮点值为3.14159，那么buf[0]=3.14159
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteOutPutRegsUInt32_BigEndian(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									int		len, 
									ULONG	buf[]);

	// ## 06功能码 ######
	// ## 06功能码 ######
	//****************************************************************************	
	// 设置单个保持寄存器
	// 功能码：0x06
	// 返回值：TRUE=成功 FALSE=失败
	// 参数：
	// 		hDevice [IN] 设备句柄，由CreateDevice创建
	// 		lDeviceID [IN] 设备ID号
	// 		addr [IN] 通信地址，写入地址减1
	// 		val [IN] 写入寄存器的数据
	//****************************************************************************	
	BOOL DEVAPI	FAR	PASCAL	DAM3000M_WriteOutPutReg(					 
									HANDLE	hDevice, 
									LONG	lDeviceID, 
									int		addr, 
									USHORT	val);

#ifdef __cplusplus
}
#endif

//#######################################################################
// 自动包含驱动函数导入库
#ifndef DEFINING
#ifndef _WIN64
#pragma comment(lib, "DAM3000M_32.lib")
#pragma message("======== Welcome to use our art company's products!")
#pragma message("======== Automatically linking with DAM3000M_32.dll...")
#pragma message("======== Successfully linked with DAM3000M_32.dll")
#else
#pragma comment(lib, "DAM3000M_64.lib")
#pragma message("======== Welcome to use our art company's products!")
#pragma message("======== Automatically linking with DAM3000M_64.dll...")
#pragma message("======== Successfully linked with DAM3000M_64.dll")
#endif // _WIN64
#endif // DEFINING

#endif // ifndef _ART_DAM3000M_SERIAL_
