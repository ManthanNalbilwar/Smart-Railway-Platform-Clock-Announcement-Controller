//lcd.h
#include "types.h"
void INIT_LCD_16X2(void);
void WRITE_TO_LCD(u8);
void CMD_TO_LCD(u8);
void DISP_CHAR(u8);
void DISP_STRING(s8 *);
void DISP_INT(u32);
void DISP_NGTVE_INT(s32);
void DISP_FLOAT(f32,u32);
void BUILD_CGRAM(u8*,u8);
