#ifndef SE_GBA_H
#define SE_GBA_H 1

#include "sb_types.h"
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "arm7.h"
#include "gba_bios.h"
#include <time.h>

// 串口自动查找所需头文件
// 注意：Windows 头文件必须在其他标准头文件之后包含
#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <setupapi.h>
  #include <devguid.h>
  #include <regstr.h>
  #pragma comment(lib, "setupapi.lib")
#elif defined(__APPLE__)
  #include <CoreFoundation/CoreFoundation.h>
  #include <IOKit/IOKitLib.h>
  #include <IOKit/serial/IOSerialKeys.h>
  #include <IOKit/usb/IOUSBLib.h>
#elif defined(__linux__)
  #include <dirent.h>
#endif
//Should be power of 2 for perf, 8192 samples gives ~85ms maximal latency for 48kHz
#define LR 14
#define PC 15
#define CPSR 16
#define SPSR 17       
#define GBA_LCD_W 240
#define GBA_LCD_H 160
#define GBA_SWAPCHAIN_SIZE 4 
#define GBA_AUDIO_DMA_ACTIVATE_THRESHOLD 12

//////////////////////////////////////////////////////////////////////////////////////////
// MMIO Register listing from GBATEK (https://problemkaputt.de/gbatek.htm#gbamemorymap) //
//////////////////////////////////////////////////////////////////////////////////////////
// LCD MMIO Registers
#define  GBA_DISPCNT  0x4000000  /* R/W LCD Control */
#define  GBA_GREENSWP 0x4000002  /* R/W Undocumented - Green Swap */
#define  GBA_DISPSTAT 0x4000004  /* R/W General LCD Status (STAT,LYC) */
#define  GBA_VCOUNT   0x4000006  /* R   Vertical Counter (LY) */
#define  GBA_BG0CNT   0x4000008  /* R/W BG0 Control */
#define  GBA_BG1CNT   0x400000A  /* R/W BG1 Control */
#define  GBA_BG2CNT   0x400000C  /* R/W BG2 Control */
#define  GBA_BG3CNT   0x400000E  /* R/W BG3 Control */
#define  GBA_BG0HOFS  0x4000010  /* W   BG0 X-Offset */
#define  GBA_BG0VOFS  0x4000012  /* W   BG0 Y-Offset */
#define  GBA_BG1HOFS  0x4000014  /* W   BG1 X-Offset */
#define  GBA_BG1VOFS  0x4000016  /* W   BG1 Y-Offset */
#define  GBA_BG2HOFS  0x4000018  /* W   BG2 X-Offset */
#define  GBA_BG2VOFS  0x400001A  /* W   BG2 Y-Offset */
#define  GBA_BG3HOFS  0x400001C  /* W   BG3 X-Offset */
#define  GBA_BG3VOFS  0x400001E  /* W   BG3 Y-Offset */
#define  GBA_BG2PA    0x4000020  /* W   BG2 Rotation/Scaling Parameter A (dx) */
#define  GBA_BG2PB    0x4000022  /* W   BG2 Rotation/Scaling Parameter B (dmx) */
#define  GBA_BG2PC    0x4000024  /* W   BG2 Rotation/Scaling Parameter C (dy) */
#define  GBA_BG2PD    0x4000026  /* W   BG2 Rotation/Scaling Parameter D (dmy) */
#define  GBA_BG2X     0x4000028  /* W   BG2 Reference Point X-Coordinate */
#define  GBA_BG2Y     0x400002C  /* W   BG2 Reference Point Y-Coordinate */
#define  GBA_BG3PA    0x4000030  /* W   BG3 Rotation/Scaling Parameter A (dx) */
#define  GBA_BG3PB    0x4000032  /* W   BG3 Rotation/Scaling Parameter B (dmx) */
#define  GBA_BG3PC    0x4000034  /* W   BG3 Rotation/Scaling Parameter C (dy) */
#define  GBA_BG3PD    0x4000036  /* W   BG3 Rotation/Scaling Parameter D (dmy) */
#define  GBA_BG3X     0x4000038  /* W   BG3 Reference Point X-Coordinate */
#define  GBA_BG3Y     0x400003C  /* W   BG3 Reference Point Y-Coordinate */
#define  GBA_WIN0H    0x4000040  /* W   Window 0 Horizontal Dimensions */
#define  GBA_WIN1H    0x4000042  /* W   Window 1 Horizontal Dimensions */
#define  GBA_WIN0V    0x4000044  /* W   Window 0 Vertical Dimensions */
#define  GBA_WIN1V    0x4000046  /* W   Window 1 Vertical Dimensions */
#define  GBA_WININ    0x4000048  /* R/W Inside of Window 0 and 1 */
#define  GBA_WINOUT   0x400004A  /* R/W Inside of OBJ Window & Outside of Windows */
#define  GBA_MOSAIC   0x400004C  /* W   Mosaic Size */
#define  GBA_BLDCNT   0x4000050  /* R/W Color Special Effects Selection */
#define  GBA_BLDALPHA 0x4000052  /* R/W Alpha Blending Coefficients */
#define  GBA_BLDY     0x4000054  /* W   Brightness (Fade-In/Out) Coefficient */

// Sound Registers
#define GBA_SOUND1CNT_L 0x4000060  /* R/W   Channel 1 Sweep register       (NR10) */
#define GBA_SOUND1CNT_H 0x4000062  /* R/W   Channel 1 Duty/Length/Envelope (NR11, NR12) */
#define GBA_SOUND1CNT_X 0x4000064  /* R/W   Channel 1 Frequency/Control    (NR13, NR14) */
#define GBA_SOUND2CNT_L 0x4000068  /* R/W   Channel 2 Duty/Length/Envelope (NR21, NR22) */
#define GBA_SOUND2CNT_H 0x400006C  /* R/W   Channel 2 Frequency/Control    (NR23, NR24) */
#define GBA_SOUND3CNT_L 0x4000070  /* R/W   Channel 3 Stop/Wave RAM select (NR30) */
#define GBA_SOUND3CNT_H 0x4000072  /* R/W   Channel 3 Length/Volume        (NR31, NR32) */
#define GBA_SOUND3CNT_X 0x4000074  /* R/W   Channel 3 Frequency/Control    (NR33, NR34) */
#define GBA_SOUND4CNT_L 0x4000078  /* R/W   Channel 4 Length/Envelope      (NR41, NR42) */
#define GBA_SOUND4CNT_H 0x400007C  /* R/W   Channel 4 Frequency/Control    (NR43, NR44) */
#define GBA_SOUNDCNT_L  0x4000080  /* R/W   Control Stereo/Volume/Enable   (NR50, NR51) */
#define GBA_SOUNDCNT_H  0x4000082  /* R/W   Control Mixing/DMA Control */
#define GBA_SOUNDCNT_X  0x4000084  /* R/W   Control Sound on/off           (NR52) */
#define GBA_SOUNDBIAS   0x4000088  /* BIOS  Sound PWM Control */
#define GBA_WAVE_RAM    0x4000090  /* R/W Channel 3 Wave Pattern RAM (2 banks!!) */
#define GBA_FIFO_A      0x40000A0  /* W   Channel A FIFO, Data 0-3 */
#define GBA_FIFO_B      0x40000A4  /* W   Channel B FIFO, Data 0-3 */

// DMA Transfer Channels
#define GBA_DMA0SAD    0x40000B0   /* W    DMA 0 Source Address */
#define GBA_DMA0DAD    0x40000B4   /* W    DMA 0 Destination Address */
#define GBA_DMA0CNT_L  0x40000B8   /* W    DMA 0 Word Count */
#define GBA_DMA0CNT_H  0x40000BA   /* R/W  DMA 0 Control */
#define GBA_DMA1SAD    0x40000BC   /* W    DMA 1 Source Address */
#define GBA_DMA1DAD    0x40000C0   /* W    DMA 1 Destination Address */
#define GBA_DMA1CNT_L  0x40000C4   /* W    DMA 1 Word Count */
#define GBA_DMA1CNT_H  0x40000C6   /* R/W  DMA 1 Control */
#define GBA_DMA2SAD    0x40000C8   /* W    DMA 2 Source Address */
#define GBA_DMA2DAD    0x40000CC   /* W    DMA 2 Destination Address */
#define GBA_DMA2CNT_L  0x40000D0   /* W    DMA 2 Word Count */
#define GBA_DMA2CNT_H  0x40000D2   /* R/W  DMA 2 Control */
#define GBA_DMA3SAD    0x40000D4   /* W    DMA 3 Source Address */
#define GBA_DMA3DAD    0x40000D8   /* W    DMA 3 Destination Address */
#define GBA_DMA3CNT_L  0x40000DC   /* W    DMA 3 Word Count */
#define GBA_DMA3CNT_H  0x40000DE   /* R/W  DMA 3 Control */

// Timer Registers
#define GBA_TM0CNT_L   0x4000100   /* R/W   Timer 0 Counter/Reload */
#define GBA_TM0CNT_H   0x4000102   /* R/W   Timer 0 Control */
#define GBA_TM1CNT_L   0x4000104   /* R/W   Timer 1 Counter/Reload */
#define GBA_TM1CNT_H   0x4000106   /* R/W   Timer 1 Control */
#define GBA_TM2CNT_L   0x4000108   /* R/W   Timer 2 Counter/Reload */
#define GBA_TM2CNT_H   0x400010A   /* R/W   Timer 2 Control */
#define GBA_TM3CNT_L   0x400010C   /* R/W   Timer 3 Counter/Reload */
#define GBA_TM3CNT_H   0x400010E   /* R/W   Timer 3 Control */

// Serial Communication (1)
#define GBA_SIODATA32    0x4000120 /*R/W   SIO Data (Normal-32bit Mode; shared with below) */
#define GBA_SIOMULTI0    0x4000120 /*R/W   SIO Data 0 (Parent)    (Multi-Player Mode) */
#define GBA_SIOMULTI1    0x4000122 /*R/W   SIO Data 1 (1st Child) (Multi-Player Mode) */
#define GBA_SIOMULTI2    0x4000124 /*R/W   SIO Data 2 (2nd Child) (Multi-Player Mode) */
#define GBA_SIOMULTI3    0x4000126 /*R/W   SIO Data 3 (3rd Child) (Multi-Player Mode) */
#define GBA_SIOCNT       0x4000128 /*R/W   SIO Control Register */
#define GBA_SIOMLT_SEND  0x400012A /*R/W   SIO Data (Local of MultiPlayer; shared below) */
#define GBA_SIODATA8     0x400012A /*R/W   SIO Data (Normal-8bit and UART Mode) */

// Keypad Input
#define GBA_KEYINPUT  0x4000130    /* R      Key Status */
#define GBA_KEYCNT    0x4000132    /* R/W    Key Interrupt Control */

// Serial Communication (2)
#define GBA_RCNT      0x4000134    /* R/W  SIO Mode Select/General Purpose Data */
#define GBA_IR        0x4000136    /* -    Ancient - Infrared Register (Prototypes only) */
#define GBA_JOYCNT    0x4000140    /* R/W  SIO JOY Bus Control */
#define GBA_JOY_RECV  0x4000150    /* R/W  SIO JOY Bus Receive Data */
#define GBA_JOY_TRANS 0x4000154    /* R/W  SIO JOY Bus Transmit Data */
#define GBA_JOYSTAT   0x4000158    /* R/?  SIO JOY Bus Receive Status */

// Interrupt, Waitstate, and Power-Down Control
#define GBA_IE      0x4000200      /* R/W  IE        Interrupt Enable Register */
#define GBA_IF      0x4000202      /* R/W  IF        Interrupt Request Flags / IRQ Acknowledge */
#define GBA_WAITCNT 0x4000204      /* R/W  WAITCNT   Game Pak Waitstate Control */
#define GBA_IME     0x4000208      /* R/W  IME       Interrupt Master Enable Register */
#define GBA_POSTFLG 0x4000300      /* R/W  POSTFLG   Undocumented - Post Boot Flag */
#define GBA_HALTCNT 0x4000301      /* W    HALTCNT   Undocumented - Power Down Control */
// #define GBA_?       0x4000410      /* ?    ?         Undocumented - Purpose Unknown / Bug ??? 0FFh */
// #define GBA_?       0x4000800      /* R/W  ?         Undocumented - Internal Memory Control (R/W) */
// #define GBA_?       0x4xx0800      /* R/W  ?         Mirrors of 4000800h (repeated each 64K) */
// #define GBA_(3DS)   0x4700000      /* W    (3DS)     Disable ARM7 bootrom overlay (3DS only) */

mmio_reg_t gba_io_reg_desc[]={
  // Interrupt, Waitstate, and Power-Down Control
  { GBA_IE     , "IE", {
    { 0 , 1, "LCD V-Blank" },
    { 1 , 1, "LCD H-Blank" },
    { 2 , 1, "LCD V-Counter Match" },
    { 3 , 1, "Timer 0 Overflow" },
    { 4 , 1, "Timer 1 Overflow" },
    { 5 , 1, "Timer 2 Overflow" },
    { 6 , 1, "Timer 3 Overflow" },
    { 7 , 1, "Serial Communication" },
    { 8 , 1, "DMA 0" },
    { 9 , 1, "DMA 1" },
    { 10, 1, "DMA 2" },
    { 11, 1, "DMA 3" },
    { 12, 1, "Keypad" },
    { 13, 1, "Game Pak (ext)" },
  } },      /* R/W  IE        Interrupt Enable Register */
  { GBA_IF     , "IF", {
    { 0 , 1, "LCD V-Blank" },
    { 1 , 1, "LCD H-Blank" },
    { 2 , 1, "LCD V-Counter Match" },
    { 3 , 1, "Timer 0 Overflow" },
    { 4 , 1, "Timer 1 Overflow" },
    { 5 , 1, "Timer 2 Overflow" },
    { 6 , 1, "Timer 3 Overflow" },
    { 7 , 1, "Serial Communication" },
    { 8 , 1, "DMA 0" },
    { 9 , 1, "DMA 1" },
    { 10, 1, "DMA 2" },
    { 11, 1, "DMA 3" },
    { 12, 1, "Keypad" },
    { 13, 1, "Game Pak (ext)" },
  } },      /* R/W  IF        Interrupt Request Flags / IRQ Acknowledge */
  { GBA_WAITCNT, "WAITCNT", {
    { 0,2,  "SRAM Wait Control (0..3 = 4,3,2,8 cycles)" },
    { 2,2,  "Wait State 0 First Access (0..3 = 4,3,2,8 cycles)" },
    { 4,1,  "Wait State 0 Second Access (0..1 = 2,1 cycles)" },
    { 5,2,  "Wait State 1 First Access (0..3 = 4,3,2,8 cycles)" },
    { 7,1,  "Wait State 1 Second Access (0..1 = 4,1 cycles)" },
    { 8,2,  "Wait State 2 First Access (0..3 = 4,3,2,8 cycles)" },
    { 10,1, "Wait State 2 Second Access (0..1 = 8,1 cycles)" },
    { 11,2, "PHI Terminal Output (0..3 = Disable, 4.19MHz, 8.38MHz, 16.78MHz)" },
    { 14,1, "Game Pak Prefetch Buffer (0=Disable, 1=Enable)" },
    { 15,1, "Game Pak Type Flag (0=GBA, 1=CGB) (IN35 signal)" },
  } },      /* R/W  WAITCNT   Game Pak Waitstate Control */
  { GBA_IME    , "IME", {0} },      /* R/W  IME       Interrupt Master Enable Register */
  { GBA_POSTFLG, "POSTFLG", {0} },      /* R/W  POSTFLG   Undocumented - Post Boot Flag */
  { GBA_HALTCNT, "HALTCNT", {0} }, 

  { GBA_DISPCNT , "DISPCNT ", { 
    { 0, 3, "BG Mode (0-5=Video Mode 0-5, 6-7=Prohibited)"},
    { 3 ,1, "Reserved / CGB Mode (0=GBA, 1=CGB)"},
    { 4 ,1, "Display Frame Select (0-1=Frame 0-1)"},
    { 5 ,1, "H-Blank Interval Free (1=Allow access to OAM during H-Blank)"},
    { 6 ,1, "OBJ Character VRAM Mapping (0=2D, 1=1D"},
    { 7 ,1, "Forced Blank (1=Allow FAST VRAM,Palette,OAM)"},
    { 8 ,1, "Screen Display BG0 (0=Off, 1=On)"},
    { 9 ,1, "Screen Display BG1 (0=Off, 1=On)"},
    { 10,1, "Screen Display BG2 (0=Off, 1=On)"},
    { 11,1, "Screen Display BG3 (0=Off, 1=On)"},
    { 12,1, "Screen Display OBJ (0=Off, 1=On)"},
    { 13,1, "Window 0 Display Flag (0=Off, 1=On)"},
    { 14,1, "Window 1 Display Flag (0=Off, 1=On)"},
    { 15,1, "OBJ Window Display Flag (0=Off, 1=On)"},
  } },
  { GBA_GREENSWP, "GREENSWP", { {0, 1, "Green Swap  (0=Normal, 1=Swap)" }} }, /* R/W Undocumented - Green Swap */
  { GBA_DISPSTAT, "DISPSTAT", { 
    { 0,1, "V-Blank flag (1=VBlank) (set in line 160..226; not 227",},
    { 1,1, "H-Blank flag (1=HBlank) (toggled in all lines, 0..227",},
    { 2,1, "V-Counter flag (1=Match) (set in selected line)",},
    { 3,1, "V-Blank IRQ Enable (1=Enable)",},
    { 4,1, "H-Blank IRQ Enable (1=Enable)",},
    { 5,1, "V-Counter IRQ Enable (1=Enable)",},
    { 6,1, "DSi: LCD Initialization Ready (0=Busy, 1=Ready",},
    { 7,1, "NDS: MSB of V-Vcount Setting (LYC.Bit8) (0..262",},
    { 8,8, "V-Count Setting (LYC) (0..227)",},

  } }, /* R/W General LCD Status (STAT,LYC) */
  { GBA_VCOUNT  , "VCOUNT  ", { 0 } }, /* R   Vertical Counter (LY) */
  { GBA_BG0CNT  , "BG0CNT  ", { 
    { 0,2 , "BG Priority (0-3, 0=Highest)"},
    { 2,2 , "Character Base Block (0-3, in units of 16 KBytes) (=BG Tile Data)"},
    { 4,2 , "NDS: MSBs of char base"},
    { 6,1 , "Mosaic (0=Disable, 1=Enable)"},
    { 7,1 , "Colors/Palettes (0=16/16, 1=256/1)"},
    { 8,5 , "Screen Base Block (0-31, in units of 2 KBytes) (=BG Map Data)"},
    { 13,1, "BG0/BG1: (NDS: Ext Palette ) BG2/BG3: Overflow (0=Transp, 1=Wrap)"},
    { 14,2, "Screen Size (0-3)"},
  } }, /* R/W BG0 Control */
  { GBA_BG1CNT  , "BG1CNT  ", { 
    { 0,2 , "BG Priority (0-3, 0=Highest)"},
    { 2,2 , "Character Base Block (0-3, in units of 16 KBytes) (=BG Tile Data)"},
    { 4,2 , "NDS: MSBs of char base"},
    { 6,1 , "Mosaic (0=Disable, 1=Enable)"},
    { 7,1 , "Colors/Palettes (0=16/16, 1=256/1)"},
    { 8,5 , "Screen Base Block (0-31, in units of 2 KBytes) (=BG Map Data)"},
    { 13,1, "BG0/BG1: (NDS: Ext Palette ) BG2/BG3: Overflow (0=Transp, 1=Wrap)"},
    { 14,2, "Screen Size (0-3)"},
  } }, /* R/W BG1 Control */
  { GBA_BG2CNT  , "BG2CNT  ", { 
    { 0,2 , "BG Priority (0-3, 0=Highest)"},
    { 2,2 , "Character Base Block (0-3, in units of 16 KBytes) (=BG Tile Data)"},
    { 4,2 , "NDS: MSBs of char base"},
    { 6,1 , "Mosaic (0=Disable, 1=Enable)"},
    { 7,1 , "Colors/Palettes (0=16/16, 1=256/1)"},
    { 8,5 , "Screen Base Block (0-31, in units of 2 KBytes) (=BG Map Data)"},
    { 13,1, "BG0/BG1: (NDS: Ext Palette ) BG2/BG3: Overflow (0=Transp, 1=Wrap)"},
    { 14,2, "Screen Size (0-3)"},
  } }, /* R/W BG2 Control */
  { GBA_BG3CNT  , "BG3CNT  ", { 
    { 0,2 , "BG Priority (0-3, 0=Highest)"},
    { 2,2 , "Character Base Block (0-3, in units of 16 KBytes) (=BG Tile Data)"},
    { 4,2 , "NDS: MSBs of char base"},
    { 6,1 , "Mosaic (0=Disable, 1=Enable)"},
    { 7,1 , "Colors/Palettes (0=16/16, 1=256/1)"},
    { 8,5 , "Screen Base Block (0-31, in units of 2 KBytes) (=BG Map Data)"},
    { 13,1, "BG0/BG1: (NDS: Ext Palette ) BG2/BG3: Overflow (0=Transp, 1=Wrap)"},
    { 14,2, "Screen Size (0-3)"},
  } }, /* R/W BG3 Control */
  { GBA_BG0HOFS , "BG0HOFS", { 0 } }, /* W   BG0 X-Offset */
  { GBA_BG0VOFS , "BG0VOFS", { 0 } }, /* W   BG0 Y-Offset */
  { GBA_BG1HOFS , "BG1HOFS", { 0 } }, /* W   BG1 X-Offset */
  { GBA_BG1VOFS , "BG1VOFS", { 0 } }, /* W   BG1 Y-Offset */
  { GBA_BG2HOFS , "BG2HOFS", { 0 } }, /* W   BG2 X-Offset */
  { GBA_BG2VOFS , "BG2VOFS", { 0 } }, /* W   BG2 Y-Offset */
  { GBA_BG3HOFS , "BG3HOFS", { 0 } }, /* W   BG3 X-Offset */
  { GBA_BG3VOFS , "BG3VOFS", { 0 } }, /* W   BG3 Y-Offset */
  { GBA_BG2PA   , "BG2PA", { 0 } }, /* W   BG2 Rotation/Scaling Parameter A (dx) */
  { GBA_BG2PB   , "BG2PB", { 0 } }, /* W   BG2 Rotation/Scaling Parameter B (dmx) */
  { GBA_BG2PC   , "BG2PC", { 0 } }, /* W   BG2 Rotation/Scaling Parameter C (dy) */
  { GBA_BG2PD   , "BG2PD", { 0 } }, /* W   BG2 Rotation/Scaling Parameter D (dmy) */
  { GBA_BG2X    , "BG2X", { 0 } }, /* W   BG2 Reference Point X-Coordinate */
  { GBA_BG2Y    , "BG2Y", { 0 } }, /* W   BG2 Reference Point Y-Coordinate */
  { GBA_BG3PA   , "BG3PA", { 0 } }, /* W   BG3 Rotation/Scaling Parameter A (dx) */
  { GBA_BG3PB   , "BG3PB", { 0 } }, /* W   BG3 Rotation/Scaling Parameter B (dmx) */
  { GBA_BG3PC   , "BG3PC", { 0 } }, /* W   BG3 Rotation/Scaling Parameter C (dy) */
  { GBA_BG3PD   , "BG3PD", { 0 } }, /* W   BG3 Rotation/Scaling Parameter D (dmy) */
  { GBA_BG3X    , "BG3X", { 0 } }, /* W   BG3 Reference Point X-Coordinate */
  { GBA_BG3Y    , "BG3Y", { 0 } }, /* W   BG3 Reference Point Y-Coordinate */
  { GBA_WIN0H   , "WIN0H", {  
    { 0, 8, "X2, Rightmost coordinate of window, plus 1 " },
    { 8, 8,  "X1, Leftmost coordinate of window"}, 
  } }, /* W   Window 0 Horizontal Dimensions */
  { GBA_WIN1H   , "WIN1H", { 
    { 0, 8, "X2, Rightmost coordinate of window, plus 1 " },
    { 8, 8, "X1, Leftmost coordinate of window"}, 
  } }, /* W   Window 1 Horizontal Dimensions */
  { GBA_WIN0V   , "WIN0V", { 
    {0, 8,  "Y2, Bottom-most coordinate of window, plus 1" },
    {8, 8,  "Y1, Top-most coordinate of window" },
  } }, /* W   Window 0 Vertical Dimensions */
  { GBA_WIN1V   , "WIN1V", { 
    {0, 8,  "Y2, Bottom-most coordinate of window, plus 1" },
    {8, 8,  "Y1, Top-most coordinate of window" },
  } }, /* W   Window 1 Vertical Dimensions */
  { GBA_WININ   , "WININ", {
    { 0 , 1,  "Window 0 BG0 Enable Bits (0=No Display, 1=Display)"},
    { 1 , 1,  "Window 0 BG1 Enable Bits (0=No Display, 1=Display)"},
    { 2 , 1,  "Window 0 BG2 Enable Bits (0=No Display, 1=Display)"},
    { 3 , 1,  "Window 0 BG3 Enable Bits (0=No Display, 1=Display)"},
    { 4 , 1,  "Window 0 OBJ Enable Bit (0=No Display, 1=Display)"},
    { 5 , 1,  "Window 0 Color Special Effect (0=Disable, 1=Enable)"},
    { 8 , 1,  "Window 1 BG0 Enable Bits (0=No Display, 1=Display)"},
    { 9 , 1,  "Window 1 BG1 Enable Bits (0=No Display, 1=Display)"},
    { 10, 1,  "Window 1 BG2 Enable Bits (0=No Display, 1=Display)"},
    { 11, 1,  "Window 1 BG3 Enable Bits (0=No Display, 1=Display)"},
    { 12, 1,  "Window 1 OBJ Enable Bit (0=No Display, 1=Display)"},
    { 13, 1,  "Window 1 Color Special Effect (0=Disable, 1=Enable)"},
  } }, /* R/W Inside of Window 0 and 1 */
  { GBA_WINOUT  , "WINOUT", { 
    { 0 , 1,  "Window 0 BG0 Enable Bits (0=No Display, 1=Display)"},
    { 1 , 1,  "Window 0 BG1 Enable Bits (0=No Display, 1=Display)"},
    { 2 , 1,  "Window 0 BG2 Enable Bits (0=No Display, 1=Display)"},
    { 3 , 1,  "Window 0 BG3 Enable Bits (0=No Display, 1=Display)"},
    { 4 , 1,  "Window 0 OBJ Enable Bit (0=No Display, 1=Display)"},
    { 5 , 1,  "Window 0 Color Special Effect (0=Disable, 1=Enable)"},
    { 8 , 1,  "Window 1 BG0 Enable Bits (0=No Display, 1=Display)"},
    { 9 , 1,  "Window 1 BG1 Enable Bits (0=No Display, 1=Display)"},
    { 10, 1,  "Window 1 BG2 Enable Bits (0=No Display, 1=Display)"},
    { 11, 1,  "Window 1 BG3 Enable Bits (0=No Display, 1=Display)"},
    { 12, 1,  "Window 1 OBJ Enable Bit (0=No Display, 1=Display)"},
    { 13, 1,  "Window 1 Color Special Effect (0=Disable, 1=Enable)"},
  } }, /* R/W Inside of OBJ Window & Outside of Windows */
  { GBA_MOSAIC  , "MOSAIC", { 
    { 0, 4, "BG Mosaic H-Size (minus 1)" },
    { 4, 4, "BG Mosaic V-Size (minus 1)" },
    { 8, 4, "OBJ Mosaic H-Size (minus 1)" },
    { 12,4, "OBJ Mosaic V-Size (minus 1)" },
  } }, /* W   Mosaic Size */
  { GBA_BLDCNT  , "BLDCNT", { 
    { 0 , 1, "BG0 1st Target Pixel (Background 0)" },
    { 1 , 1, "BG1 1st Target Pixel (Background 1)" },
    { 2 , 1, "BG2 1st Target Pixel (Background 2)" },
    { 3 , 1, "BG3 1st Target Pixel (Background 3)" },
    { 4 , 1, "OBJ 1st Target Pixel (Top-most OBJ pixel)" },
    { 5 , 1, "BD  1st Target Pixel (Backdrop)" },
    { 6 , 2, "Color Effect (0: None 1: Alpha 2: Lighten 3: Darken)" },
    { 8 , 1, "BG0 2nd Target Pixel (Background 0)" },
    { 9 , 1, "BG1 2nd Target Pixel (Background 1)" },
    { 10, 1, "BG2 2nd Target Pixel (Background 2)" },
    { 11, 1, "BG3 2nd Target Pixel (Background 3)" },
    { 12, 1, "OBJ 2nd Target Pixel (Top-most OBJ pixel)" },
    { 13, 1, "BD  2nd Target Pixel (Backdrop)" },
  } }, /* R/W Color Special Effects Selection */
  { GBA_BLDALPHA, "BLDALPHA", { 
    {0, 4, "EVA Coef. (1st Target) (0..16 = 0/16..16/16, 17..31=16/16)"},
    {8, 4, "EVB Coef. (2nd Target) (0..16 = 0/16..16/16, 17..31=16/16)"},
  } }, /* R/W Alpha Blending Coefficients */
  { GBA_BLDY    , "BLDY", { 0 } }, /* W   Brightness (Fade-In/Out) Coefficient */  

  // Sound Registers
  { GBA_SOUND1CNT_L, "SOUND1CNT_L", {
    {0,3, "Number of sweep shift (n=0-7)"},
    {3,1, "Sweep Frequency Direction (0=Increase, 1=Decrease)"},
    {4,3, "Sweep Time; units of 7.8ms (0-7, min=7.8ms, max=54.7ms)"},
  } }, /* R/W   Channel 1 Sweep register       (NR10) */
  { GBA_SOUND1CNT_H, "SOUND1CNT_H", {
    { 0,6, "Sound length; units of (64-n)/256s (0-63)"},
    { 6,2, "Wave Pattern Duty (0-3, see below)"},
    { 8,3, "Envelope Step-Time; units of n/64s (1-7, 0=No Envelope)"},
    { 11,1, "Envelope Direction (0=Decrease, 1=Increase)"},
    { 12,4, "Initial Volume of envelope (1-15, 0=No Sound)"},
  } }, /* R/W   Channel 1 Duty/Length/Envelope (NR11, NR12) */
  { GBA_SOUND1CNT_X, "SOUND1CNT_X", {
    { 0,11, "Frequency; 131072/(2048-n)Hz (0-2047)"},
    { 14,1,  "Length Flag (1=Stop output when length in NR11 expires)"},
    { 15,1,  "Initial (1=Restart Sound)"},
  } }, /* R/W   Channel 1 Frequency/Control    (NR13, NR14) */
  { GBA_SOUND2CNT_L, "SOUND2CNT_L", {
    { 0,6, "Sound length; units of (64-n)/256s (0-63)"},
    { 6,2, "Wave Pattern Duty (0-3, see below)"},
    { 8,3, "Envelope Step-Time; units of n/64s (1-7, 0=No Envelope)"},
    { 11,1, "Envelope Direction (0=Decrease, 1=Increase)"},
    { 12,4, "Initial Volume of envelope (1-15, 0=No Sound)"},
  } }, /* R/W   Channel 2 Duty/Length/Envelope (NR21, NR22) */
  { GBA_SOUND2CNT_H, "SOUND2CNT_H", {
    { 0 ,11, "Frequency; 131072/(2048-n)Hz (0-2047)"},
    { 14,1,  "Length Flag (1=Stop output when length in NR11 expires)"},
    { 15,1,  "Initial (1=Restart Sound)"},
  } }, /* R/W   Channel 2 Frequency/Control    (NR23, NR24) */
  { GBA_SOUND3CNT_L, "SOUND3CNT_L", {
    { 5, 1, "Wave RAM Dimension (0=One bank, 1=Two banks)" },
    { 6, 1, "Wave RAM Bank Number (0-1, see below)" },
    { 7, 1, "Sound Channel 3 Off (0=Stop, 1=Playback)" },
  } }, /* R/W   Channel 3 Stop/Wave RAM select (NR30) */
  { GBA_SOUND3CNT_H, "SOUND3CNT_H", {
    { 0,8, "Sound length; units of (256-n)/256s (0-255)"},
    { 13,2, "Sound Volume (0=Mute/Zero, 1=100%, 2=50%, 3=25%)"},
    { 15,1, "Force Volume (0=Use above, 1=Force 75% regardless of above)"},
  } }, /* R/W   Channel 3 Length/Volume        (NR31, NR32) */
  { GBA_SOUND3CNT_X, "SOUND3CNT_X", {
    { 0,11, "Sample Rate; 2097152/(2048-n) Hz (0-2047)" }, 
    { 14,1, "Length Flag (1=Stop output when length in NR31 expires)" }, 
    { 15,1, "Initial (1=Restart Sound)" }, 
  } }, /* R/W   Channel 3 Frequency/Control    (NR33, NR34) */
  { GBA_SOUND4CNT_L, "SOUND4CNT_L", {
    { 0, 6, "Sound length; units of (64-n)/256s (0-63)" },
    { 8, 3, "Envelope Step-Time; units of n/64s (1-7, 0=No Envelope)" },
    { 11, 1, "Envelope Direction (0=Decrease, 1=Increase)" },
    { 12, 4, "Initial Volume of envelope (1-15, 0=No Sound)" },
  } }, /* R/W   Channel 4 Length/Envelope      (NR41, NR42) */
  { GBA_SOUND4CNT_H, "SOUND4CNT_H", {
    { 0, 1, "Dividing Ratio of Frequencies (r)"},
    { 3, 1, "Counter Step/Width (0=15 bits, 1=7 bits)"},
    { 4, 1, "Shift Clock Frequency (s)"},
    { 14, 1, "Length Flag (1=Stop output when length in NR41 expires)"},
    { 15, 1, "Initial (1=Restart Sound)"},
  } }, /* R/W   Channel 4 Frequency/Control    (NR43, NR44) */
  { GBA_SOUNDCNT_L , "SOUNDCNT_L", {
    { 0,1, "Sound 1 Master Volume RIGHT" },
    { 1,1, "Sound 2 Master Volume RIGHT" },
    { 2,1, "Sound 3 Master Volume RIGHT" },
    { 3,1, "Sound 4 Master Volume RIGHT" },
    { 4,1, "Sound 1 Master Volume LEFT" },
    { 5,1, "Sound 2 Master Volume LEFT" },
    { 6,1, "Sound 3 Master Volume LEFT" },
    { 7,1, "Sound 4 Master Volume LEFT" },

    { 8,1, "Sound 1 Enable RIGHT" },
    { 9,1, "Sound 2 Enable RIGHT" },
    { 10,1, "Sound 3 Enable RIGHT" },
    { 11,1, "Sound 4 Enable RIGHT" },
    { 12,1, "Sound 1 Enable LEFT" },
    { 13,1, "Sound 2 Enable LEFT" },
    { 14,1, "Sound 3 Enable LEFT" },
    { 15,1, "Sound 4 Enable LEFT" },
  } }, /* R/W   Control Stereo/Volume/Enable   (NR50, NR51) */
  { GBA_SOUNDCNT_H , "SOUNDCNT_H", {
    { 0 ,2, "Sound # 1-4 Volume (0=25%, 1=50%, 2=100%, 3=Prohibited)" },
    { 2 ,1, "DMA Sound A Volume (0=50%, 1=100%)" },
    { 3 ,1, "DMA Sound B Volume (0=50%, 1=100%)" },
    { 8 ,1, "DMA Sound A Enable RIGHT (0=Disable, 1=Enable)" },
    { 9 ,1, "DMA Sound A Enable LEFT (0=Disable, 1=Enable)" },
    { 10,1, "DMA Sound A Timer Select (0=Timer 0, 1=Timer 1)" },
    { 11,1, "DMA Sound A Reset FIFO (1=Reset)" },
    { 12,1, "DMA Sound B Enable RIGHT (0=Disable, 1=Enable)" },
    { 13,1, "DMA Sound B Enable LEFT (0=Disable, 1=Enable)" },
    { 14,1, "DMA Sound B Timer Select (0=Timer 0, 1=Timer 1)" },
    { 15,1, "DMA Sound B Reset FIFO (1=Reset)" },
  } }, /* R/W   Control Mixing/DMA Control */
  { GBA_SOUNDCNT_X , "SOUNDCNT_X", {
    {0, 1, "Sound 1 ON flag (Read Only)" },
    {1, 1, "Sound 2 ON flag (Read Only)" },
    {2, 1, "Sound 3 ON flag (Read Only)" },
    {3, 1, "Sound 4 ON flag (Read Only)" },
    {7, 1, "PSG/FIFO Master Enable (0=Disable, 1=Enable) (Read/Write)" },
  } }, /* R/W   Control Sound on/off           (NR52) */
  { GBA_SOUNDBIAS  , "SOUNDBIAS", {
   { 1,9,"Bias Level (Default=100h, converting signed samples into unsigned)"},
   { 14,2,"Amplitude Resolution/Sampling Cycle (Default=0, see below)"},
  } }, /* BIOS  Sound PWM Control */
  { GBA_WAVE_RAM   , "WAVE_RAM", { 0 } }, /* R/W Channel 3 Wave Pattern RAM (2 banks!!) */
  { GBA_FIFO_A     , "FIFO_A", { 0 } }, /* W   Channel A FIFO, Data 0-3 */
  { GBA_FIFO_B     , "FIFO_B", { 0 } }, /* W   Channel B FIFO, Data 0-3 */  

  // DMA Transfer Channels
  { GBA_DMA0SAD  , "DMA0SAD", { 0 } },   /* W    DMA 0 Source Address */
  { GBA_DMA0DAD  , "DMA0DAD", { 0 } },   /* W    DMA 0 Destination Address */
  { GBA_DMA0CNT_L, "DMA0CNT_L", { 0 } },   /* W    DMA 0 Word Count */
  { GBA_DMA0CNT_H, "DMA0CNT_H", {
    { 5,  2,  "Dest Addr Control (0=Incr,1=Decr,2=Fixed,3=Incr/Reload)" },
    { 7,  2,  "Source Adr Control (0=Incr,1=Decr,2=Fixed,3=Prohibited)" },
    { 9,  1,  "DMA Repeat (0=Off, 1=On) (Must be zero if Bit 11 set)" },
    { 10, 1,  "DMA Transfer Type (0=16bit, 1=32bit)" },
    { 12, 2,  "DMA Start Timing (0=Immediately, 1=VBlank, 2=HBlank, 3=Prohibited)" },
    { 14, 1,  "IRQ upon end of Word Count (0=Disable, 1=Enable)" },
    { 15, 1,  "DMA Enable (0=Off, 1=On)" },
  } },   /* R/W  DMA 0 Control */
  { GBA_DMA1SAD  , "DMA1SAD", { 0 } },   /* W    DMA 1 Source Address */
  { GBA_DMA1DAD  , "DMA1DAD", { 0 } },   /* W    DMA 1 Destination Address */
  { GBA_DMA1CNT_L, "DMA1CNT_L", { 0 } },   /* W    DMA 1 Word Count */
  { GBA_DMA1CNT_H, "DMA1CNT_H", {
    { 5,  2,  "Dest Addr Control (0=Incr,1=Decr,2=Fixed,3=Incr/Reload)" },
    { 7,  2,  "Source Adr Control (0=Incr,1=Decr,2=Fixed,3=Prohibited)" },
    { 9,  1,  "DMA Repeat (0=Off, 1=On) (Must be zero if Bit 11 set)" },
    { 10, 1,  "DMA Transfer Type (0=16bit, 1=32bit)" },
    { 12, 2,  "DMA Start Timing (0=Immediately, 1=VBlank, 2=HBlank, 3=Sound)" },
    { 14, 1,  "IRQ upon end of Word Count (0=Disable, 1=Enable)" },
    { 15, 1,  "DMA Enable (0=Off, 1=On)" },
  } },   /* R/W  DMA 1 Control */
  { GBA_DMA2SAD  , "DMA2SAD", { 0 } },   /* W    DMA 2 Source Address */
  { GBA_DMA2DAD  , "DMA2DAD", { 0 } },   /* W    DMA 2 Destination Address */
  { GBA_DMA2CNT_L, "DMA2CNT_L", { 0 } },   /* W    DMA 2 Word Count */
  { GBA_DMA2CNT_H, "DMA2CNT_H", {
    { 5,  2,  "Dest Addr Control (0=Incr,1=Decr,2=Fixed,3=Incr/Reload)" },
    { 7,  2,  "Source Adr Control (0=Incr,1=Decr,2=Fixed,3=Prohibited)" },
    { 9,  1,  "DMA Repeat (0=Off, 1=On) (Must be zero if Bit 11 set)" },
    { 10, 1,  "DMA Transfer Type (0=16bit, 1=32bit)" },
    { 12, 2,  "DMA Start Timing (0=Immediately, 1=VBlank, 2=HBlank, 3=Sound)" },
    { 14, 1,  "IRQ upon end of Word Count (0=Disable, 1=Enable)" },
    { 15, 1,  "DMA Enable (0=Off, 1=On)" },
  } },   /* R/W  DMA 2 Control */
  { GBA_DMA3SAD  , "DMA3SAD", { 0 } },   /* W    DMA 3 Source Address */
  { GBA_DMA3DAD  , "DMA3DAD", { 0 } },   /* W    DMA 3 Destination Address */
  { GBA_DMA3CNT_L, "DMA3CNT_L", { 0 } },   /* W    DMA 3 Word Count */
  { GBA_DMA3CNT_H, "DMA3CNT_H", {
    { 5,  2,  "Dest Addr Control (0=Incr,1=Decr,2=Fixed,3=Incr/Reload)" },
    { 7,  2,  "Source Adr Control (0=Incr,1=Decr,2=Fixed,3=Prohibited)" },
    { 9,  1,  "DMA Repeat (0=Off, 1=On) (Must be zero if Bit 11 set)" },
    { 10, 1,  "DMA Transfer Type (0=16bit, 1=32bit)" },
    { 11, 1,  "Game Pak DRQ (0=Normal, 1=DRQ <from> Game Pak, DMA3)" },
    { 12, 2,  "DMA Start Timing (0=Immediately, 1=VBlank, 2=HBlank, 3=Video Capture)" },
    { 14, 1,  "IRQ upon end of Word Count (0=Disable, 1=Enable)" },
    { 15, 1,  "DMA Enable (0=Off, 1=On)" },
  } },   /* R/W  DMA 3 Control */  

  // Timer Registers
  { GBA_TM0CNT_L, "TM0CNT_L", {0} },   /* R/W   Timer 0 Counter/Reload */
  { GBA_TM0CNT_H, "TM0CNT_H", {
    { 0 ,2, "Prescaler Selection (0=F/1, 1=F/64, 2=F/256, 3=F/1024)" },
    { 2 ,1, "Count-up (0=Normal, 1=Incr. on prev. Timer overflow)" },
    { 6 ,1, "Timer IRQ Enable (0=Disable, 1=IRQ on Timer overflow)" },
    { 7 ,1, "Timer Start/Stop (0=Stop, 1=Operate)" },
  } },   /* R/W   Timer 0 Control */
  { GBA_TM1CNT_L, "TM1CNT_L", {0} },   /* R/W   Timer 1 Counter/Reload */
  { GBA_TM1CNT_H, "TM1CNT_H", {
    { 0 ,2, "Prescaler Selection (0=F/1, 1=F/64, 2=F/256, 3=F/1024)" },
    { 2 ,1, "Count-up (0=Normal, 1=Incr. on prev. Timer overflow)" },
    { 6 ,1, "Timer IRQ Enable (0=Disable, 1=IRQ on Timer overflow)" },
    { 7 ,1, "Timer Start/Stop (0=Stop, 1=Operate)" },
  } },   /* R/W   Timer 1 Control */
  { GBA_TM2CNT_L, "TM2CNT_L", {0} },   /* R/W   Timer 2 Counter/Reload */
  { GBA_TM2CNT_H, "TM2CNT_H", {
    { 0 ,2, "Prescaler Selection (0=F/1, 1=F/64, 2=F/256, 3=F/1024)" },
    { 2 ,1, "Count-up (0=Normal, 1=Incr. on prev. Timer overflow)" },
    { 6 ,1, "Timer IRQ Enable (0=Disable, 1=IRQ on Timer overflow)" },
    { 7 ,1, "Timer Start/Stop (0=Stop, 1=Operate)" },
  } },   /* R/W   Timer 2 Control */
  { GBA_TM3CNT_L, "TM3CNT_L", {0} },   /* R/W   Timer 3 Counter/Reload */
  { GBA_TM3CNT_H, "TM3CNT_H", {
    { 0 ,2, "Prescaler Selection (0=F/1, 1=F/64, 2=F/256, 3=F/1024)" },
    { 2 ,1, "Count-up (0=Normal, 1=Incr. on prev. Timer overflow)" },
    { 6 ,1, "Timer IRQ Enable (0=Disable, 1=IRQ on Timer overflow)" },
    { 7 ,1, "Timer Start/Stop (0=Stop, 1=Operate)" },
  } },   /* R/W   Timer 3 Control */  

  // Serial Communication (1)
  { GBA_SIODATA32  , "SIODATA32", {0} }, /*R/W   SIO Data (Normal-32bit Mode; shared with below) */
  { GBA_SIOMULTI0  , "SIOMULTI0", {0} }, /*R/W   SIO Data 0 (Parent)    (Multi-Player Mode) */
  { GBA_SIOMULTI1  , "SIOMULTI1", {0} }, /*R/W   SIO Data 1 (1st Child) (Multi-Player Mode) */
  { GBA_SIOMULTI2  , "SIOMULTI2", {0} }, /*R/W   SIO Data 2 (2nd Child) (Multi-Player Mode) */
  { GBA_SIOMULTI3  , "SIOMULTI3", {0} }, /*R/W   SIO Data 3 (3rd Child) (Multi-Player Mode) */
  { GBA_SIOCNT     , "SIOCNT", {0} }, /*R/W   SIO Control Register */
  { GBA_SIOMLT_SEND, "SIOMLT_SEND", {0} }, /*R/W   SIO Data (Local of MultiPlayer; shared below) */
  { GBA_SIODATA8   , "SIODATA8", {0} }, /*R/W   SIO Data (Normal-8bit and UART Mode) */  

  // Keypad Input
  { GBA_KEYINPUT, "GBA_KEYINPUT", {
    { 0, 1, "Button A" },
    { 1, 1, "Button B" },
    { 2, 1, "Select" },
    { 3, 1, "Start" },
    { 4, 1, "Right" },
    { 5, 1, "Left" },
    { 6, 1, "Up" },
    { 7, 1, "Down" },
    { 8, 1, "Button R" },
    { 9, 1, "Button L" },
  } },    /* R      Key Status */
  { GBA_KEYCNT  , "GBA_KEYCNT", {
    { 0, 1, "Button A" },
    { 1, 1, "Button B" },
    { 2, 1, "Select" },
    { 3, 1, "Start" },
    { 4, 1, "Right" },
    { 5, 1, "Left" },
    { 6, 1, "Up" },
    { 7, 1, "Down" },
    { 8, 1, "Button R" },
    { 9, 1, "Button L" },
    { 14,1, "Button IRQ Enable (0=Disable, 1=Enable)" },
    { 15,1, "Button IRQ Condition (0=OR, 1=AND)"},
  } },    /* R/W    Key Interrupt Control */  

  // Serial Communication (2)
  { GBA_RCNT     , "RCNT", {0} },     /* R/W  SIO Mode Select/General Purpose Data */
  { GBA_IR       , "IR", {0} },     /* -    Ancient - Infrared Register (Prototypes only) */
  { GBA_JOYCNT   , "JOYCNT", {0} },     /* R/W  SIO JOY Bus Control */
  { GBA_JOY_RECV , "JOY_RECV", {0} },     /* R/W  SIO JOY Bus Receive Data */
  { GBA_JOY_TRANS, "JOY_TRANS", {0} },     /* R/W  SIO JOY Bus Transmit Data */
  { GBA_JOYSTAT  , "JOYSTAT", {0} },     /* R/?  SIO JOY Bus Receive Status */  
};

