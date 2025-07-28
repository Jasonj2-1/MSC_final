//  *
 /*
 *  Created on: 14 June 2025
 *      Author: Jiajun Xu
 * Description: The code is finished independently with some references from LEEDS API

*///

#include "Util/watchdog.h"
#include "Util/macros.h"
#include "Util/delay.h"
#include "stdio.h"
#include "DE1SoC_Addresses/DE1SoC_Addresses.h"
#include "DE1SoC_WM8731/DE1SoC_WM8731.h"
#include "HPS_GPIO/HPS_GPIO.h"
#include "HPS_I2C/HPS_I2C.h"
#include "FPGA_PIO/FPGA_PIO.h"
#include "HPS_PrivateTimer.h"
#include "DE1SoC_LT24/DE1SoC_LT24.h"
#include "LCD_display.h"
#include "DE1SoC_LT24/DE1SoC_LT24.h"
#include "BasicFont/BasicFont.h"
// file
#include "FatFS/ff.h"
#include "FatFS/diskio.h"
//Include Floating Point Math Libraries
#include <math.h>
//Debugging Function (same as last lab)
#include <stdlib.h>

void exitOnFail(HpsErr_t status){
    if (ERR_IS_ERROR(status)) {
        exit((int)status); //Add breakpoint here to catch failure
    }
}

// File
//#define MAX_FRAMES 9584640
#define BUFFER_SIZE 512
#define M_PI     3.14159265358979323846
#define LCD_WIDTH  240
#define LCD_HEIGHT 320
#define Period1 2250000 * 3
#define Period2 2250000
// 224999999 = 1s
// 225000 = 1ms
// 2250000 = 10ms

	  FATFS fs;
	  FATFS fs_second;
	  FIL music_file;
	  FIL second_file;
	  FRESULT res;  // mount result
	  BYTE buffer[BUFFER_SIZE];
	  UINT bytesRead;
	  UINT bytesRead_second;


//	  __attribute__((section(".sdram_data")))

	    // Red LEDs base address
	    volatile unsigned int *LEDR_ptr = (unsigned int *)0xFF200000;
	    // KEY buttons base address
	    volatile unsigned int *KEY_ptr  = (unsigned int *)0xFF200050;
	    // Peripheral base addresses.
	    volatile unsigned int *key_ptr = (unsigned int *)LSC_BASE_KEYS;
	    // Switch base address
	    volatile unsigned int *SW_ptr = (unsigned int *) 0xFF200040;

	    // Store the state of the keys last time we checked.
	    // This allows us to determine when a key is pressed, then released.
	    unsigned int keys_pressed = 0;
	    unsigned int key_last_state = 0;

	    bool music_enable = 0;
	    bool music_Ready = 0;


