#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "gb/timer.h"
#include "gb/rom.h"
#include "gb/mem.h"
#include "gb/cpu.h"
#include "gb/lcd.h"
#include "gb/sdl.h"

#include "ui_file_selector.h"
#include "ui_dialog.h"
#include "framebuf.h"
#include "screen.h"
#include "llapi.h"

void emu_loop(void);
//==============================


void app_run_gb(void)
{
    char *select_path = ui_file_select_malloc("Select .GB File", "/", true, false);
    printf("Start App..\r\n"); 

    printf("sel:%S\r\n", select_path);
    
    int r = 0;
    r = rom_load(select_path);
    if(!r)
        ui_dialog_alert("Error", "Failed to load ROM.");

    r = lcd_init();
    if(r)
        ui_dialog_alert("Error", "Failed to init screen.");
    gfb_clear(get_frame_buffer(), COLOR_SET);
    screen_flush();

    printf("LCD OK!\n");

    mem_init();
    printf("Mem OK!\n");

    cpu_init();
    printf("CPU OK!\n");
    emu_loop();
    
    llapi_delay_ms(200);
    while (1)
    { 
        llapi_delay_ms(1000); 
    }

}