// Interrupt sources
#define GBA_INT_LCD_VBLANK 0   
#define GBA_INT_LCD_HBLANK 1     
#define GBA_INT_LCD_VCOUNT 2     
#define GBA_INT_TIMER0     3     
#define GBA_INT_TIMER1     4     
#define GBA_INT_TIMER2     5     
#define GBA_INT_TIMER3     6     
#define GBA_INT_SERIAL     7     
#define GBA_INT_DMA0       8     
#define GBA_INT_DMA1       9     
#define GBA_INT_DMA2       10    
#define GBA_INT_DMA3       11    
#define GBA_INT_KEYPAD     12    
#define GBA_INT_GAMEPAK    13    
#define GBA_BG_PALETTE  0x00000000                    
#define GBA_OBJ_PALETTE 0x00000200                    
#define GBA_OBJ_TILES0_2   0x00010000
#define GBA_OBJ_TILES3_5   0x00014000
#define GBA_OAM         0x07000000

#define GBA_BACKUP_NONE        0
#define GBA_BACKUP_EEPROM      1
#define GBA_BACKUP_EEPROM_512B 2
#define GBA_BACKUP_EEPROM_8KB  3
#define GBA_BACKUP_SRAM        4
#define GBA_BACKUP_FLASH_64K   5 
#define GBA_BACKUP_FLASH_128K  6  
#define GBA_BACKUP_SRAM_128K   7
#define GBA_BACKUP_DIRECT      8

#define GBA_REQ_1B    0x01
#define GBA_REQ_2B    0x02
#define GBA_REQ_4B    0x04
#define GBA_REQ_DEBUG 0x20
#define GBA_REQ_READ  0x40
#define GBA_REQ_WRITE 0x80

typedef struct {     
  uint8_t *bios;
  uint8_t wram0[256*1024];
  uint8_t wram1[32*1024];
  uint8_t io[1024];
  uint8_t palette[1024];
  uint8_t vram[128*1024];
  uint8_t oam[1024];
  uint8_t *cart_rom;
  uint8_t cart_backup[128*1024];
  uint8_t flash_chip_id[4];
  uint32_t openbus_word;
  uint32_t eeprom_word; 
  uint32_t eeprom_addr; 
  uint32_t prefetch_en; 
  uint32_t prefetch_size; 
  uint32_t requests;
  uint32_t bios_word;
  uint32_t sram_word;
  uint32_t mmio_word;
  uint8_t wait_state_table[16*4];
  // Tracks the place of the squashable pipeline bubble that enters the pipeline after a single cycle data read
  // Each bit represents one clock cycle in time with bit 0 meaning the current cycle of the last stage of the 
  // CPU pipeline and larger bits indicating future cycles. 

  // This is shifted to the right every clock cycle to mimic the bubble traveling through the pipeline. 
  // Some complexity is introduced because this bubble may be squashed by upstream push back. This is modelled by 
  // extra shift rights for the multicycling of the various stages. 
  uint32_t pipeline_bubble_shift_register;
  // Lookup tables to accelerate MMIO masking / Open bus behavior
  uint32_t mmio_data_mask_lookup[256];
  uint8_t  mmio_reg_valid_lookup[256];
  uint8_t mmio_debug_access_buffer[16*1024];
} gba_mem_t;

typedef struct {
  unsigned rom_size; 
  uint8_t backup_type;
  bool backup_is_dirty;
  bool in_chip_id_mode; 
  int flash_state;
  int flash_bank; 
  int sram_bank; 
  uint32_t gpio_data;
} gba_cartridge_t;
typedef struct{
  int source_addr;
  int dest_addr;
  int length;
  int current_transaction;
  bool last_enable;
  bool last_vblank;
  bool last_hblank;
  uint32_t latched_transfer;
  int startup_delay; 
  bool activate_audio_dma;
  bool video_dma_active;
} gba_dma_t; 
typedef struct{
  int scan_clock; 
  bool last_vblank;
  bool last_hblank;
  int last_lcd_y; 
  struct {
    int32_t internal_bgx;
    int32_t internal_bgy;
    int32_t render_bgx;
    int32_t render_bgy;
    bool wrote_bgx;
    bool wrote_bgy;
  }aff[2];
  uint16_t dispcnt_pipeline[3];
  int fast_forward_ticks;
  float ghosting_strength;
  uint32_t mosaic_y_counter;
}gba_ppu_t;
typedef struct{
  bool last_enable; 
  uint16_t reload_value; 
  uint16_t pending_reload_value; 
  int startup_delay;
}gba_timer_t;
typedef struct{
  uint32_t step_counter;
  int32_t length[4];
  uint32_t volume[4];
  uint32_t frequency[4];
  int32_t  env_direction[4]; //1: increase 0: nochange -1: decrease
  uint32_t env_period[4];
  uint32_t env_period_timer[4];
  bool env_overflow[4]; 
  //Only channel 1
  uint32_t sweep_period;
  uint32_t sweep_timer;
  int32_t  sweep_direction; 
  uint32_t sweep_shift;
  bool     sweep_enable;
  bool     sweep_subtracted;
  bool use_length[4];
  bool active[4];
  bool powered[4];
  float chan_t[4];
  uint16_t lfsr4;
}gba_frame_sequencer_t;
typedef struct{
  struct{
    int8_t data[64];
    int read_ptr; 
    int write_ptr; 
    bool request_dma_fill;
  }fifo[2];
  double current_sim_time;
  double current_sample_generated_time;
  uint16_t wave_freq_timer;
  uint16_t wave_sample_offset;
  uint8_t curr_wave_sample;
  uint8_t curr_wave_data;
  float capacitor_l,capacitor_r;
  gba_frame_sequencer_t sequencer;
  uint32_t audio_clock; 
}gba_audio_t; 
typedef struct{
  uint32_t serial_state;
  uint32_t serial_bits_clocked;
  uint64_t input_register;
  uint64_t output_register;
  uint32_t state;
  uint8_t status_register;
  //Used to create an RTC that is real time in the game world. 
  uint64_t initial_rtc_time;
  uint64_t total_clocks_ticked;
}gba_rtc_t;
typedef struct{
  int ticks_till_transfer_done; 
  bool last_active; 
}gba_sio_t; 

typedef struct{
  uint32_t bess_version; //Versioning field must be 1
  /* 
  r0-r15 
  CPSR 16
  SPSR 17 
  R13_fiq 22 
  R13_irq 24 
  R13_svc 26 
  R13_abt 28 
  R13_und 30 
  R14_fiq 23 
  R14_irq 25 
  R14_svc 27 
  R14_abt 29 
  R14_und 31 
  SPSR_fiq 32 
  SPSR_irq 33 
  SPSR_svc 34 
  SPSR_abt 35 
  SPSR_und 36 
  */
  uint32_t cpu_registers[37];
  uint32_t wram0_seg;
  uint32_t wram1_seg;
  uint32_t io_seg;
  uint32_t palette_seg;
  uint32_t vram_seg;
  uint32_t oam_seg;
  uint32_t cart_backup_seg;
  uint16_t timer_reload_values[4];
  uint32_t padding[18];
}gba_bess_info_t;
typedef struct{
  uint16_t dac;
  uint16_t value;
  uint8_t last_clk;
}gba_solar_sensor_t;

// ROM实时读取协议类型
typedef enum {
  GBA_ROM_PROTOCOL_NONE = 0,
  GBA_ROM_PROTOCOL_FILE = 1,
  GBA_ROM_PROTOCOL_SERIAL = 2,
  GBA_ROM_PROTOCOL_NET = 3
} gba_rom_protocol_t;

// 串口端口类型定义
#ifdef _WIN32
  // windows.h 已经在文件开头包含了
  typedef HANDLE serial_port_t;
  #define INVALID_SERIAL_PORT INVALID_HANDLE_VALUE
#else
  #include <fcntl.h>
  #include <termios.h>
  #include <unistd.h>
  #include <errno.h>
  #include <sys/ioctl.h>
  typedef int serial_port_t;
  #define INVALID_SERIAL_PORT -1
#endif

// 前向声明gba_scratch_t以便在gba_t中使用
typedef struct gba_scratch_t gba_scratch_t;

typedef struct gba_t{
  gba_mem_t mem;
  arm7_t cpu;
  gba_cartridge_t cart;
  gba_ppu_t ppu;
  gba_rtc_t rtc;
  gba_dma_t dma[4]; 
  gba_sio_t sio; 
  gba_bess_info_t bess;
  //There is a 2 cycle penalty when the CPU takes over from the DMA
  bool last_transaction_dma; 
  bool activate_dmas; 
  bool dma_wait_ppu;
  gba_timer_t timers[4];
  uint32_t timer_ticks_before_event;
  uint32_t deferred_timer_ticks;
  uint32_t global_timer;
  gba_audio_t audio;
  bool prev_key_interrupt;
  uint32_t first_target_buffer[GBA_LCD_W];
  uint32_t second_target_buffer[GBA_LCD_W];
  uint8_t window[GBA_LCD_W];
  uint8_t *framebuffer;
  // Some HW has up to a 4 cycle delay before its IF propagates. 
  // This array acts as a FIFO to keep track of that. 
  uint16_t pipelined_if[5];
  int active_if_pipe_stages; 
  int last_cpu_tick;
  int residual_dma_ticks; 
  bool stop_mode; 
  bool frame_in_progress;
  bool pause_after_frame; 
  gba_solar_sensor_t solar_sensor;
  gba_scratch_t *scratch; // 用于实时ROM读取
} gba_t; 

// 完整定义gba_scratch_t结构
// 串口写入缓存项
typedef struct {
  uint32_t addr;      // 写入地址
  uint8_t data;       // 写入数据
  bool is_rom;        // true=ROM区域(writeRom), false=RAM区域(writeRam)
} gba_serial_write_entry_t;

#define GBA_SERIAL_WRITE_BUFFER_SIZE 2048

struct gba_scratch_t{
  uint8_t framebuffer[GBA_LCD_W*GBA_LCD_H*4];
  uint8_t bios[16*1024];
  FILE * log_cmp_file; 
  bool skip_bios_intro;
  char save_file_path[SB_FILE_PATH_SIZE];
  // 实时ROM读取相关字段
  bool use_realtime_rom;
  gba_rom_protocol_t rom_protocol;
  size_t realtime_rom_size;
  char rom_source_address[SB_FILE_PATH_SIZE];
  char rom_cache_path[SB_FILE_PATH_SIZE];
  uint8_t *rom_cache_data;      // cache数据指针
  uint8_t *rom_cache_valid;     // 每个字节的有效位标记(1=已缓存, 0=未缓存)
  FILE *rom_source_file;        // 对于FILE协议，保持文件句柄打开
  void *rom_source_serial;      // 对于SERIAL协议，保持串口句柄
  int custom_backup_type;       // 自定义存档类型 (-1=自动检测, 其他=具体类型)
  // 串口写入缓存
  gba_serial_write_entry_t serial_write_buffer[GBA_SERIAL_WRITE_BUFFER_SIZE];
  int serial_write_count;       // 当前缓存的写入数量
  // RAM预读缓存 (用于GBA_BACKUP_DIRECT)
  uint8_t *ram_prefetch_cache;  // 预读缓存数据 (2KB)
  uint32_t ram_prefetch_start;  // 缓存起始地址
  uint16_t ram_prefetch_size;   // 缓存大小 (通常2048字节)
  bool ram_prefetch_valid;      // 缓存是否有效
  // save同步延迟计数器
  int save_sync_stable_frames;  // 存档稳定的帧数计数(连续10帧未修改才同步)
  bool save_sync_pending;       // 是否有待同步的Flash数据
};

// 日志输出配置宏
#ifndef GBA_SERIAL_LOG_ENABLED
  #define GBA_SERIAL_LOG_ENABLED 1  // 默认启用日志
#endif

#ifndef GBA_SERIAL_LOG_TO_FILE
  #define GBA_SERIAL_LOG_TO_FILE 0  // 默认输出到控制台，设置为1则输出到文件
#endif

#ifndef GBA_SERIAL_LOG_FILE_PATH
  #define GBA_SERIAL_LOG_FILE_PATH "log.txt"  // 默认日志文件路径
#endif

// 日志文件句柄(全局静态变量)
static FILE* log_file_handle = NULL;
static bool log_file_initialized = false;

// 日志输出函数
static void log_printf(const char* format, ...) {
#if GBA_SERIAL_LOG_ENABLED
  va_list args;
  va_start(args, format);
  
#if GBA_SERIAL_LOG_TO_FILE
  // 输出到文件
  if (!log_file_initialized) {
    log_file_handle = fopen(GBA_SERIAL_LOG_FILE_PATH, "w");
    log_file_initialized = true;
    if (log_file_handle) {
      fprintf(log_file_handle, "=== SkyEmu Serial Log ===\n");
      fflush(log_file_handle);
    }
  }
  
  if (log_file_handle) {
    vfprintf(log_file_handle, format, args);
    fflush(log_file_handle);  // 立即刷新到文件
  }
#else
  // 输出到控制台
  vprintf(format, args);
  fflush(stdout);
#endif
  
  va_end(args);
#else
  (void)format;  // 避免未使用参数警告
#endif
}

// 关闭日志文件
static void log_close() {
#if GBA_SERIAL_LOG_ENABLED && GBA_SERIAL_LOG_TO_FILE
  if (log_file_handle) {
    fprintf(log_file_handle, "=== Log Closed ===\n");
    fclose(log_file_handle);
    log_file_handle = NULL;
    log_file_initialized = false;
  }
#endif
}

// 实时ROM读取函数的前向声明(需要在gba_dword_lookup之前)
static uint8_t gba_read_rom_byte(gba_scratch_t *scratch, size_t offset);

static void gba_process_audio_writes(gba_t* gba);
static uint8_t gba_audio_process_byte_write(gba_t *gba, uint32_t addr, uint8_t value);
static bool gba_run_ar_cheat(gba_t* gba, const uint32_t* buffer, uint32_t size);
static FORCE_INLINE void gba_recompute_waitstate_table(gba_t* gba,uint16_t waitcnt);
static FORCE_INLINE uint32_t gba_read32(gba_t*gba, unsigned baddr);
static FORCE_INLINE void gba_store32(gba_t*gba, unsigned baddr, uint32_t data);

// 串口ROM/RAM访问函数前向声明
static bool gba_serial_write_rom(serial_port_t port, uint32_t addr_word, const uint8_t* data, uint16_t data_len);
static bool gba_serial_read_ram(serial_port_t port, uint32_t addr, uint8_t* buffer, uint16_t length_byte);
static bool gba_serial_write_ram(serial_port_t port, uint32_t addr, const uint8_t* data, uint16_t data_len);
static bool gba_serial_flash_program(serial_port_t port, uint32_t addr, const uint8_t* data, uint16_t data_len);
static void gba_flush_serial_writes(gba_scratch_t *scratch);
static void gba_buffer_serial_write(gba_scratch_t *scratch, uint32_t addr, uint8_t data, bool is_rom);

//Returns offset into savestate where bess info can be found
static uint32_t gba_save_best_effort_state(gba_t* gba){
  gba->bess.bess_version = 1; 
  for(int i=0;i<37;++i)gba->bess.cpu_registers[i]=gba->cpu.registers[i];
  
  gba->bess.wram0_seg = ((uint8_t*)gba->mem.wram0)-(uint8_t*)gba;
  gba->bess.wram1_seg = ((uint8_t*)gba->mem.wram1)-(uint8_t*)gba;
  gba->bess.io_seg = ((uint8_t*)gba->mem.io)-(uint8_t*)gba;
  gba->bess.palette_seg = ((uint8_t*)gba->mem.palette)-(uint8_t*)gba;
  gba->bess.vram_seg = ((uint8_t*)gba->mem.vram)-(uint8_t*)gba;
  gba->bess.oam_seg = ((uint8_t*)gba->mem.oam)-(uint8_t*)gba;
  gba->bess.cart_backup_seg = ((uint8_t*)gba->mem.cart_backup)-(uint8_t*)gba;
  for(int i=0;i<4;++i)gba->bess.timer_reload_values[i]=gba->timers[i].reload_value;
  return ((uint8_t*)&gba->bess)-(uint8_t*)(gba); 
}
static bool gba_load_best_effort_state(gba_t* gba,uint8_t *save_state_data, uint32_t size, uint32_t bess_offset){
  if(bess_offset+sizeof(gba_bess_info_t)>size)return false;
  gba_bess_info_t * bess = (gba_bess_info_t*)(save_state_data+bess_offset);
  if(bess->bess_version!=1)return false; 
  if(bess->wram0_seg+sizeof(gba->mem.wram0) > size) return false; 
  if(bess->wram1_seg+sizeof(gba->mem.wram1) > size) return false;  
  if(bess->io_seg+sizeof(gba->mem.io) > size) return false;  
  if(bess->palette_seg+sizeof(gba->mem.palette) > size) return false;  
  if(bess->vram_seg+sizeof(gba->mem.vram) > size) return false;  
  if(bess->oam_seg+sizeof(gba->mem.oam) > size) return false;  
  if(bess->cart_backup_seg+sizeof(gba->mem.cart_backup) > size) return false; 
  for(int i=0;i<37;++i)gba->cpu.registers[i]=bess->cpu_registers[i];
  memcpy(gba->mem.wram0,save_state_data+bess->wram0_seg,sizeof(gba->mem.wram0));
  memcpy(gba->mem.wram1,save_state_data+bess->wram1_seg,sizeof(gba->mem.wram1));
  memcpy(gba->mem.io,save_state_data+bess->io_seg,sizeof(gba->mem.io));
  memcpy(gba->mem.palette,save_state_data+bess->palette_seg,sizeof(gba->mem.palette));
  memcpy(gba->mem.vram,save_state_data+bess->vram_seg,sizeof(gba->mem.vram));
  memcpy(gba->mem.oam,save_state_data+bess->oam_seg,sizeof(gba->mem.oam));
  memcpy(gba->mem.cart_backup,save_state_data+bess->cart_backup_seg,sizeof(gba->mem.cart_backup));
  for(int i=0;i<4;++i)gba->timers[i].pending_reload_value=gba->timers[i].reload_value=bess->timer_reload_values[i];

  gba_recompute_waitstate_table(gba,gba_read32(gba, GBA_WAITCNT));
  return true; 
}

static FORCE_INLINE sb_debug_mmio_access_t gba_debug_mmio_access(gba_t*gba, unsigned baddr, int trigger_breakpoint){
  baddr&=0xffff;
  baddr/=4;

  if(trigger_breakpoint!=-1){
    gba->mem.mmio_debug_access_buffer[baddr]&=0x7f;
    if(trigger_breakpoint!=0)gba->mem.mmio_debug_access_buffer[baddr]|=0x80;
  }
  uint8_t flag = gba->mem.mmio_debug_access_buffer[baddr];

  sb_debug_mmio_access_t access; 
  access.read_since_reset = flag&0x1;
  access.read_in_tick = flag&0x2;

  access.write_since_reset = flag&0x10;
  access.write_in_tick = flag&0x20;
  access.trigger_breakpoint = flag&0x80;
  return access; 
}