unsigned int getPressedKeys() {
	      // Store the current state of the keys.
	      unsigned int key_current_state = *key_ptr;

	      // If the key was down last cycle, and is up now, mark as pressed.
	      unsigned int keys_pressed = (~key_current_state) & (key_last_state);

	      // Save the key state for next time, so we can compare the next state to this.
	      key_last_state = key_current_state;

	      // Return result.
	      return keys_pressed;
}
//
// Main Function
// =============
int main(void) {

    //Drivers
    FPGAPIOCtx_t* leds;
    WM8731Ctx_t*  audio;
    HPSGPIOCtx_t* gpio;
    HPSI2CCtx_t*  hpsi2c;
    FPGAPIOCtx_t* gpio1;
    LT24Ctx_t* lt24;

    /* Pointers to peripherals - #defines in DE1SoC_Addresses.h*/
    // Red LEDs base address
    volatile unsigned int *LEDR_ptr
        = (unsigned int *) LSC_BASE_RED_LEDS;
    // ARM A9 Private Timer base address
    volatile unsigned int *private_timer_base = LSC_BASE_PRIV_TIM;
    // ARM A9 Private Timer Load
    volatile unsigned int *private_timer_load
           = private_timer_base + (0x00/sizeof(unsigned int));
    // ARM A9 Private Timer Value
    volatile unsigned int *private_timer_value
           = private_timer_base + (0x04/sizeof(unsigned int));
    // ARM A9 Private Timer Control
    volatile unsigned int *private_timer_control
           = private_timer_base + (0x08/sizeof(unsigned int));
    // ARM A9 Private Timer Interrupt
    volatile unsigned int *private_timer_interrupt
           = private_timer_base + (0x0C/sizeof(unsigned int));


    *private_timer_load = 0xFFFFFFFF;
    // Set the "Prescaler" value to 0, Enable the timer (E = 1), Set Automatic reload
    // on overflow (A = 1), and disable ISR (I = 0)
    *private_timer_control   = (0 << 8) | (0 << 2) | (1 << 1) | (1 << 0);

    uint32_t lastTimerValue1 = *private_timer_value;
    uint32_t lastTimerValue2 = *private_timer_value;



    //Initialise the GPIO1 port
    exitOnFail(FPGA_PIO_initialise(LSC_BASE_GPIO_JP1, LSC_CONFIG_GPIO, &gpio1));
    //Initialise the LCD Display.
    exitOnFail(LT24_initialise(&gpio1->gpio, LSC_BASE_LT24HWDATA, &lt24));

    //Initialise Drivers

    exitOnFail(HPS_GPIO_initialise(LSC_BASE_ARM_GPIO, ARM_GPIO_DIR, ARM_GPIO_I2C_GENERAL_MUX, 0, &gpio));
    exitOnFail(HPS_I2C_initialise(LSC_BASE_I2C_GENERAL, I2C_SPEED_STANDARD, &hpsi2c));
    exitOnFail(WM8731_initialise(LSC_BASE_AUDIOCODEC, &hpsi2c->i2c, &audio));

    //Clear both FIFOs
    WM8731_clearFIFO(audio, true,true);
    //Initialise Phase Accumulator
    // SampleRate = 48khz
    unsigned int sampleRate;
    WM8731_getSampleRate(audio, &sampleRate);

    res = f_mount(&fs, "0:", 1);
    exitOnFail(res);
    res = f_open(&music_file, "0:/split.wav", FA_READ);
    exitOnFail(res);

    // Parameters
    uint32_t frame = 0;
    uint32_t space;

    // Music player
    // Song 1
    bool music_enable = 1;     // The music is playing
    bool music_ready = 0;
    bool music_reset = 1;
    unsigned int reset_time = 1;
    // Song 2
    bool music_enable2 = 0;
    bool music_reset2 = 1;

    // Volume and Sound bar
    bool soundbar_initialize = 1;
    bool soundbar_increase_flag = 1;
    bool soundbar_decrease_flag = 1;
    bool music_playing_flag1 = 1;
    bool music_playing_flag2 = 1;
    bool Progress_bar_Frame_flag = 1;
    bool Second_song_Load_enable = 0;
    bool Second_Progress_bar_Frame_flag = 1;
    bool Fast_Write = 0;
    bool total_reset_second = 1;
    bool Music2_Ready_word = 1;
    bool Music2_Clearall = 1;
    bool Progressbar2_enable = 1;
    bool LoadingMusic1_flag = 1;
    bool music_playing_flag_second = 1;
    bool isMusic1 = 1;
    bool isMusic2 = 0;


    unsigned int increase = 0;
    unsigned int decrease = 0;
    uint8_t volume = 0x70;

    // Loop
    unsigned int loop = 3;
    unsigned int loop_SEC = 3;
    unsigned int draw_index = 0;
    float pre_x = 1, pre_y = 50;
    float sample_value;
    float y_amplitude;
    unsigned int frameSec = 0;


    int16_t left_16, right_16;
    int32_t left_32, right_32;

    UINT filesize = f_size(&music_file);
    UINT  MAX_FRAMES = filesize;
    UINT SampleArraySize = MAX_FRAMES/4;

	volatile int32_t MUSIC_RAM[SampleArraySize][2];
	BYTE* MUSIC_SEC = malloc(filesize);
	uint32_t total_frames = 0;
	uint32_t total_frames_second = 0;
	uint32_t progress = 0;
	uint32_t progress_second = 0;
	uint32_t frame_secondsong = 0;

    int16_t left_Sec16;
    int16_t right_Sec16;
    int32_t left_Sec32;
    int32_t right_Sec32;

    uint32_t time1, time2, interval;

// Preload: Read music data into RAM
// It takes some time to finish
    while (total_frames < SampleArraySize) {
//    	time1 = *private_timer_value;
        res = f_read(&music_file, buffer, BUFFER_SIZE, &bytesRead);
//        time2 = *private_timer_value;
//
//        interval = time1 - time2;
//        printf("time is %u\n", interval);
        if (res != FR_OK || bytesRead == 0) break;

        for (unsigned int i = 0; i + 3 < bytesRead; i += 4) {
        	left_16  = buffer[i]   | (buffer[i+1] << 8);
        	right_16 = buffer[i+2] | (buffer[i+3] << 8);

          // Align
        	MUSIC_RAM[total_frames][0] = ((int32_t)left_16) << 8;         //Left sample
        	MUSIC_RAM[total_frames][1] = ((int32_t)right_16) << 8;        //Right sample

        	if (Progress_bar_Frame_flag) {
        		Progress_bar_Frame(lt24);
        		Progress_bar_Frame_flag = 0;
        	}
        	// 30 pieces of bar
        	if (total_frames % (SampleArraySize / 30) == 0){
        		Progress_bar(lt24, &progress);
        		progress += 1;
        	}

        	if (LoadingMusic1_flag){
        		LCD_draw_string(lt24, "Music1 is loading...", 103, 80);
        		LoadingMusic1_flag = 0;
        	}

            total_frames++;
            ResetWDT();
        }
    }

    music_Ready = 1;

    // 576043

    f_close(&music_file);

    res = f_open(&music_file, "0:/Second.pcm", FA_READ);

    UINT filesize_second = f_size(&music_file);
    UINT SampleArraySizeSec = filesize_second / 4;
	volatile int32_t MUSIC_RAM_SEC[SampleArraySizeSec][2];
    UINT second_frame = filesize_second / 16;
    exitOnFail(res);
    FRESULT res = f_read(&music_file, &MUSIC_SEC[0], 16, &bytesRead_second);
    exitOnFail(res);
	UINT offset = bytesRead_second;

 while (1) {
	unsigned keys_pressed = getPressedKeys();
	unsigned SW_value = *SW_ptr;
	// Progress bar for the song 2


// frame:
//	if (Second_Progress_bar_Frame_flag)

	// SW 0 to begin the music play
	while (music_Ready) {
		SW_value = *SW_ptr;
		LCD_draw_string(lt24, "MUSIC READY", 30, 170);
		LCD_draw_string(lt24, "PRESS SW0 TO CONTINUE", 10, 230);

		printf("filesize_second = %d \n", filesize_second);
		printf("second_frame = %d \n", second_frame);
		printf("filesize = %d \n", filesize);

		if (SW_value & 0x1) {
			music_Ready = 0;   // Press SW 0 Play music at first time
			LT24_clearDisplay(lt24, LT24_BLACK);
		}
		ResetWDT();
	}

	// Song 2 progress bar
//	unsigned int currentTimerValue2 = *private_timer_value;
//	if ((lastTimerValue1 - currentTimerValue2 >= Period2) && total_reset_second){



   // 176 pieces of the progress bar
	if ((total_frames_second * 176 / second_frame) > progress_second){
		Progress_bar_second(lt24, &progress_second);
		progress_second += 1;
	}


	if (Progressbar2_enable) {
		Progress_Secondbar_Frame(lt24);
		Progressbar2_enable = 0;
	}

	// total_frames_second max = filesize_second/16 = second_frame


//	lastTimerValue2 = currentTimerValue2;
//	}


// Preloading the Music 2 into memory
	// Slow Write
	// When the music is playing
//	if ((frame % 512 == 0) && total_reset_second && music_enable) {
//		f_read(&music_file, &MUSIC_SEC[offset], 16, &bytesRead_second);
//		offset += bytesRead_second;
//		total_frames_second += 1;   // Every 16 bytes written, frame + 1
//	}

	// Fast Write if Music stop/ not resume
	if (Fast_Write && total_reset_second){
		f_read(&music_file, &MUSIC_SEC[offset], 16, &bytesRead_second);
		offset += bytesRead_second;
		total_frames_second += 1;
	}

	// Music 2 is ready
    if (offset >= filesize_second - 32){
    	total_reset_second = 0;      // Write stop
    	if (Music2_Ready_word) {
    	LCD_draw_string(lt24, "Music2 is Ready to Play", 10, 180);
    	Music2_Ready_word = 0;
    	}
    }


	// Song 1 playing if SW 1 = 0
	if ((frame < total_frames) && music_enable && !music_Ready && !(SW_value & 0x2)){
		if (reset_time) {
			music_reset = 1;            // Ready to reset
			reset_time -= 1;
		}
			WM8731_getFIFOSpace(audio, &space);
			FPGA_PIO_setOutput(leds, space, UINT32_MAX);
           if (space > 0) {
			WM8731_writeSample(audio, MUSIC_RAM[frame][0], MUSIC_RAM[frame][1]);
			frame ++;
		}

           if (music_playing_flag1){
				isMusic1 = 1;
				isMusic2 = 0;
           LCD_draw_string(lt24, "Music1 is Playing", 10, 230);
           music_playing_flag1 = 0;
           }
		ResetWDT();
	  }



	unsigned int currentTimerValue1 = *private_timer_value;
	// Draw the music data wave
	// One Channel: Normalization
	if (lastTimerValue1 - currentTimerValue1 >= Period1) {
		if (music_enable) {
		// Left channel music data waveform
		sample_value =  MUSIC_RAM[frame][0] / 8388352.0f;          // Data: [-0.5, 0.5]
		y_amplitude = 50 - 100 * sample_value;                     // y_amplitude: [0, 100]

//		printf("the left channel data is %f \n", sample_value);

		Drawline(lt24, pre_x, pre_y, loop, y_amplitude, LT24_GREEN);

		if (loop >= LCD_WIDTH) {
			loop = 0;
			LT24_clearDisplayforwave(lt24, LT24_BLACK);
		}

		// Only trigger once a time
		// Initialize sound bar pattern
		if (soundbar_initialize) {
			Soundbar(lt24, 200, 220, 20, 40, LT24_GREEN);
			Soundbar_word (lt24, 202, 208);
			soundbar_initialize = !soundbar_initialize;
		}

		if (soundbar_increase_flag){
			Soundbar_increase(lt24, 200, 260, &increase, &decrease, LT24_GREEN);
			soundbar_increase_flag = 0;
		}

		if (soundbar_decrease_flag){
			Soundbar_decrease(lt24, 200, 260, &increase, &decrease);
			soundbar_decrease_flag = 0;
		}

		pre_x = loop;
		pre_y = y_amplitude;
		loop += 2;
	}
		lastTimerValue1 = currentTimerValue1;
	}




    if (*private_timer_interrupt & 0x1) {
    // If the timer interrupt flag is set, clear the flag
    *private_timer_interrupt = 0x1;
    }



	// If the music 1 play over stop it
    // Reset all the parameters
	if ((frame >= total_frames) && music_reset) {
		 music_enable = 0;

		 LT24_clearDisplay(lt24, LT24_BLACK);
		 Progress_bar_second_Recover(lt24, &progress_second);
		 Progress_Secondbar_Frame(lt24);
//		Soundbar_clear(lt24, 200, 200, 20, 120);       // Soundbar Clear
//		Soundbar_clear(lt24, 0, 170, 120, 150);        // Word Clear
//		LT24_clearDisplayforwave(lt24, LT24_BLACK);
		if (offset >= filesize_second - 32) {
    	LCD_draw_string(lt24, "Music2 is Ready to Play", 10, 180);
		}

	    soundbar_initialize = 1;
	    soundbar_increase_flag = 1;
	    soundbar_decrease_flag = 1;
	    music_playing_flag1 = 1;

	    Fast_Write = 1;
	    increase = 0;
	    decrease = 0;
	    volume = 0x70;

        *LEDR_ptr = 0xff;
        reset_time = 1;
        loop = 3;
        draw_index = 0;

        pre_x = 1;
        pre_y = y_amplitude;

        LCD_draw_string(lt24, "PRESS KEY1 TO PLAY MUSIC AGAIN", 10, 230);
        music_reset = 0;
        printf("frame reset \n");
	}



	// KEY:
	// Press KEY0 to stop and recover music
	// Preload song faster
	// music_enable from 1 --- 0 then 0 --- 1 resume play
	if (!(SW_value & 0x2)) {
	if ((frame < total_frames) && (keys_pressed & 0x1)) {
		 music_enable = !music_enable;
		 Fast_Write = !Fast_Write;
	   }

	// Press KEY1 to play the music again after whole music play over
	if ((keys_pressed & 0x2) && !music_enable) {
	     WM8731_clearFIFO(audio, true, true);
		 music_enable = 1;
		 frame = 0;
		 frame_secondsong = 0;
		 Fast_Write = 0;
		 WM8731_writeRegister(audio, WM8731_REG_LEFTOUTCNTRL, volume);
		 WM8731_writeRegister(audio, WM8731_REG_RIGHTOUTCNTRL, volume);

		 	     printf("the filesize is %d \n", filesize);
		 	     printf("the  offset is %d \n",  offset);
		 Soundbar_clear(lt24, 10, 230, 200, 20);     // Clear the word not soundbar
		 soundbar_initialize = 1;

		 ResetWDT();
	   }
	}

	// Press KEY2, increase volume
	if ((keys_pressed & 0x4) && (music_enable || music_enable2)) {
		increase += 1;
		soundbar_increase_flag = 1;
		volume = (volume + 0x03) & 0x7F;

		printf("volume = 0x%02X\n", volume);

		if (volume > 0x58 && volume < 0x7C) {

		    printf(" volume middle \n");
	    int status_leftout = WM8731_writeRegister(audio, WM8731_REG_LEFTOUTCNTRL, volume); //-24dB Volume. Unmute.
	    exitOnFail(status_leftout);
	    int status_rightout = WM8731_writeRegister(audio, WM8731_REG_RIGHTOUTCNTRL, volume); //-24dB Volume. Unmute.
	    exitOnFail(status_rightout);
		} else if (volume <= 0x58) {   // Mute
			 volume = 0x58;
			 printf(" volume low \n");
			 WM8731_writeRegister(audio, WM8731_REG_LEFTOUTCNTRL, volume);
			 WM8731_writeRegister(audio, WM8731_REG_RIGHTOUTCNTRL, volume);
	    } else if (volume >= 0x7C){   // Highest
		     volume = 0x7C;
		     printf(" volume high \n");
			 WM8731_writeRegister(audio, WM8731_REG_LEFTOUTCNTRL, volume);
			 WM8731_writeRegister(audio, WM8731_REG_RIGHTOUTCNTRL, volume);
	    }
	}


	// Press, KEY3 decrease volume
	if ((keys_pressed & 0x8) && (music_enable || music_enable2)) {
		decrease += 1;
		soundbar_decrease_flag = !soundbar_decrease_flag;
		volume = (volume - 0x03) & 0x7F;

		printf("volume = 0x%02X\n", volume);

		if (volume > 0x58 && volume < 0x7C) {
	    printf(" volume middle \n");
	    int status_leftout = WM8731_writeRegister(audio, WM8731_REG_LEFTOUTCNTRL, volume); //-24dB Volume. Unmute.
	    exitOnFail(status_leftout);
	    int status_rightout = WM8731_writeRegister(audio, WM8731_REG_RIGHTOUTCNTRL, volume); //-24dB Volume. Unmute.
	    exitOnFail(status_rightout);
		} else if (volume <= 0x58) {
			printf(" volume low \n");
		   volume = 0x58;
			 WM8731_writeRegister(audio, WM8731_REG_LEFTOUTCNTRL, volume);
			 WM8731_writeRegister(audio, WM8731_REG_RIGHTOUTCNTRL, volume);
	   } else if (volume >= 0x7C) {
		   printf(" volume high \n");
		   volume = 0x7C;
			 WM8731_writeRegister(audio, WM8731_REG_LEFTOUTCNTRL, volume);
			 WM8731_writeRegister(audio, WM8731_REG_RIGHTOUTCNTRL, volume);
	   }
	}


	// From music2 to music 1
	if (!(SW_value & 0x2) && isMusic2) {     // SW 1 = 0, MUSIC 2 HAS PLAYED
		 	 music_enable = 1;
		 	 LT24_clearDisplay(lt24, LT24_BLACK);

		    soundbar_initialize = 1;
		    soundbar_increase_flag = 1;
		    soundbar_decrease_flag = 1;
		    music_playing_flag1 = 1;

		    frame = 0;
		    reset_time = 1;

	        loop = 3;
	        pre_x = 1;
	        pre_y = y_amplitude;

	        printf("reset execute /n");
	}

	// from music 1 to music 2

	if (isMusic1 && !isMusic2 && (SW_value & 0x2)) {   // SW 1 = 1, MUSIC 1 HAS PLAYED

			isMusic1 = 0;
			isMusic2 = 1;

			LT24_clearDisplay(lt24, LT24_BLACK);

			LCD_draw_string(lt24, "Music2 is Playing", 10, 230);
			music_enable2 = 1;
			soundbar_initialize = 1;
		    soundbar_increase_flag = 1;
		    soundbar_decrease_flag = 1;
		    music_playing_flag2 = 1;
		    music_enable = 0;
	        loop_SEC = 3;
	        draw_index = 0;
	        frameSec = 0;
	        pre_x = 1;
	        pre_y = 50;

	}


	// IF SW 1 is pressed Play Song 2
	if ((SW_value & 0x2) && !(SW_value & 0x8)){

		if ((frame_secondsong < (offset - 32)) && music_enable2 && !music_Ready){
			if (reset_time){
				music_reset2 = 1;
			}

	        if (music_playing_flag2){
	        LCD_draw_string(lt24, "Music2 is Playing", 10, 230);
	        music_playing_flag2 = 0;
	        }

	    left_Sec16  = MUSIC_SEC[frame_secondsong] | MUSIC_SEC[frame_secondsong + 1] << 8;
	    right_Sec16 = MUSIC_SEC[frame_secondsong + 2] | MUSIC_SEC[frame_secondsong + 3] << 8;
	    left_Sec32  = ((int32_t)left_Sec16) << 8;
	    right_Sec32 = ((int32_t)right_Sec16) << 8;

		WM8731_getFIFOSpace(audio, &space);

       if (space > 0) {
    	   WM8731_writeSample(audio, left_Sec32, right_Sec32);
    	   frame_secondsong += 4;
    	   frameSec += 1;
    	   ResetWDT();
			}
		}

	// If the music play over stop it
    // Reset all the parameters
	if ((frame_secondsong >= (offset - 32)) && music_reset2) {
		music_enable2 = 0;

		LT24_clearDisplay(lt24, LT24_BLACK);
//		Soundbar_clear(lt24, 200, 200, 20, 120);       // Soundbar Clear
//		Soundbar_clear(lt24, 0, 170, 120, 150);        // Word Clear
//		LT24_clearDisplayforwave(lt24, LT24_BLACK);

	    soundbar_initialize = 1;
	    soundbar_increase_flag = 1;
	    soundbar_decrease_flag = 1;
	    music_playing_flag2 = 1;

	    increase = 0;
	    decrease = 0;
	    volume = 0x70;

        *LEDR_ptr = 0xff;
        reset_time = 1;
        loop_SEC = 3;

        frameSec = 0;

        pre_x = 1;
        pre_y = y_amplitude;
        LCD_draw_string(lt24, "PRESS KEY1 TO PLAY MUSIC AGAIN", 10, 230);
        music_reset2 = 0;
        printf("frame reset \n");
	}

	unsigned int currentTimerValue2 = *private_timer_value;
	// Draw the music data wave
	// One Channel: Normalization
	if (lastTimerValue2 - currentTimerValue2 >= Period1) {
		if (music_enable2) {
		// Left channel music data waveform
		sample_value =  left_Sec32 / 8388352.0f;          // Data: [-0.5, 0.5]
		y_amplitude = 50 - 100 * sample_value;                     // y_amplitude: [0, 100]

//		printf("the left channel data is %f \n", sample_value);

		Drawline(lt24, pre_x, pre_y, loop_SEC, y_amplitude, LT24_RED);

		if (loop_SEC >= LCD_WIDTH) {
			loop_SEC = 0;
			LT24_clearDisplayforwave(lt24, LT24_BLACK);
		}

		// Only trigger once a time
		// Initialize sound bar pattern
		if (soundbar_initialize) {
			Soundbar(lt24, 200, 220, 20, 40, LT24_GREEN);
			Soundbar_word (lt24, 202, 208);
			soundbar_initialize = !soundbar_initialize;
		}

		if (soundbar_increase_flag){
			Soundbar_increase(lt24, 200, 260, &increase, &decrease, LT24_GREEN);
			soundbar_increase_flag = 0;
		}

		if (soundbar_decrease_flag){
			Soundbar_decrease(lt24, 200, 260, &increase, &decrease);
			soundbar_decrease_flag = 0;
		}

		pre_x = loop_SEC;
		pre_y = y_amplitude;
		loop_SEC += 2;
	}
		lastTimerValue2 = currentTimerValue2;
	}


	if ((frame_secondsong < (offset - 32)) && (keys_pressed & 0x1)) {
		 music_enable2 = !music_enable2;
	   }

	// Press KEY1 to play the music again after whole music play over
	if ((keys_pressed & 0x2) && !music_enable2) {
	     WM8731_clearFIFO(audio, true, true);
		 music_enable2 = 1;
		 frame_secondsong = 0;
		 WM8731_writeRegister(audio, WM8731_REG_LEFTOUTCNTRL, volume);
		 WM8731_writeRegister(audio, WM8731_REG_RIGHTOUTCNTRL, volume);
		 	     printf("the filesize is %d \n", filesize);
		 	     printf("the  offset is %d \n",  offset);
		 	    Soundbar_clear(lt24, 10, 230, 200, 20);     // Clear the word not soundbar
		 ResetWDT();
	   }

	}
	ResetWDT();
   }
}