static void gba_tick_keypad(sb_joy_t*joy, gba_t* gba); 
static FORCE_INLINE void gba_tick_timers(gba_t* gba);
static void gba_compute_timers(gba_t* gba); 
static void FORCE_INLINE gba_send_interrupt(gba_t*gba,int delay,int if_bit);
// Returns a pointer to the data backing the baddr (when not DWORD aligned, it
// ignores the lowest 2 bits. 
static FORCE_INLINE uint32_t * gba_dword_lookup(gba_t* gba,unsigned baddr, int req_type);
static FORCE_INLINE uint32_t gba_read32(gba_t*gba, unsigned baddr){return *gba_dword_lookup(gba,baddr,GBA_REQ_READ|GBA_REQ_4B);}
static FORCE_INLINE uint16_t gba_read16(gba_t*gba, unsigned baddr){
  uint32_t* val = gba_dword_lookup(gba,baddr,GBA_REQ_READ|GBA_REQ_2B);
  int offset = SB_BFE(baddr,1,1);
  return ((uint16_t*)val)[offset];
}
static FORCE_INLINE uint8_t gba_read8(gba_t*gba, unsigned baddr){
  uint32_t* val = gba_dword_lookup(gba,baddr,GBA_REQ_READ|GBA_REQ_1B);
  int offset = SB_BFE(baddr,0,2);
  return ((uint8_t*)val)[offset];
}        
static FORCE_INLINE uint8_t gba_read8_debug(gba_t*gba, unsigned baddr){
  uint32_t* val = gba_dword_lookup(gba,baddr,GBA_REQ_READ|GBA_REQ_1B|GBA_REQ_DEBUG);
  int offset = SB_BFE(baddr,0,2);
  return ((uint8_t*)val)[offset];
}         
static FORCE_INLINE void gba_process_flash_state_machine(gba_t* gba, unsigned baddr, uint8_t data){
  #define FLASH_DEFAULT 0x0
  #define FLASH_RECV_AA 0x1
  #define FLASH_RECV_55 0x2
  #define FLASH_ERASE_RECV_AA 0x3
  #define FLASH_ERASE_RECV_55 0x4

  #define FLASH_ENTER_CHIP_ID 0x90 
  #define FLASH_EXIT_CHIP_ID  0xF0 
  #define FLASH_PREP_ERASE    0x80
  #define FLASH_ERASE_CHIP 0x10 
  #define FLASH_ERASE_4KB 0x30 
  #define FLASH_WRITE_BYTE 0xA0
  #define FLASH_SET_BANK 0xB0
  int state = gba->cart.flash_state;
  gba->cart.flash_state=FLASH_DEFAULT;
  baddr&=0xffff;
  switch(state){
    default: 
      log_printf("Unknown flash state %02x\n",gba->cart.flash_state);
    case FLASH_DEFAULT:
      if(baddr==0x5555 && data == 0xAA) gba->cart.flash_state = FLASH_RECV_AA;
      break;
    case FLASH_RECV_AA:
      if(baddr==0x2AAA && data == 0x55) gba->cart.flash_state = FLASH_RECV_55;
      break;
    case FLASH_RECV_55:
      if(baddr==0x5555){
        // Process command
        switch(data){
          case FLASH_ENTER_CHIP_ID:gba->cart.in_chip_id_mode = true; break;
          case FLASH_EXIT_CHIP_ID: gba->cart.in_chip_id_mode = false; break;
          case FLASH_PREP_ERASE:   gba->cart.flash_state = FLASH_PREP_ERASE; break;
          case FLASH_WRITE_BYTE:   gba->cart.flash_state = FLASH_WRITE_BYTE; break;
          case FLASH_SET_BANK:     gba->cart.flash_state = FLASH_SET_BANK; break;
          default: printf("Unknown flash command: %02x\n",data);break;
        }
      }
      break;
    case FLASH_PREP_ERASE:
      if(baddr==0x5555 && data == 0xAA) gba->cart.flash_state = FLASH_ERASE_RECV_AA;
      break;
    case FLASH_ERASE_RECV_AA:
      if(baddr==0x2AAA && data == 0x55) gba->cart.flash_state = FLASH_ERASE_RECV_55;
      break;
    case FLASH_ERASE_RECV_55:
      if(baddr==0x5555|| data ==FLASH_ERASE_4KB){
        int size = gba->cart.backup_type == GBA_BACKUP_FLASH_64K? 64*1024 : 128*1024;
        int erase_4k_off = gba->cart.flash_bank*64*1024+SB_BFE(baddr,12,4)*4096;
        // Process command
        switch(data){
          case FLASH_ERASE_CHIP:
            log_printf("Erase Flash Chip %d bytes\n",size);
            for(int i=0;i<size;++i)gba->mem.cart_backup[i]=0xff;
            break;
          case FLASH_ERASE_4KB:for(int i=0;i<4096;++i)gba->mem.cart_backup[erase_4k_off+i]=0xff; break;
          default: printf("Unknown flash erase command: %02x\n",data);break;
        }
        gba->cart.backup_is_dirty=true;
      }
      break;
    case FLASH_WRITE_BYTE:
      gba->mem.cart_backup[gba->cart.flash_bank*64*1024+baddr] &= data; 
      gba->cart.backup_is_dirty=true;
      break;
    case FLASH_SET_BANK:
      gba->cart.flash_bank = data&1; 
      break;
  }
}
static uint64_t gba_rev_bits(uint64_t data, int bits){
  uint64_t out = 0;
  for(int i=0;i<bits;++i){
    out<<=1;
    out|=data&1;
    data>>=1;
  }
  return out;
}
static uint8_t gba_bin_to_bcd(uint8_t bin){
  bin%=100;
  return (bin%10)|((bin/10)<<4);
}
/* Only simulates a small subset of the RTC needed to make time events work in the pokemon games. */
static FORCE_INLINE void gba_process_rtc_state_machine(gba_t* gba){
  uint32_t data = gba->cart.gpio_data;
  bool clk  = SB_BFE(data,0,1);
  bool io_dat = SB_BFE(data,1,1);
  bool cs   = SB_BFE(data,2,1);
  #define SERIAL_INIT 0 
  #define SERIAL_CLK_LOW 1
  #define SERIAL_CLK_HIGH 2  

  #define GBA_RTC_RESET     0
  #define GBA_RTC_UNUSED    1
  #define GBA_RTC_DATE_TIME 2
  #define GBA_RTC_FORCE_IRQ 3
  #define GBA_RTC_STATUS    4
  #define GBA_RTC_UNUSED2   5
  #define GBA_RTC_TIME      6
  #define GBA_RTC_UNUSED3   7

  gba->rtc.status_register &= ~((1<<7));
  gba->rtc.status_register |= 0x40;

  if(cs==0){
    gba->rtc.serial_state=SERIAL_INIT;
    gba->rtc.serial_bits_clocked=0;
    gba->rtc.state = 0;
    gba->rtc.input_register = 0;
    gba->rtc.output_register = 0;
  }else{
    time_t time_secs= gba->rtc.initial_rtc_time+(gba->rtc.total_clocks_ticked)/(16*1024*1024);
    struct tm * tm = localtime(&time_secs);
    uint8_t second = gba_bin_to_bcd(tm->tm_sec);
    uint8_t minute = gba_bin_to_bcd(tm->tm_min);
    uint8_t hour  = gba_bin_to_bcd(tm->tm_hour);
    uint8_t day   = gba_bin_to_bcd(tm->tm_mday);
    uint8_t month = gba_bin_to_bcd(tm->tm_mon+1);
    uint8_t year  = gba_bin_to_bcd(tm->tm_year%100);
    uint8_t day_of_week=gba_bin_to_bcd(tm->tm_wday);
    bool new_bit = false; 
    
    if(gba->rtc.serial_state==SERIAL_CLK_LOW&&clk){
      gba->rtc.input_register<<=1;
      gba->rtc.input_register|=((uint64_t)io_dat);
      new_bit = true;
    
      bool out_bit = (gba->rtc.output_register&1);
      gba->mem.cart_rom[0x0000C4] |=(out_bit<<1);
      gba->rtc.output_register>>=1;
    }
    
    gba->rtc.serial_state= clk? SERIAL_CLK_HIGH: SERIAL_CLK_LOW;

    if(new_bit){
      gba->rtc.serial_bits_clocked++;
      if(gba->rtc.serial_bits_clocked==8) {
        // Check whether the command should be interpreted MSB-first or LSB-first.
        gba->rtc.input_register&=0xff;
        if((gba->rtc.input_register >> 4) == 6) {
          gba->rtc.input_register = (gba->rtc.input_register << 4) | (gba->rtc.input_register >> 4);
          gba->rtc.input_register&=0xff;
          gba->rtc.input_register = ((gba->rtc.input_register & 0x33) << 2) | ((gba->rtc.input_register & 0xCC) >> 2);
          gba->rtc.input_register&=0xff;
          gba->rtc.input_register = ((gba->rtc.input_register & 0x55) << 1) | ((gba->rtc.input_register & 0xAA) >> 1);
          gba->rtc.input_register&=0xff;
        } 
        gba->rtc.state= SB_BFE(gba->rtc.input_register,0,8);
        log_printf("RTC Command %d\n",gba->rtc.state);
      }
      int  cmd = SB_BFE(gba->rtc.state,4,3);
      bool read = SB_BFE(gba->rtc.state,7,1);
      switch(cmd){
        case GBA_RTC_STATUS:{
          if(gba->rtc.serial_bits_clocked==8) gba->rtc.output_register = gba->rtc.status_register;
          if(gba->rtc.serial_bits_clocked==16){
            if(!read){
              gba->rtc.status_register=SB_BFE(gba->rtc.input_register,0,8);
            }
            gba->rtc.serial_bits_clocked=0;
          }
          break;
        }
        case GBA_RTC_DATE_TIME:{
          if(gba->rtc.serial_bits_clocked==8) gba->rtc.output_register =
            (((uint64_t)(year&0xff)       )<<(0*8ull))|
            (((uint64_t)(month&0xff)      )<<(1*8ull))|
            (((uint64_t)(day&0xff)        )<<(2*8ull))|
            (((uint64_t)(day_of_week&0xff))<<(3*8ull))|
            (((uint64_t)(hour&0xff)       )<<(4*8ull))|
            (((uint64_t)(minute&0xff)     )<<(5*8ull))|
            (((uint64_t)(second&0xff)     )<<(6*8ull));
          if(gba->rtc.serial_bits_clocked==8*8){
            if(!read){
              year  = SB_BFE(gba->rtc.input_register,6*8,8);
              month = SB_BFE(gba->rtc.input_register,5*8,8);
              day   = SB_BFE(gba->rtc.input_register,4*8,8);
              day_of_week = SB_BFE(gba->rtc.input_register,3*8,8);
              hour   = SB_BFE(gba->rtc.input_register,2*8,8);
              minute = SB_BFE(gba->rtc.input_register,1*8,8);
              second = SB_BFE(gba->rtc.input_register,0*8,8);
            }
            gba->rtc.serial_bits_clocked=0;
          }
          break;
        }
        case GBA_RTC_TIME:{
          if(gba->rtc.serial_bits_clocked==8) gba->rtc.output_register =
            ((uint64_t)(hour&0xff)<<(0*8))|
            ((uint64_t)(minute&0xff)<<(1*8))|
            ((uint64_t)(second&0xff)<<(2*8));
          if(gba->rtc.serial_bits_clocked==4*8){
            if(!read){
              hour   = SB_BFE(gba->rtc.input_register,0*8,8);
              minute = SB_BFE(gba->rtc.input_register,1*8,8);
              second = SB_BFE(gba->rtc.input_register,2*8,8);
            }
            gba->rtc.serial_bits_clocked=0;
          }
          break;
        }
        default:
        case GBA_RTC_UNUSED: case GBA_RTC_UNUSED2: case GBA_RTC_UNUSED3: case GBA_RTC_FORCE_IRQ:
          log_printf("Error: Unknown RTC Command %d\n",cmd);
        case GBA_RTC_RESET:
          if(gba->rtc.serial_bits_clocked==8){
            gba->rtc.serial_bits_clocked=0;
          }
        break;
      }
      

    }
  }
}
static FORCE_INLINE void gba_process_backup_write(gba_t*gba, unsigned baddr, uint32_t data){
  if(gba->cart.backup_type==GBA_BACKUP_FLASH_64K||gba->cart.backup_type==GBA_BACKUP_FLASH_128K){
    gba_process_flash_state_machine(gba,baddr,data);
  }else if(gba->cart.backup_type==GBA_BACKUP_SRAM){
    if(gba->mem.cart_backup[baddr&0x7fff]!=(data&0xff)){
      gba->mem.cart_backup[baddr&0x7fff]=data&0xff; 
      gba->cart.backup_is_dirty=true;
    }
  }else if(gba->cart.backup_type==GBA_BACKUP_SRAM_128K){
    if(gba->mem.cart_backup[gba->cart.sram_bank*0x10000+(baddr&0xffff)]!=(data&0xff)){
      gba->mem.cart_backup[gba->cart.sram_bank*0x10000+(baddr&0xffff)]=data&0xff;
      gba->cart.backup_is_dirty=true;
    }
  }else if(gba->cart.backup_type==GBA_BACKUP_DIRECT){
    // 串口直接访问：写入已在gba_store32/16/8中处理(缓存写入)
    // 这里不需要额外操作，写入会被缓存并在下次读取时刷新
  }
}
static void gba_process_solar_sensor(gba_t*gba){
  uint16_t data_and_dir = gba->cart.gpio_data&(gba->cart.gpio_data>>16);
  bool clk = SB_BFE(data_and_dir,0,1);
  bool rst = SB_BFE(data_and_dir,1,1);
  bool cs  = SB_BFE(data_and_dir,2,1);
  if(!cs){
    if(rst){gba->solar_sensor.dac=0;}
    else if(!clk&&gba->solar_sensor.last_clk){
      gba->solar_sensor.dac++;
    }
    bool flag =  gba->solar_sensor.dac> gba->solar_sensor.value;
    gba->solar_sensor.last_clk=clk;
    //printf("DAC Value: %d clk: %d rst: %d dir:%d flag:%d\n",gba->solar_sensor.dac,clk,rst,SB_BFE(gba->cart.gpio_data,16,16),flag);
    if(SB_BFE(gba->cart.gpio_data,16+3,1)==0){
      gba->mem.cart_rom[0x0000C4] &=~(1<<3);
      gba->mem.cart_rom[0x0000C4] |=(flag<<3);
    }
  }else{
    gba->solar_sensor.dac=0;
  }
}
static FORCE_INLINE void gba_store32(gba_t*gba, unsigned baddr, uint32_t data){
  // 串口模式：缓存写入，延迟到下次读取时合并发送
  if(gba->scratch && gba->scratch->use_realtime_rom && 
     gba->scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL){
    
    // SRAM区域(0x0E000000-0x0FFFFFFF，包括镜像)
    if(gba->cart.backup_type==GBA_BACKUP_DIRECT && (baddr&0xfe000000)==0x0E000000){
      uint32_t ram_addr = baddr & 0xFFFF;
      gba_buffer_serial_write(gba->scratch, ram_addr, data & 0xFF, false);
      return;
    }
    // ROM区域(0x08000000-0x0DFFFFFF)/ (0x0A000000-0x0BFFFFFF)/ (0x0C000000-0x0DFFFFFF)
    else if(baddr >= 0x08000000 && baddr < 0x0E000000){
      uint32_t rom_addr = baddr & 0x1FFFFFF;
      if(!(baddr >= 0x000000C4 && baddr <= 0x000000C8) && baddr != 0x000010c7 && baddr != 0x000010c9) {
        for(int i = 0; i < 4; i++){
          gba_buffer_serial_write(gba->scratch, rom_addr + i, (data >> (i*8)) & 0xFF, true);
        }
        return;
      }
    }
  }
  if(baddr==0x09000000) {
    gba->cart.sram_bank = data & 1;
    printf("Set SRAM Bank to %d\n",gba->cart.sram_bank);
  }
  
  if(baddr>=0x08000000){
    //Mask is 0xfe to catch the sram mirror at 0x0f and 0x0e
    if((baddr&0xfe000000)==0xE000000){gba_process_backup_write(gba,baddr,data>>((baddr&3)*8));return;}
    if(baddr>=0x080000C4&& baddr<0x080000C8){
      if(baddr==0x080000c4){
        gba->cart.gpio_data = data;
      }    
      gba->mem.cart_rom[0x0000C4] =gba->cart.gpio_data&~SB_BFE(gba->cart.gpio_data,16,16);
      gba_process_solar_sensor(gba);
      gba_process_rtc_state_machine(gba);
      return;
    }
  }
  uint32_t *val=gba_dword_lookup(gba,baddr,GBA_REQ_WRITE|GBA_REQ_4B);
  *val= data;
}
static FORCE_INLINE void gba_store16(gba_t*gba, unsigned baddr, uint32_t data){
  // 串口模式：缓存写入，延迟到下次读取时合并发送
  if(gba->scratch && gba->scratch->use_realtime_rom && 
     gba->scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL){
    
    // SRAM区域(0x0E000000-0x0FFFFFFF，包括镜像)
    if(gba->cart.backup_type==GBA_BACKUP_DIRECT && (baddr&0xfe000000)==0x0E000000){
      uint32_t ram_addr = baddr & 0xFFFF;
      gba_buffer_serial_write(gba->scratch, ram_addr, data & 0xFF, false);
      return;
    }
    // ROM区域(0x08000000-0x0DFFFFFF)/ (0x0A000000-0x0BFFFFFF)/ (0x0C000000-0x0DFFFFFF)
    else if(baddr >= 0x08000000 && baddr < 0x0E000000){
      uint32_t rom_addr = baddr & 0x1FFFFFF;
      if(!(baddr >= 0x000000C4 && baddr <= 0x000000C8) && baddr != 0x000010c7 && baddr != 0x000010c9) {
        for(int i = 0; i < 2; i++){
          gba_buffer_serial_write(gba->scratch, rom_addr + i, (data >> (i*8)) & 0xFF, true);
        }
        return;
      }
    }
  }

  if(baddr==0x09000000) {
    gba->cart.sram_bank = data & 1;
    printf("Set SRAM Bank to %d\n",gba->cart.sram_bank);
  }
  
  if(baddr>=0x08000000){
    //Mask is 0xfe to catch the sram mirror at 0x0f and 0x0e
    if((baddr&0xfe000000)==0xE000000){
      gba_process_backup_write(gba,baddr,data>>  ((baddr&1)*8));
      return;
    }
    if(baddr>=0x080000C4&& baddr<0x080000C8){
      int addr = baddr&~1;
      if(addr==0x080000c4)gba->cart.gpio_data =(gba->cart.gpio_data&0xffff0000)|(data&0xffff);
      if(addr==0x080000c6)gba->cart.gpio_data =(gba->cart.gpio_data&0x0000ffff)|((data&0xffff)<<16);
      gba->mem.cart_rom[0x0000C4] =gba->cart.gpio_data&~SB_BFE(gba->cart.gpio_data,16,16);
      gba_process_solar_sensor(gba);
      gba_process_rtc_state_machine(gba);
      return;
    }
  }
  uint32_t* val = gba_dword_lookup(gba,baddr,GBA_REQ_WRITE|GBA_REQ_2B);
  int offset = SB_BFE(baddr,1,1);
  ((uint16_t*)val)[offset]=data; 
}
static FORCE_INLINE void gba_store8(gba_t*gba, unsigned baddr, uint32_t data){
  // 串口模式：缓存写入，延迟到下次读取时合并发送
  if(gba->scratch && gba->scratch->use_realtime_rom && 
     gba->scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL){
    
    // SRAM区域(0x0E000000-0x0FFFFFFF，包括镜像)
    if(gba->cart.backup_type==GBA_BACKUP_DIRECT && (baddr&0xfe000000)==0x0E000000){
      uint32_t ram_addr = baddr & 0xFFFF;
      gba_buffer_serial_write(gba->scratch, ram_addr, data & 0xFF, false);
      return;
    }
    // ROM区域(0x08000000-0x0DFFFFFF)/ (0x0A000000-0x0BFFFFFF)/ (0x0C000000-0x0DFFFFFF)
    else if(baddr >= 0x08000000 && baddr < 0x0E000000){
      uint32_t rom_addr = baddr & 0x1FFFFFF;
      if(!(baddr >= 0x000000C4 && baddr <= 0x000000C8) && baddr != 0x000010c7 && baddr != 0x000010c9) {
        gba_buffer_serial_write(gba->scratch, rom_addr, data & 0xFF, true);
        return;
      }
    }
  }
  
  if(baddr==0x09000000) {
    gba->cart.sram_bank = data & 1;
    printf("Set SRAM Bank to %d\n",gba->cart.sram_bank);
  }
  
  if(baddr>=0x05000000){
    // 8 bit stores to palette mirror across 8 bit halves
    if((baddr&0xff000000)==0x5000000){gba_store16(gba,baddr&~1,(data&0xff)*0x0101); return; }
    if(((baddr&0xff000000)==0x06000000)&&((baddr&0x1ffff)<=0x0013FFF)){gba_store16(gba,baddr&~1,(data&0xff)*0x0101);return;}
    //Mask is 0xfe to catch the sram mirror at 0x0f and 0x0e
    if((baddr&0xfe000000)==0xE000000){ gba_process_backup_write(gba,baddr,data); return; }
    // Remaining 8 bit ops are not supported on VRAM or ROM
    return; 
  }
  uint32_t *val = gba_dword_lookup(gba,baddr,GBA_REQ_WRITE|GBA_REQ_1B);
  int offset = SB_BFE(baddr,0,2);
  ((uint8_t*)val)[offset]=data; 
} 
static FORCE_INLINE void gba_store8_debug(gba_t*gba, unsigned baddr, uint32_t data){
  if(baddr>=0x05000000){
    // 8 bit stores to palette mirror across 8 bit halves
    if((baddr&0xff000000)==0x5000000){gba_store16(gba,baddr&~1,(data&0xff)*0x0101); return; }
    if(((baddr&0xff000000)==0x06000000)&&((baddr&0x1ffff)<=0x0013FFF)){gba_store16(gba,baddr&~1,(data&0xff)*0x0101);return;}
    //Mask is 0xfe to catch the sram mirror at 0x0f and 0x0e
    if((baddr&0xfe000000)==0xE000000){ gba_process_backup_write(gba,baddr,data); return; }
    // Remaining 8 bit ops are not supported on VRAM or ROM
    return; 
  }
  uint32_t *val = gba_dword_lookup(gba,baddr,GBA_REQ_WRITE|GBA_REQ_1B|GBA_REQ_DEBUG);
  int offset = SB_BFE(baddr,0,2);
  ((uint8_t*)val)[offset]=data; 
} 
static FORCE_INLINE void gba_io_store8(gba_t*gba, unsigned baddr, uint8_t data){gba->mem.io[baddr&0xfff]=data;}
static FORCE_INLINE void gba_io_store16(gba_t*gba, unsigned baddr, uint16_t data){*(uint16_t*)(gba->mem.io+(baddr&0xfff))=data;}
static FORCE_INLINE void gba_io_store32(gba_t*gba, unsigned baddr, uint32_t data){*(uint32_t*)(gba->mem.io+(baddr&0xfff))=data;}

static FORCE_INLINE uint8_t  gba_io_read8(gba_t*gba, unsigned baddr) {return gba->mem.io[baddr&0xfff];}
static FORCE_INLINE uint16_t gba_io_read16(gba_t*gba, unsigned baddr){return *(uint16_t*)(gba->mem.io+(baddr&0xfff));}
static FORCE_INLINE uint32_t gba_io_read32(gba_t*gba, unsigned baddr){return *(uint32_t*)(gba->mem.io+(baddr&0xfff));}
static FORCE_INLINE void gba_recompute_waitstate_table(gba_t* gba,uint16_t waitcnt){
  // TODO: Make the waitstate for the ROM configureable 
  const int wait_state_table[16*4]={
    1,1,1,1, //0x00 (bios)
    1,1,1,1, //0x01 (bios)
    3,3,6,6, //0x02 (256k WRAM)
    1,1,1,1, //0x03 (32k WRAM)
    1,1,1,1, //0x04 (IO)
    1,1,2,2, //0x05 (BG/OBJ Palette)
    1,1,2,2, //0x06 (VRAM)
    1,1,1,1, //0x07 (OAM)
    4,4,8,8, //0x08 (GAMEPAK ROM 0)
    4,4,8,8, //0x09 (GAMEPAK ROM 0)
    4,4,8,8, //0x0A (GAMEPAK ROM 1)
    4,4,8,8, //0x0B (GAMEPAK ROM 1)
    4,4,8,8, //0x0C (GAMEPAK ROM 2)
    4,4,8,8, //0x0D (GAMEPAK ROM 2)
    4,4,4,4, //0x0E (GAMEPAK SRAM)
    1,1,1,1, //0x0F (unused)
  };
  for(int i=0;i<16*4;++i){
    gba->mem.wait_state_table[i]=wait_state_table[i];
  }
  uint8_t sram_wait = SB_BFE(waitcnt,0,2);
  uint8_t wait_first[3];
  uint8_t wait_second[3];

  wait_first[0]  = SB_BFE(waitcnt,2,2);
  wait_second[0] = SB_BFE(waitcnt,4,1);
  wait_first[1]  = SB_BFE(waitcnt,5,2);
  wait_second[1] = SB_BFE(waitcnt,7,1);
  wait_first[2]  = SB_BFE(waitcnt,8,2);
  wait_second[2] = SB_BFE(waitcnt,10,1);
  uint8_t prefetch_en = SB_BFE(waitcnt,14,1);

  int primary_table[4]={4,3,2,8};

  //Each waitstate is two entries in table
  for(int ws=0;ws<3;++ws){
    for(int i=0;i<2;++i){
      uint8_t w_first = primary_table[wait_first[ws]];
      uint8_t w_second = wait_second[ws]?1:2;
      if(ws==1)w_second = wait_second[ws]?1:4;
      if(ws==2)w_second = wait_second[ws]?1:8;
      w_first+=1;w_second+=1;
      //Wait 0
      int wait16b = w_second; 
      int wait32b = w_second*2; 

      int wait16b_nonseq = w_first; 
      int wait32b_nonseq = w_first+w_second;

      gba->mem.wait_state_table[(0x08+i+ws*2)*4+0] = wait16b;
      gba->mem.wait_state_table[(0x08+i+ws*2)*4+1] = wait16b_nonseq;
      gba->mem.wait_state_table[(0x08+i+ws*2)*4+2] = wait32b;
      gba->mem.wait_state_table[(0x08+i+ws*2)*4+3] = wait32b_nonseq;
    }
  }
  gba->mem.prefetch_en = prefetch_en;
  gba->mem.prefetch_size = 0;

  //SRAM
  gba->mem.wait_state_table[(0x0E*4)+0]= 1+primary_table[sram_wait];
  gba->mem.wait_state_table[(0x0E*4)+1]= 1+primary_table[sram_wait];
  gba->mem.wait_state_table[(0x0E*4)+2]= 1+primary_table[sram_wait];
  gba->mem.wait_state_table[(0x0E*4)+3]= 1+primary_table[sram_wait];
  waitcnt&=(1<<15); // Force cartridge to report as GBA cart
  gba_io_store16(gba,GBA_WAITCNT,waitcnt);
}
static FORCE_INLINE void gba_compute_access_cycles(gba_t *gba, uint32_t address,int request_size/*0: 1B,1: 2B,3: 4B*/){
  int bank = SB_BFE(address,24,4);
  bool prefetch_en= gba->mem.prefetch_en;
  if(SB_UNLIKELY(!prefetch_en)){
    if(gba->cpu.i_cycles)request_size|=1;
    if(request_size&1)gba->cpu.next_fetch_sequential =false;
    gba->mem.prefetch_size = 0;
  }
  uint32_t wait = gba->mem.wait_state_table[bank*4+request_size];
  if(SB_LIKELY(prefetch_en)){    
    gba->mem.prefetch_size+=gba->cpu.i_cycles;
    if(bank>=0x08&&bank<=0x0D){
      if(SB_UNLIKELY(request_size&1)){
        uint32_t pc = gba->cpu.prefetch_pc;    
        if(pc>=0x08000000){
          // Check if the bubble made it to the execute stage before being squashed, 
          // and apply the bubble cycle if it was not squashed. 
          // Note, only a single pipeline bubble is tracked using this infrastructure. 
          int pc_bank = SB_BFE(pc,24,4);
          int prefetch_cycles = gba->mem.wait_state_table[pc_bank*4]; 
          int prefetch_phase = (gba->mem.prefetch_size)%prefetch_cycles;
          if(gba->mem.prefetch_size>gba->cpu.i_cycles&&prefetch_phase==prefetch_cycles-1)wait+=1;
        }
        //Non sequential->reset prefetch buffer
        gba->mem.prefetch_size = 0;
        gba->cpu.next_fetch_sequential =false;
      }else{
        //Sequential fetch from prefetch buffer based on available wait states
        if(gba->mem.prefetch_size>=wait){
          gba->mem.prefetch_size-=wait-1; 
          wait = 1; 
        }else{
          wait -= gba->mem.prefetch_size;
          gba->mem.prefetch_size=0;
        }
      }
    }else gba->mem.prefetch_size+=wait; 
  }
  gba->mem.requests+=wait;
}
static FORCE_INLINE uint32_t gba_compute_access_cycles_dma(gba_t *gba, uint32_t address,int request_size/*0: 1B,1: 2B,3: 4B*/){
  int bank = SB_BFE(address,24,4);
  uint32_t wait = gba->mem.wait_state_table[bank*4+request_size];
  return wait;
}
static FORCE_INLINE void gba_process_mmio_read(gba_t *gba, uint32_t address);

// Memory IO functions for the emulated CPU                  
static FORCE_INLINE uint32_t arm7_read32(void* user_data, uint32_t address){
  gba_compute_access_cycles((gba_t*)user_data,address,3);
  uint32_t value = gba_read32((gba_t*)user_data,address);
  return value;
}
static FORCE_INLINE uint32_t arm7_read16(void* user_data, uint32_t address){
  gba_compute_access_cycles((gba_t*)user_data,address,1);
  uint16_t value = gba_read16((gba_t*)user_data,address);
  return value;
}
static FORCE_INLINE uint32_t arm7_read32_seq(void* user_data, uint32_t address, bool seq){
  gba_compute_access_cycles((gba_t*)user_data,address,seq?2:3);
  return gba_read32((gba_t*)user_data,address);
}
static FORCE_INLINE uint32_t arm7_read16_seq(void* user_data, uint32_t address, bool seq){
  gba_compute_access_cycles((gba_t*)user_data,address,seq?0:1);
  return gba_read16((gba_t*)user_data,address);
}
void gba_cpu_breakpoint(void* user_data){
  gba_t * gba = (gba_t*)user_data;
  gba->frame_in_progress=false; 
  gba->pause_after_frame=true;
  log_printf("Hit ARM7 Breakpoint\n");
}
//Used to process special behavior triggered by MMIO write
static bool gba_process_mmio_write(gba_t *gba, uint32_t address, uint32_t data, int req_size_bytes);

static FORCE_INLINE uint8_t arm7_read8(void* user_data, uint32_t address){
  gba_compute_access_cycles((gba_t*)user_data,address,1);
  return gba_read8((gba_t*)user_data,address);
}
static FORCE_INLINE void gba_dma_write32(gba_t* gba, uint32_t address, uint32_t data){
  if((address&0xfffffC00)==0x04000000){
    if(gba_process_mmio_write(gba,address,data,4))return;
  }
  gba_store32(gba,address,data);
}
static FORCE_INLINE void gba_dma_write16(gba_t* gba, uint32_t address, uint16_t data){
  if((address&0xfffffC00)==0x04000000){
    if(gba_process_mmio_write(gba,address,data,2))return; 
  }
  gba_store16(gba,address,data);
}
static FORCE_INLINE void arm7_write32(void* user_data, uint32_t address, uint32_t data){
  gba_compute_access_cycles((gba_t*)user_data,address,3); 
  gba_dma_write32((gba_t*)user_data,address,data);
}
static FORCE_INLINE void arm7_write16(void* user_data, uint32_t address, uint16_t data){
  gba_compute_access_cycles((gba_t*)user_data,address,1); 
  gba_dma_write16((gba_t*)user_data,address,data);
}
static FORCE_INLINE void arm7_write8(void* user_data, uint32_t address, uint8_t data)  {
  gba_compute_access_cycles((gba_t*)user_data,address,1); 
  if((address&0xfffff000)==0x04000000){
    if(gba_process_mmio_write((gba_t*)user_data,address,data,1))return; 
  }
  gba_store8((gba_t*)user_data,address,data);
}
// Try to load a GBA rom, return false on invalid rom
bool gba_load_rom(sb_emu_state_t*emu,gba_t* gba, gba_scratch_t *scratch);
// Sync Flash backup to serial device when save occurs
static void gba_serial_sync_flash_backup(gba_t* gba, int size);
// Sync SRAM_128K backup to serial device when save occurs
static void gba_serial_sync_sram_backup(gba_t* gba, int size);

static FORCE_INLINE uint32_t * gba_dword_lookup(gba_t* gba,unsigned addr, int req_type){
  uint32_t *ret = &gba->mem.openbus_word;
  switch(addr>>24){
    case 0x0: if(addr<0x4000){
      if(gba->cpu.registers[15]<0x4000)gba->mem.bios_word = *(uint32_t*)(gba->mem.bios+(addr&~3));
      //else gba->mem.bios_word=0;
      gba->mem.openbus_word=gba->mem.bios_word;
     } break;
    case 0x1: break;
    case 0x2: 
      ret = (uint32_t*)(gba->mem.wram0+(addr&0x3fffc)); 
      gba->mem.openbus_word=*ret;
      break;
    case 0x3: 
      ret = (uint32_t*)(gba->mem.wram1+(addr&0x7ffc)); 
      gba->mem.openbus_word=*ret;
      break;
    case 0x4: 
      if(SB_LIKELY(addr<=0x40003FF)){
        if(req_type&GBA_REQ_READ){
          int io_reg = (addr>>2)&0xff;
          if(SB_LIKELY(gba->mem.mmio_reg_valid_lookup[io_reg])){
            gba_process_mmio_read(gba,addr);
            gba->mem.mmio_word = (*(uint32_t*)(gba->mem.io+(addr&0x3fc)))&gba->mem.mmio_data_mask_lookup[io_reg];
            ret = &gba->mem.mmio_word;
          }
        }else ret = (uint32_t*)(gba->mem.io+(addr&0x3fc));
        if(!(req_type&GBA_REQ_DEBUG)){
          gba->mem.mmio_debug_access_buffer[(addr&0xffff)/4]|=(req_type&GBA_REQ_WRITE)?0x70:0xf;
          if(gba->mem.mmio_debug_access_buffer[(addr&0xffff)/4]&0x80)gba->cpu.trigger_breakpoint(gba);
        }
      }
      break;
    case 0x5: 
      ret = (uint32_t*)(gba->mem.palette+(addr&0x3fc)); 
      gba->mem.openbus_word=*ret;
      break;
    case 0x6: 
      if(addr&0x10000){
        ret = (uint32_t*)(gba->mem.vram+(addr&0x07ffc)+0x10000);
        gba->mem.openbus_word= *ret;
        if(addr&0x08000){
          uint16_t dispcnt = gba_io_read16(gba, GBA_DISPCNT);
          int bg_mode = SB_BFE(dispcnt,0,3);
          //Don't allow writes to mirrored VRAM in bitmap mode. See also vram-mirror.gba
          //Needed for Acrobat Kid. Still requires testing to verify correct behavior
          if(bg_mode>2&&!(addr&0x04000)){ret = &gba->mem.openbus_word;*ret=0;}
        }
      }else ret = (uint32_t*)(gba->mem.vram+(addr&0x1fffc));
      gba->mem.openbus_word=*ret;
      break;
    case 0x7: 
      ret = (uint32_t*)(gba->mem.oam+(addr&0x3fc));
      gba->mem.openbus_word=*ret;
      break;
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xC:
    case 0xD:{
        int maddr = addr&0x1fffffc;
        if(SB_UNLIKELY(maddr>=gba->cart.rom_size)){
          gba->mem.openbus_word = ((maddr/2)&0xffff)|(((maddr/2+1)&0xffff)<<16);
          // Return ready when done writting EEPROM (required by Minish Cap)
          if(gba->cart.backup_type==GBA_BACKUP_EEPROM) gba->mem.openbus_word = 1; 
        }else{
          // 检查是否需要实时读取ROM
          if(gba->scratch && gba->scratch->use_realtime_rom){
            // 实时读取模式：检查缓存并按需加载
            uint8_t bytes[4];
            for(int i = 0; i < 4; i++){
              size_t offset = maddr + i;
              if(offset < gba->scratch->realtime_rom_size){
                bytes[i] = gba_read_rom_byte(gba->scratch, offset);
              }else{
                bytes[i] = 0xFF;
              }
            }
            gba->mem.openbus_word = bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24);
          }else{
            // 普通模式：直接从内存读取
            gba->mem.openbus_word = *(uint32_t*)(gba->mem.cart_rom+maddr);
          }
          
          if(req_type&0x3){
            uint16_t res16 = gba->mem.openbus_word >> (addr&2)*8;
            gba->mem.openbus_word = res16*0x10001u;
          }
        }
      }
      break;
    case 0xE:
    case 0xF:
      if(gba->cart.backup_type==GBA_BACKUP_DIRECT){
        // 串口直接访问：使用预读取+缓存机制
        // SRAM是8位设备，读取1字节并复制到32位字的所有字节
        if(gba->scratch && gba->scratch->use_realtime_rom && 
           gba->scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL) {
          // 先刷新所有缓存的写入
          gba_flush_serial_writes(gba->scratch);

          uint32_t ram_addr = addr & 0xFFFF;  // 地址范围0-0xFFFF
          serial_port_t port = *(serial_port_t*)gba->scratch->rom_source_serial;
          uint8_t data = 0xFF;
          
          // 检查缓存是否命中
          bool cache_hit = false;
          if (gba->scratch->ram_prefetch_valid && 
              gba->scratch->ram_prefetch_cache &&
              ram_addr >= gba->scratch->ram_prefetch_start &&
              ram_addr < gba->scratch->ram_prefetch_start + gba->scratch->ram_prefetch_size) {
            // 缓存命中
            uint32_t offset = ram_addr - gba->scratch->ram_prefetch_start;
            data = gba->scratch->ram_prefetch_cache[offset];
            cache_hit = true;
          } else {
            // 缓存未命中，预读取2KB数据
            if (!gba->scratch->ram_prefetch_cache) {
              gba->scratch->ram_prefetch_cache = (uint8_t*)malloc(2048);
            }
            
            if (gba->scratch->ram_prefetch_cache) {
              // 计算预读起始地址(对齐到2KB边界)
              uint32_t prefetch_start = (ram_addr / 2048) * 2048;
              uint16_t prefetch_size = 2048;
              
              // 确保不超出边界
              if (prefetch_start + prefetch_size > 0x10000) {
                prefetch_size = 0x10000 - prefetch_start;
              }
              
              // 预读取数据
              if (gba_serial_read_ram(port, prefetch_start, gba->scratch->ram_prefetch_cache, prefetch_size)) {
                gba->scratch->ram_prefetch_start = prefetch_start;
                gba->scratch->ram_prefetch_size = prefetch_size;
                gba->scratch->ram_prefetch_valid = true;
                
                // 从缓存中获取数据
                uint32_t offset = ram_addr - prefetch_start;
                data = gba->scratch->ram_prefetch_cache[offset];
                cache_hit = true;
                log_printf("[Serial] RAM prefetch: addr=0x%04x, size=%u bytes\n", 
                        prefetch_start, prefetch_size);
              }
            }
          }
          
          if (!cache_hit) {
            // 缓存失败，单独读取
            gba_serial_read_ram(port, ram_addr, &data, 1);
          }
          
          // 模拟8位SRAM：将同一字节复制到32位字的所有字节
          gba->mem.sram_word = data * 0x01010101;
          ret = &gba->mem.sram_word;
        } else {
          // 没有串口，返回0xff
          gba->mem.sram_word = 0xffffffff;
          ret = &gba->mem.sram_word;
        }
      }else if(gba->cart.backup_type==GBA_BACKUP_SRAM){
        gba->mem.sram_word= gba->mem.cart_backup[(addr&0x7fff)]*0x01010101;
        ret = &gba->mem.sram_word;
      }else if(gba->cart.backup_type==GBA_BACKUP_SRAM_128K){
        gba->mem.sram_word = gba->mem.cart_backup[(addr&0xffff)+gba->cart.sram_bank*64*1024]*0x01010101;
        ret = &gba->mem.sram_word;
      }else if(gba->cart.backup_type==GBA_BACKUP_EEPROM) ret = (uint32_t*)&gba->mem.eeprom_word;
      else if(gba->cart.backup_type==GBA_BACKUP_NONE){
        gba->mem.sram_word= 0xffffffff;
        ret = &gba->mem.sram_word;
      }else{
        //Flash
        if(gba->cart.in_chip_id_mode&&addr<=0xE000001){
          gba->mem.openbus_word = *(uint32_t*)gba->mem.flash_chip_id;
          ret = &gba->mem.openbus_word;
        }else{
          gba->mem.sram_word = gba->mem.cart_backup[(addr&0xffff)+gba->cart.flash_bank*64*1024]*0x01010101;
          ret = &gba->mem.sram_word;
        }
      }
      gba->mem.openbus_word=(*ret&0xffff)*0x10001;
      break;
  }
  return ret;
}

static FORCE_INLINE void gba_audio_fifo_push(gba_t*gba, int fifo, int8_t data){
  int size = (gba->audio.fifo[fifo].write_ptr-gba->audio.fifo[fifo].read_ptr)&0x1f; 
  if(size<28){
    gba->audio.fifo[fifo].write_ptr = (gba->audio.fifo[fifo].write_ptr+1)&0x1f;
    gba->audio.fifo[fifo].data[gba->audio.fifo[fifo].write_ptr]= data; 
  }else{
    //gba->audio.fifo[fifo].write_ptr=gba->audio.fifo[fifo].read_ptr = 0; 
  }
}
static void gba_recompute_mmio_mask_table(gba_t* gba){
  for(int io_reg = 0; io_reg<256;io_reg++){
    uint32_t dword_address = 0x04000000+io_reg*4;
    uint32_t data_mask =0xffffffff;
    bool valid = true;
    if(dword_address==0x4000008)data_mask&=0xdfffdfff;
    else if(dword_address==0x4000048)data_mask &= 0x3f3f3f3f;
    else if(dword_address==0x4000050)data_mask &= 0x1F1F3FFF;
    else if(dword_address==0x4000060)data_mask &= 0xFFC0007F;
    else if(dword_address==0x4000064||dword_address==0x400006C||dword_address==0x4000074)data_mask &= 0x4000;
    else if(dword_address==0x4000068)data_mask &= 0xFFC0;
    else if(dword_address==0x4000070)data_mask &= 0xE00000E0;
    else if(dword_address==0x4000078)data_mask &= 0xff00;
    else if(dword_address==0x400007C)data_mask &= 0x40FF;
    else if(dword_address==0x4000080)data_mask &= 0x770FFF77;
    else if(dword_address==0x4000084)data_mask &= 0x008F;
    else if(dword_address==0x4000088||dword_address==0x4000134||dword_address==0x4000140||dword_address==0x4000158||dword_address==0x4000204||dword_address==0x4000208)data_mask = 0x0000ffff;
    else if(dword_address==0x40000B8||dword_address==0x40000C4||dword_address==0x40000D0)data_mask&=0xf7e00000;
    else if(dword_address==0x40000DC)data_mask&=0xFFE00000; 
    else if((dword_address>=0x4000010&& dword_address<=0x4000046) ||
            (dword_address==0x400004C) ||
            (dword_address>=0x4000054&& dword_address<=0x400005E)||
            (dword_address==0x400008C)||
            (dword_address>=0x40000A0&&dword_address<=0x40000B6)||
            (dword_address>=0x40000BC&&dword_address<=0x40000C2)||
            (dword_address>=0x40000C8&&dword_address<=0x40000CE)||
            (dword_address>=0x40000D4&&dword_address<=0x40000DA)||
            (dword_address>=0x40000E0&&dword_address<=0x40000FE)||
            (dword_address==0x400100C))valid = false;
    gba->mem.mmio_data_mask_lookup[io_reg]=data_mask;
    gba->mem.mmio_reg_valid_lookup[io_reg]=valid;
  }
}

static FORCE_INLINE void gba_process_mmio_read(gba_t *gba, uint32_t address){
  // Force recomputing timers on timer read
  if(address>= GBA_TM0CNT_L&&address<=GBA_TM3CNT_H)gba_compute_timers(gba);
}
static bool gba_process_mmio_write(gba_t *gba, uint32_t address, uint32_t data, int req_size_bytes){
  uint32_t address_u32 = address&~3; 
  uint32_t word_mask = 0xffffffff;
  uint32_t word_data = data; 
  if(req_size_bytes==2){
    word_data<<= (address&2)*8;
    word_mask =0x0000ffffu<< ((address&2)*8u);
  }else if(req_size_bytes==1){
    word_data<<= (address&3)*8;
    word_mask =0x000000ffu<< ((address&3)*8u);
  }
  word_data&=word_mask;

  if(address_u32== GBA_IE){
    uint16_t IE = gba_io_read16(gba,GBA_IE);
    uint16_t IF = gba_io_read16(gba,GBA_IF);
  
    IE = ((IE&~word_mask)|(word_data&word_mask))>>0;
    IF &= ~((word_data)>>16);
    gba_io_store16(gba,GBA_IE,IE);
    gba_io_store16(gba,GBA_IF,IF);

    return true; 
  }else if(address_u32==GBA_SOUNDCNT_L){
    uint16_t soundcnt_h = SB_BFE(data,16,16);
    // Channel volume for each FIFO
    for(int i=0;i<2;++i){
      int timer = SB_BFE(soundcnt_h,10+i*4,1);
      bool reset = SB_BFE(soundcnt_h,11+i*4,1);
      if(reset){
        gba->audio.fifo[i].read_ptr=0;
        gba->audio.fifo[i].write_ptr=0;  
        for(int d=0;d<32;++d)gba->audio.fifo[i].data[d]=0;
      }
    }
  }else if(address_u32 == GBA_TM0CNT_L||address_u32==GBA_TM1CNT_L||address_u32==GBA_TM2CNT_L||address_u32==GBA_TM3CNT_L){
    gba_compute_timers(gba);
    int timer_off = (address_u32-GBA_TM0CNT_L)/4;
    if(word_mask&0xffff){
      gba->timers[timer_off+0].pending_reload_value = word_data&(word_mask&0xffff);
    }
    if(word_mask&0xffff0000){
      gba_store16(gba,address_u32+2,(word_data>>16)&0xffff);
      gba->timers[timer_off+0].reload_value =gba->timers[timer_off+0].pending_reload_value;
    }
    gba->timer_ticks_before_event=0;
    return true;
  }else if(address_u32==GBA_POSTFLG){
    //Only BIOS can update Post Flag and haltcnt
    if(gba->cpu.registers[15]<0x4000){
      //Writes to haltcnt halt the CPU
      if(word_mask&0xff00){
        if(word_data&0x8000){
          gba->stop_mode = true;
          gba->frame_in_progress=false;
        }
        gba->cpu.wait_for_interrupt = true;
      }
      uint32_t data = gba_io_read32(gba,address_u32);
      //POST can only be initialized once, then other writes are dropped. 
      if((word_mask&0xff)&&(data&0xff))word_mask&=~0xff;
      data&=~word_mask;
      data|=word_data&word_mask;
      gba_io_store32(gba,address_u32,data);
    }
    return true; 
  /*
  Ignore these for now since it is causing audio pops in Metroid Zero
  }else if(address_u32==GBA_FIFO_A){
    // See: https://github.com/mgba-emu/mgba/issues/1847
    for(int i=0;i<4;++i){
      if(word_mask&(0xff<<(8*i)))gba_audio_fifo_push(gba,0,SB_BFE(word_data,8*i,8));
      else gba_audio_fifo_push(gba,0,gba->audio.fifo[0].data[(gba->audio.fifo[0].write_ptr)&0x1f]);
    }
  }else if(address_u32==GBA_FIFO_B){
    for(int i=0;i<4;++i){
      if(word_mask&(0xff<<(8*i)))gba_audio_fifo_push(gba,1,SB_BFE(word_data,8*i,8));
      else gba_audio_fifo_push(gba,1,gba->audio.fifo[1].data[(gba->audio.fifo[1].write_ptr)&0x1f]);
    }
  */
  }else if(address_u32==GBA_BG2X||address_u32==GBA_BG3X){
    int aff_bg = (address_u32-GBA_BG2X)/0x10;
    gba->ppu.aff[aff_bg].wrote_bgx= true;
  }else if(address_u32==GBA_BG2Y||address_u32==GBA_BG3Y){
    int aff_bg = (address_u32-GBA_BG2Y)/0x10;
    gba->ppu.aff[aff_bg].wrote_bgy = true;
  }else if(address_u32==GBA_DMA0CNT_L||address_u32==GBA_DMA1CNT_L||
           address_u32==GBA_DMA2CNT_L||address_u32==GBA_DMA3CNT_L){
    gba->activate_dmas=true;
  }else if (address_u32==GBA_WAITCNT){
     uint16_t waitcnt = gba_io_read16(gba,GBA_WAITCNT);
     waitcnt = ((waitcnt&~word_mask)|(word_data&word_mask));
     gba_recompute_waitstate_table(gba,waitcnt);
  }else if(address_u32==GBA_KEYINPUT){
    if(word_mask&0xffff0000){
      gba_store16(gba,GBA_KEYINPUT,(word_data>>16)&0xffff);
    }
    gba_tick_keypad(NULL,gba);
  }else if(address_u32>=GBA_SOUND1CNT_L&&address_u32<GBA_WAVE_RAM){
    for(int i=0;i<4;++i){
      if(word_mask&(0xff<<(i*8))){
        uint8_t data =gba_audio_process_byte_write(gba,address_u32+i,SB_BFE(word_data,(i*8),8));
        gba_io_store8(gba,address_u32+i,data);
      }
    }
    gba_process_audio_writes(gba);
    return true;
  }
  return false;
}
int gba_search_rom_for_backup_string(gba_t* gba){
  int btype = GBA_BACKUP_NONE; 
  for(int b = 0; b< gba->cart.rom_size;++b){
    const char* strings[]={"EEPROM_", "SRAM_", "FLASH_","FLASH512_","FLASH1M_"};
    int backup_type[]= {GBA_BACKUP_EEPROM,GBA_BACKUP_SRAM,GBA_BACKUP_FLASH_64K, GBA_BACKUP_FLASH_64K, GBA_BACKUP_FLASH_128K};
    for(int type = 0; type<sizeof(strings)/sizeof(strings[0]);++type){
      int str_off = 0; 
      bool matches = true; 
      const char* str = strings[type];
      while(str[str_off] && matches){
        if(b+str_off>=gba->cart.rom_size)matches=false; 
        else if(str[str_off]!=gba->mem.cart_rom[b+str_off])matches = false;
        ++str_off;
      }
      if(matches){
        if(btype!=backup_type[type]&&btype!=GBA_BACKUP_NONE){
          log_printf("Found multiple backup types, defaulting to none\n");
          return GBA_BACKUP_NONE;
        }
        btype = backup_type[type];
      }
    }
  }
  return btype; 
}

// 实时ROM读取辅助函数 - 读取指定范围的字节
static bool gba_load_realtime_rom_bytes_from_file(FILE* file, size_t offset, uint8_t* buffer, size_t size) {
  if (!file) {
    return false;
  }
  
  if (fseek(file, offset, SEEK_SET) != 0) {
    log_printf("Failed to seek to offset %zu in ROM file\n", offset);
    return false;
  }
  
  size_t bytes_read = fread(buffer, 1, size, file);
  if (bytes_read != size) {
    log_printf("Failed to read %zu bytes at offset %zu: read %zu bytes\n", size, offset, bytes_read);
    return false;
  }
  
  return true;
}


// DTR控制函数(模拟Python的ser.dtr操作)
static void gba_serial_set_dtr(serial_port_t port, bool state) {
#ifdef _WIN32
  EscapeCommFunction(port, state ? SETDTR : CLRDTR);
#else
  int flag = TIOCM_DTR;
  if (state) {
    ioctl(port, TIOCMBIS, &flag); // Set DTR
  } else {
    ioctl(port, TIOCMBIC, &flag); // Clear DTR
  }
#endif
}

// 自动查找串口设备(VID=0x0483, PID=0x0721)
static char* gba_auto_find_serial_port() {
#ifdef _WIN32
  // Windows: 使用SetupAPI枚举串口设备
  static char port_path[256] = {0};
  HDEVINFO deviceInfoSet;
  SP_DEVINFO_DATA deviceInfoData;
  DWORD deviceIndex = 0;
  
  log_printf("[Serial] Searching for USB serial devices (Windows)...\n");
  
  // 获取所有串口设备
  deviceInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
  if (deviceInfoSet == INVALID_HANDLE_VALUE) {
    log_printf("[Serial] ERROR: Failed to get device information set\n");
    return NULL;
  }
  
  deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
  
  // 枚举所有串口设备
  while (SetupDiEnumDeviceInfo(deviceInfoSet, deviceIndex++, &deviceInfoData)) {
    HKEY hDeviceRegistryKey;
    char portName[256] = {0};
    DWORD size = sizeof(portName);
    
    // 获取端口名称 (COM1, COM2, 等)
    hDeviceRegistryKey = SetupDiOpenDevRegKey(deviceInfoSet, &deviceInfoData,
                                               DICS_FLAG_GLOBAL, 0,
                                               DIREG_DEV, KEY_READ);
    if (hDeviceRegistryKey != INVALID_HANDLE_VALUE) {
      DWORD type = REG_SZ;
      if (RegQueryValueExA(hDeviceRegistryKey, "PortName", NULL, &type,
                           (LPBYTE)portName, &size) == ERROR_SUCCESS) {
        
        // 获取设备的硬件ID以提取VID和PID
        char hardwareID[1024] = {0};
        if (SetupDiGetDeviceRegistryPropertyA(deviceInfoSet, &deviceInfoData,
                                               SPDRP_HARDWAREID, NULL,
                                               (PBYTE)hardwareID, sizeof(hardwareID), NULL)) {
          
          // 解析 VID 和 PID (格式: USB\VID_0483&PID_0721)
          int vid = 0, pid = 0;
          if (sscanf(hardwareID, "USB\\VID_%x&PID_%x", &vid, &pid) == 2) {
            log_printf("[Serial]   Found: %s (VID=0x%04x, PID=0x%04x)\n", portName, vid, pid);
            
            if (vid == 0x0483 && pid == 0x0721) {
              log_printf("[Serial]   *** Matched target device!\n");
              snprintf(port_path, sizeof(port_path), "\\\\.\\%s", portName);
              RegCloseKey(hDeviceRegistryKey);
              SetupDiDestroyDeviceInfoList(deviceInfoSet);
              return port_path;
            }
          }
        }
      }
      RegCloseKey(hDeviceRegistryKey);
    }
  }
  
  SetupDiDestroyDeviceInfoList(deviceInfoSet);
  log_printf("[Serial] No matching device found (VID=0x0483, PID=0x0721)\n");
  return NULL;
  
#elif defined(__APPLE__)
  // macOS: 使用IOKit查找USB设备
  static char port_path[256] = {0};
  io_iterator_t serialPortIterator = 0;
  io_object_t serialPort;
  
  // 创建匹配字典
  CFMutableDictionaryRef matchingDict = IOServiceMatching(kIOSerialBSDServiceValue);
  if (!matchingDict) return NULL;
  
  // 查找所有串口设备
  kern_return_t kr = IOServiceGetMatchingServices(kIOMasterPortDefault, matchingDict, &serialPortIterator);
  if (kr != KERN_SUCCESS) return NULL;
  
  log_printf("[Serial] Searching for USB serial devices...\n");
  
  while ((serialPort = IOIteratorNext(serialPortIterator))) {
    // 获取设备路径
    CFTypeRef devicePath = IORegistryEntryCreateCFProperty(serialPort,
                                                           CFSTR(kIOCalloutDeviceKey),
                                                           kCFAllocatorDefault, 0);
    if (devicePath) {
      if (CFStringGetCString(devicePath, port_path, sizeof(port_path), kCFStringEncodingUTF8)) {
        // 获取USB父设备以检查VID/PID
        io_registry_entry_t parent;
        kern_return_t kr = IORegistryEntryGetParentEntry(serialPort, kIOServicePlane, &parent);
        while (kr == KERN_SUCCESS) {
          CFTypeRef vid = IORegistryEntryCreateCFProperty(parent, CFSTR("idVendor"), kCFAllocatorDefault, 0);
          CFTypeRef pid = IORegistryEntryCreateCFProperty(parent, CFSTR("idProduct"), kCFAllocatorDefault, 0);
          
          if (vid && pid) {
            int vendor_id = 0, product_id = 0;
            CFNumberGetValue(vid, kCFNumberIntType, &vendor_id);
            CFNumberGetValue(pid, kCFNumberIntType, &product_id);
            
            log_printf("[Serial]   Found: %s (VID=0x%04x, PID=0x%04x)\n", port_path, vendor_id, product_id);
            
            if (vendor_id == 0x0483 && product_id == 0x0721) {
              log_printf("[Serial]   *** Matched target device!\n");
              if (vid) CFRelease(vid);
              if (pid) CFRelease(pid);
              IOObjectRelease(parent);
              CFRelease(devicePath);
              IOObjectRelease(serialPort);
              IOObjectRelease(serialPortIterator);
              return port_path;
            }
            if (vid) CFRelease(vid);
            if (pid) CFRelease(pid);
          }
          
          io_registry_entry_t next_parent;
          kr = IORegistryEntryGetParentEntry(parent, kIOServicePlane, &next_parent);
          IOObjectRelease(parent);
          parent = next_parent;
        }
      }
      CFRelease(devicePath);
    }
    IOObjectRelease(serialPort);
  }
  IOObjectRelease(serialPortIterator);
  
  log_printf("[Serial] No matching device found (VID=0x0483, PID=0x0721)\n");
  return NULL;
#elif defined(__linux__)
  // Linux: 读取/sys/bus/usb-serial/devices/
  static char port_path[256] = {0};
  DIR *dir = opendir("/sys/class/tty");
  if (!dir) return NULL;
  
  log_printf("[Serial] Searching for USB serial devices...\n");
  
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strncmp(entry->d_name, "ttyUSB", 6) == 0 || strncmp(entry->d_name, "ttyACM", 6) == 0) {
      char path[512];
      snprintf(path, sizeof(path), "/sys/class/tty/%s/device/../idVendor", entry->d_name);
      
      FILE *f = fopen(path, "r");
      if (f) {
        int vid = 0;
        fscanf(f, "%x", &vid);
        fclose(f);
        
        snprintf(path, sizeof(path), "/sys/class/tty/%s/device/../idProduct", entry->d_name);
        f = fopen(path, "r");
        if (f) {
          int pid = 0;
          fscanf(f, "%x", &pid);
          fclose(f);
          
          snprintf(port_path, sizeof(port_path), "/dev/%s", entry->d_name);
          log_printf("[Serial]   Found: %s (VID=0x%04x, PID=0x%04x)\n", port_path, vid, pid);
          
          if (vid == 0x0483 && pid == 0x0721) {
            log_printf("[Serial]   *** Matched target device!\n");
            closedir(dir);
            return port_path;
          }
        }
      }
    }
  }
  closedir(dir);
  
  log_printf("[Serial] No matching device found (VID=0x0483, PID=0x0721)\n");
  return NULL;
#else
  log_printf("[Serial] Auto-detection not implemented for this platform\n");
  return NULL;
#endif
}

static serial_port_t gba_open_serial_port(const char* port_name) {
  // 如果是 "AUTO"，自动查找设备
  if (strcmp(port_name, "AUTO") == 0) {
    log_printf("[Serial] AUTO mode: searching for device...\n");
    const char* found_port = gba_auto_find_serial_port();
    if (!found_port) {
      log_printf("[Serial] ERROR: No device found in AUTO mode\n");
      return INVALID_SERIAL_PORT;
    }
    port_name = found_port;
    log_printf("[Serial] Using auto-detected port: %s\n", port_name);
  }
  
#ifdef _WIN32
  HANDLE handle = CreateFileA(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL,
                               OPEN_EXISTING, 0, NULL);
  if (handle == INVALID_HANDLE_VALUE) {
    log_printf("Failed to open serial port: %s\n", port_name);
    return INVALID_SERIAL_PORT;
  }
  
  DCB dcb = {0};
  dcb.DCBlength = sizeof(DCB);
  if (!GetCommState(handle, &dcb)) {
    CloseHandle(handle);
    return INVALID_SERIAL_PORT;
  }
  
  dcb.BaudRate = CBR_115200;
  dcb.ByteSize = 8;
  dcb.StopBits = ONESTOPBIT;
  dcb.Parity = NOPARITY;
  
  if (!SetCommState(handle, &dcb)) {
    CloseHandle(handle);
    return INVALID_SERIAL_PORT;
  }
  
  // 执行DTR重置序列
  gba_serial_set_dtr(handle, true);
  Sleep(100); // 等待100ms
  gba_serial_set_dtr(handle, false);
  Sleep(100); // 等待100ms
  
  log_printf("Serial port opened and reset: %s\n", port_name);
  
  return handle;
#else
  int fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    log_printf("Failed to open serial port: %s\n", port_name);
    return INVALID_SERIAL_PORT;
  }
  
  struct termios tty;
  if (tcgetattr(fd, &tty) != 0) {
    close(fd);
    return INVALID_SERIAL_PORT;
  }
  
  // 设置波特率 115200
  cfsetospeed(&tty, B115200);
  cfsetispeed(&tty, B115200);
  
  // 配置为8N1(8数据位，无校验，1停止位)
  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8位数据
  tty.c_cflag &= ~PARENB;                          // 无校验
  tty.c_cflag &= ~PARODD;
  tty.c_cflag &= ~CSTOPB;                          // 1停止位
  tty.c_cflag &= ~CRTSCTS;                         // 禁用硬件流控
  tty.c_cflag |= (CLOCAL | CREAD);                 // 启用接收，忽略调制解调器状态
  
  // 禁用软件流控
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
  
  // 原始模式
  tty.c_lflag = 0;  // 非规范模式，无回显
  tty.c_oflag = 0;  // 禁用输出处理
  
  // 设置读取超时
  tty.c_cc[VMIN] = 0;   // 非阻塞读取
  tty.c_cc[VTIME] = 20; // 2秒超时(单位0.1秒)，匹配Python的timeout=2
  
  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    close(fd);
    return INVALID_SERIAL_PORT;
  }
  
  // 执行DTR重置序列(就像Python代码中的 ser.dtr = True; ser.dtr = False)
  //printf("[Serial] Performing DTR reset sequence...\n");
  gba_serial_set_dtr(fd, true);
  log_printf("[Serial] DTR = HIGH\n");
  usleep(10000); // 等待10ms
  gba_serial_set_dtr(fd, false);
  log_printf("[Serial] DTR = LOW\n");
  usleep(20000); // 等待20ms，给设备更多时间初始化
  
  // 清空可能残留的数据
  tcflush(fd, TCIOFLUSH);
  
  log_printf("[Serial] Port %s opened successfully\n", port_name);
  log_printf("[Serial] Configuration: 115200 8N1, no flow control\n");
  
  return fd;
#endif
}

static void gba_close_serial_port(serial_port_t port) {
  if (port == INVALID_SERIAL_PORT) return;
#ifdef _WIN32
  CloseHandle(port);
#else
  close(port);
#endif
}

static size_t gba_serial_write(serial_port_t port, const uint8_t* data, size_t size) {
#ifdef _WIN32
  DWORD written = 0;
  if (!WriteFile(port, data, size, &written, NULL)) {
    log_printf("[Serial] WriteFile error: %lu\n", GetLastError());
    return 0;
  }
  FlushFileBuffers(port); // 确保数据发送
  return written;
#else
  ssize_t written = write(port, data, size);
  if (written < 0) {
    log_printf("[Serial] Write error: %s (errno=%d)\n", strerror(errno), errno);
    return 0;
  }
  // 确保数据发送完成
  tcdrain(port);
  return written;
#endif
}

static size_t gba_serial_read(serial_port_t port, uint8_t* data, size_t size) {
#ifdef _WIN32
  DWORD read_count = 0;
  if (!ReadFile(port, data, size, &read_count, NULL)) {
    return 0;
  }
  return read_count;
#else
  size_t total_read = 0;
  while (total_read < size) {
    ssize_t n = read(port, data + total_read, size - total_read);
    if (n < 0) {
      if (errno == EAGAIN || errno == EINTR) {
        continue; // 重试
      }
      return total_read; // 错误
    } else if (n == 0) {
      break; // 超时或连接关闭
    }
    total_read += n;
  }
  return total_read;
#endif
}

// 根据Python协议实现串口读取ROM
// Python代码: cmd.extend(struct.pack("<H", 2 + 1 + 4 + 2 + 2))
//            cmd.append(0xf6)
//            cmd.extend(struct.pack("<I", addr_word << 1))
//            cmd.extend(struct.pack("<H", length_byte))
//            cmd.extend([0, 0])
static bool gba_serial_read_rom(serial_port_t port, uint32_t addr_word, uint8_t* buffer, uint16_t length_byte) {
  // 构建命令包: readRom(addr_word, length_byte)
  // 注意：Python中addr_word是字地址，传输时需要<<1转为字节地址
  uint8_t cmd[11];
  uint16_t cmd_len = 11; // 2(长度) + 1(命令) + 4(地址) + 2(长度) + 2(填充) = 11
  
  // 填充命令总长度(小端序)
  cmd[0] = cmd_len & 0xFF;
  cmd[1] = (cmd_len >> 8) & 0xFF;
  
  // 命令码 0xf6 (读ROM)
  cmd[2] = 0xf6;
  
  // 地址：将字地址转为字节地址(addr_word << 1)，小端序
  uint32_t byte_addr = addr_word << 1;
  cmd[3] = byte_addr & 0xFF;
  cmd[4] = (byte_addr >> 8) & 0xFF;
  cmd[5] = (byte_addr >> 16) & 0xFF;
  cmd[6] = (byte_addr >> 24) & 0xFF;
  
  // 数据长度(小端序)
  cmd[7] = length_byte & 0xFF;
  cmd[8] = (length_byte >> 8) & 0xFF;
  
  // 填充字节
  cmd[9] = 0;
  cmd[10] = 0;
  
  // 打印详细的命令信息
  // printf("\n[Serial] === Read ROM Command ===\n");
  // printf("[Serial] addr_word=0x%08x, byte_addr=0x%08x, length=%u bytes\n", 
  //        addr_word, byte_addr, length_byte);
  // printf("[Serial] Command bytes (11): ");
  // for (int i = 0; i < 11; i++) {
  //   printf("%02x ", cmd[i]);
  // }
  // printf("\n");
  // printf("[Serial] Breakdown:\n");
  // printf("[Serial]   Length: %02x %02x (%u)\n", cmd[0], cmd[1], cmd_len);
  // printf("[Serial]   Command: %02x\n", cmd[2]);
  // printf("[Serial]   Address: %02x %02x %02x %02x (0x%08x)\n", 
  //        cmd[3], cmd[4], cmd[5], cmd[6], byte_addr);
  // printf("[Serial]   Data Length: %02x %02x (%u)\n", cmd[7], cmd[8], length_byte);
  // printf("[Serial]   Padding: %02x %02x\n", cmd[9], cmd[10]);
  
  // 清空串口缓冲区
#ifndef _WIN32
  tcflush(port, TCIOFLUSH);
#else
  PurgeComm(port, PURGE_RXCLEAR | PURGE_TXCLEAR);
#endif
  
  // 发送命令
  // printf("[Serial] Sending command...\n");
  size_t written = gba_serial_write(port, cmd, 11);
  if (written != 11) {
    log_printf("[Serial] ERROR: Failed to write command (wrote %zu/11 bytes)\n", written);
    return false;
  }
  // printf("[Serial] Command sent successfully (%zu bytes)\n", written);
  
  
  // 读取响应 (length_byte + 2)
  size_t response_size = length_byte + 2;
  uint8_t* response = (uint8_t*)malloc(response_size);
  if (!response) {
    log_printf("[Serial] ERROR: Failed to allocate %zu bytes for response\n", response_size);
    return false;
  }
  
  // 尝试一次性读取所有数据(类似Python的ser.read(size))
  size_t total_read = 0;
  int retry_count = 0;
  const int max_retries = 30; // 减少重试次数，但增加每次等待时间
  
  while (total_read < response_size && retry_count < max_retries) {
    size_t read_count = gba_serial_read(port, response + total_read, response_size - total_read);
    if (read_count > 0) {
      total_read += read_count;
      // printf("[Serial] Read %zu bytes, total: %zu/%zu (%.1f%%)\n", 
      //        read_count, total_read, response_size, (total_read * 100.0) / response_size);
      // 如果还没读完，稍微等待一下
      if (total_read < response_size) {
#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
      }
    } else {
      retry_count++;
      if (retry_count == 1 || retry_count % 5 == 0) {
        log_printf("[Serial] Waiting for data... retry %d, received: %zu/%zu bytes\n", 
               retry_count, total_read, response_size);
      }
      // 每次重试等待更长时间
#ifdef _WIN32
      Sleep(10);
#else
      usleep(10000);
#endif
    }
  }
  
  if (total_read < response_size) {
    log_printf("[Serial] ERROR: Failed to read complete response (read %zu/%zu bytes after %d retries)\n", 
           total_read, response_size, retry_count);
    if (total_read > 0) {
      log_printf("[Serial] Received data: ");
      for (size_t i = 0; i < (total_read < 32 ? total_read : 32); i++) {
        log_printf("%02x ", response[i]);
      }
      if (total_read > 32) printf("...");
      log_printf("\n");
    }
    free(response);
    return false;
  }
  
  // printf("[Serial] Response received successfully\n");
  // printf("[Serial] First 16 bytes: ");
  // for (size_t i = 0; i < (response_size < 16 ? response_size : 16); i++) {
  //   printf("%02x ", response[i]);
  // }
  // if (response_size > 16) printf("...");
  // printf("\n");
  
  // 跳过前2字节，复制数据
  memcpy(buffer, response + 2, length_byte);
  free(response);
  
  return true;
}

// 刷新串口写入缓存 - 合并连续地址的写入(ROM和RAM分别合并)
static void gba_flush_serial_writes(gba_scratch_t *scratch) {
  if (!scratch || scratch->serial_write_count == 0) {
    return;
  }
  
  static int flush_count = 0;
  if (flush_count < 10 || flush_count % 100 == 0) {
    log_printf("[Serial] Flushing write buffer #%d: %d writes pending\n", 
           flush_count, scratch->serial_write_count);
  }
  flush_count++;
  
  serial_port_t port = *(serial_port_t*)scratch->rom_source_serial;
  
  // 合并连续地址的写入(保持原始顺序，ROM和RAM分别合并)
  int i = 0;
  static int batch_count = 0;
  
  while (i < scratch->serial_write_count) {
    gba_serial_write_entry_t *start = &scratch->serial_write_buffer[i];
    
    // 查找连续的写入(必须是同类型且地址连续)
    int count = 1;
    while (i + count < scratch->serial_write_count) {
      gba_serial_write_entry_t *next = &scratch->serial_write_buffer[i + count];
      // 关键检查：同类型(ROM/RAM)且地址连续
      if (next->is_rom == start->is_rom && 
          next->addr == start->addr + count) {
        count++;
      } else {
        break;  // 遇到不同类型或地址不连续，停止合并
      }
    }
    
    // 如果找到多个连续的写入，合并发送
    if (count > 1) {
      // 分配数据缓冲区
      uint8_t *data = (uint8_t*)malloc(count);
      if (data) {
        for (int j = 0; j < count; j++) {
          data[j] = scratch->serial_write_buffer[i + j].data;
        }
        
        if (batch_count < 200 || batch_count % 400 == 0) {
          log_printf("[Serial] Batch #%d: %s addr=0x%08x, %d bytes (merged) ", 
                 batch_count, start->is_rom ? "ROM" : "RAM", start->addr, count);
          //print data (first 16byte)
          for (int j = 0; j < (count < 16 ? count : 16); j++) {
            log_printf("%02x ", data[j]);
          }
          if (count > 16) printf("...");
          printf("\n");
        }
        batch_count++;
        
        // 根据类型选择写入函数
        if (start->is_rom) {
          uint32_t addr_word = start->addr >> 1; // 转为字地址
          gba_serial_write_rom(port, addr_word, data, count);
        } else {
          gba_serial_write_ram(port, start->addr, data, count);
        }
        
        free(data);
      }
    } else {
      // 单个写入
      if (batch_count < 200 || batch_count % 400 == 0) {
        log_printf("[Serial] Batch #%d: %s addr=0x%08x, 1 byte\n", 
               batch_count, start->is_rom ? "ROM" : "RAM", start->addr);
      }
      batch_count++;
      
      if (start->is_rom) {
        uint32_t addr_word = start->addr >> 1; // 转为字地址
        uint16_t data = start->data; // 单字节数据
        gba_serial_write_rom(port, addr_word, (uint8_t*)&data, 2);
      } else {
        gba_serial_write_ram(port, start->addr, &start->data, 1);
      }
    }
    
    i += count;
  }
  
  // 清空缓存
  scratch->serial_write_count = 0;
}

// 添加写入到缓存
static void gba_buffer_serial_write(gba_scratch_t *scratch, uint32_t addr, uint8_t data, bool is_rom) {
  if (!scratch) return;
  
  // 任何写入(无论ROM还是RAM)都会使RAM预读缓存失效
  if (scratch->ram_prefetch_valid) {
    scratch->ram_prefetch_valid = false;
      log_printf("[Serial] RAM prefetch cache invalidated due to %s write at 0x%08x\n",
            is_rom ? "ROM" : "RAM", addr);
  }
  // 疑似写命令,部分地址标记为不缓存
  if (data == 0x30 || data == 0x20 || data == 0x60) {
    for (int i = 0; i < 512; i++) {
        scratch->rom_cache_valid[addr + i] = 0xFF;
    }
  }
  scratch->rom_cache_valid[addr] = 0xFF;

  // 如果缓存满了，先刷新
  if (scratch->serial_write_count >= GBA_SERIAL_WRITE_BUFFER_SIZE) {
    gba_flush_serial_writes(scratch);
  }
  
  // 添加到缓存
  gba_serial_write_entry_t *entry = &scratch->serial_write_buffer[scratch->serial_write_count++];
  entry->addr = addr;
  entry->data = data;
  entry->is_rom = is_rom;
}

// 串口写入ROM(命令0xf5)
// Python: writeRom(addr_word, dat)
static bool gba_serial_write_rom(serial_port_t port, uint32_t addr_word, const uint8_t* data, uint16_t data_len) {
  if (port == INVALID_SERIAL_PORT || !data || data_len == 0) {
    return false;
  }
  
  // 构建命令包
  uint16_t cmd_len = 2 + 1 + 4 + data_len + 2;
  uint8_t* cmd = (uint8_t*)malloc(cmd_len);
  if (!cmd) return false;
  
  // Python: struct.pack("<H", 2 + 1 + 4 + len(dat) + 2)
  // 长度字段包含：长度字段自己(2) + 命令(1) + 地址(4) + 数据(data_len) + 填充(2)
  uint16_t body_len = 2 + 1 + 4 + data_len + 2;
  cmd[0] = body_len & 0xFF;
  cmd[1] = (body_len >> 8) & 0xFF;
  cmd[2] = 0xf5; // 写ROM命令
  
  // 地址(字地址，不需要<<1)
  cmd[3] = addr_word & 0xFF;
  cmd[4] = (addr_word >> 8) & 0xFF;
  cmd[5] = (addr_word >> 16) & 0xFF;
  cmd[6] = (addr_word >> 24) & 0xFF;
  
  // 数据
  memcpy(cmd + 7, data, data_len);
  
  // 填充
  cmd[7 + data_len] = 0;
  cmd[8 + data_len] = 0;
  
  // 发送命令
  size_t written = gba_serial_write(port, cmd, cmd_len);
  free(cmd);
  
  if (written != cmd_len) {
    log_printf("[Serial] ERROR: Failed to write ROM command\n");
    return false;
  }
  
  // 尝试读取1字节ACK(非阻塞，超时后继续)
  uint8_t ack;
  size_t ack_read = 0;
  int retry = 0;
  while (ack_read == 0 && retry < 5) {
    ack_read = gba_serial_read(port, &ack, 1);
    if (ack_read == 0) {
      retry++;
#ifdef _WIN32
      Sleep(1);
#else
      usleep(1000);
#endif
    }
  }
  
  // ACK读取失败只记录警告，不中断操作
  if (ack_read != 1) {
    static int warn_count = 0;
    if (warn_count < 3) {
      log_printf("[Serial] WARNING: No ACK for ROM write (retry %d)\n", retry);
      warn_count++;
    }
  }
  
  return true;
}

// 串口读取RAM(命令0xf8)
// Python: readRam(addr, length_byte)
static bool gba_serial_read_ram(serial_port_t port, uint32_t addr, uint8_t* buffer, uint16_t length_byte) {
  if (port == INVALID_SERIAL_PORT || !buffer || length_byte == 0) {
    return false;
  }
  
  // 构建命令包
  uint8_t cmd[11];
  // Python: struct.pack("<H", 2 + 1 + 4 + 2 + 2) = 11
  // 长度字段包含：长度字段自己(2) + 命令(1) + 地址(4) + 数据长度(2) + 填充(2)
  uint16_t cmd_body_len = 2 + 1 + 4 + 2 + 2;

  cmd[0] = cmd_body_len & 0xFF;
  cmd[1] = (cmd_body_len >> 8) & 0xFF;
  cmd[2] = 0xf8; // 读RAM命令
  
  // 地址(字节地址)
  cmd[3] = addr & 0xFF;
  cmd[4] = (addr >> 8) & 0xFF;
  cmd[5] = (addr >> 16) & 0xFF;
  cmd[6] = (addr >> 24) & 0xFF;
  
  // 长度
  cmd[7] = length_byte & 0xFF;
  cmd[8] = (length_byte >> 8) & 0xFF;
  
  // 填充
  cmd[9] = 0;
  cmd[10] = 0;
  
  // 发送命令
  size_t written = gba_serial_write(port, cmd, 11);
  if (written != 11) {
    log_printf("[Serial] ERROR: Failed to write RAM read command\n");
    return false;
  }
  
  // 读取响应 (length_byte + 2)
  size_t response_size = length_byte + 2;
  uint8_t* response = (uint8_t*)malloc(response_size);
  if (!response) {
    return false;
  }
  
  size_t total_read = 0;
  int retry_count = 0;
  const int max_retries = 20;
  
  while (total_read < response_size && retry_count < max_retries) {
    size_t read_count = gba_serial_read(port, response + total_read, response_size - total_read);
    if (read_count > 0) {
      total_read += read_count;
      if (total_read < response_size) {
#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
      }
    } else {
      retry_count++;
#ifdef _WIN32
      Sleep(10);
#else
      usleep(10000);
#endif
    }
  }
  
  if (total_read < response_size) {
    log_printf("[Serial] ERROR: Failed to read RAM response (%zu/%zu bytes)\n", total_read, response_size);
    free(response);
    return false;
  }
  
  // 跳过前2字节，复制数据
  memcpy(buffer, response + 2, length_byte);
  free(response);
  
  return true;
}