// Testing


///*
// * DE1-SoC LT24 Example
// *
// * Copies a test image to the LT24
// * LCD using the LT24 bare metal driver.
// *
// *  Created on: 09 Feb 2018
// *  Updated on: 11 Feb 2024
// */
//
//#include "Util/watchdog.h"
//
//#include "FPGA_PIO/FPGA_PIO.h"
//#include "DE1SoC_Addresses/DE1SoC_Addresses.h"
//#include "DE1SoC_LT24/DE1SoC_LT24.h"
//#include "Util/delay.h"
//#include "LCD_display.h"
//#include "DE1SoC_LT24/DE1SoC_LT24.h"
//#include "BasicFont/BasicFont.h"
//#include <math.h>
//
//
//
////Debugging Function (same as last lab)
//#include <stdlib.h>
//void exitOnFail(HpsErr_t status){
//    if (ERR_IS_ERROR(status)) {
//        exit((int)status); //Add breakpoint here to catch failure
//    }
//}
//
////
//// Main Function
//// =============
//int main(void) {
//    //Variables
//    FPGAPIOCtx_t* gpio1;
//    LT24Ctx_t* lt24;
//    //Initialise the GPIO1 port
//    exitOnFail(FPGA_PIO_initialise(LSC_BASE_GPIO_JP1, LSC_CONFIG_GPIO, &gpio1));
//    //Initialise the LCD Display.
//    exitOnFail(LT24_initialise(&gpio1->gpio, LSC_BASE_LT24HWDATA, &lt24));
//    ResetWDT();
//
//
//    //Main Run Loop
//    while (1) {
//
//    	LCD_draw_string(lt24, "HELLO WORLD", 150, 200);
//
//        ResetWDT(); //Just reset the watchdog.
//    }
//}