// 串口写入RAM(命令0xf7)
// Python: writeRam(addr, dat)
static bool gba_serial_write_ram(serial_port_t port, uint32_t addr, const uint8_t* data, uint16_t data_len) {
  if (port == INVALID_SERIAL_PORT || !data || data_len == 0) {
    return false;
  }
  
  // 构建命令包
  uint16_t cmd_len = 2 + 1 + 4 + data_len + 2;
  uint8_t* cmd = (uint8_t*)malloc(cmd_len);
  if (!cmd) return false;
  
  // Python: struct.pack("<H", 2 + 1 + 4 + len(dat) + 2)
  // 长度字段包含：长度字段自己(2) + 命令(1) + 地址(4) + 数据(data_len) + 填充(2)
  uint16_t body_len = 2 + 1 + 4 + data_len + 2;
  cmd[0] = body_len & 0xFF;
  cmd[1] = (body_len >> 8) & 0xFF;
  cmd[2] = 0xf7; // 写RAM命令
  
  // 地址(字节地址)
  cmd[3] = addr & 0xFF;
  cmd[4] = (addr >> 8) & 0xFF;
  cmd[5] = (addr >> 16) & 0xFF;
  cmd[6] = (addr >> 24) & 0xFF;
  
  // 数据
  memcpy(cmd + 7, data, data_len);
  
  // 填充
  cmd[7 + data_len] = 0;
  cmd[8 + data_len] = 0;
  
  // 发送命令
  size_t written = gba_serial_write(port, cmd, cmd_len);
  free(cmd);
  
  if (written != cmd_len) {
    log_printf("[Serial] ERROR: Failed to write RAM command\n");
    return false;
  }
  
  // 尝试读取1字节ACK(非阻塞，超时后继续)
  uint8_t ack;
  size_t ack_read = 0;
  int retry = 0;
  while (ack_read == 0 && retry < 5) {
    ack_read = gba_serial_read(port, &ack, 1);
    if (ack_read == 0) {
      retry++;
#ifdef _WIN32
      Sleep(1);
#else
      usleep(1000);
#endif
    }
  }
  
  // ACK读取失败只记录警告，不中断操作
  if (ack_read != 1) {
    static int warn_count = 0;
    if (warn_count < 3) {
      log_printf("[Serial] WARNING: No ACK for RAM write (retry %d)\n", retry);
      warn_count++;
    }
  }
  
  return true;
}

// 串口Flash编程(命令0xf9)
// 用于将Flash内容写入到卡带的物理Flash
static bool gba_serial_flash_program(serial_port_t port, uint32_t addr, const uint8_t* data, uint16_t data_len) {
  if (port == INVALID_SERIAL_PORT || !data || data_len == 0) {
    return false;
  }
  
  // 构建命令包
  uint16_t cmd_len = 2 + 1 + 4 + data_len + 2;
  uint8_t* cmd = (uint8_t*)malloc(cmd_len);
  if (!cmd) return false;
  
  // 长度字段包含：长度字段自己(2) + 命令(1) + 地址(4) + 数据(data_len) + 填充(2)
  uint16_t body_len = 2 + 1 + 4 + data_len + 2;
  cmd[0] = body_len & 0xFF;
  cmd[1] = (body_len >> 8) & 0xFF;
  cmd[2] = 0xf9; // Flash编程命令
  
  // 地址(字节地址)
  cmd[3] = addr & 0xFF;
  cmd[4] = (addr >> 8) & 0xFF;
  cmd[5] = (addr >> 16) & 0xFF;
  cmd[6] = (addr >> 24) & 0xFF;
  
  // 数据
  memcpy(cmd + 7, data, data_len);
  
  // 填充
  cmd[7 + data_len] = 0;
  cmd[8 + data_len] = 0;
  
  // 发送命令
  size_t written = gba_serial_write(port, cmd, cmd_len);
  free(cmd);
  
  if (written != cmd_len) {
    log_printf("[Serial] ERROR: Failed to write Flash program command\n");
    return false;
  }
  
  // Flash编程需要更长的等待时间
#ifdef _WIN32
  Sleep(100);
#else
  usleep(100000);
#endif
  
  // 读取ACK
  uint8_t ack;
  size_t ack_read = 0;
  int retry = 0;
  while (ack_read == 0 && retry < 10) {
    ack_read = gba_serial_read(port, &ack, 1);
    if (ack_read == 0) {
      retry++;
#ifdef _WIN32
      Sleep(10);
#else
      usleep(10000);
#endif
    }
  }
  
  if (ack_read != 1 || ack != 0xAA) {
    log_printf("[Serial] WARNING: No ACK for Flash program (retry %d)\n", retry);
    return false;
  }
  
  return true;
}

// 读取backup字节(支持串口)
static uint8_t gba_read_backup_byte(gba_t* gba, uint32_t addr) {
  // 检查是否使用串口ROM
  if (gba->scratch && gba->scratch->use_realtime_rom && 
      gba->scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL) {
    // 串口模式：读取前先刷新所有缓存的写入
    if (gba->scratch->serial_write_count > 0) {
      gba_flush_serial_writes(gba->scratch);
    }
    
    // 通过串口读取RAM
    serial_port_t port = *(serial_port_t*)gba->scratch->rom_source_serial;
    uint8_t data = 0xFF;
    if (gba_serial_read_ram(port, addr, &data, 1)) {
      return data;
    }
    return 0xFF;
  }
  // 普通模式：从内存读取
  return gba->mem.cart_backup[addr];
}

// 写入backup字节(支持串口)
static void gba_write_backup_byte(gba_t* gba, uint32_t addr, uint8_t data) {
  // 检查是否使用串口ROM
  if (gba->scratch && gba->scratch->use_realtime_rom && 
      gba->scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL) {
    // 通过串口写入RAM
    serial_port_t port = *(serial_port_t*)gba->scratch->rom_source_serial;
    gba_serial_write_ram(port, addr, &data, 1);
    return;
  }
  // 普通模式：写入内存
  gba->mem.cart_backup[addr] = data;
}

// 字节级ROM读取 - 支持按需加载和缓存
// 当缓存未命中时，会读取一整块数据以优化性能
#define GBA_ROM_CACHE_CHUNK_SIZE 4096

static uint8_t gba_read_rom_byte(gba_scratch_t *scratch, size_t offset) {
  if (!scratch->use_realtime_rom) {
    return 0xFF; // 不应该调用到这里
  }
  
  // 串口模式：读取前先刷新所有缓存的写入
  if (scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL && scratch->serial_write_count > 0) {
    gba_flush_serial_writes(scratch);
  }
  
  if (offset >= scratch->realtime_rom_size) {
    return 0xFF;
  }
  
  // 检查缓存是否已有这个字节
  if (scratch->rom_cache_valid[offset] == 1) {
    return scratch->rom_cache_data[offset];
  }

  // 缓存未命中，读取一整块数据以优化后续访问
  // 计算块的起始地址(对齐到块大小)
  size_t chunk_start = (offset / GBA_ROM_CACHE_CHUNK_SIZE) * GBA_ROM_CACHE_CHUNK_SIZE;
  size_t chunk_size = GBA_ROM_CACHE_CHUNK_SIZE;
  
  // 确保不超出ROM大小
  if (chunk_start + chunk_size > scratch->realtime_rom_size) {
    chunk_size = scratch->realtime_rom_size - chunk_start;
  }
  
  // 检查这个块是否已经部分或全部缓存
  bool need_read = false;
  for (size_t i = 0; i < chunk_size; i++) {
    if (scratch->rom_cache_valid[chunk_start + i] != 1) {
      need_read = true;
      break;
    }
  }

  if (scratch->rom_cache_valid[offset] == 0xFF) {
    chunk_size = 2; // 如果这个字节标记为不缓存，缩小块大小以减少无用读取
    chunk_start = offset - (offset % 2); // 对齐到2字节
    need_read = true;
  }
  
  if (need_read) {
    // 读取整个块
    bool success = false;
    uint8_t* temp_buffer = (uint8_t*)malloc(chunk_size);
    if (!temp_buffer) {
      return 0xFF;
    }
    
    switch (scratch->rom_protocol) {
      case GBA_ROM_PROTOCOL_FILE:
        if (scratch->rom_source_file) {
          success = gba_load_realtime_rom_bytes_from_file(scratch->rom_source_file, chunk_start, temp_buffer, chunk_size);
        }
        break;
      
      case GBA_ROM_PROTOCOL_SERIAL:
        if (scratch->rom_source_serial) {
          serial_port_t port = *(serial_port_t*)scratch->rom_source_serial;
          uint32_t addr_word = chunk_start / 2;
          success = gba_serial_read_rom(port, addr_word, temp_buffer, chunk_size);
          // if (success) {
          //   printf("Loaded ROM chunk: offset=0x%zx, size=%zu bytes\n", chunk_start, chunk_size);
          // }
        }
        break;
      
      case GBA_ROM_PROTOCOL_NET:
        // TODO: 实现网络块读取
        break;
      
      default:
        break;
    }
    
    // 更新缓存
    if (success) {
      memcpy(scratch->rom_cache_data + chunk_start, temp_buffer, chunk_size);
      for (size_t i = 0; i < chunk_size; i++) {
        if (scratch->rom_cache_valid[chunk_start + i] == 0) {
          scratch->rom_cache_valid[chunk_start + i] = 1;
        }
      }
    }
    
    free(temp_buffer);
  }
  // if (scratch->rom_cache_valid[offset] == 0xFF) {
  //   log_printf("Byte at offset 0x%zx is marked as non-cacheable, value=0x%02x\n", 
  //          offset, scratch->rom_cache_data[offset]);
  // }
  return scratch->rom_cache_data[offset];
}

static bool gba_save_rom_cache(gba_scratch_t *scratch, const uint8_t* data, size_t size) {
  if (!scratch->use_realtime_rom || !data || size == 0 || !scratch->rom_cache_valid) {
    return false;
  }
  
  FILE* file = fopen(scratch->rom_cache_path, "wb");
  if (!file) {
    log_printf("Failed to open cache file for writing: %s\n", scratch->rom_cache_path);
    return false;
  }
  
  // 写入文件头：魔数 + 版本 + ROM大小
  uint32_t magic = 0x43484147; // "GACH" (GBA Cache)
  uint32_t version = 1;
  uint32_t rom_size = (uint32_t)size;
  
  fwrite(&magic, sizeof(magic), 1, file);
  fwrite(&version, sizeof(version), 1, file);
  fwrite(&rom_size, sizeof(rom_size), 1, file);
  
  // 写入ROM数据
  size_t data_written = fwrite(data, 1, size, file);
  if (data_written != size) {
    log_printf("Failed to write ROM data to cache\n");
    fclose(file);
    return false;
  }
  
  // 写入valid标志位
  size_t valid_written = fwrite(scratch->rom_cache_valid, 1, size, file);
  if (valid_written != size) {
    log_printf("Failed to write valid flags to cache\n");
    fclose(file);
    return false;
  }
  
  fclose(file);
  
  // 统计已缓存的字节数
  size_t cached_bytes = 0;
  for (size_t i = 0; i < size; i++) {
    if (scratch->rom_cache_valid[i]) cached_bytes++;
  }
  
  log_printf("Successfully saved ROM cache to: %s (%zu/%zu bytes cached)\n", 
         scratch->rom_cache_path, cached_bytes, size);
  return true;
}

static bool gba_load_rom_cache(gba_scratch_t *scratch, uint8_t* buffer, size_t size) {
  if (!scratch->use_realtime_rom || !scratch->rom_cache_valid) {
    return false;
  }
  
  FILE* file = fopen(scratch->rom_cache_path, "rb");
  if (!file) {
    log_printf("Cache file not found: %s\n", scratch->rom_cache_path);
    return false;
  }
  
  // 读取并验证文件头
  uint32_t magic = 0, version = 0, rom_size = 0;
  
  if (fread(&magic, sizeof(magic), 1, file) != 1 || magic != 0x43484147) {
    log_printf("Invalid cache file: bad magic number\n");
    fclose(file);
    return false;
  }
  
  if (fread(&version, sizeof(version), 1, file) != 1 || version != 1) {
    log_printf("Invalid cache file: unsupported version %u\n", version);
    fclose(file);
    return false;
  }
  
  if (fread(&rom_size, sizeof(rom_size), 1, file) != 1 || rom_size != size) {
    log_printf("Cache file size mismatch: expected %zu, got %u\n", size, rom_size);
    fclose(file);
    return false;
  }
  
  // 读取ROM数据
  size_t data_read = fread(buffer, 1, size, file);
  if (data_read != size) {
    log_printf("Failed to read ROM data from cache: read %zu/%zu bytes\n", data_read, size);
    fclose(file);
    return false;
  }
  
  // 读取valid标志位
  size_t valid_read = fread(scratch->rom_cache_valid, 1, size, file);
  if (valid_read != size) {
    log_printf("Failed to read valid flags from cache: read %zu/%zu bytes\n", valid_read, size);
    fclose(file);
    return false;
  }
  
  fclose(file);
  
  // 统计已缓存的字节数
  size_t cached_bytes = 0;
  for (size_t i = 0; i < size; i++) {
    if (scratch->rom_cache_valid[i]) cached_bytes++;
  }
  
  log_printf("Successfully loaded ROM cache from: %s (%zu/%zu bytes cached)\n", 
         scratch->rom_cache_path, cached_bytes, size);
  return true;
}

void gba_unload(gba_t*gba,gba_scratch_t *scratch){
  log_printf("Unloading GBA\n");
  if(scratch->log_cmp_file)fclose(scratch->log_cmp_file);
  scratch->log_cmp_file=NULL;
  
  // 保存实时ROM的cache
  if (scratch->use_realtime_rom && scratch->rom_cache_data && scratch->realtime_rom_size > 0) {
    gba_save_rom_cache(scratch, scratch->rom_cache_data, scratch->realtime_rom_size);
    
    // 关闭文件句柄
    if (scratch->rom_source_file) {
      fclose(scratch->rom_source_file);
      scratch->rom_source_file = NULL;
    }
    
    // 关闭串口句柄
    if (scratch->rom_source_serial) {
      serial_port_t port = *(serial_port_t*)scratch->rom_source_serial;
      gba_close_serial_port(port);
      free(scratch->rom_source_serial);
      scratch->rom_source_serial = NULL;
    }
    
    // 释放缓存内存
    // free(scratch->rom_cache_data); // 由模拟器释放
    scratch->rom_cache_data = NULL;
    
    free(scratch->rom_cache_valid);
    scratch->rom_cache_valid = NULL;
    
    // 释放RAM预读缓存
    if (scratch->ram_prefetch_cache) {
      free(scratch->ram_prefetch_cache);
      scratch->ram_prefetch_cache = NULL;
    }
  }
}

// 串口模式下切换Flash bank
static void gba_serial_flash_switch_bank(serial_port_t port, int bank) {
  bank = (bank == 0) ? 0 : 1;
  
  log_printf("[Serial] Switching Flash bank to %d\n", bank);
  
  // 发送bank切换命令序列
  uint8_t cmd_aa = 0xAA;
  uint8_t cmd_55 = 0x55;
  uint8_t cmd_b0 = 0xB0;
  uint8_t bank_byte = (uint8_t)bank;
  
  gba_serial_write_ram(port, 0x5555, &cmd_aa, 1);
  gba_serial_write_ram(port, 0x2AAA, &cmd_55, 1);
  gba_serial_write_ram(port, 0x5555, &cmd_b0, 1);
  gba_serial_write_ram(port, 0x0000, &bank_byte, 1);
}

// 串口模式下切换SRAM_128K bank (写入0x09000000切换)
static void gba_serial_sram_switch_bank(serial_port_t port, int bank) {
  bank = (bank == 0) ? 0 : 1;
  
  log_printf("[Serial] Switching SRAM bank to %d\n", bank);
  uint16_t bank_byte = bank;
  gba_serial_write_rom(port, 0x01000000>>1, (uint8_t*)&bank_byte, 2);
}

// 串口模式下擦除Flash芯片
static bool gba_serial_flash_erase_chip(serial_port_t port) {
  log_printf("[Serial] Erasing Flash chip...\n");
  
  // 发送Chip-Erase命令序列
  uint8_t cmd_aa = 0xAA;
  uint8_t cmd_55 = 0x55;
  uint8_t cmd_80 = 0x80;
  uint8_t cmd_10 = 0x10;
  
  gba_serial_write_ram(port, 0x5555, &cmd_aa, 1);
  gba_serial_write_ram(port, 0x2AAA, &cmd_55, 1);
  gba_serial_write_ram(port, 0x5555, &cmd_80, 1);
  gba_serial_write_ram(port, 0x5555, &cmd_aa, 1);
  gba_serial_write_ram(port, 0x2AAA, &cmd_55, 1);
  gba_serial_write_ram(port, 0x5555, &cmd_10, 1); // Chip-Erase
  
  // 等待擦除完成(轮询0x0000地址，直到返回0xFF)
  log_printf("[Serial] Waiting for erase to complete...\n");
  uint8_t status;
  int max_retries = 60; // 最多等待60秒
  int retry = 0;
  
  while (retry < max_retries) {
    // 等待1秒
#ifdef _WIN32
    Sleep(1000);
#else
    usleep(1000000);
#endif
    
    // 读取状态
    if (!gba_serial_read_ram(port, 0x0000, &status, 1)) {
      log_printf("[Serial] ERROR: Failed to read erase status\n");
      return false;
    }
    
    log_printf("[Serial] Erase status: 0x%02x\n", status);
    
    if (status == 0xFF) {
      log_printf("[Serial] Flash erase completed\n");
      return true;
    }
    
    retry++;
  }
  
  log_printf("[Serial] ERROR: Flash erase timeout after %d seconds\n", max_retries);
  return false;
}

// 串口模式下加载Flash存档
static bool gba_serial_load_flash_backup(gba_t* gba, uint32_t flash_size) {
  if (!gba || !gba->scratch || !gba->scratch->rom_source_serial) {
    return false;
  }
  
  serial_port_t port = *(serial_port_t*)gba->scratch->rom_source_serial;
  
  log_printf("[Serial] Loading Flash backup from cartridge (%u bytes)...\n", flash_size);
  
  // 如果是128KB Flash，需要分两个bank读取
  if (flash_size == 128 * 1024) {
    // 读取Bank 0 (前64KB)
    log_printf("[Serial] Reading Flash bank 0...\n");
    gba_serial_flash_switch_bank(port, 0);
    
    const uint32_t chunk_size = 4096;
    for (uint32_t offset = 0; offset < 64 * 1024; offset += chunk_size) {
      if (!gba_serial_read_ram(port, offset, gba->mem.cart_backup + offset, chunk_size)) {
        log_printf("[Serial] ERROR: Failed to read Flash bank 0 at offset 0x%05x\n", offset);
        return false;
      }
    }
    
    // 读取Bank 1 (后64KB)
    log_printf("[Serial] Reading Flash bank 1...\n");
    gba_serial_flash_switch_bank(port, 1);
    
    for (uint32_t offset = 0; offset < 64 * 1024; offset += chunk_size) {
      if (!gba_serial_read_ram(port, offset, gba->mem.cart_backup + 64 * 1024 + offset, chunk_size)) {
        log_printf("[Serial] ERROR: Failed to read Flash bank 1 at offset 0x%05x\n", offset);
        return false;
      }
    }
    
    // 切换回bank 0
    gba_serial_flash_switch_bank(port, 0);
    
  } else {
    // 64KB Flash，直接读取
    const uint32_t chunk_size = 4096;
    for (uint32_t offset = 0; offset < flash_size; offset += chunk_size) {
      if (!gba_serial_read_ram(port, offset, gba->mem.cart_backup + offset, chunk_size)) {
        log_printf("[Serial] ERROR: Failed to read Flash at offset 0x%05x\n", offset);
        return false;
      }
    }
  }
  
  log_printf("[Serial] Flash backup loaded successfully\n");
  return true;
}

// 串口模式下加载SRAM_128K存档
static bool gba_serial_load_sram_backup(gba_t* gba, int size) {
  if (!gba || !gba->scratch || !gba->scratch->rom_source_serial) {
    return false;
  }
  
  serial_port_t port = *(serial_port_t*)gba->scratch->rom_source_serial;
  
  log_printf("[Serial] Loading SRAM_128K backup from cartridge (128KB)...\n");
  
  const uint32_t chunk_size = 4096;
  
  // 读取Bank 0 (前64KB)
  log_printf("[Serial] Reading SRAM bank 0...\n");
  gba_serial_sram_switch_bank(port, 0);
  
  for (uint32_t offset = 0; offset < 64 * 1024; offset += chunk_size) {
    if (!gba_serial_read_ram(port, offset, gba->mem.cart_backup + offset, chunk_size)) {
      log_printf("[Serial] ERROR: Failed to read SRAM bank 0 at offset 0x%05x\n", offset);
      return false;
    }
  }
  if (size <= 64 * 1024) {
    log_printf("[Serial] SRAM_128K backup loaded successfully (only bank 0 used)\n");
    return true;
  }
  // 读取Bank 1 (后64KB)
  log_printf("[Serial] Reading SRAM bank 1...\n");
  gba_serial_sram_switch_bank(port, 1);
  
  for (uint32_t offset = 0; offset < 64 * 1024; offset += chunk_size) {
    if (!gba_serial_read_ram(port, offset, gba->mem.cart_backup + 64 * 1024 + offset, chunk_size)) {
      log_printf("[Serial] ERROR: Failed to read SRAM bank 1 at offset 0x%05x\n", offset);
      return false;
    }
  }
  
  // 切换回bank 0
  gba_serial_sram_switch_bank(port, 0);
  
  log_printf("[Serial] SRAM_128K backup loaded successfully\n");
  return true;
}

// 串口模式下同步Flash到卡带
// 当backup_is_dirty变为false时调用，表示发生了存档
static void gba_serial_sync_flash_backup(gba_t* gba, int size) {
  if (!gba || !gba->scratch) return;
  
  // 检查是否是串口模式
  if (!gba->scratch->use_realtime_rom || 
      gba->scratch->rom_protocol != GBA_ROM_PROTOCOL_SERIAL) {
    return;
  }
  
  // 检查是否是Flash类型
  if (gba->cart.backup_type != GBA_BACKUP_FLASH_64K && 
      gba->cart.backup_type != GBA_BACKUP_FLASH_128K) {
    return;
  }
  
  serial_port_t port = *(serial_port_t*)gba->scratch->rom_source_serial;
  
  log_printf("[Serial] Syncing Flash backup to 卡带 (%u bytes)...\n", size);
  
  const uint32_t chunk_size = 4096;
  bool success = true;
  
  // 如果是128KB Flash，需要分两个bank写入
  if (size == 128 * 1024) {
    // 切换到Bank 0并擦除
    log_printf("[Serial] Programming Flash bank 0...\n");
    gba_serial_flash_switch_bank(port, 0);
    
    // 擦除整个芯片
    if (!gba_serial_flash_erase_chip(port)) {
      log_printf("[Serial] ERROR: Failed to erase Flash chip\n");
      return;
    }
    
    for (uint32_t offset = 0; offset < 64 * 1024 && success; offset += chunk_size) {
      log_printf("[Serial] Programming Flash: bank=0, offset=0x%05x, size=%u bytes\n", offset, chunk_size);
      success = gba_serial_flash_program(port, offset, gba->mem.cart_backup + offset, chunk_size);
      if (!success) {
        log_printf("[Serial] ERROR: Flash program failed at bank 0, offset 0x%05x\n", offset);
        break;
      }
    }
    
    if (success) {
      // 写入Bank 1 (后64KB)
      log_printf("[Serial] Programming Flash bank 1...\n");
      gba_serial_flash_switch_bank(port, 1);
      
      for (uint32_t offset = 0; offset < 64 * 1024 && success; offset += chunk_size) {
        log_printf("[Serial] Programming Flash: bank=1, offset=0x%05x, size=%u bytes\n", offset, chunk_size);
        success = gba_serial_flash_program(port, offset, gba->mem.cart_backup + 64 * 1024 + offset, chunk_size);
        if (!success) {
          log_printf("[Serial] ERROR: Flash program failed at bank 1, offset 0x%05x\n", offset);
          break;
        }
      }
      
      // 切换回bank 0
      gba_serial_flash_switch_bank(port, 0);
    }
  } else {
    // 64KB Flash，先擦除再写入
    if (!gba_serial_flash_erase_chip(port)) {
      log_printf("[Serial] ERROR: Failed to erase Flash chip\n");
      return;
    }
    
    for (uint32_t offset = 0; offset < size && success; offset += chunk_size) {
      log_printf("[Serial] Programming Flash: offset=0x%05x, size=%u bytes\n", offset, chunk_size);
      success = gba_serial_flash_program(port, offset, gba->mem.cart_backup + offset, chunk_size);
      if (!success) {
        log_printf("[Serial] ERROR: Flash program failed at offset 0x%05x\n", offset);
        break;
      }
    }
  }
  
  if (success) {
    log_printf("[Serial] Flash sync completed successfully\n");
  } else {
    log_printf("[Serial] Flash sync failed\n");
  }
}

// 串口模式下同步SRAM_128K到卡带
static void gba_serial_sync_sram_backup(gba_t* gba, int size) {
  if (!gba || !gba->scratch) return;
  
  // 检查是否是串口模式
  if (!gba->scratch->use_realtime_rom || 
      gba->scratch->rom_protocol != GBA_ROM_PROTOCOL_SERIAL) {
    return;
  }
  
  // 检查是否是SRAM_128K类型
  if (gba->cart.backup_type != GBA_BACKUP_SRAM_128K) {
    return;
  }
  
  serial_port_t port = *(serial_port_t*)gba->scratch->rom_source_serial;
  
  log_printf("[Serial] Syncing SRAM_128K backup to cartridge (128KB)...\n");
  
  const uint32_t chunk_size = 4096;
  bool success = true;
  
  // 写入Bank 0 (前64KB)
  log_printf("[Serial] Writing SRAM bank 0...\n");
  gba_serial_sram_switch_bank(port, 0);
  
  for (uint32_t offset = 0; offset < 64 * 1024 && success; offset += chunk_size) {
    log_printf("[Serial] Writing SRAM: bank=0, offset=0x%05x, size=%u bytes\n", offset, chunk_size);
    success = gba_serial_write_ram(port, offset, gba->mem.cart_backup + offset, chunk_size);
    if (!success) {
      log_printf("[Serial] ERROR: SRAM write failed at bank 0, offset 0x%05x\n", offset);
      break;
    }
  }
  
  if (success && size > 64 * 1024) {
    // 写入Bank 1 (后64KB)
    log_printf("[Serial] Writing SRAM bank 1...\n");
    gba_serial_sram_switch_bank(port, 1);
    
    for (uint32_t offset = 0; offset < 64 * 1024 && success; offset += chunk_size) {
      log_printf("[Serial] Writing SRAM: bank=1, offset=0x%05x, size=%u bytes\n", offset, chunk_size);
      success = gba_serial_write_ram(port, offset, gba->mem.cart_backup + 64 * 1024 + offset, chunk_size);
      if (!success) {
        log_printf("[Serial] ERROR: SRAM write failed at bank 1, offset 0x%05x\n", offset);
        break;
      }
    }
    
    // 切换回bank 0
    gba_serial_sram_switch_bank(port, 0);
  }
  
  if (success) {
    log_printf("[Serial] SRAM_128K sync completed successfully\n");
  } else {
    log_printf("[Serial] SRAM_128K sync failed\n");
  }
}

bool gba_load_rom(sb_emu_state_t*emu,gba_t* gba, gba_scratch_t *scratch){
  memset(gba,0,sizeof(gba_t));
  memset(scratch,0,sizeof(gba_scratch_t));
  if(!sb_path_has_file_ext(emu->rom_path, ".gba"))return false; 

  if(emu->rom_size>32*1024*1024){
    log_printf("ROMs with sizes >32MB (%zu bytes) are too big for the GBA\n",emu->rom_size); 
    return false;
  }  

  // 检查ROM数据开头是否为 "READREALTIME" (兼容 \r\n 和 \n)
  bool is_realtime_rom = false;
  bool has_crlf = false;
  
  // 先检查是否有 READREALTIME 标记(兼容两种换行符)
  if (emu->rom_size >= 13) {
    if (strncmp((char*)emu->rom_data, "READREALTIME\n", 13) == 0) {
      is_realtime_rom = true;
      has_crlf = false;
      log_printf("Detected READREALTIME ROM configuration (LF)\n");
    } else if (emu->rom_size >= 14 && strncmp((char*)emu->rom_data, "READREALTIME\r\n", 14) == 0) {
      is_realtime_rom = true;
      has_crlf = true;
      log_printf("Detected READREALTIME ROM configuration (CRLF)\n");
    }
  }
  
  if (is_realtime_rom) {
    // 如果是 Windows 风格的 \r\n，先统一转换为 \n
    if (has_crlf) {
      log_printf("Converting CRLF to LF...\n");
      size_t read_pos = 0, write_pos = 0;
      while (read_pos < emu->rom_size) {
        if (read_pos + 1 < emu->rom_size && 
            emu->rom_data[read_pos] == '\r' && 
            emu->rom_data[read_pos + 1] == '\n') {
          // 跳过 \r，只保留 \n
          emu->rom_data[write_pos++] = '\n';
          read_pos += 2;
        } else {
          emu->rom_data[write_pos++] = emu->rom_data[read_pos++];
        }
      }
      emu->rom_size = write_pos;
      log_printf("Converted: new size = %zu bytes\n", emu->rom_size);
    }
    
    // 解析READREALTIME配置
    char* config_data = (char*)emu->rom_data;
    char* line_ptr = config_data + 13; // 跳过 "READREALTIME\n"
    
    // 解析第二行：ROM大小
    size_t rom_size = 0;
    if (sscanf(line_ptr, "%zu", &rom_size) != 1) {
      log_printf("Failed to parse ROM size from READREALTIME configuration\n");
      return false;
    }
    line_ptr = strchr(line_ptr, '\n');
    if (!line_ptr) {
      log_printf("Invalid READREALTIME configuration format\n");
      return false;
    }
    line_ptr++; // 跳过换行符
    
    // 解析第三行：协议类型
    char protocol_str[32];
    if (sscanf(line_ptr, "%31s", protocol_str) != 1) {
      log_printf("Failed to parse protocol from READREALTIME configuration\n");
      return false;
    }
    line_ptr = strchr(line_ptr, '\n');
    if (!line_ptr) {
      log_printf("Invalid READREALTIME configuration format\n");
      return false;
    }
    line_ptr++; // 跳过换行符
    
    // 解析第四行：地址
    char address[SB_FILE_PATH_SIZE];
    int addr_len = 0;
    char* addr_start = line_ptr;
    char* newline = strchr(line_ptr, '\n');
    if (newline) {
      addr_len = newline - line_ptr;
    } else {
      addr_len = strlen(line_ptr);
    }
    if (addr_len >= SB_FILE_PATH_SIZE) {
      addr_len = SB_FILE_PATH_SIZE - 1;
    }
    strncpy(address, addr_start, addr_len);
    address[addr_len] = '\0';
    
    // 移动到第五行
    if (newline) {
      line_ptr = newline + 1;
    } else {
      line_ptr = NULL;
    }
    
    // 解析第五行(可选)：存档类型
    int backup_type = -1; // -1表示使用自动检测
    if (line_ptr && *line_ptr) {
      char backup_str[32];
      if (sscanf(line_ptr, "%31s", backup_str) == 1) {
        if (strcmp(backup_str, "NONE") == 0) {
          backup_type = GBA_BACKUP_NONE;
        } else if (strcmp(backup_str, "EEPROM") == 0) {
          backup_type = GBA_BACKUP_EEPROM;
        } else if (strcmp(backup_str, "EEPROM_512B") == 0) {
          backup_type = GBA_BACKUP_EEPROM_512B;
        } else if (strcmp(backup_str, "EEPROM_8KB") == 0) {
          backup_type = GBA_BACKUP_EEPROM_8KB;
        } else if (strcmp(backup_str, "SRAM") == 0) {
          backup_type = GBA_BACKUP_SRAM;
        } else if (strcmp(backup_str, "FLASH_64K") == 0) {
          backup_type = GBA_BACKUP_FLASH_64K;
        } else if (strcmp(backup_str, "FLASH_128K") == 0) {
          backup_type = GBA_BACKUP_FLASH_128K;
        } else if (strcmp(backup_str, "SRAM_128K") == 0) {
          backup_type = GBA_BACKUP_SRAM_128K;
        } else if (strcmp(backup_str, "AUTO") == 0) {
          backup_type = -1; // 自动检测
        } else {
          log_printf("Unknown backup type: %s, using AUTO\n", backup_str);
          backup_type = -1;
        }
      }
    }
    
    // 设置协议类型
    if (strcmp(protocol_str, "FILE") == 0) {
      scratch->rom_protocol = GBA_ROM_PROTOCOL_FILE;
    } else if (strcmp(protocol_str, "SERIAL") == 0) {
      scratch->rom_protocol = GBA_ROM_PROTOCOL_SERIAL;
    } else if (strcmp(protocol_str, "NET") == 0) {
      scratch->rom_protocol = GBA_ROM_PROTOCOL_NET;
    } else {
      log_printf("Unknown protocol type: %s\n", protocol_str);
      return false;
    }
    
    log_printf("READREALTIME ROM configuration:\n");
    log_printf("  Size: %zu bytes\n", rom_size);
    log_printf("  Protocol: %s\n", protocol_str);
    log_printf("  Address: %s\n", address);
    if (backup_type >= 0) {
      const char* backup_names[] = {"NONE", "EEPROM", "EEPROM_512B", "EEPROM_8KB", "SRAM", "FLASH_64K", "FLASH_128K"};
      log_printf("  Backup Type: %s\n", backup_names[backup_type]);
    } else {
      log_printf("  Backup Type: AUTO\n");
    }
    
    // 设置实时ROM参数
    scratch->use_realtime_rom = true;
    scratch->realtime_rom_size = rom_size;
    scratch->custom_backup_type = backup_type;
    scratch->serial_write_count = 0; // 初始化写入缓存计数
    
    // 初始化RAM预读缓存
    scratch->ram_prefetch_cache = NULL;
    scratch->ram_prefetch_start = 0;
    scratch->ram_prefetch_size = 0;
    scratch->ram_prefetch_valid = false;
    
    strncpy(scratch->rom_source_address, address, SB_FILE_PATH_SIZE - 1);
    scratch->rom_source_address[SB_FILE_PATH_SIZE - 1] = '\0';
    
    // 设置cache路径(保存到save_data_base_path目录下)
    snprintf(scratch->rom_cache_path, SB_FILE_PATH_SIZE, "%s_rom_cache.gba", emu->save_data_base_path);
    
    // 分配ROM数据缓冲区
    scratch->rom_cache_data = (uint8_t*)malloc(rom_size);
    if (!scratch->rom_cache_data) {
      log_printf("Failed to allocate memory for ROM cache (%zu bytes)\n", rom_size);
      return false;
    }
    
    // 分配缓存有效位数组
    scratch->rom_cache_valid = (uint8_t*)calloc(rom_size, 1); // 初始化为0(未缓存)
    for (int i = 0; i < 0xFF; i++) {
      if (i >= 0xC4 && i <= 0xC8) continue;
      scratch->rom_cache_valid[i] = 0xFF;
    }
    if (!scratch->rom_cache_valid) {
      log_printf("Failed to allocate memory for ROM cache valid bits (%zu bytes)\n", rom_size);
      free(scratch->rom_cache_data);
      scratch->rom_cache_data = NULL;
      return false;
    }
    
    // 初始化ROM数据为0xFF
    memset(scratch->rom_cache_data, 0xFF, rom_size);
    
    // 打开文件或串口句柄
    if (scratch->rom_protocol == GBA_ROM_PROTOCOL_FILE) {
      scratch->rom_source_file = fopen(address, "rb");
      if (!scratch->rom_source_file) {
        log_printf("Failed to open ROM source file: %s\n", address);
        free(scratch->rom_cache_data);
        free(scratch->rom_cache_valid);
        scratch->rom_cache_data = NULL;
        scratch->rom_cache_valid = NULL;
        return false;
      }
      log_printf("Opened ROM source file: %s\n", address);
    } else if (scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL) {
      serial_port_t port = gba_open_serial_port(address);
      if (port == INVALID_SERIAL_PORT) {
        log_printf("Failed to open serial port: %s\n", address);
        free(scratch->rom_cache_data);
        free(scratch->rom_cache_valid);
        scratch->rom_cache_data = NULL;
        scratch->rom_cache_valid = NULL;
        return false;
      }
      // 分配并存储串口句柄
      scratch->rom_source_serial = malloc(sizeof(serial_port_t));
      if (!scratch->rom_source_serial) {
        gba_close_serial_port(port);
        free(scratch->rom_cache_data);
        free(scratch->rom_cache_valid);
        scratch->rom_cache_data = NULL;
        scratch->rom_cache_valid = NULL;
        return false;
      }
      *(serial_port_t*)scratch->rom_source_serial = port;
      log_printf("Opened serial port: %s\n", address);
    }
    
    // // 尝试从cache文件加载已缓存的数据和valid标志
    // bool loaded_from_cache = gba_load_rom_cache(scratch, scratch->rom_cache_data, rom_size);
    // if (loaded_from_cache) {
    //   // valid标志已经从cache文件加载，不需要额外设置
    //   log_printf("Loaded ROM cache with validity flags\n");
    // } else {
    //   // cache不可用，所有字节标记为未缓存
    //   log_printf("Cache not available, will load on-demand from source\n");
    // }
    
    // 更新emu状态，使用cache数据
    free(emu->rom_data); // 释放原有ROM数据
    emu->rom_data = scratch->rom_cache_data;
    emu->rom_size = rom_size;

    if (scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL && scratch->custom_backup_type == -1) {
      // 检测存档类型
      log_printf("Detecting backup type for serial ROM...\n");
      // 只检测SRAM_128K和Flash类型
      // SRAM类型的特点是写入后立即变化，Flash类型需要特殊命令写入
      // 1. 备份 0x0000
      uint8_t original_data[16];
      if (!gba_serial_read_ram(*(serial_port_t*)scratch->rom_source_serial, 0x0000, original_data, sizeof(original_data))) {
        log_printf("Failed to read original data for backup detection\n");
      } else {
        uint8_t test_data[16];
        // 2. 写入测试数据 0xAA55AA55...
        for (int i = 0; i < 16; i += 4) {
          test_data[i] = 0xAA;
          test_data[i + 1] = 0x55;
          test_data[i + 2] = 0xAA;
          test_data[i + 3] = 0x55;
          if (!gba_serial_write_ram(*(serial_port_t*)scratch->rom_source_serial, 0x0000 + i, test_data + i, 4)) {
            log_printf("Failed to write test data for backup detection\n");
          }
        }
        // 3. 读取回写数据
        uint8_t verify_data[16];
        if (!gba_serial_read_ram(*(serial_port_t*)scratch->rom_source_serial, 0x0000, verify_data, sizeof(verify_data))) {
          log_printf("Failed to read back data for backup detection\n");
        } else {
          // 4. 比较数据
          if (memcmp(test_data, verify_data, sizeof(test_data)) == 0) {
            // 数据匹配，说明是SRAM类型
            scratch->custom_backup_type = GBA_BACKUP_SRAM_128K;
            log_printf("Detected backup type: SRAM_128K\n");
            // 恢复原数据
            if (!gba_serial_write_ram(*(serial_port_t*)scratch->rom_source_serial, 0x0000, original_data, sizeof(original_data))) {
              log_printf("Failed to restore original data after backup detection\n");
            }
          } else {
            // 数据不匹配，可能是Flash类型
            // 通过读取Flash ID来确定Flash类型（64K还是128K）
            log_printf("Data mismatch, detecting Flash type by reading Chip ID...\n");
            
            serial_port_t port = *(serial_port_t*)scratch->rom_source_serial;
            
            // 备份 0x5555, 0x2AAA, 0x0000 的原始数据
            uint8_t temp5555, temp2AAA, temp0000;
            gba_serial_read_ram(port, 0x5555, &temp5555, 1);
            gba_serial_read_ram(port, 0x2AAA, &temp2AAA, 1);
            gba_serial_read_ram(port, 0x0000, &temp0000, 1);
            
            // 发送读取Flash ID的命令序列
            uint8_t cmd_aa = 0xAA;
            uint8_t cmd_55 = 0x55;
            uint8_t cmd_90 = 0x90;
            uint8_t cmd_f0 = 0xF0;
            
            gba_serial_write_ram(port, 0x5555, &cmd_aa, 1);  // 写入 0xAA 到 0x5555
            gba_serial_write_ram(port, 0x2AAA, &cmd_55, 1);  // 写入 0x55 到 0x2AAA
            gba_serial_write_ram(port, 0x5555, &cmd_90, 1);  // 发送读取ID命令 0x90
            
            // 读取Flash ID（16位）
            uint8_t id_byte0, id_byte1;
            gba_serial_read_ram(port, 0x0000, &id_byte0, 1);  // Manufacturer ID
            gba_serial_read_ram(port, 0x0001, &id_byte1, 1);  // Device ID
            uint16_t flash_id = (id_byte0 << 8) | id_byte1;
            
            // 退出ID模式
            gba_serial_write_ram(port, 0x5555, &cmd_aa, 1);
            gba_serial_write_ram(port, 0x2AAA, &cmd_55, 1);
            gba_serial_write_ram(port, 0x5555, &cmd_f0, 1);  // 退出命令 0xF0
            gba_serial_write_ram(port, 0x0000, &cmd_f0, 1);  // 也写到 0x0000 确保退出
            
            // 根据Flash ID判断类型
            // 64KB Flash IDs
            bool is_64k_flash = (flash_id == 0xBFD4 ||  // SST 39VF512
                                flash_id == 0x1F3D ||  // Atmel AT29LV512
                                flash_id == 0xC21C ||  // Macronix MX29L512
                                flash_id == 0x321B);   // Panasonic MN63F805MNP
            
            // 128KB Flash IDs (带bank切换)
            bool is_128k_flash = (flash_id == 0xC209 ||  // Macronix MX29L010
                                 flash_id == 0x6213 ||  // SANYO LE26FV10N1TS
                                 flash_id == 0xBF5B);   // Unlicensed SST49LF080A
            
            if (is_64k_flash) {
              scratch->custom_backup_type = GBA_BACKUP_FLASH_64K;
              log_printf("Detected Flash ID: 0x%04X -> FLASH_64K\n", flash_id);
            } else if (is_128k_flash) {
              scratch->custom_backup_type = GBA_BACKUP_FLASH_128K;
              log_printf("Detected Flash ID: 0x%04X -> FLASH_128K (with bank switching)\n", flash_id);
            } else {
              // 未知ID，可能是EEPROM或无存档
              scratch->custom_backup_type = -2; // 标记为未知
            }
            
            // 恢复原始数据
            gba_serial_write_ram(port, 0x5555, &temp5555, 1);
            gba_serial_write_ram(port, 0x2AAA, &temp2AAA, 1);
            gba_serial_write_ram(port, 0x0000, &temp0000, 1);
          }
        }
      }
    }
  }

  // 设置scratch指针用于实时ROM读取
  gba->scratch = scratch;
  
  gba->mem.bios=scratch->bios;
  bool loaded_bios= se_load_bios_file("GBA BIOS", emu->save_file_path, "gba_bios.bin", scratch->bios,16*1024);
  if(!loaded_bios){
    memcpy(scratch->bios,gba_bios_bin,sizeof(gba_bios_bin));
    scratch->skip_bios_intro=true;
  }
  gba->cart.rom_size = emu->rom_size; 
  gba->mem.cart_rom = emu->rom_data;

  // 设置backup类型
  if (scratch->use_realtime_rom && scratch->custom_backup_type >= 0) {
    // 使用自定义配置的backup类型
    gba->cart.backup_type = scratch->custom_backup_type;
    const char* backup_names[] = {"NONE", "EEPROM", "EEPROM_512B", "EEPROM_8KB", "SRAM", "FLASH_64K", "FLASH_128K", "SRAM_128K", "DIRECT"};
    log_printf("Using custom backup type: %s\n", backup_names[gba->cart.backup_type]);
  } else if (scratch->use_realtime_rom && scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL && scratch->custom_backup_type == -2) {
    gba->cart.backup_type = GBA_BACKUP_DIRECT; // 先设为DIRECT，实际访问时通过gba_read_backup_byte/gba_write_backup_byte处理
    log_printf("Serial ROM: Backup type set to DIRECT (will use serial RAM access)\n");
  } else {
    // 自动检测backup类型
    gba->cart.backup_type = gba_search_rom_for_backup_string(gba);
  }

  // 加载存档数据
  bool backup_loaded = false;
  
  // 如果是串口模式且是Flash或SRAM_128K类型，从串口读取
  if (scratch->use_realtime_rom && 
      scratch->rom_protocol == GBA_ROM_PROTOCOL_SERIAL &&
      (gba->cart.backup_type == GBA_BACKUP_FLASH_64K || 
       gba->cart.backup_type == GBA_BACKUP_FLASH_128K ||
       gba->cart.backup_type == GBA_BACKUP_SRAM_128K ||
       gba->cart.backup_type == GBA_BACKUP_SRAM)) {
    
    if (gba->cart.backup_type == GBA_BACKUP_SRAM) {
      // 对于SRAM类型，假设是32KB SRAM
      backup_loaded = gba_serial_load_sram_backup(gba, 32 * 1024);
      if (!backup_loaded) {
        log_printf("[Serial] WARNING: Failed to load SRAM from serial, initializing to 0xFF\n");
        for(int i=0;i<sizeof(gba->mem.cart_backup);++i) gba->mem.cart_backup[i]=0xff;
      }
    }else if (gba->cart.backup_type == GBA_BACKUP_SRAM_128K) {
      backup_loaded = gba_serial_load_sram_backup(gba, 128 * 1024);
      if (!backup_loaded) {
        log_printf("[Serial] WARNING: Failed to load SRAM_128K from serial, initializing to 0xFF\n");
        for(int i=0;i<sizeof(gba->mem.cart_backup);++i) gba->mem.cart_backup[i]=0xff;
      }
    } else {
      uint32_t flash_size = (gba->cart.backup_type == GBA_BACKUP_FLASH_64K) ? 64 * 1024 : 128 * 1024;
      backup_loaded = gba_serial_load_flash_backup(gba, flash_size);
      if (!backup_loaded) {
        log_printf("[Serial] WARNING: Failed to load Flash from serial, initializing to 0xFF\n");
        for(int i=0;i<sizeof(gba->mem.cart_backup);++i) gba->mem.cart_backup[i]=0xff;
      }
    }
  } else {
    // 从文件加载
    size_t bytes=0;
    uint8_t*data = sb_load_file_data(emu->save_file_path,&bytes);
    if(data){
      log_printf("Loaded save file: %s, bytes: %zu\n",emu->save_file_path,bytes);
      if(bytes>=128*1024)bytes=128*1024;
      memcpy(gba->mem.cart_backup, data, bytes);
      sb_free_file_data(data);
      backup_loaded = true;
    }else{
      log_printf("Could not find save file: %s\n",emu->save_file_path);
      for(int i=0;i<sizeof(gba->mem.cart_backup);++i) gba->mem.cart_backup[i]=0xff;
    }
  }

  // Setup flash chip id (this is not used if the cartridge does not have flash backup storage)
  if(gba->cart.backup_type==GBA_BACKUP_FLASH_64K){
    gba->mem.flash_chip_id[1]=0xd4;
    gba->mem.flash_chip_id[0]=0xbf;
  }else{
    gba->mem.flash_chip_id[1]=0x13;
    gba->mem.flash_chip_id[0]=0x62;
  }

  gba->cpu = arm7_init(gba);
  
  for(int bg = 2;bg<4;++bg){
    gba_io_store16(gba,GBA_BG2PA+(bg-2)*0x10,1<<8);
    gba_io_store16(gba,GBA_BG2PB+(bg-2)*0x10,0<<8);
    gba_io_store16(gba,GBA_BG2PC+(bg-2)*0x10,0<<8);
    gba_io_store16(gba,GBA_BG2PD+(bg-2)*0x10,1<<8);
  }
  gba_store16(gba,0x04000088,512);
  gba_store32(gba,0x040000DC,0x84000000);
  gba_recompute_waitstate_table(gba,0);
  gba_recompute_mmio_mask_table(gba);

  if(scratch->skip_bios_intro){
    log_printf("No GBA bios using bundled bios\n");
    memcpy(gba->mem.bios,gba_bios_bin,sizeof(gba_bios_bin));
    const uint32_t initial_regs[37]={
      0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,0x3007f00,0x0000000,0x8000000,
      0xdf,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
      0x3007fa0,0x0,0x3007fe0,0x0,0x0,0x0,0x0,0x0,
      0x0,0x0,0x0,0x0,0x0,
    };
    for(int i=0;i<37;++i)gba->cpu.registers[i]=initial_regs[i];
    const uint32_t initial_mmio_writes[]={
      0x4000000,0x80,
      0x4000004,0x7e0000,
      0x4000020,0x100,
      0x4000024,0x1000000,
      0x4000030,0x100,
      0x4000034,0x1000000,
      0x4000080,0xe0000,
      0x4000084,0xf,
      0x4000088,0x200,
      0x4000100,0xff8a,
      0x4000130,0x3ff,
      0x4000134,0x8000,
      0x4000300,0x1,
    };
    for(int i=0;i<sizeof(initial_mmio_writes)/sizeof(uint32_t);i+=2){
      uint32_t addr=initial_mmio_writes[i+0];
      uint32_t data=initial_mmio_writes[i+1];
      arm7_write32(gba, addr,data);
    }
    gba_store32(gba,GBA_IE,0x1);
    gba_store16(gba,GBA_DISPCNT,0x9140);
  }else{
    gba->cpu.registers[PC]  = 0x0000000; 
    gba->cpu.registers[CPSR]= 0x000000d3; 
  }
  if(gba->cpu.log_cmp_file){fclose(gba->cpu.log_cmp_file);gba->cpu.log_cmp_file=NULL;};
  gba->cpu.log_cmp_file =se_load_log_file(scratch->save_file_path, "log.bin");
  gba->cpu.executed_instructions+=2;
  gba->audio.current_sample_generated_time=gba->audio.current_sim_time=0;
  gba->rtc.initial_rtc_time=time(NULL);
  return true; 
}  
    
#define GBA_LCD_HBLANK_END   (295)
#define GBA_LCD_HBLANK_START (GBA_LCD_W)
#define GBA_LCD_VBLANK_START (GBA_LCD_H*1232)
#define GBA_LCD_VBLANK_END   (227*1232)

//Returns true if the fast forward failed to be more efficient in main emu loop
static FORCE_INLINE int gba_ppu_compute_max_fast_forward(gba_t* gba, bool render){
  int scanline_clock = (gba->ppu.scan_clock)%1232;
  //If inside hblank, can fastforward to outside of hblank
  if(scanline_clock>=GBA_LCD_HBLANK_START*4&&scanline_clock<=GBA_LCD_HBLANK_END*4) return GBA_LCD_HBLANK_END*4-scanline_clock-1;
  //If inside hrender, can fastforward to hblank if not the first pixel and not visible
  bool not_visible = !render||gba->ppu.scan_clock>GBA_LCD_VBLANK_START; 
  if(not_visible&& (scanline_clock>=1 && scanline_clock<=GBA_LCD_HBLANK_START*4))return GBA_LCD_HBLANK_START*4-scanline_clock-1; 
  return 3-((gba->ppu.scan_clock)%4);
}
static FORCE_INLINE void gba_tick_ppu(gba_t* gba, bool render){
  if(SB_LIKELY(gba->ppu.fast_forward_ticks>0)){
    gba->ppu.fast_forward_ticks--;
    return; 
  }

  if(gba->ppu.scan_clock>=280896)gba->ppu.scan_clock-=280896;
  int lcd_y = (gba->ppu.scan_clock)/1232;
  int lcd_x = ((gba->ppu.scan_clock)%1232)/4;
  gba->ppu.scan_clock++;
  gba->ppu.fast_forward_ticks = gba_ppu_compute_max_fast_forward(gba, render)+1;
  gba->ppu.scan_clock+=gba->ppu.fast_forward_ticks;
  if(lcd_x==0||lcd_x==GBA_LCD_HBLANK_START||lcd_x==GBA_LCD_HBLANK_END){
    uint16_t disp_stat = gba_io_read16(gba, GBA_DISPSTAT)&~0x7;
    uint16_t vcount_cmp = SB_BFE(disp_stat,8,8);
    int vcount = (lcd_y+ (lcd_x>=GBA_LCD_HBLANK_END))%228;
    bool vblank = lcd_y>=160&&lcd_y<227;
    bool hblank = lcd_x>=GBA_LCD_HBLANK_START&&lcd_x< GBA_LCD_HBLANK_END;
    disp_stat |= vblank ? 0x1: 0; 
    disp_stat |= hblank ? 0x2: 0;      
    disp_stat |= vcount==vcount_cmp ? 0x4: 0;   
    gba_io_store16(gba,GBA_DISPSTAT,disp_stat);
    gba_io_store16(gba,GBA_VCOUNT,vcount);   
    uint32_t new_if = 0;
    if(hblank!=gba->ppu.last_hblank){
      gba->ppu.last_hblank = hblank;
      bool hblank_irq_en = SB_BFE(disp_stat,4,1);
      if(hblank&&hblank_irq_en) new_if|= (1<< GBA_INT_LCD_HBLANK); 
      gba->activate_dmas|=gba->dma_wait_ppu;
      if(!hblank){
        gba->ppu.dispcnt_pipeline[0]=gba->ppu.dispcnt_pipeline[1];
        gba->ppu.dispcnt_pipeline[1]=gba->ppu.dispcnt_pipeline[2];
        gba->ppu.dispcnt_pipeline[2]=gba_io_read16(gba, GBA_DISPCNT);
      }
    }
    if(lcd_y != gba->ppu.last_lcd_y){
      if(vblank!=gba->ppu.last_vblank){
        if(vblank)gba->frame_in_progress=false;
        gba->ppu.last_vblank = vblank;
        bool vblank_irq_en = SB_BFE(disp_stat,3,1);
        if(vblank&&vblank_irq_en) new_if|= (1<< GBA_INT_LCD_VBLANK); 
        gba->activate_dmas|=gba->dma_wait_ppu;
      }
      gba->ppu.last_lcd_y  = lcd_y;
      if(lcd_y==vcount_cmp) {
        bool vcnt_irq_en = SB_BFE(disp_stat,5,1);
        if(vcnt_irq_en)new_if |= (1<<GBA_INT_LCD_VCOUNT);
      }
    }
    gba_send_interrupt(gba,3,new_if);
  }

  if(!render)return; 
  
  if(lcd_x==GBA_LCD_HBLANK_START){
    uint16_t dispcnt = gba->ppu.dispcnt_pipeline[0];
    int bg_mode = SB_BFE(dispcnt,0,3);
    // From Mirei: Affine registers are only incremented when bg_mode is not 0
    // and the bg is enabled.
    if(bg_mode!=0){
      for(int aff=0;aff<2;++aff){
        bool bg_en = SB_BFE(dispcnt,8+aff+2,1);
        if(!bg_en)continue;
        int32_t b = (int16_t)gba_io_read16(gba,GBA_BG2PB+(aff)*0x10);
        int32_t d = (int16_t)gba_io_read16(gba,GBA_BG2PD+(aff)*0x10);
        uint16_t bgcnt = gba_io_read16(gba, GBA_BG2CNT+aff*2);
        bool mosaic = SB_BFE(bgcnt,6,1);
        if(mosaic){
          uint16_t mos_reg = gba_io_read16(gba,GBA_MOSAIC);
          int mos_y = SB_BFE(mos_reg,4,4)+1;
          if((lcd_y%mos_y)==0){
            gba->ppu.aff[aff].render_bgx+=b*mos_y;
            gba->ppu.aff[aff].render_bgy+=d*mos_y;
          }
        }else{
          gba->ppu.aff[aff].render_bgx+=b;
          gba->ppu.aff[aff].render_bgy+=d;
        }
      }
    }
  }
  bool reload_ref_points = lcd_x==GBA_LCD_HBLANK_END|| (lcd_y==0&&lcd_x==0);
  if(reload_ref_points){
    //Latch BGX and BGY registers
    for(int aff=0;aff<2;++aff){
      if(gba->ppu.aff[aff].wrote_bgx||(lcd_y==0&&lcd_x==0)){
        gba->ppu.aff[aff].render_bgx = gba_io_read32(gba,GBA_BG2X+(aff)*0x10);
        gba->ppu.aff[aff].render_bgx = SB_BFE(gba->ppu.aff[aff].render_bgx,0,28);
        gba->ppu.aff[aff].render_bgx = ((int32_t)(gba->ppu.aff[aff].render_bgx<<4))>>4;
        gba->ppu.aff[aff].wrote_bgx  = false; 
      }
      if(gba->ppu.aff[aff].wrote_bgy||(lcd_y==0&&lcd_x==0)){
        gba->ppu.aff[aff].render_bgy = gba_io_read32(gba,GBA_BG2Y+(aff)*0x10);
        gba->ppu.aff[aff].render_bgy = SB_BFE(gba->ppu.aff[aff].render_bgy,0,28);
        gba->ppu.aff[aff].render_bgy = ((int32_t)(gba->ppu.aff[aff].render_bgy<<4))>>4;
        gba->ppu.aff[aff].wrote_bgy  = false; 
      }
    }
  }
  uint16_t dispcnt = gba_io_read16(gba,GBA_DISPCNT);
  int bg_mode = SB_BFE(dispcnt,0,3);
  int obj_vram_map_2d = !SB_BFE(dispcnt,6,1);
  int forced_blank = SB_BFE(dispcnt,7,1);
  bool visible = lcd_x<240 && lcd_y<160;
  //Render sprites over scanline when it completes
  if((lcd_y<159 || lcd_y ==227) && lcd_x == GBA_LCD_HBLANK_START){
    int sprite_lcd_y = (lcd_y+1)%228;
    uint16_t mos_reg = gba_io_read16(gba,GBA_MOSAIC);
    int mos_y = SB_BFE(mos_reg,12,4)+1;
    // Partial fix to https://github.com/skylersaleh/SkyEmu/issues/316
    if(++gba->ppu.mosaic_y_counter>=mos_y||sprite_lcd_y ==0){
      gba->ppu.mosaic_y_counter=0;
    }
    //Render sprites over scanline when it completes
    uint8_t default_window_control =0x3f;//bitfield [0-3:bg0-bg3 enable 4:obj enable, 5: special effect enable]
    bool winout_enable = SB_BFE(dispcnt,13,3)!=0;
    uint16_t WINOUT = gba_io_read16(gba, GBA_WINOUT);
    if(winout_enable)default_window_control = SB_BFE(WINOUT,0,8);
  
    for(int x=0;x<240;++x){gba->window[x] = default_window_control;}
    uint8_t obj_window_control = default_window_control;
    bool obj_window_enable = SB_BFE(dispcnt,15,1);
    if(obj_window_enable)obj_window_control = SB_BFE(WINOUT,8,6);
    bool display_obj = SB_BFE(dispcnt,12,1);
    if(display_obj){
      for(int o=0;o<128;++o){
        uint16_t attr0 = *(uint16_t*)(gba->mem.oam+o*8+0);
        //Attr0
        uint8_t y_coord = SB_BFE(attr0,0,8);
        bool rot_scale =  SB_BFE(attr0,8,1);
        bool double_size = SB_BFE(attr0,9,1)&&rot_scale;
        bool obj_disable = SB_BFE(attr0,9,1)&&!rot_scale;
        if(obj_disable) continue; 

        int obj_mode = SB_BFE(attr0,10,2); //(0=Normal, 1=Semi-Transparent, 2=OBJ Window, 3=Prohibited)
        bool mosaic  = SB_BFE(attr0,12,1);
        bool colors_or_palettes = SB_BFE(attr0,13,1);
        int obj_shape = SB_BFE(attr0,14,2);//(0=Square,1=Horizontal,2=Vertical,3=Prohibited)
        uint16_t attr1 = *(uint16_t*)(gba->mem.oam+o*8+2);

        int rotscale_param = SB_BFE(attr1,9,5);
        bool h_flip = SB_BFE(attr1,12,1)&&!rot_scale;
        bool v_flip = SB_BFE(attr1,13,1)&&!rot_scale;
        int obj_size = SB_BFE(attr1,14,2);
        // Size  Square   Horizontal  Vertical
        // 0     8x8      16x8        8x16
        // 1     16x16    32x8        8x32
        // 2     32x32    32x16       16x32
        // 3     64x64    64x32       32x64
        const int xsize_lookup[16]={
          8,16,8,0,
          16,32,8,0,
          32,32,16,0,
          64,64,32,0
        };
        const int ysize_lookup[16]={
          8,8,16,0,
          16,8,32,0,
          32,16,32,0,
          64,32,64,0
        }; 

        int y_size = ysize_lookup[obj_size*4+obj_shape];

        if(((sprite_lcd_y-y_coord)&0xff) <y_size*(double_size?2:1)){
          int16_t x_coord = SB_BFE(attr1,0,9);
          if (SB_BFE(x_coord,8,1))x_coord|=0xfe00;

          int x_size = xsize_lookup[obj_size*4+obj_shape];
          int x_start = x_coord>=0?x_coord:0;
          int x_end   = x_coord+x_size*(double_size?2:1);
          if(x_end>=240)x_end=240;
          //Attr2
          //Skip objects disabled by window
          uint16_t attr2 = *(uint16_t*)(gba->mem.oam+o*8+4);
          int tile_base = SB_BFE(attr2,0,10);
          // Always place sprites as the highest priority
          int priority = SB_BFE(attr2,10,2);
          int palette = SB_BFE(attr2,12,4);
          for(int x = x_start; x< x_end;++x){
            int sx = (x-x_coord);
            int sy = (sprite_lcd_y-y_coord)&0xff;
            if(mosaic){
              uint16_t mos_reg = gba_io_read16(gba,GBA_MOSAIC);
              int mos_x = SB_BFE(mos_reg,8,4)+1;
              int mos_y = SB_BFE(mos_reg,12,4)+1;
              sx = ((x/mos_x)*mos_x-x_coord);
              if(sx<0)sx=0;
              sy = (sprite_lcd_y-y_coord)&0xff;
              sy -= gba->ppu.mosaic_y_counter;
              if(sy<0){sy =0;} 
            }
            if(rot_scale){
              uint32_t param_base = rotscale_param*0x20; 
              int32_t a = *(int16_t*)(gba->mem.oam+param_base+0x6);
              int32_t b = *(int16_t*)(gba->mem.oam+param_base+0xe);
              int32_t c = *(int16_t*)(gba->mem.oam+param_base+0x16);
              int32_t d = *(int16_t*)(gba->mem.oam+param_base+0x1e);
   
              int64_t x1 = sx<<8;
              int64_t y1 = sy<<8;
              int64_t objref_x = (x_size<<(double_size?8:7));
              int64_t objref_y = (y_size<<(double_size?8:7));
              
              int64_t x2 = a*(x1-objref_x) + b*(y1-objref_y)+(x_size<<15);
              int64_t y2 = c*(x1-objref_x) + d*(y1-objref_y)+(y_size<<15);

              sx = (x2>>16);
              sy = (y2>>16);
              if(sx>=x_size||sy>=y_size||sx<0||sy<0)continue;
            }else{
              if(h_flip)sx=x_size-sx-1;
              if(v_flip)sy=y_size-sy-1;
            }
            int tx = sx%8;
            int ty = sy%8;
                      
            int y_tile_stride = obj_vram_map_2d? 32 : x_size/8*(colors_or_palettes? 2:1);
            int tile = tile_base + (((sx/8))*(colors_or_palettes? 2:1))+(sy/8)*y_tile_stride;
            // Don't allow the column indices to overflow into the row indices in 2D mode. 
            // See: https://github.com/skylersaleh/SkyEmu/issues/13
            if(obj_vram_map_2d){
              tile = (tile_base + (((sx/8))*(colors_or_palettes? 2:1)))&31;
              tile|= (tile_base + (sy/8)*y_tile_stride)&~31;
            }
            //Tiles >511 are not rendered in bg_mode3-5 since that memory is used to store the bitmap graphics. 
            if(tile<512&&bg_mode>=3&&bg_mode<=5)continue;
            uint8_t palette_id;
            int obj_tile_base = GBA_OBJ_TILES0_2;
            bool transparent = false; 
            if(colors_or_palettes==false){
              palette_id= gba->mem.vram[obj_tile_base+tile*8*4+tx/2+ty*4];
              palette_id= (palette_id>>((tx&1)*4))&0xf;
              transparent=palette_id==0;
              palette_id+=palette*16;
            }else{
              palette_id=gba->mem.vram[obj_tile_base+tile*8*4+tx+ty*8];
              transparent=palette_id==0;
            }

            uint32_t col = *(uint16_t*)(gba->mem.palette+GBA_OBJ_PALETTE+palette_id*2);
            //Handle window objects(not displayed but control the windowing of other things)
            if(obj_mode==2&&!transparent){gba->window[x]=obj_window_control; 
            }else if(obj_mode!=3){
              int type =4;
              col=col|(type<<17)|((5-priority)<<28)|((0x7)<<25);
              if(obj_mode==1)col|=1<<16;
              if((col>>17)>(gba->first_target_buffer[x]>>17)){
                if(transparent){
                  //Update priority for transparent pixels (needed for golden sun)
                  if(SB_BFE(gba->first_target_buffer[x],17,3)!=5)
                    gba->first_target_buffer[x]=(gba->first_target_buffer[x]&(0x0fffffff))|(col&0xf0000000);
                }else gba->first_target_buffer[x]=col;
              }
            }  
          }
        }
      }
    }
    int enabled_windows = SB_BFE(dispcnt,13,3); // [0: win0, 1:win1, 2: objwin]
    if(enabled_windows){
      for(int win=1;win>=0;--win){
        bool win_enable = SB_BFE(dispcnt,13+win,1);
        if(!win_enable)continue;
        uint16_t WINH = gba_io_read16(gba, GBA_WIN0H+2*win);
        uint16_t WINV = gba_io_read16(gba, GBA_WIN0V+2*win);
        int win_xmin = SB_BFE(WINH,8,8);
        int win_xmax = SB_BFE(WINH,0,8);
        int win_ymin = SB_BFE(WINV,8,8);
        int win_ymax = SB_BFE(WINV,0,8);
        // Garbage values of X2>240 or X1>X2 are interpreted as X2=240.
        // Garbage values of Y2>160 or Y1>Y2 are interpreted as Y2=160. 
        if(win_xmin>win_xmax)win_xmax=240;
        if(win_ymin>win_ymax)win_ymax=161;
        if(win_xmax>240)win_xmax=240;
        if(sprite_lcd_y<win_ymin||sprite_lcd_y>=win_ymax)continue;
        uint16_t winin = gba_io_read16(gba,GBA_WININ);
        uint8_t win_value = SB_BFE(winin,win*8,6);
        for(int x=win_xmin;x<win_xmax;++x)gba->window[x] = win_value;
      }
      int backdrop_type = 5;
      uint32_t backdrop_col = (*(uint16_t*)(gba->mem.palette + GBA_BG_PALETTE+0*2))|(backdrop_type<<17);
      for(int x=0;x<240;++x){
        uint8_t window_control = gba->window[x];
        if(SB_BFE(window_control,4,1)==0)gba->first_target_buffer[x]=backdrop_col;
      }
    }
  }

  if(visible){
    uint8_t window_control =gba->window[lcd_x];
    if(bg_mode==6 ||bg_mode==7){
      //Palette 0 is taken as the background
    }else if (bg_mode<=5){     
      for(int bg = 3; bg>=0;--bg){
        uint32_t col =0;         
        if((bg<2&&bg_mode==2)||(bg==3&&bg_mode==1)||(bg!=2&&bg_mode>=3))continue;
        bool bg_en = SB_BFE(dispcnt,8+bg,1)&&SB_BFE(gba->ppu.dispcnt_pipeline[0],8+bg,1);
        if(!bg_en || SB_BFE(window_control,bg,1)==0)continue;

        bool rot_scale = bg_mode>=1&&bg>=2;
        uint16_t bgcnt = gba_io_read16(gba, GBA_BG0CNT+bg*2);
        int priority = SB_BFE(bgcnt,0,2);
        int character_base = SB_BFE(bgcnt,2,2);
        bool mosaic = SB_BFE(bgcnt,6,1);
        bool colors = SB_BFE(bgcnt,7,1);
        int screen_base = SB_BFE(bgcnt,8,5);
        bool display_overflow =SB_BFE(bgcnt,13,1);
        int screen_size = SB_BFE(bgcnt,14,2);

        int screen_size_x = (screen_size&1)?512:256;
        int screen_size_y = (screen_size>=2)?512:256;
        
        int bg_x = 0;
        int bg_y = 0;
        
        if(rot_scale){
          screen_size_x = screen_size_y = (16*8)<<screen_size;
          if(bg_mode==3||bg_mode==4){
            screen_size_x=240;
            screen_size_y=160;
          }else if(bg_mode==5){
            screen_size_x=160;
            screen_size_y=128;
          }
          colors = true;

          int32_t bgx = gba->ppu.aff[bg-2].render_bgx;
          int32_t bgy = gba->ppu.aff[bg-2].render_bgy;

          int32_t a = (int16_t)gba_io_read16(gba,GBA_BG2PA+(bg-2)*0x10);
          int32_t c = (int16_t)gba_io_read16(gba,GBA_BG2PC+(bg-2)*0x10);

          // Shift lcd_coords into fixed point
          int64_t x2 = a*lcd_x + (((int64_t)bgx));
          int64_t y2 = c*lcd_x + (((int64_t)bgy));
          if(mosaic){
            int16_t mos_reg = gba_io_read16(gba,GBA_MOSAIC);
            int mos_x = SB_BFE(mos_reg,0,4)+1;
            x2 = a*((lcd_x/mos_x)*mos_x) + (((int64_t)bgx));
            y2 = c*((lcd_x/mos_x)*mos_x) + (((int64_t)bgy));
          }


          bg_x = (x2>>8);
          bg_y = (y2>>8);

          if(display_overflow==0){
            if(bg_x<0||bg_x>=screen_size_x||bg_y<0||bg_y>=screen_size_y)continue; 
          }else{
            bg_x%=screen_size_x;
            bg_y%=screen_size_y;
          }
                              
        }else{
          int16_t hoff = gba_io_read16(gba,GBA_BG0HOFS+bg*4);
          int16_t voff = gba_io_read16(gba,GBA_BG0VOFS+bg*4);
          hoff=(hoff<<7)>>7;
          voff=(voff<<7)>>7;
          bg_x = (hoff+lcd_x);
          bg_y = (voff+lcd_y);
          if(mosaic){
            uint16_t mos_reg = gba_io_read16(gba,GBA_MOSAIC);
            int mos_x = SB_BFE(mos_reg,0,4)+1;
            int mos_y = SB_BFE(mos_reg,4,4)+1;
            bg_x = hoff+(lcd_x/mos_x)*mos_x;
            bg_y = voff+(lcd_y/mos_y)*mos_y;
          }
        }
        if(bg_mode==3){
          int p = bg_x+bg_y*240;
          int addr = p*2; 
          col  = *(uint16_t*)(gba->mem.vram+addr);
        }else if(bg_mode==4){
          int p = bg_x+bg_y*240;
          int frame_sel = SB_BFE(dispcnt,4,1);
          int addr = p*1+0xA000*frame_sel; 
          uint8_t pallete_id = gba->mem.vram[addr];
          if(pallete_id==0)continue;
          col = *(uint16_t*)(gba->mem.palette+GBA_BG_PALETTE+pallete_id*2);
        }else if(bg_mode==5){
          int p = bg_x+bg_y*160;
          int frame_sel = SB_BFE(dispcnt,4,1);
          int addr = p*2+0xA000*frame_sel; 
          col  = *(uint16_t*)(gba->mem.vram+addr);
        }else{
          bg_x = bg_x&(screen_size_x-1);
          bg_y = bg_y&(screen_size_y-1);
          int bg_tile_x = bg_x/8;
          int bg_tile_y = bg_y/8;

          int tile_off = bg_tile_y*(screen_size_x/8)+bg_tile_x;

          int screen_base_addr =    screen_base*2048;
          int character_base_addr = character_base*16*1024;

          uint16_t tile_data =0;

          int px = bg_x%8;
          int py = bg_y%8;

          if(rot_scale)tile_data=gba->mem.vram[screen_base_addr+tile_off];
          else{
            int tile_off = (bg_tile_y%32)*32+(bg_tile_x%32);
            if(bg_tile_x>=32)tile_off+=32*32;
            if(bg_tile_y>=32)tile_off+=32*32*(screen_size==3?2:1);
            tile_data=*(uint16_t*)(gba->mem.vram+screen_base_addr+tile_off*2);

            int h_flip = SB_BFE(tile_data,10,1);
            int v_flip = SB_BFE(tile_data,11,1);
            if(h_flip)px=7-px;
            if(v_flip)py=7-py;
          }
          int tile_id = SB_BFE(tile_data,0,10);
          int palette = SB_BFE(tile_data,12,4);

          uint8_t tile_d=tile_id;
          if(colors==false){
            int addr = character_base_addr+tile_id*8*4+px/2+py*4;
            tile_d=gba->mem.vram[addr];
            tile_d= (tile_d>>((px&1)*4))&0xf;
            //There is an undocumented GBA quirk where tiles over 64KB are not loaded
            //https://github.com/skylersaleh/SkyEmu/issues/292
            if(tile_d==0||SB_UNLIKELY(addr>=0x10000))continue;
            tile_d+=palette*16;
          }else{
            //There is an undocumented GBA quirk where tiles over 64KB are not loaded
            //https://github.com/skylersaleh/SkyEmu/issues/292
            int addr=character_base_addr+tile_id*8*8+px+py*8;
            tile_d=gba->mem.vram[addr];
            if(tile_d==0||SB_UNLIKELY(addr>=0x10000))continue;
          }
          uint8_t pallete_id = tile_d;
          col = *(uint16_t*)(gba->mem.palette+GBA_BG_PALETTE+pallete_id*2);
        }
        col |= (bg<<17) | ((5-priority)<<28)|((4-bg)<<25);
        if(col>gba->first_target_buffer[lcd_x]){
          uint32_t t = gba->first_target_buffer[lcd_x];
          gba->first_target_buffer[lcd_x]=col;
          col = t;
        }
        if(col>gba->second_target_buffer[lcd_x])gba->second_target_buffer[lcd_x]=col;          
      }
    }
    uint32_t col = gba->first_target_buffer[lcd_x];
    int r = SB_BFE(col,0,5);
    int g = SB_BFE(col,5,5);
    int b = SB_BFE(col,10,5);
    uint32_t type = SB_BFE(col,17,3);

    bool effect_enable = SB_BFE(window_control,5,1);
    uint16_t bldcnt = gba_io_read16(gba,GBA_BLDCNT);
    int mode = SB_BFE(bldcnt,6,2);

    //Semitransparent objects are always selected for blending
    if(SB_BFE(col,16,1)){
      uint32_t col2 = gba->second_target_buffer[lcd_x];
      uint32_t type2 = SB_BFE(col2,17,3);
      bool blend = SB_BFE(bldcnt,8+type2,1);
      if(blend){mode=1;effect_enable=true;}
      else effect_enable &= SB_BFE(bldcnt,type,1);
    }else effect_enable &= SB_BFE(bldcnt,type,1);
    if(effect_enable){
      uint16_t bldy = gba_io_read16(gba,GBA_BLDY);
      float evy = SB_BFE(bldy,0,5)/16.;
      if(evy>1.0)evy=1;
      switch(mode){
        case 0: break; //None
        case 1: {
          uint32_t col2 = gba->second_target_buffer[lcd_x];
          uint32_t type2 = SB_BFE(col2,17,3);
          bool blend = SB_BFE(bldcnt,8+type2,1);
          if(blend){
            uint16_t bldalpha= gba_io_read16(gba,GBA_BLDALPHA);
            int r2 = SB_BFE(col2,0,5);
            int g2 = SB_BFE(col2,5,5);
            int b2 = SB_BFE(col2,10,5);
            int eva = SB_BFE(bldalpha,0,5);
            int evb = SB_BFE(bldalpha,8,5);
            if(eva>16)eva=16;
            if(evb>16)evb=16;
            r = (r*eva+r2*evb)/16;
            g = (g*eva+g2*evb)/16;
            b = (b*eva+b2*evb)/16;
            if(r>31)r = 31;
            if(g>31)g = 31;
            if(b>31)b = 31;
          }
        }break; //Alpha Blend
        case 2: //Lighten
          r = r+(31-r)*evy;
          g = g+(31-g)*evy;
          b = b+(31-b)*evy;  
          break; 
        case 3: //Darken
          r = r-(r)*evy;
          g = g-(g)*evy;
          b = b-(b)*evy;         
          break; 
      }
    }
    if(forced_blank){
      r=g=b=255;
      if(gba->stop_mode)r=g=b=0;
    }

    int backdrop_type = 5;
    uint32_t backdrop_col = (*(uint16_t*)(gba->mem.palette + GBA_BG_PALETTE+0*2))|(backdrop_type<<17);
    gba->first_target_buffer[lcd_x] = backdrop_col;
    gba->second_target_buffer[lcd_x] = backdrop_col;

    int p = (lcd_x+lcd_y*240)*4;
    float screen_blend_factor = 0.3*gba->ppu.ghosting_strength;
    uint16_t green_swap = gba_io_read16(gba,GBA_GREENSWP);
    gba->framebuffer[p+0] = r*8*(1.0-screen_blend_factor)+gba->framebuffer[p+0]*screen_blend_factor;
    gba->framebuffer[p+2] = b*8*(1.0-screen_blend_factor)+gba->framebuffer[p+2]*screen_blend_factor;  

    if(green_swap&1){
      if(p&4) gba->framebuffer[p+1-4] = g*8*(1.0-screen_blend_factor)+gba->framebuffer[p+1-4]*screen_blend_factor;
      else gba->framebuffer[p+1+4] = g*8*(1.0-screen_blend_factor)+gba->framebuffer[p+1+4]*screen_blend_factor;
    }else{
      gba->framebuffer[p+1] = g*8*(1.0-screen_blend_factor)+gba->framebuffer[p+1]*screen_blend_factor;
    }
  }
}
static void gba_tick_keypad(sb_joy_t*joy, gba_t* gba){
  uint16_t reg_value = 0;
  //Null joy updates are used to tick the joypad when mmios are set
  if(joy){
    reg_value|= !(joy->inputs[SE_KEY_A]>0.3)     <<0;
    reg_value|= !(joy->inputs[SE_KEY_B]>0.3)     <<1;
    reg_value|= !(joy->inputs[SE_KEY_SELECT]>0.3)<<2;
    reg_value|= !(joy->inputs[SE_KEY_START]>0.3) <<3;
    reg_value|= !(joy->inputs[SE_KEY_RIGHT]>0.3) <<4;
    reg_value|= !(joy->inputs[SE_KEY_LEFT]>0.3)  <<5;
    reg_value|= !(joy->inputs[SE_KEY_UP]>0.3)    <<6;
    reg_value|= !(joy->inputs[SE_KEY_DOWN]>0.3)  <<7;
    reg_value|= !(joy->inputs[SE_KEY_R]>0.3)     <<8;
    reg_value|= !(joy->inputs[SE_KEY_L]>0.3)     <<9;
    gba_io_store16(gba, GBA_KEYINPUT, reg_value);
  }else reg_value = gba_io_read16(gba, GBA_KEYINPUT);

  uint16_t keycnt = gba_io_read16(gba,GBA_KEYCNT);
  bool irq_enable = SB_BFE(keycnt,14,1);
  bool irq_condition = SB_BFE(keycnt,15,1);//[0: any key, 1: all keys]
  int if_bit = 0;
  if(irq_enable||gba->stop_mode){
    uint16_t pressed = SB_BFE(reg_value,0,10)^0x3ff;
    uint16_t mask = SB_BFE(keycnt,0,10);

    if(irq_condition&&((pressed&mask)==mask))if_bit|= 1<<GBA_INT_KEYPAD;
    if(!irq_condition&&((pressed&mask)!=0))if_bit|= 1<<GBA_INT_KEYPAD;

    if(if_bit)gba->stop_mode=false;

    if(if_bit&&!gba->prev_key_interrupt&&irq_enable){
      gba_send_interrupt(gba,4,if_bit);
      gba->prev_key_interrupt = true;
    }else gba->prev_key_interrupt = false;

  }

}
uint64_t gba_read_eeprom_bitstream(gba_t *gba, uint32_t source_address, int offset, int size, int elem_size, int dir){
  uint64_t data = 0; 
  for(int i=0;i<size;++i){
    data|= ((uint64_t)(gba_read16(gba,source_address+(i+offset)*elem_size*dir)&1))<<(size-i-1);
  }
  return data; 
}
void gba_store_eeprom_bitstream(gba_t *gba, uint32_t source_address, int offset, int size, int elem_size, int dir,uint64_t data){
  for(int i=0;i<size;++i){
    gba_store16(gba,source_address+(i+offset)*elem_size*dir,data>>(size-i-1)&1);
  }
}
static FORCE_INLINE int gba_tick_dma(gba_t*gba, int last_tick){
  int ticks =0;
  gba->activate_dmas=false;
  gba->dma_wait_ppu=false;
  for(int i=0;i<4;++i){
    uint16_t cnt_h=gba_io_read16(gba, GBA_DMA0CNT_H+12*i);
    bool enable = SB_BFE(cnt_h,15,1);
    if(enable){
      bool type = SB_BFE(cnt_h,10,1); // 0: 16b 1:32b

      if(!gba->dma[i].last_enable){
        gba->dma[i].last_enable = enable;
        gba->dma[i].source_addr=gba_io_read32(gba,GBA_DMA0SAD+12*i);
        gba->dma[i].dest_addr=gba_io_read32(gba,GBA_DMA0DAD+12*i);
        //GBA Suite says that these need to be force aligned
        if(type){
          gba->dma[i].dest_addr&=~3;
          gba->dma[i].source_addr&=~3;
        }else{
          gba->dma[i].dest_addr&=~1;
          gba->dma[i].source_addr&=~1;
        }
        gba->dma[i].current_transaction=0;
        gba->dma[i].startup_delay=2;
      }
      int  dst_addr_ctl = SB_BFE(cnt_h,5,2); // 0: incr 1: decr 2: fixed 3: incr reload
      int  src_addr_ctl = SB_BFE(cnt_h,7,2); // 0: incr 1: decr 2: fixed 3: not allowed
      bool dma_repeat = SB_BFE(cnt_h,9,1); 
      int  mode = SB_BFE(cnt_h,12,2);
      bool irq_enable = SB_BFE(cnt_h,14,1);
      bool force_first_write_sequential = false;
      int transfer_bytes = type? 4:2; 
      bool skip_dma = false;
      if(gba->dma[i].current_transaction==0){
        if(mode==3 && i ==0)continue;
        if(gba->dma[i].startup_delay>=0){
          gba->dma[i].startup_delay-=last_tick; 
          if(gba->dma[i].startup_delay>=0){
            gba->activate_dmas=true;
            continue;
          }
          gba->dma[i].startup_delay=-1;
        }
        if(dst_addr_ctl==3){        
          gba->dma[i].dest_addr=gba_io_read32(gba,GBA_DMA0DAD+12*i);
        }
        bool last_vblank = gba->dma[i].last_vblank;
        bool last_hblank = gba->dma[i].last_hblank;
        gba->dma[i].last_vblank = gba->ppu.last_vblank;
        gba->dma[i].last_hblank = gba->ppu.last_hblank;
        if(mode ==1 && (!gba->ppu.last_vblank||last_vblank)){
          gba->dma_wait_ppu=true;
          continue; 
        }
        if(mode==2){
          gba->dma_wait_ppu=true;
          uint16_t vcount = gba_io_read16(gba,GBA_VCOUNT);
          if(vcount>=160||!gba->ppu.last_hblank||last_hblank)continue;
        }
        //Video dma
        if(mode==3 && i ==3){
          gba->dma_wait_ppu=true;
          uint16_t vcount = gba_io_read16(gba,GBA_VCOUNT);
          if(!gba->ppu.last_hblank||last_hblank)continue;
          //Video dma starts at scanline 2
          if(vcount==2){gba->dma[i].video_dma_active=true;}
          if(!gba->dma[i].video_dma_active)continue;
          if(vcount==161){
            dma_repeat=false;
            gba->dma[i].video_dma_active=false;
          }
        }
        
        if(dst_addr_ctl==3){
          gba->dma[i].dest_addr=gba_io_read32(gba,GBA_DMA0DAD+12*i);
          //GBA Suite says that these need to be force aligned
          if(type) gba->dma[i].dest_addr&=~3;
          else gba->dma[i].dest_addr&=~1;
        }
        bool audio_dma = (mode==3) && (i==1||i==2);
        if(audio_dma){
          if(gba->dma[i].activate_audio_dma==false)continue;
          gba->dma[i].activate_audio_dma=false;
        }
        if(gba->dma[i].source_addr>=0x08000000&&gba->dma[i].dest_addr>=0x08000000){
          force_first_write_sequential=true;
        }else{
          if(gba->dma[i].dest_addr>=0x08000000){
            // Allow the in process prefetech to finish before starting DMA
            if(!gba->mem.prefetch_size&&gba->mem.prefetch_en)ticks+=gba_compute_access_cycles_dma(gba,gba->dma[i].dest_addr,2)>4;
          }
        }
        if(gba->dma[i].source_addr>=0x08000000){
            if(gba->mem.prefetch_en)ticks+=gba_compute_access_cycles_dma(gba,gba->dma[i].source_addr,2)<=4;
        }
        gba->last_transaction_dma=true;
        uint32_t cnt = gba_io_read16(gba,GBA_DMA0CNT_L+12*i);

        if(i!=3)cnt&=0x3fff;
        if(cnt==0)cnt = i==3? 0x10000: 0x4000;

        static const uint32_t src_mask[] = { 0x07FFFFFF, 0x0FFFFFFF, 0x0FFFFFFF, 0x0FFFFFFF};
        static const uint32_t dst_mask[] = { 0x07FFFFFF, 0x07FFFFFF, 0x07FFFFFF, 0x0FFFFFFF};
        gba->dma[i].source_addr&=src_mask[i];
        gba->dma[i].dest_addr  &=dst_mask[i];
        gba_io_store16(gba,GBA_DMA0CNT_L+12*i,cnt);
      }
      const static int dir_lookup[4]={1,-1,0,1};
      int src_dir = dir_lookup[src_addr_ctl];
      int dst_dir = dir_lookup[dst_addr_ctl];

      uint32_t src = gba->dma[i].source_addr;
      uint32_t dst = gba->dma[i].dest_addr;
      uint32_t cnt = gba_io_read16(gba,GBA_DMA0CNT_L+12*i);

      // ROM ignores direction and always increments
      if(src>=0x08000000&&src<0x0e000000) src_dir=1;
      if(dst>=0x08000000&&dst<0x0e000000) dst_dir=1;

      // EEPROM DMA transfers
      if(i==3 && gba->cart.backup_type==GBA_BACKUP_EEPROM){
        int src_in_eeprom = (src&0x1ffffff)>=gba->cart.rom_size||(src&0x1ffffff)>=0x01ffff00;
        int dst_in_eeprom = (dst&0x1ffffff)>=gba->cart.rom_size||(dst&0x1ffffff)>=0x01ffff00;
        src_in_eeprom &= src>=0x8000000 && src<=0xDFFFFFF;
        dst_in_eeprom &= dst>=0x8000000 && dst<=0xDFFFFFF;
        skip_dma = src_in_eeprom || dst_in_eeprom;
        if(dst_in_eeprom){
          if(cnt==73){
            // Write data 6 bit address
            uint32_t addr = gba_read_eeprom_bitstream(gba, src, 2, 6, type?4:2, src_dir);
            uint64_t data = gba_read_eeprom_bitstream(gba, src, 2+6, 64, type?4:2, src_dir); 
            ((uint64_t*)gba->mem.cart_backup)[addr]=data;
            gba->cart.backup_is_dirty=true;
          }else if(cnt==81){
            // Write data 14 bit address
            uint32_t addr = gba_read_eeprom_bitstream(gba, src, 2, 14, type?4:2, src_dir)&0x3ff;
            uint64_t data = gba_read_eeprom_bitstream(gba, src, 2+14, 64, type?4:2, src_dir); 
            ((uint64_t*)gba->mem.cart_backup)[addr]=data;
            gba->cart.backup_is_dirty=true;
          }else if(cnt==9){
            // 2 bits "11" (Read Request)
            // 6 bits eeprom address (MSB first)
            // 1 bit "0"
            // Write data 6 bit address
            gba->mem.eeprom_addr = gba_read_eeprom_bitstream(gba, src, 2, 6, type?4:2, src_dir);
          }else if(cnt==17){
            // 2 bits "11" (Read Request)
            // 14 bits eeprom address (MSB first)
            // 1 bit "0"
            // Write data 6 bit address
            gba->mem.eeprom_addr = gba_read_eeprom_bitstream(gba, src, 2, 14, type?4:2, src_dir)&0x3ff;
          }else{
            log_printf("Bad cnt: %d for eeprom write\n",cnt);
          }
          gba->dma[i].current_transaction=cnt;
        }
        if(src_in_eeprom){
          if(cnt==68){
            uint64_t data = ((uint64_t*)gba->mem.cart_backup)[gba->mem.eeprom_addr];
            gba_store_eeprom_bitstream(gba, dst, 4, 64, type?4:2, dst_dir,data);
          }else{
            log_printf("Bad cnt: %d for eeprom read\n",cnt);
          }
          gba->dma[i].current_transaction=cnt;
        }
      }
      bool audio_dma = (mode==3) && (i==1||i==2);
      if(audio_dma){
        int fifo = i-1;
        dst&=~3;
        src&=~3;
        for(int x=0;x<4;++x){
          uint32_t src_addr=src+x*4*src_dir;
          uint32_t data = gba_read32(gba,src_addr);
          gba_audio_fifo_push(gba,fifo,SB_BFE(data,0,8));
          gba_audio_fifo_push(gba,fifo,SB_BFE(data,8,8));
          gba_audio_fifo_push(gba,fifo,SB_BFE(data,16,8));
          gba_audio_fifo_push(gba,fifo,SB_BFE(data,24,8));
          ticks+=gba_compute_access_cycles_dma(gba, src_addr, x!=0? 2:3);
          ticks+=gba_compute_access_cycles_dma(gba, dst, x!=0||force_first_write_sequential? 2:3);
        }
        dst_addr_ctl= 2; 
        transfer_bytes=4;
        cnt=4;
        skip_dma=true;
        gba->dma[i].current_transaction=cnt;
      }else if(!skip_dma){
        // This code is complicated to handle the per channel DMA latches that are present
        // Correct implementation is needed to pass latch.gba, Pokemon Pinball (intro explosion),
        // and the text in Lufia
        // TODO: There in theory should be separate latches per DMA, but that breaks Hello Kitty
        // and Tomb Raider
        if(gba->dma[i].current_transaction<cnt){
          int x = gba->dma[i].current_transaction++;
          int dst_addr = dst+x*transfer_bytes*dst_dir;
          int src_addr = src+x*transfer_bytes*src_dir;
          if(type){
            if(src_addr>=0x02000000){
              gba->dma[i].latched_transfer = gba_read32(gba,src_addr);
              ticks+=gba_compute_access_cycles_dma(gba, src_addr, x!=0? 2:3);
            }
            gba_dma_write32(gba,dst_addr,gba->dma[i].latched_transfer);
            ticks+=gba_compute_access_cycles_dma(gba, dst_addr, x!=0||force_first_write_sequential? 2:3);
          }else{
            int v = 0;
            if(src_addr>=0x02000000){
              v= gba->dma[i].latched_transfer = (gba_read16(gba,src_addr))&0xffff;
              gba->dma[i].latched_transfer |= gba->dma[i].latched_transfer<<16;
              ticks+=gba_compute_access_cycles_dma(gba, src_addr, x!=0? 0:1);
            }else v = gba->dma[i].latched_transfer>>(((dst_addr)&0x3)*8);
            gba_dma_write16(gba,dst_addr,v&0xffff);
            ticks+=gba_compute_access_cycles_dma(gba, dst_addr, x!=0||force_first_write_sequential? 0:1);
          }
        }
      }
      
      if(gba->dma[i].current_transaction>=cnt){
        if(dst_addr_ctl==0||dst_addr_ctl==3)     dst+=cnt*transfer_bytes;
        else if(dst_addr_ctl==1)dst-=cnt*transfer_bytes;
        if(src_addr_ctl==0)     src+=cnt*transfer_bytes;
        else if(src_addr_ctl==1)src-=cnt*transfer_bytes;
        
        gba->dma[i].source_addr=src;
        gba->dma[i].dest_addr=dst;

        if(irq_enable){
          uint16_t if_bit = 1<<(GBA_INT_DMA0+i);
          gba_send_interrupt(gba,4,if_bit);
        }
        if(!dma_repeat||mode==0){
          cnt_h&=0x7fff;
          //gba_io_store16(gba, GBA_DMA0CNT_L+12*i,0);
          //Reload on incr reload     
          enable =false;
          gba_io_store16(gba, GBA_DMA0CNT_H+12*i,cnt_h);
        }else{
          gba->dma[i].current_transaction=0;
        }
      }
    }
    gba->dma[i].last_enable = enable;
    if(ticks)break;
  }
  gba->activate_dmas|=ticks!=0;
 
  if(gba->last_transaction_dma&&ticks==0){
    ticks+=2; 
    gba->last_transaction_dma=false;
  }

  return ticks; 
}                                              
static FORCE_INLINE void gba_tick_sio(gba_t* gba){
  //Just a stub for now;
  uint16_t siocnt = gba_io_read16(gba,GBA_SIOCNT);
  bool active = SB_BFE(siocnt,7,1);
  bool irq_enabled = SB_BFE(siocnt,14,1);
  if(active){
    if(gba->sio.last_active==false){
      gba->sio.last_active =true;
      gba->sio.ticks_till_transfer_done=8*8;
    }
    bool internal_clock = SB_BFE(siocnt,0,1);
    if(internal_clock)gba->sio.ticks_till_transfer_done--;
    if(gba->sio.ticks_till_transfer_done<=0){
      if(irq_enabled){
        uint16_t if_bit = 1<<(GBA_INT_SERIAL);
        gba_send_interrupt(gba,4,if_bit);
      }
      siocnt&= ~(1<<7);
      gba_io_store16(gba,GBA_SIOCNT,siocnt);
      gba->sio.last_active=false;
      gba_io_store8(gba,GBA_SIODATA8,0);
      gba_io_store32(gba,GBA_SIODATA32,0);
    }
    
  }
}
static FORCE_INLINE void gba_tick_timers(gba_t* gba){
  gba->deferred_timer_ticks+=1;
  if(SB_UNLIKELY(gba->deferred_timer_ticks>=gba->timer_ticks_before_event))gba_compute_timers(gba); 
}
static void gba_compute_timers(gba_t* gba){
  int ticks = gba->deferred_timer_ticks; 
  uint32_t old_global_timer = gba->global_timer;
  gba->global_timer+=gba->deferred_timer_ticks;

  gba->deferred_timer_ticks=0;
  int last_timer_overflow = 0; 
  int timer_ticks_before_event = 32768;
  const int prescaler_lookup[]={0,6,8,10};
  for(int t=0;t<4;++t){ 
    uint16_t tm_cnt_h = gba_io_read16(gba,GBA_TM0CNT_H+t*4);
    bool enable = SB_BFE(tm_cnt_h,7,1);
    if(enable){
      int compensated_ticks = ticks;
      uint16_t prescale = SB_BFE(tm_cnt_h,0,2);
      bool count_up     = SB_BFE(tm_cnt_h,2,1)&&t!=0;
      bool irq_en       = SB_BFE(tm_cnt_h,6,1);
      uint16_t value = gba_io_read16(gba,GBA_TM0CNT_L+t*4);
      if(enable!=gba->timers[t].last_enable&&enable){
        gba->timers[t].startup_delay=2;
        value = gba->timers[t].reload_value;
        gba_io_store16(gba,GBA_TM0CNT_L+t*4,value);
      }
      if(gba->timers[t].startup_delay>=0){
        gba->timers[t].startup_delay-=ticks; 
        gba->timers[t].last_enable = enable;
        if(gba->timers[t].startup_delay>=0){
          if(gba->timers[t].startup_delay<timer_ticks_before_event)timer_ticks_before_event=gba->timers[t].startup_delay;
          continue;
        }
        compensated_ticks=-gba->timers[t].startup_delay;
        gba->timers[t].startup_delay=-1;
      }
      if(count_up){
        if(last_timer_overflow){
          uint32_t v= value;
          v+=last_timer_overflow;
          last_timer_overflow=0;
          while(v>0xffff){
            v=(v+gba->timers[t].reload_value)-0x10000;
            last_timer_overflow++;
          }
          value=v;
        }
      }else{
        last_timer_overflow=0;
        int prescale_duty = prescaler_lookup[prescale];

        int increment = (gba->global_timer>>prescale_duty)-(old_global_timer>>prescale_duty);
        int v = value+increment;
        while(v>0xffff){
          v=(v+gba->timers[t].reload_value)-0x10000;
          last_timer_overflow++;
        }
        value = v; 
        int ticks_before_overflow = (int)(0xffff-value)<<(prescale_duty);
        if(ticks_before_overflow<timer_ticks_before_event)timer_ticks_before_event=ticks_before_overflow;
      }
      if(last_timer_overflow){
        uint16_t soundcnt_h = gba_io_read16(gba,GBA_SOUNDCNT_H);
        if(t<2){
          for(int i=0;i<2;++i){
            int timer = SB_BFE(soundcnt_h,10+i*4,1);
            if(timer!=t)continue;
            int samples_to_pop=last_timer_overflow;
            int size = (gba->audio.fifo[i].write_ptr-gba->audio.fifo[i].read_ptr)&0x1f;
            while(samples_to_pop--&& size){
              gba->audio.fifo[i].read_ptr=(gba->audio.fifo[i].read_ptr+1)&0x1f;
              --size;
            }
            if(size<GBA_AUDIO_DMA_ACTIVATE_THRESHOLD)gba->dma[i+1].activate_audio_dma=gba->activate_dmas=true;
          }
        }
        if(irq_en){
          uint16_t if_bit = 1<<(GBA_INT_TIMER0+t);
          gba_send_interrupt(gba,4,if_bit);        
        }
      }
      gba->timers[t].reload_value=gba->timers[t].pending_reload_value;
      
      gba_io_store16(gba,GBA_TM0CNT_L+t*4,value);
    }else last_timer_overflow=0;
    gba->timers[t].last_enable = enable;
  }
  gba->timer_ticks_before_event=timer_ticks_before_event;
}
static FORCE_INLINE float gba_compute_vol_env_slope(int length_of_step,int dir){
  float step_time = length_of_step/64.0;
  float slope = 1./step_time;
  if(dir==0)slope*=-1;
  if(length_of_step==0)slope=0;
  return slope/16.;
} 
static FORCE_INLINE float gba_polyblep(float t,float dt){
  if(t<=dt){    
    t = t/dt;
    return t+t-t*t-1.0;;
  }else if (t >= 1-dt){
    t=(t-1.0)/dt;
    return t*t+t+t+1.0;
  }else return 0; 
}
static FORCE_INLINE float gba_bandlimited_square(float t, float duty_cycle,float dt){
  float t2 = t - duty_cycle;
  if(t2< 0.0)t2 +=1.0;
  float y = t < duty_cycle ? -1 : 1;
  y -= gba_polyblep(t,dt);
  y += gba_polyblep(t2,dt);
  return y;
}
static FORCE_INLINE void gba_send_interrupt(gba_t*gba,int delay,int if_bit){
  if(if_bit){
    gba->active_if_pipe_stages|=1<<delay;
    gba->pipelined_if[delay]|= if_bit;
  }
}
static FORCE_INLINE void gba_tick_interrupts(gba_t*gba){
  if(SB_UNLIKELY(gba->active_if_pipe_stages)){
    uint16_t if_bit = gba->pipelined_if[0];
    if(if_bit){
      uint16_t if_val = gba_io_read16(gba,GBA_IF);
      if_val |= if_bit;
      gba_io_store16(gba,GBA_IF,if_val);
    }
    gba->pipelined_if[0]=gba->pipelined_if[1];
    gba->pipelined_if[1]=gba->pipelined_if[2];
    gba->pipelined_if[2]=gba->pipelined_if[3];
    gba->pipelined_if[3]=gba->pipelined_if[4];
    gba->pipelined_if[4]=0;
    gba->active_if_pipe_stages>>=1;
  }
}

// Thanks fleroviux!
uint64_t gba_decrypt_arv3(uint64_t code){
  const uint32_t S0 = 0x7AA9648F;
  const uint32_t S1 = 0x7FAE6994;
  const uint32_t S2 = 0xC0EFAAD5;
  const uint32_t S3 = 0x42712C57;

  uint32_t l = code >> 32;
  uint32_t r = code & 0xFFFFFFFF;

  uint32_t tmp = 0x9E3779B9 << 5;

  for (int i = 0; i < 32; i++) {
    r -= ((l << 4) + S2) ^ (l + tmp) ^ ((l >> 5) + S3);
    l -= ((r << 4) + S0) ^ (r + tmp) ^ ((r >> 5) + S1);
    tmp -= 0x9E3779B9;
  }

  return ((uint64_t)l << 32) | r;
}
bool gba_handle_ar_if_instruction(gba_t* gba, uint32_t left, uint32_t right){
  uint32_t address = ((left<<4)&0x0F000000) | (left&0x000FFFFF);
  uint8_t current_code = (left>>24)&0xFF;
  uint32_t left_compare = gba_read32(gba,address);
  uint32_t right_compare = right;
  int32_t left_compare_signed = 0;
  int32_t right_compare_signed = 0;

  switch(current_code&0x6){
    case 0:{
      left_compare &= 0xFF;
      right_compare &= 0xFF;
      left_compare_signed = (int8_t)(left_compare);
      right_compare_signed = (int8_t)(right_compare);
      break;
    }
    case 0x2:{
      left_compare &= 0xFFFF;
      right_compare &= 0xFFFF;
      left_compare_signed = (int16_t)(left_compare);
      right_compare_signed = (int16_t)(right_compare);
      break;
    }
    case 0x4:{
      left_compare_signed = (int32_t)(left_compare);
      right_compare_signed = (int32_t)(right_compare);
      break;
    }
    case 0x6:{
      log_printf("Invalid AR if instruction\n");
      return false;
    }
  }

  switch(current_code&0x38){
    case 0x8:{
      // Equal
      return left_compare==right_compare;
    }
    case 0x10:{
      // Not equal
      return left_compare!=right_compare;
    }
    case 0x18:{
      // Signed <
      return left_compare_signed<right_compare_signed;
    }
    case 0x20:{
      // Signed >
      return left_compare_signed>right_compare_signed;
    }
    case 0x28:{
      // Unsigned <
      return left_compare<right_compare;
    }
    case 0x30:{
      // Unsigned >
      return left_compare>right_compare;
    }
    case 0x38:{
      // Logical AND
      return left_compare&&right_compare;
    }
  }

  return false;
}
bool gba_run_ar_cheat(gba_t* gba, const uint32_t* buffer, uint32_t size){
  if(size%2!=0){
    log_printf("Invalid Action Replay cheat size:%d\n",size);
    return false;
  }

  // stack for if statements
  // the first element is always true
  bool if_stack[32] = {true};
  size_t if_stack_index = 0;

  // Let's treat the AR button as always pressed for now
  bool ar_button_pressed = true;

  for(int i=0;i<size;i+=2){
    uint64_t code=gba_decrypt_arv3(buffer[i+1] | ((uint64_t)buffer[i] << 32));
    uint32_t left=code>>32;
    uint32_t right=code&0xFFFFFFFF;

    if (!if_stack[if_stack_index])
      continue;

    if (right == 0x1DC0DE){
      continue;
    }

    if (left!=0){
      uint8_t current_code = (left>>24)&0xFF;
      uint32_t address = ((left<<4)&0x0F000000) | (left&0x000FFFFF);

      switch(current_code){
        case 0x00:{
          uint32_t offset=right>>8;
          uint8_t data=right&0xFF;
          gba_store8(gba,address+offset,data);
          break;
        }
        case 0x02:{
          uint32_t offset=right>>16;
          uint16_t data=right&0xFFFF;
          gba_store16(gba,address+offset*2,data);
          break;
        }
        case 0x04:{
          uint32_t data=right;
          gba_store32(gba,address,data);
          break;
        }
        case 0x40:{
          uint32_t offset=right>>8;
          uint8_t data=right&0xFF;
          address=gba_read32(gba,address);
          gba_store8(gba,address+offset,data);
          break;
        }
        case 0x42:{
          uint32_t offset=right>>16;
          uint16_t data=right&0xFFFF;
          address=gba_read32(gba,address);
          gba_store16(gba,address+offset*2,data);
          break;
        }
        case 0x44:{
          uint32_t data=right;
          address=gba_read32(gba,address);
          gba_store32(gba,address,data);
          break;
        }
        case 0x80:{
          uint8_t data=right&0xFF;
          uint8_t old_data=gba_read8(gba,address);
          gba_store8(gba,address,old_data+data);
          break;
        }
        case 0x82:{
          uint16_t data=right&0xFFFF;
          uint16_t old_data=gba_read16(gba,address);
          gba_store16(gba,address,old_data+data);
          break;
        }
        case 0x84:{
          uint32_t data=right;
          uint32_t old_data=gba_read32(gba,address);
          gba_store32(gba,address,old_data+data);
          break;
        }
        case 0xC4:{
          continue;
        }
        case 0xC6:{
          uint32_t address=0x4000000|(left&0xFFFFFF);
          uint16_t data=right&0xFFFF;
          gba_store16(gba,address,data);
          break;
        }
        case 0xC7:{
          uint32_t address=0x4000000|(left&0xFFFFFF);
          uint32_t data=right;
          gba_store32(gba,address,data);
          break;
        }
        default:{
          // if instruction
          bool condition = gba_handle_ar_if_instruction(gba,left,right);

          switch(current_code&0xC0){
            case 0x00:{
              if(!condition){
                // skip next instruction
                i+=2;
                continue;
              }
            }
            case 0x40:{
              if (!condition){
                // skip next two instructions
                i+=4;
                continue;
              }
              break;
            }
            case 0x80:{
              break;
            }
            case 0xC0:{
              if (!condition){
                // turn off all codes
                return true;
              }
              break;
            }
          }

          if_stack_index++;

          if (if_stack_index == 32){
            log_printf("Action Replay if stack size exceeded\n");
            return false;
          }

          if_stack[if_stack_index] = condition;
          break;
        }
      }
    }else{
      uint8_t current_code = (right>>24)&0xFF;

      if (right == 0){
        // end of code list
        return true;
      }

      switch(current_code){
        case 0x60:{
          // else
          if_stack[if_stack_index]^=true;
          break;
        }
        case 0x40:{
          // end if
          if (if_stack_index == 0){
            log_printf("Unexpected Action Replay end if instruction\n");
            return false;
          }
          if_stack_index--;
          break;
        }
        case 0x08: {
          // AR slowdown
          // Probably has no effect on emulators
          break;
        }
        case 0x18:
        case 0x1A:
        case 0x1C:
        case 0x1E:{
          if(i+3>=size)return false;
          uint32_t address=0x8000000|((right&0xFFFFFF)<<1);
          uint64_t decrypted=gba_decrypt_arv3(buffer[i+3] | ((uint64_t)buffer[i+2] << 32));
          uint16_t data=decrypted>>32;
          gba->mem.cart_rom[address&0x1FFFFFF]=data;
          gba->mem.cart_rom[(address+1)&0x1FFFFFF]=data>>8;
          i+=2;
          break;
        }
        case 0x10:{
          // IF AR_BUTTON THEN [a0aaaaa]=zz
          if(i+3>=size)return false;
          if (ar_button_pressed) {
            uint64_t decrypted=gba_decrypt_arv3(buffer[i+3] | ((uint64_t)buffer[i+2] << 32));
            uint32_t address=((right<<4)&0x0F000000) | (right&0x000FFFFF);
            uint8_t data = decrypted>>32;
            gba_store8(gba,address,data);
          }
          i+=2;
          break;
        }
        case 0x12:{
          // IF AR_BUTTON THEN [a0aaaaa]=zzzz
          if(i+3>=size)return false;
          if (ar_button_pressed) {
            uint64_t decrypted=gba_decrypt_arv3(buffer[i+3] | ((uint64_t)buffer[i+2] << 32));
            uint32_t address=((right<<4)&0x0F000000) | (right&0x000FFFFF);
            uint16_t data = decrypted>>32;
            gba_store16(gba,address,data);
          }
          i+=2;
          break;
        }
        case 0x14:{
          // IF AR_BUTTON THEN [a0aaaaa]=zzzzzzzz
          if(i+3>=size)return false;
          if (ar_button_pressed) {
            uint64_t decrypted=gba_decrypt_arv3(buffer[i+3] | ((uint64_t)buffer[i+2] << 32));
            uint32_t address=((right<<4)&0x0F000000) | (right&0x000FFFFF);
            uint32_t data = decrypted>>32;
            gba_store32(gba,address,data);
          }
          i+=2;
          break;
        }
        case 0x80:
        case 0x82:
        case 0x84:{
          // 00000000 8naaaaaa 000000yy ssccssss  repeat cc times [a0aaaaa]=yy
          // (with yy=yy+ss, a0aaaaa=a0aaaaa+ssss after each step)
          if(i+3>=size)return false;
          uint64_t decrypted=gba_decrypt_arv3(buffer[i+3] | ((uint64_t)buffer[i+2] << 32));
          uint32_t address=((right<<4)&0x0F000000) | (right&0x000FFFFF);
          uint8_t repeat=(decrypted>>16)&0xFF;
          uint8_t data_increment=(decrypted>>24)&0xFF;
          uint32_t address_increment=decrypted&0xFFFF;
          uint32_t data=decrypted>>32;

          if((current_code&0xF)==0x2){
            address_increment*=2;
          } else if((current_code&0xF)==0x4){
            address_increment*=4;
          }

          for (int j=0;j<repeat;j++){
            if((current_code&0xF)==0x2){
              gba_store16(gba,address,data);
            } else if((current_code&0xF)==0x4){
              gba_store32(gba,address,data);
            } else {
              gba_store8(gba,address,data);
            }
            address+=address_increment;
            data+=data_increment;
          }

          i+=2;
          break;
        }
        default:{
          return false;
        }
      }
    }
  }

  return true;
}

// BEGIN GB REUSE CODE SHIM//
#define sb_compute_next_sweep_freq gba_compute_next_sweep_freq
#define sb_tick_frame_sweep gba_tick_frame_sweep
#define sb_tick_frame_seq gba_tick_frame_seq
#define sb_process_audio_writes gba_process_audio_writes
#define sb_process_audio gba_tick_audio
#define sb_frame_sequencer_t gba_frame_sequencer_t
#define sb_audio_t gba_audio_t
#define sb_gb_t gba_t
#define sb_read8_io  gba_audio_read8
#define sb_store8_io gba_audio_store8
#define sb_bandlimited_square gba_bandlimited_square
#define sb_gbc_enable(a) (true)
#define sb_read_wave_ram gba_read_wave_ram
#define GBA_AUDIO 1

#define SB_IO_AUD1_TONE_SWEEP   0xff10
#define SB_IO_AUD1_LENGTH_DUTY  0xff11
#define SB_IO_AUD1_VOL_ENV      0xff12
#define SB_IO_AUD1_FREQ         0xff13
#define SB_IO_AUD1_FREQ_HI      0xff14

#define SB_IO_AUD2_LENGTH_DUTY  0xff16
#define SB_IO_AUD2_VOL_ENV      0xff17
#define SB_IO_AUD2_FREQ         0xff18
#define SB_IO_AUD2_FREQ_HI      0xff19

#define SB_IO_AUD3_POWER        0xff1A
#define SB_IO_AUD3_LENGTH       0xff1B
#define SB_IO_AUD3_VOL          0xff1C
#define SB_IO_AUD3_FREQ         0xff1D
#define SB_IO_AUD3_FREQ_HI      0xff1E
#define SB_IO_AUD3_WAVE_BASE    0xff30

#define SB_IO_AUD4_LENGTH       0xff20
#define SB_IO_AUD4_VOL_ENV      0xff21
#define SB_IO_AUD4_POLY         0xff22
#define SB_IO_AUD4_COUNTER      0xff23
#define SB_IO_MASTER_VOLUME     0xff24
#define SB_IO_SOUND_OUTPUT_SEL  0xff25

#define SB_IO_SOUND_ON_OFF      0xff26

static int gba_lookup_gb_reg(int gb_reg){
  switch(gb_reg){
    case SB_IO_AUD1_TONE_SWEEP  : return GBA_SOUND1CNT_L; 
    case SB_IO_AUD1_LENGTH_DUTY : return GBA_SOUND1CNT_H; 
    case SB_IO_AUD1_VOL_ENV     : return GBA_SOUND1CNT_H+1; 
    case SB_IO_AUD1_FREQ        : return GBA_SOUND1CNT_X; 
    case SB_IO_AUD1_FREQ_HI     : return GBA_SOUND1CNT_X+1; 
    case SB_IO_AUD2_LENGTH_DUTY : return GBA_SOUND2CNT_L; 
    case SB_IO_AUD2_VOL_ENV     : return GBA_SOUND2CNT_L+1; 
    case SB_IO_AUD2_FREQ        : return GBA_SOUND2CNT_H; 
    case SB_IO_AUD2_FREQ_HI     : return GBA_SOUND2CNT_H+1; 
    case SB_IO_AUD3_POWER       : return GBA_SOUND3CNT_L; 
    case SB_IO_AUD3_LENGTH      : return GBA_SOUND3CNT_H; 
    case SB_IO_AUD3_VOL         : return GBA_SOUND3CNT_H+1; 
    case SB_IO_AUD3_FREQ        : return GBA_SOUND3CNT_X; 
    case SB_IO_AUD3_FREQ_HI     : return GBA_SOUND3CNT_X+1; 
    case SB_IO_AUD4_LENGTH      : return GBA_SOUND4CNT_L; 
    case SB_IO_AUD4_VOL_ENV     : return GBA_SOUND4CNT_L+1; 
    case SB_IO_AUD4_POLY        : return GBA_SOUND4CNT_H; 
    case SB_IO_AUD4_COUNTER     : return GBA_SOUND4CNT_H+1; 
    case SB_IO_MASTER_VOLUME    : return GBA_SOUNDCNT_L; 
    case SB_IO_SOUND_OUTPUT_SEL : return GBA_SOUNDCNT_L+1; 
    case SB_IO_SOUND_ON_OFF     : return GBA_SOUNDCNT_X; 
  }
  log_printf("Unknown GB register:%04x\n",gb_reg);
  return 0; 
}
static int gba_inverse_lookup_gb_reg(int gb_reg){
  switch(gb_reg){
    case GBA_SOUND1CNT_L   : return SB_IO_AUD1_TONE_SWEEP;
    case GBA_SOUND1CNT_H   : return SB_IO_AUD1_LENGTH_DUTY;
    case GBA_SOUND1CNT_H+1 : return SB_IO_AUD1_VOL_ENV;
    case GBA_SOUND1CNT_X   : return SB_IO_AUD1_FREQ;
    case GBA_SOUND1CNT_X+1 : return SB_IO_AUD1_FREQ_HI;
    case GBA_SOUND2CNT_L   : return SB_IO_AUD2_LENGTH_DUTY;
    case GBA_SOUND2CNT_L+1 : return SB_IO_AUD2_VOL_ENV;
    case GBA_SOUND2CNT_H   : return SB_IO_AUD2_FREQ;
    case GBA_SOUND2CNT_H+1 : return SB_IO_AUD2_FREQ_HI;
    case GBA_SOUND3CNT_L   : return SB_IO_AUD3_POWER;
    case GBA_SOUND3CNT_H   : return SB_IO_AUD3_LENGTH;
    case GBA_SOUND3CNT_H+1 : return SB_IO_AUD3_VOL;
    case GBA_SOUND3CNT_X   : return SB_IO_AUD3_FREQ;
    case GBA_SOUND3CNT_X+1 : return SB_IO_AUD3_FREQ_HI;
    case GBA_SOUND4CNT_L   : return SB_IO_AUD4_LENGTH;
    case GBA_SOUND4CNT_L+1 : return SB_IO_AUD4_VOL_ENV;
    case GBA_SOUND4CNT_H   : return SB_IO_AUD4_POLY;
    case GBA_SOUND4CNT_H+1 : return SB_IO_AUD4_COUNTER;
    case GBA_SOUNDCNT_L    : return SB_IO_MASTER_VOLUME;
    case GBA_SOUNDCNT_L+1  : return SB_IO_SOUND_OUTPUT_SEL;
    case GBA_SOUNDCNT_X    : return SB_IO_SOUND_ON_OFF;
  }
  return 0; 
}
static uint8_t gba_audio_read8(gba_t* gba, int addr){
  return gba_io_read8(gba,gba_lookup_gb_reg(addr));
}

static void gba_audio_store8(gba_t* gba, int addr, uint8_t data){
  addr = gba_lookup_gb_reg(addr);
  if(addr)gba_io_store8(gba,addr,data);
}
static uint8_t gba_audio_process_byte_write(gba_t *gba, uint32_t addr, uint8_t value){
  gba_t * gb = gba;
  sb_frame_sequencer_t *seq = &gba->audio.sequencer;
  addr = gba_inverse_lookup_gb_reg(addr);
  int i = (addr-SB_IO_AUD1_LENGTH_DUTY)/5;
  if(!addr)return value; 
  if(addr==SB_IO_SOUND_ON_OFF){
    value&=0xf0;
    value|=sb_read8_io(gb,SB_IO_SOUND_ON_OFF)&0xf;
  }
  if(addr>=SB_IO_AUD3_WAVE_BASE&&addr<SB_IO_AUD3_WAVE_BASE+16){
    bool wave_active = SB_BFE(sb_read8_io(gb,SB_IO_SOUND_ON_OFF),2,1);
    if(wave_active){
      //Addr locked to the read pointer when the wave channel is active
      addr= SB_IO_AUD3_WAVE_BASE+((gb->audio.wave_sample_offset)%32)/2;
    }
  }
  if(addr==SB_IO_AUD1_LENGTH_DUTY||addr==SB_IO_AUD2_LENGTH_DUTY||addr==SB_IO_AUD3_LENGTH||addr==SB_IO_AUD4_LENGTH){
    uint8_t length_duty = value;
    if(i==2) seq->length[i] = 256-SB_BFE(length_duty,0,8);
    else seq->length[i] = 64-SB_BFE(length_duty,0,6);
  }else if(addr==SB_IO_AUD1_VOL_ENV||addr==SB_IO_AUD2_VOL_ENV||addr==SB_IO_AUD4_VOL_ENV){
    bool power = SB_BFE(value,3,5)!=0;
    seq->powered[i]= power;
    seq->active[i]&=power;
    seq->env_direction[i] = (SB_BFE(value,3,1)?1:-1);
    seq->env_period[i] = SB_BFE(value,0,3);
    if (seq->env_period[i]== 0 &&!seq->env_overflow[i]) {
      seq->volume[i] = (seq->volume[i] + 1) & 0xf;
    }
  }else if( addr==SB_IO_AUD1_FREQ||addr==SB_IO_AUD1_FREQ_HI||
            addr==SB_IO_AUD2_FREQ||addr==SB_IO_AUD2_FREQ_HI||
            addr==SB_IO_AUD3_FREQ||addr==SB_IO_AUD3_FREQ_HI
          ){
    sb_store8_io(gb,addr,value);
    uint8_t freq_lo = sb_read8_io(gb,SB_IO_AUD1_FREQ+i*5);
    uint8_t freq_hi = sb_read8_io(gb,SB_IO_AUD1_FREQ_HI+i*5);
    seq->frequency[i] = freq_lo | ((int)(SB_BFE(freq_hi,0,3))<<8u);
  }
  return value;
}
static uint8_t sb_read_wave_ram(sb_gb_t*gb,int byte){
  return gba_io_read8(gb,GBA_WAVE_RAM+byte);
}
static int sb_compute_next_sweep_freq(sb_frame_sequencer_t*seq){
  int shift = seq->sweep_shift?seq->sweep_shift:8;
  int32_t increment = (seq->frequency[0] >> shift)*seq->sweep_direction;
  int32_t new_frequency = seq->frequency[0]+increment;
  seq->sweep_subtracted|=seq->sweep_direction==-1;
  return new_frequency;
}
static void sb_tick_frame_sweep(sb_frame_sequencer_t*seq){
  int32_t new_frequency = sb_compute_next_sweep_freq(seq);
  if(new_frequency>2047){
    seq->active[0]=false; 
    new_frequency = 2047; 
  }else if(new_frequency<0)new_frequency=0;
  if(seq->sweep_shift){
    seq->frequency[0]= new_frequency;
    new_frequency = sb_compute_next_sweep_freq(seq);
    if(new_frequency>2047){
      seq->active[0]=false; 
      new_frequency = 2047; 
    }
  }
}
static void sb_tick_frame_seq(sb_gb_t*gb,sb_frame_sequencer_t* seq){
  int step = (seq->step_counter++)%8;
  //Tick sweep
  if(step==2||step==6){
    if(seq->active[0]&&seq->sweep_enable){
      if(seq->sweep_timer>0)seq->sweep_timer--;
      if(seq->sweep_timer == 0){
        if(seq->sweep_period > 0){
          seq->sweep_timer = seq->sweep_period;
          sb_tick_frame_sweep(seq);
        }else seq->sweep_timer = 8;
      }
    }
  }
  //Tick envelope
  if(step==7){
    for(int i=0;i<4;++i){
      if(i==2)continue;
      if(seq->env_period[i]){
        if(seq->env_period_timer[i]>0)seq->env_period_timer[i]--;
        if(seq->env_period_timer[i]==0){
          seq->env_period_timer[i]=seq->env_period[i];
          int volume = seq->volume[i];
          volume+=seq->env_direction[i];
          if(volume<=0){volume=0;seq->env_overflow[i]=true;}
          if(volume>0xF){volume=0xF;seq->env_overflow[i]=true;};
          seq->volume[i]=volume;
        }
      }
    }
  }
  if((step%2)==0){
    //Tick length
    for(int i=0;i<4;++i){
      if(!seq->use_length[i])continue;
      if(seq->length[i]>0)seq->length[i]--;
      if(seq->length[i]==0){
        seq->active[i]=false; 
        seq->length[i] = i==2?256:64; 
        seq->use_length[i] = false;
      }
    }
  }
  int nrf_52 = sb_read8_io(gb,SB_IO_SOUND_ON_OFF)&0xf0;
  for(int i=0;i<4;++i){
    seq->active[i]&=seq->powered[i];
    bool active = seq->active[i];
    nrf_52|=active<<i;
  }
  sb_store8_io(gb,SB_IO_SOUND_ON_OFF,nrf_52);
}
static void sb_process_audio_writes(sb_gb_t* gb){
  sb_audio_t* audio = &gb->audio;
  sb_frame_sequencer_t* seq = &audio->sequencer;
  int nrf_52 = sb_read8_io(gb,SB_IO_SOUND_ON_OFF)&0xf0;
  bool master_enable = SB_BFE(nrf_52,7,1);
  if(!master_enable){
    for(int i=SB_IO_AUD1_TONE_SWEEP;i<SB_IO_SOUND_ON_OFF;++i){
      sb_store8_io(gb,i,0);
    }
    for(int i=0;i<4;++i){
      if(sb_gbc_enable(gb)||i!=3){
        seq->active[i]=false;
        seq->powered[i]=false;
        seq->length[i]=0;
      }
      seq->use_length[i]=false;
    }
    
  }else{
    uint8_t freq_sweep1 = sb_read8_io(gb, SB_IO_AUD1_TONE_SWEEP);
    seq->sweep_period=SB_BFE(freq_sweep1, 4, 3);
    seq->sweep_shift=SB_BFE(freq_sweep1, 0, 3);
    seq->sweep_direction= SB_BFE(freq_sweep1, 3,1)? -1. : 1; 
    for(int i=0;i<4;++i){
      bool prev_length_en = seq->use_length[i];
      uint8_t freq_hi = sb_read8_io(gb,SB_IO_AUD1_FREQ_HI+i*5);
      seq->use_length[i]= SB_BFE(freq_hi,6,1);
      uint8_t vol_env = sb_read8_io(gb,SB_IO_AUD1_VOL_ENV+i*5);
      if(i==2){
        bool power = SB_BFE(sb_read8_io(gb,SB_IO_AUD3_POWER),7,1);
        seq->powered[i]=power;
      }
      if(i!=0){
        uint8_t freq_lo = sb_read8_io(gb,SB_IO_AUD1_FREQ+i*5);
        seq->frequency[i] = freq_lo | ((int)(SB_BFE(freq_hi,0,3))<<8u);
      }
      if(i==2){
        seq->env_direction[i] = 0;
        seq->env_period[i] = 0;
      }else{
        seq->env_direction[i] = (SB_BFE(vol_env,3,1)?1:-1);
        seq->env_period[i] = SB_BFE(vol_env,0,3);
      }
      bool triggered = SB_BFE(freq_hi,7,1);
      if(triggered){
        uint8_t length_duty = sb_read8_io(gb, SB_IO_AUD1_LENGTH_DUTY+i*5);
        uint8_t freq_lo = sb_read8_io(gb,SB_IO_AUD1_FREQ+i*5);
        seq->frequency[i] = freq_lo | ((int)(SB_BFE(freq_hi,0,3))<<8u);
        seq->volume[i] = SB_BFE(vol_env,4,4);
       
        if(seq->length[i]==0)seq->length[i]=i==2?256:64;
        if(i==3)seq->lfsr4 = 0x7FFF;
        if(i==2){
          audio->wave_sample_offset=31;
          audio->wave_freq_timer=4;
        }
        seq->env_period_timer[i]=0;
        seq->env_overflow[i]=false;
        seq->chan_t[i]=0;
        seq->active[i]=true;
        if(i==0){
          seq->sweep_subtracted=false;
          seq->sweep_enable = seq->sweep_period||seq->sweep_shift;
          seq->sweep_timer=seq->sweep_period;
          if(seq->sweep_timer==0)seq->sweep_timer=8; 
          if (seq->sweep_shift && sb_compute_next_sweep_freq(seq) > 2047) {
            seq->active[0] = false;
          } 
          seq->sweep_enable = seq->sweep_period>0||seq->sweep_shift>0;
        }
      }
      if(i==0&&seq->sweep_subtracted&&seq->sweep_direction!=-1){
        seq->active[0]= false; 
        seq->sweep_enable = false;
      }
      if(seq->use_length[i]&&!prev_length_en){
        bool second_half_of_length_period = (seq->step_counter&1);
        if(second_half_of_length_period){
          if(seq->length[i])seq->length[i]--;
          if(seq->length[i]==0){
            if(triggered) seq->length[i]=i==2?255:63;
            else{
              seq->active[i]=false;
              seq->use_length[i]=triggered&&seq->use_length[i];
            }
          }
        }
      }
      sb_store8_io(gb, SB_IO_AUD1_FREQ_HI+i*5,freq_hi&0x7f);
    }
  }
  nrf_52 = sb_read8_io(gb,SB_IO_SOUND_ON_OFF)&0xf0;
  for(int i=0;i<4;++i){
    seq->active[i]&=seq->powered[i];
    bool active = seq->active[i];
    nrf_52|=active<<i;
  }
  sb_store8_io(gb,SB_IO_SOUND_ON_OFF,nrf_52);
}
static FORCE_INLINE void sb_process_audio(sb_gb_t *gb, sb_emu_state_t*emu, double delta_time, int cycles){

  sb_audio_t* audio = &gb->audio;
  sb_frame_sequencer_t* seq = &audio->sequencer;

  if(delta_time>1.0/60.)delta_time = 1.0/60.;
  audio->current_sim_time +=delta_time;
  #ifdef GBA_AUDIO
    uint32_t prev_audio_clock = audio->audio_clock;
    audio->audio_clock+=cycles;
    cycles = (audio->audio_clock-(prev_audio_clock&~3))/4;
    uint32_t frame_cycles = (audio->audio_clock-(prev_audio_clock&~32767))/32768;
    while(frame_cycles--)gba_tick_frame_seq(gb,seq);
  #endif

  int freq_tim = audio->wave_freq_timer;
  freq_tim-=cycles;
  if(freq_tim<0){
    int wave_inc_count = (-freq_tim-1)/((2048-seq->frequency[2])*2)+1;
    audio->wave_sample_offset+= wave_inc_count;
    freq_tim +=(2048-seq->frequency[2])*2*wave_inc_count;
    unsigned wav_samp = (audio->wave_sample_offset)%32;
    int dat =sb_read_wave_ram(gb,wav_samp/2);
    audio->curr_wave_data = dat;
    int offset = (wav_samp&1)? 0:4;
    audio->curr_wave_sample = ((dat>>offset)&0xf);
  }
  audio->wave_freq_timer=freq_tim;

  audio->current_sample_generated_time -= (int)(audio->current_sim_time);
  audio->current_sim_time -= (int)(audio->current_sim_time);

  if(audio->current_sample_generated_time >audio->current_sim_time)return; 

  int nrf_52 = sb_read8_io(gb,SB_IO_SOUND_ON_OFF)&0xf0;

  bool master_enable = SB_BFE(nrf_52,7,1);
  if(!master_enable)return;
  float sample_delta_t = 1.0/SE_AUDIO_SAMPLE_RATE;

  const static float duty_lookup[]={0.125,0.25,0.5,0.75};
  uint8_t length_duty1 = sb_read8_io(gb, SB_IO_AUD1_LENGTH_DUTY);
  float duty1 = duty_lookup[SB_BFE(length_duty1,6,2)];
  uint8_t length_duty2 = sb_read8_io(gb, SB_IO_AUD2_LENGTH_DUTY);
  float duty2 = duty_lookup[SB_BFE(length_duty2,6,2)];

  uint8_t power3 = sb_read8_io(gb,SB_IO_AUD3_POWER);
  uint8_t vol_env3 = sb_read8_io(gb,SB_IO_AUD3_VOL);
  int channel3_shift = SB_BFE(vol_env3,5,2)-1;
  if(SB_BFE(power3,7,1)==0||channel3_shift==-1)channel3_shift=4;

  uint8_t poly4 = sb_read8_io(gb,SB_IO_AUD4_POLY);
  float r4 = SB_BFE(poly4,0,3);
  uint8_t s4 = SB_BFE(poly4,4,4);
  bool sevenBit4 = SB_BFE(poly4,3,1);
  if(r4==0)r4=0.5;

  uint8_t master_vol = sb_read8_io(gb,SB_IO_MASTER_VOLUME);
  float master_left = SB_BFE(master_vol,4,3)/7.;
  float master_right = SB_BFE(master_vol,0,3)/7.;

  uint8_t chan_sel = sb_read8_io(gb,SB_IO_SOUND_OUTPUT_SEL);
  //These are type int to allow them to be multiplied to enable/disable
  float chan_l[6]={0};float chan_r[6]={0};
  for(int i=0;i<4;++i){
    chan_l[i] = SB_BFE(chan_sel,i,1);
    chan_r[i] = SB_BFE(chan_sel,i+4,1);
  }

  #ifdef GBA_AUDIO
  {
    uint16_t soundcnt_h = gba_io_read16(gb,GBA_SOUNDCNT_H);
    //These are type int to allow them to be multiplied to enable/disable
    uint16_t chan_sel = gba_io_read16(gb,GBA_SOUNDCNT_L);
    /* soundcnth 
    0-1   R/W  Sound # 1-4 Volume   (0=25%, 1=50%, 2=100%, 3=Prohibited)
    2     R/W  DMA Sound A Volume   (0=50%, 1=100%)
    3     R/W  DMA Sound B Volume   (0=50%, 1=100%)
    4-7   -    Not used
    8     R/W  DMA Sound A Enable RIGHT (0=Disable, 1=Enable)
    9     R/W  DMA Sound A Enable LEFT  (0=Disable, 1=Enable)
    10    R/W  DMA Sound A Timer Select (0=Timer 0, 1=Timer 1)
    11    W?   DMA Sound A Reset FIFO   (1=Reset)
    12    R/W  DMA Sound B Enable RIGHT (0=Disable, 1=Enable)
    13    R/W  DMA Sound B Enable LEFT  (0=Disable, 1=Enable)
    14    R/W  DMA Sound B Timer Select (0=Timer 0, 1=Timer 1)
    15    W?   DMA Sound B Reset FIFO   (1=Reset)*/ 
    float psg_volume_lookup[4]={0.25,0.5,1.0,0.};
    float psg_volume = psg_volume_lookup[SB_BFE(soundcnt_h,0,2)]*0.25;

    float r_vol = SB_BFE(chan_sel,0,3)/7.*psg_volume;
    float l_vol = SB_BFE(chan_sel,4,3)/7.*psg_volume;
    for(int i=0;i<4;++i){
      chan_r[i] *= r_vol;
      chan_l[i] *= l_vol;
    }
    // Channel volume for each FIFO
    for(int i=0;i<2;++i){
      // Volume
      chan_r[i+4]=chan_l[i+4]= SB_BFE(soundcnt_h,2+i,1)? 1.0: 0.5;
      chan_r[i+4]*= SB_BFE(soundcnt_h,8+i*4,1);
      chan_l[i+4]*= SB_BFE(soundcnt_h,9+i*4,1);
    }
    gba_io_store16(gb,GBA_SOUNDCNT_H,soundcnt_h&~((1<<11)|(1<<15)));
    master_left=master_right=1;
  }
  #endif

  float freq_hz[4];
  for(int i=0;i<2;++i){freq_hz[i]= 131072./(2048-seq->frequency[i]);}
  freq_hz[2]= (65536.)/(2048-seq->frequency[2]);
  freq_hz[3] = 524288.0/r4/pow(2.0,s4+1);
  while(audio->current_sample_generated_time < audio->current_sim_time){

    audio->current_sample_generated_time+=sample_delta_t;
    
    if((sb_ring_buffer_size(&emu->audio_ring_buff)+3>SB_AUDIO_RING_BUFFER_SIZE)) continue;

    //Advance each channel    
    for(int i=0;i<4;++i)seq->chan_t[i]  +=sample_delta_t*freq_hz[i];
    //Generate new noise value if needed
    if(seq->chan_t[3]>=1.0) {
      int bit = (seq->lfsr4 ^ (seq->lfsr4 >> 1)) & 1;
      seq->lfsr4 >>= 1;
      seq->lfsr4 |= bit << 14;
      if (sevenBit4) {
        seq->lfsr4 &= ~(1 << 7);
        seq->lfsr4 |= bit << 6;
      }
    }
    
    //Loopback
    for(int i=0;i<4;++i) seq->chan_t[i]-=(int)seq->chan_t[i];
    
    //Compute and clamp Volume Envelopes
    float v[4];
    for(int i=0;i<4;++i)v[i] = seq->active[i]?seq->volume[i]/15.:0;
    v[2] = 1.0;

    int dat = audio->curr_wave_sample >>channel3_shift;
    int wav_offset = 8>>channel3_shift; 
    
    float channels[6];
    channels[0] = sb_bandlimited_square(seq->chan_t[0],duty1,sample_delta_t*freq_hz[0])*v[0];
    channels[1] = sb_bandlimited_square(seq->chan_t[1],duty2,sample_delta_t*freq_hz[1])*v[1];
    channels[2] = (dat-wav_offset)/8.;
    channels[3] = ((seq->lfsr4 & 1) * 2.-1.)*v[3];

    #ifdef GBA_AUDIO
      for(int i=0;i<2;++i)channels[4+i] = audio->fifo[i].data[audio->fifo[i].read_ptr&0x1f]/128.;
    #else
      for(int i=0;i<2;++i)channels[4+i] =0;
    #endif 

    //Mix channels
    float sample_volume_l = 0;
    float sample_volume_r = 0;
    for(int i=0;i<6;++i){
      float l = channels[i]*chan_l[i];
      float r = channels[i]*chan_r[i];
      if(l>=-2.&&l<=2)sample_volume_l+=l;
      if(r>=-2.&&r<=2)sample_volume_r+=r;
    }
    
    sample_volume_l*=0.25;
    sample_volume_r*=0.25;
    sample_volume_l*=master_left;
    sample_volume_r*=master_right;

    const float lowpass_coef = 0.999;
    emu->mix_l_volume = emu->mix_l_volume*lowpass_coef + fabs(sample_volume_l)*(1.0-lowpass_coef);
    emu->mix_r_volume = emu->mix_r_volume*lowpass_coef + fabs(sample_volume_r)*(1.0-lowpass_coef); 
    
    for(int i=0;i<6;++i){
      emu->audio_channel_output[i] = emu->audio_channel_output[i]*lowpass_coef 
                                  + fabs(channels[i])*(1.0-lowpass_coef); 
    }
    // Clipping
    if(sample_volume_l>1.0)sample_volume_l=1;
    if(sample_volume_r>1.0)sample_volume_r=1;
    if(sample_volume_l<-1.0)sample_volume_l=-1;
    if(sample_volume_r<-1.0)sample_volume_r=-1;
    if(!(audio->capacitor_l<2&&audio->capacitor_l>-2))audio->capacitor_l=0;
    if(!(audio->capacitor_r<2&&audio->capacitor_r>-2))audio->capacitor_r=0;
    float out_l = sample_volume_l-audio->capacitor_l;
    float out_r = sample_volume_r-audio->capacitor_r;
    audio->capacitor_l = (sample_volume_l-out_l)*0.996;
    audio->capacitor_r = (sample_volume_r-out_r)*0.996;
    // Quantization
    unsigned write_entry0 = (emu->audio_ring_buff.write_ptr++)%SB_AUDIO_RING_BUFFER_SIZE;
    unsigned write_entry1 = (emu->audio_ring_buff.write_ptr++)%SB_AUDIO_RING_BUFFER_SIZE;

    emu->audio_ring_buff.data[write_entry0] = out_l*32760;
    emu->audio_ring_buff.data[write_entry1] = out_r*32760;
  }
}

#undef sb_compute_next_sweep_freq
#undef sb_tick_frame_sweep
#undef sb_tick_frame_seq
#undef sb_process_audio_writes
#undef sb_process_audio
#undef sb_frame_sequencer_t
#undef sb_audio_t
#undef sb_gb_t
#undef sb_read8_io 
#undef sb_store8_io
#undef sb_bandlimited_square
#undef sb_gbc_enable
#undef sb_read_wave_ram



#undef SB_IO_AUD1_TONE_SWEEP   
#undef SB_IO_AUD1_LENGTH_DUTY  
#undef SB_IO_AUD1_VOL_ENV      
#undef SB_IO_AUD1_FREQ         
#undef SB_IO_AUD1_FREQ_HI      
#undef SB_IO_AUD2_LENGTH_DUTY  
#undef SB_IO_AUD2_VOL_ENV      
#undef SB_IO_AUD2_FREQ         
#undef SB_IO_AUD2_FREQ_HI      
#undef SB_IO_AUD3_POWER        
#undef SB_IO_AUD3_LENGTH       
#undef SB_IO_AUD3_VOL          
#undef SB_IO_AUD3_FREQ         
#undef SB_IO_AUD3_FREQ_HI      
#undef SB_IO_AUD3_WAVE_BASE    
#undef SB_IO_AUD4_LENGTH       
#undef SB_IO_AUD4_VOL_ENV      
#undef SB_IO_AUD4_POLY         
#undef SB_IO_AUD4_COUNTER      
#undef SB_IO_MASTER_VOLUME     
#undef SB_IO_SOUND_OUTPUT_SEL  
#undef SB_IO_SOUND_ON_OFF     

#undef GBA_AUDIO 

// END GB REUSE CODE SHIM//

void gba_cpu_trigger_breakpoint(void* data){
  gba_t*gba =(gba_t*)data;
  gba->frame_in_progress=false;
  gba->pause_after_frame=true;
}

void gba_ptrs_init(gba_t* gba,gba_scratch_t *scratch, uint8_t* rom_data) {

  gba->framebuffer = scratch->framebuffer;
  gba->mem.bios    = scratch->bios;
  gba->mem.cart_rom = rom_data;
  gba->cpu.log_cmp_file = scratch->log_cmp_file;
  gba->cpu.read8 = arm7_read8;
  gba->cpu.read16 = arm7_read16;
  gba->cpu.read32 = arm7_read32;
  gba->cpu.read16_seq = arm7_read16_seq;
  gba->cpu.read32_seq = arm7_read32_seq;
  gba->cpu.write8 = arm7_write8;
  gba->cpu.write16 = arm7_write16;
  gba->cpu.write32 = arm7_write32;
  gba->cpu.user_data=gba;  
}

void gba_tick(sb_emu_state_t* emu, gba_t* gba,gba_scratch_t *scratch){
  gba_ptrs_init(gba, scratch, emu->rom_data);
  gba->cpu.user_data=gba;
  gba->cpu.trigger_breakpoint=gba_cpu_trigger_breakpoint;

  uint64_t* d = (uint64_t*)gba->mem.mmio_debug_access_buffer;
  for(int i=0;i<sizeof(gba->mem.mmio_debug_access_buffer)/8;++i){
    d[i]&=0x9191919191919191ULL;
  }

  gba_tick_keypad(&emu->joy,gba);
  gba->frame_in_progress=true;
  float solar_value = emu->joy.solar_sensor;
  if(!(solar_value <1.00))solar_value=1.00;
  if(!(solar_value >0.00))solar_value=0.00;
  gba->solar_sensor.value = 0xE7-solar_value*(0xE7-0x32);
  gba->ppu.ghosting_strength = emu->screen_ghosting_strength;
  while(gba->frame_in_progress){
    int ticks = gba->activate_dmas? gba_tick_dma(gba,gba->last_cpu_tick) :0;
    if(!ticks&&gba->residual_dma_ticks){ticks=gba->residual_dma_ticks;gba->residual_dma_ticks=0;}
    if(!ticks){
      gba->cpu.i_cycles=0;
      gba->mem.requests=0;
      if(!gba->cpu.phased_op_id){
        uint16_t int_if = gba_io_read16(gba,GBA_IF);
        if(SB_UNLIKELY(int_if)){
          int_if &= gba_io_read16(gba,GBA_IE);
          uint32_t ime = gba_io_read32(gba,GBA_IME);
          int_if *= SB_BFE(ime,0,1);
          if(int_if)arm7_process_interrupts(&gba->cpu);
        }
      }
      arm7_exec_instruction(&gba->cpu);
      gba->last_cpu_tick=ticks = gba->mem.requests+gba->cpu.i_cycles; 
    }
    gba_tick_sio(gba);
    int ppu_fast_forward = gba->ppu.fast_forward_ticks;
    int timer_fast_forward = gba->timer_ticks_before_event-gba->deferred_timer_ticks;
    int fast_forward_ticks=ppu_fast_forward<timer_fast_forward?ppu_fast_forward:timer_fast_forward; 
    if(fast_forward_ticks>ticks){
      if(gba->cpu.wait_for_interrupt)ticks=fast_forward_ticks;
      else fast_forward_ticks=ticks;
    }
    if(SB_UNLIKELY(gba->active_if_pipe_stages)){
      for(int i=0;i<fast_forward_ticks;++i)gba_tick_interrupts(gba);
    }
    gba->rtc.total_clocks_ticked+=fast_forward_ticks;
    gba->deferred_timer_ticks+=fast_forward_ticks;
    gba->ppu.fast_forward_ticks-=fast_forward_ticks;
    ticks -=fast_forward_ticks>ticks?ticks:fast_forward_ticks;
    double delta_t = ((double)ticks+fast_forward_ticks)/(16*1024*1024);
    gba_tick_audio(gba, emu,delta_t,ticks+fast_forward_ticks);
  
    bool last_activate_dmas =gba->activate_dmas;
    gba->rtc.total_clocks_ticked+=ticks;
    for(int t = 0;t<ticks;++t){
      if(gba->activate_dmas&&!last_activate_dmas){gba->residual_dma_ticks=ticks-t-1;gba->last_cpu_tick=t+1;}
      gba_tick_interrupts(gba);
      gba_tick_timers(gba);
      gba_tick_ppu(gba,emu->render_frame);
    }
  } 
  emu->joy.rumble = SB_BFE(gba->cart.gpio_data,3,1); 
  //LCD turns off in stop mode
  if(gba->stop_mode)memset(scratch->framebuffer,0,sizeof(scratch->framebuffer));
  if(gba->pause_after_frame){
    emu->run_mode=SB_MODE_PAUSE;
    gba->pause_after_frame=false;
  }       
}

#endif
