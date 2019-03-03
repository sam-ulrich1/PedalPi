// CC-by-www.Electrosmash.com open-source project.
// Fork by Sam Ulrich

// Effects:
//      0 - Clean
//      1 - Bitcrusher
//      2 - Booster
//      3 - Delay
//      4 - Distortion
//      5 - Echo
//      6 - Fuzz
//      8 - Looper
//      9 - Octaver
//      10 - Reverb
//      11 - Tremelo

#include <bcm2835.h>
#include <stdio.h>

// Define Input Pins
#define PUSH1 			RPI_GPIO_P1_08  	//GPIO14
#define PUSH2 			RPI_V2_GPIO_P1_38  	//GPIO20
#define TOGGLE_SWITCH 	RPI_V2_GPIO_P1_32 	//GPIO12
#define FOOT_SWITCH 	RPI_GPIO_P1_10 		//GPIO15
#define LED   			RPI_V2_GPIO_P1_36 	//GPIO16

//Define Delay Effect parameters MAX_DELAY 800000 is 4 seconds approx.
#define DELAY_MAX 10000000
#define DELAY_MIN 0

uint8_t PUSH1_val;
uint8_t PUSH2_val;

// Booster Variables
uint32_t booster_value = 2047;

// Delay Variables
uint32_t Delay_Buffer[DELAY_MAX];

// Distortion Variables
uint32_t distortion_value = 100;

// Distortion Variables
uint32_t fuzz_value = 100;

// Echo Variables
uint32_t Echo_Buffer[DELAY_MAX];
uint32_t DelayCounter = 0;
uint32_t Delay_Depth = 100000; //default starting delay is 100000 is 0.5 sec approx.

// Looper Variables
uint32_t recording=0;
uint32_t record_length=100;
uint32_t delay;

// Octaver Variables
uint32_t Delay_Buffer[DELAY_MAX];
uint32_t DelayWrite = 0;
uint32_t DelayRead = 0;
uint32_t octaver_value = 1;
uint32_t divider = 0;

// Reverb Varaibles
uint32_t Echo_Buffer1[DELAY_MAX];
uint32_t Echo_Buffer2[DELAY_MAX];
uint32_t Echo_Buffer3[DELAY_MAX];
uint32_t DelayCounter1 = 0;
uint32_t DelayCounter2 = 0;
uint32_t DelayCounter3 = 0;
uint32_t Delay_Depth1 = 10000; //default starting delay is 100000 is 0.5 sec approx.
uint32_t Delay_Depth2 = 5000; //default starting delay is 100000 is 0.25 sec approx.
uint32_t Delay_Depth3 = 2500; //default starting delay is 100000 is 0.125 sec approx.

// Tremelo Variables

//To create the sinewave samples I have used this website:
//http://www.daycounter.com/Calculators/Sine-Generator-Calculator.phtml
//the generated signal is 1000 points and 4095 max. amplitude

uint32_t waveform[]= {0x800,0x80c,0x819,0x826,0x833,0x840,0x84d,0x85a,0x866,0x873,0x880,0x88d,0x89a,0x8a7,0x8b3,0x8c0,
                      0x8cd,0x8da,0x8e7,0x8f3,0x900,0x90d,0x91a,0x926,0x933,0x940,0x94c,0x959,0x966,0x973,0x97f,0x98c,
                      0x998,0x9a5,0x9b2,0x9be,0x9cb,0x9d7,0x9e4,0x9f0,0x9fd,0xa09,0xa16,0xa22,0xa2e,0xa3b,0xa47,0xa53,
                      0xa60,0xa6c,0xa78,0xa84,0xa91,0xa9d,0xaa9,0xab5,0xac1,0xacd,0xad9,0xae5,0xaf1,0xafd,0xb09,0xb15,
                      0xb21,0xb2d,0xb38,0xb44,0xb50,0xb5c,0xb67,0xb73,0xb7e,0xb8a,0xb96,0xba1,0xbac,0xbb8,0xbc3,0xbcf,
                      0xbda,0xbe5,0xbf0,0xbfc,0xc07,0xc12,0xc1d,0xc28,0xc33,0xc3e,0xc49,0xc53,0xc5e,0xc69,0xc74,0xc7e,
                      0xc89,0xc94,0xc9e,0xca9,0xcb3,0xcbd,0xcc8,0xcd2,0xcdc,0xce6,0xcf1,0xcfb,0xd05,0xd0f,0xd19,0xd23,
                      0xd2c,0xd36,0xd40,0xd4a,0xd53,0xd5d,0xd66,0xd70,0xd79,0xd82,0xd8c,0xd95,0xd9e,0xda7,0xdb0,0xdb9,
                      0xdc2,0xdcb,0xdd4,0xddd,0xde6,0xdee,0xdf7,0xdff,0xe08,0xe10,0xe19,0xe21,0xe29,0xe31,0xe39,0xe41,
                      0xe49,0xe51,0xe59,0xe61,0xe69,0xe70,0xe78,0xe7f,0xe87,0xe8e,0xe96,0xe9d,0xea4,0xeab,0xeb2,0xeb9,
                      0xec0,0xec7,0xece,0xed5,0xedb,0xee2,0xee8,0xeef,0xef5,0xefc,0xf02,0xf08,0xf0e,0xf14,0xf1a,0xf20,
                      0xf26,0xf2b,0xf31,0xf37,0xf3c,0xf42,0xf47,0xf4c,0xf51,0xf57,0xf5c,0xf61,0xf66,0xf6a,0xf6f,0xf74,
                      0xf79,0xf7d,0xf82,0xf86,0xf8a,0xf8f,0xf93,0xf97,0xf9b,0xf9f,0xfa3,0xfa6,0xfaa,0xfae,0xfb1,0xfb5,
                      0xfb8,0xfbb,0xfbf,0xfc2,0xfc5,0xfc8,0xfcb,0xfce,0xfd0,0xfd3,0xfd6,0xfd8,0xfdb,0xfdd,0xfdf,0xfe2,
                      0xfe4,0xfe6,0xfe8,0xfea,0xfeb,0xfed,0xfef,0xff0,0xff2,0xff3,0xff5,0xff6,0xff7,0xff8,0xff9,0xffa,
                      0xffb,0xffc,0xffc,0xffd,0xffe,0xffe,0xffe,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xfff,0xffe,0xffe,
                      0xffe,0xffd,0xffc,0xffc,0xffb,0xffa,0xff9,0xff8,0xff7,0xff6,0xff5,0xff3,0xff2,0xff0,0xfef,0xfed,
                      0xfeb,0xfea,0xfe8,0xfe6,0xfe4,0xfe2,0xfdf,0xfdd,0xfdb,0xfd8,0xfd6,0xfd3,0xfd0,0xfce,0xfcb,0xfc8,
                      0xfc5,0xfc2,0xfbf,0xfbb,0xfb8,0xfb5,0xfb1,0xfae,0xfaa,0xfa6,0xfa3,0xf9f,0xf9b,0xf97,0xf93,0xf8f,
                      0xf8a,0xf86,0xf82,0xf7d,0xf79,0xf74,0xf6f,0xf6a,0xf66,0xf61,0xf5c,0xf57,0xf51,0xf4c,0xf47,0xf42,
                      0xf3c,0xf37,0xf31,0xf2b,0xf26,0xf20,0xf1a,0xf14,0xf0e,0xf08,0xf02,0xefc,0xef5,0xeef,0xee8,0xee2,
                      0xedb,0xed5,0xece,0xec7,0xec0,0xeb9,0xeb2,0xeab,0xea4,0xe9d,0xe96,0xe8e,0xe87,0xe7f,0xe78,0xe70,
                      0xe69,0xe61,0xe59,0xe51,0xe49,0xe41,0xe39,0xe31,0xe29,0xe21,0xe19,0xe10,0xe08,0xdff,0xdf7,0xdee,
                      0xde6,0xddd,0xdd4,0xdcb,0xdc2,0xdb9,0xdb0,0xda7,0xd9e,0xd95,0xd8c,0xd82,0xd79,0xd70,0xd66,0xd5d,
                      0xd53,0xd4a,0xd40,0xd36,0xd2c,0xd23,0xd19,0xd0f,0xd05,0xcfb,0xcf1,0xce6,0xcdc,0xcd2,0xcc8,0xcbd,
                      0xcb3,0xca9,0xc9e,0xc94,0xc89,0xc7e,0xc74,0xc69,0xc5e,0xc53,0xc49,0xc3e,0xc33,0xc28,0xc1d,0xc12,
                      0xc07,0xbfc,0xbf0,0xbe5,0xbda,0xbcf,0xbc3,0xbb8,0xbac,0xba1,0xb96,0xb8a,0xb7e,0xb73,0xb67,0xb5c,
                      0xb50,0xb44,0xb38,0xb2d,0xb21,0xb15,0xb09,0xafd,0xaf1,0xae5,0xad9,0xacd,0xac1,0xab5,0xaa9,0xa9d,
                      0xa91,0xa84,0xa78,0xa6c,0xa60,0xa53,0xa47,0xa3b,0xa2e,0xa22,0xa16,0xa09,0x9fd,0x9f0,0x9e4,0x9d7,
                      0x9cb,0x9be,0x9b2,0x9a5,0x998,0x98c,0x97f,0x973,0x966,0x959,0x94c,0x940,0x933,0x926,0x91a,0x90d,
                      0x900,0x8f3,0x8e7,0x8da,0x8cd,0x8c0,0x8b3,0x8a7,0x89a,0x88d,0x880,0x873,0x866,0x85a,0x84d,0x840,
                      0x833,0x826,0x819,0x80c,0x800,0x7f3,0x7e6,0x7d9,0x7cc,0x7bf,0x7b2,0x7a5,0x799,0x78c,0x77f,0x772,
                      0x765,0x758,0x74c,0x73f,0x732,0x725,0x718,0x70c,0x6ff,0x6f2,0x6e5,0x6d9,0x6cc,0x6bf,0x6b3,0x6a6,
                      0x699,0x68c,0x680,0x673,0x667,0x65a,0x64d,0x641,0x634,0x628,0x61b,0x60f,0x602,0x5f6,0x5e9,0x5dd,
                      0x5d1,0x5c4,0x5b8,0x5ac,0x59f,0x593,0x587,0x57b,0x56e,0x562,0x556,0x54a,0x53e,0x532,0x526,0x51a,
                      0x50e,0x502,0x4f6,0x4ea,0x4de,0x4d2,0x4c7,0x4bb,0x4af,0x4a3,0x498,0x48c,0x481,0x475,0x469,0x45e,
                      0x453,0x447,0x43c,0x430,0x425,0x41a,0x40f,0x403,0x3f8,0x3ed,0x3e2,0x3d7,0x3cc,0x3c1,0x3b6,0x3ac,
                      0x3a1,0x396,0x38b,0x381,0x376,0x36b,0x361,0x356,0x34c,0x342,0x337,0x32d,0x323,0x319,0x30e,0x304,
                      0x2fa,0x2f0,0x2e6,0x2dc,0x2d3,0x2c9,0x2bf,0x2b5,0x2ac,0x2a2,0x299,0x28f,0x286,0x27d,0x273,0x26a,
                      0x261,0x258,0x24f,0x246,0x23d,0x234,0x22b,0x222,0x219,0x211,0x208,0x200,0x1f7,0x1ef,0x1e6,0x1de,
                      0x1d6,0x1ce,0x1c6,0x1be,0x1b6,0x1ae,0x1a6,0x19e,0x196,0x18f,0x187,0x180,0x178,0x171,0x169,0x162,
                      0x15b,0x154,0x14d,0x146,0x13f,0x138,0x131,0x12a,0x124,0x11d,0x117,0x110,0x10a,0x103,0xfd,0xf7,
                      0xf1,0xeb,0xe5,0xdf,0xd9,0xd4,0xce,0xc8,0xc3,0xbd,0xb8,0xb3,0xae,0xa8,0xa3,0x9e,
                      0x99,0x95,0x90,0x8b,0x86,0x82,0x7d,0x79,0x75,0x70,0x6c,0x68,0x64,0x60,0x5c,0x59,
                      0x55,0x51,0x4e,0x4a,0x47,0x44,0x40,0x3d,0x3a,0x37,0x34,0x31,0x2f,0x2c,0x29,0x27,
                      0x24,0x22,0x20,0x1d,0x1b,0x19,0x17,0x15,0x14,0x12,0x10,0xf,0xd,0xc,0xa,0x9,
                      0x8,0x7,0x6,0x5,0x4,0x3,0x3,0x2,0x1,0x1,0x1,0x0,0x0,0x0,0x0,0x0,
                      0x0,0x0,0x1,0x1,0x1,0x2,0x3,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xc,
                      0xd,0xf,0x10,0x12,0x14,0x15,0x17,0x19,0x1b,0x1d,0x20,0x22,0x24,0x27,0x29,0x2c,
                      0x2f,0x31,0x34,0x37,0x3a,0x3d,0x40,0x44,0x47,0x4a,0x4e,0x51,0x55,0x59,0x5c,0x60,
                      0x64,0x68,0x6c,0x70,0x75,0x79,0x7d,0x82,0x86,0x8b,0x90,0x95,0x99,0x9e,0xa3,0xa8,
                      0xae,0xb3,0xb8,0xbd,0xc3,0xc8,0xce,0xd4,0xd9,0xdf,0xe5,0xeb,0xf1,0xf7,0xfd,0x103,
                      0x10a,0x110,0x117,0x11d,0x124,0x12a,0x131,0x138,0x13f,0x146,0x14d,0x154,0x15b,0x162,0x169,0x171,
                      0x178,0x180,0x187,0x18f,0x196,0x19e,0x1a6,0x1ae,0x1b6,0x1be,0x1c6,0x1ce,0x1d6,0x1de,0x1e6,0x1ef,
                      0x1f7,0x200,0x208,0x211,0x219,0x222,0x22b,0x234,0x23d,0x246,0x24f,0x258,0x261,0x26a,0x273,0x27d,
                      0x286,0x28f,0x299,0x2a2,0x2ac,0x2b5,0x2bf,0x2c9,0x2d3,0x2dc,0x2e6,0x2f0,0x2fa,0x304,0x30e,0x319,
                      0x323,0x32d,0x337,0x342,0x34c,0x356,0x361,0x36b,0x376,0x381,0x38b,0x396,0x3a1,0x3ac,0x3b6,0x3c1,
                      0x3cc,0x3d7,0x3e2,0x3ed,0x3f8,0x403,0x40f,0x41a,0x425,0x430,0x43c,0x447,0x453,0x45e,0x469,0x475,
                      0x481,0x48c,0x498,0x4a3,0x4af,0x4bb,0x4c7,0x4d2,0x4de,0x4ea,0x4f6,0x502,0x50e,0x51a,0x526,0x532,
                      0x53e,0x54a,0x556,0x562,0x56e,0x57b,0x587,0x593,0x59f,0x5ac,0x5b8,0x5c4,0x5d1,0x5dd,0x5e9,0x5f6,
                      0x602,0x60f,0x61b,0x628,0x634,0x641,0x64d,0x65a,0x667,0x673,0x680,0x68c,0x699,0x6a6,0x6b3,0x6bf,
                      0x6cc,0x6d9,0x6e5,0x6f2,0x6ff,0x70c,0x718,0x725,0x732,0x73f,0x74c,0x758,0x765,0x772,0x77f,0x78c,
                      0x799,0x7a5,0x7b2,0x7bf,0x7cc,0x7d9,0x7e6,0x7f3,0x800,};
uint32_t sample=0;
uint32_t speed=1;
uint32_t max_count=25;

uint32_t read_timer=0;
uint32_t input_signal=0;

uint8_t FOOT_SWITCH_val;
uint8_t TOGGLE_SWITCH_val;
uint8_t PUSH1_val;
uint8_t PUSH2_val;

uint8_t effect=0;  // Set the default effect to Clean
uint8_t effectStrength=0;
uint8_t lastEffect = 0;
uint8_t lastEffectStrength = 0;

void changeEffectStrength() {
    if (PUSH1_val==0) {
        bcm2835_delay(100); //100ms delay for buttons debouncing.
        if (effectStrength < 12) {
            effectStrength++;
        } else if (effectStrength >= 12) {
            effectStrength = 0;
        }
        printf("Effect Strength: %d\n", effectStrength);
    }
}

void changeEffectType() {
    if (PUSH2_val==0) {
        bcm2835_delay(100); //100ms delay for buttons debouncing
        if (effect < 11) {
            effect++;
        } else if (effect >= 11) {
            effect = 0;
        }
    }
}

void readHardware() {
    read_timer++;
    if (read_timer==50000)
    {
        read_timer=0;
        PUSH1_val = bcm2835_gpio_lev(PUSH1);
        PUSH2_val = bcm2835_gpio_lev(PUSH2);
        TOGGLE_SWITCH_val = bcm2835_gpio_lev(TOGGLE_SWITCH);
        FOOT_SWITCH_val = bcm2835_gpio_lev(FOOT_SWITCH);
        bcm2835_gpio_write(LED,!FOOT_SWITCH_val); //light the effect when the footswitch is activated.
        changeEffectType();
        changeEffectStrength();
    }
}

void bitCrusherEffect() {
    input_signal = input_signal << effectStrength;
}

void boosterEffect() {
    booster_value = 2047 + (500 * (effectStrength / 4));
    input_signal= (int)((float)(input_signal) * (float)((float) booster_value / (float) 4095.0));
}

void delayEffect() {
    Delay_Depth = 100000 + (effectStrength * 50000);
    Delay_Buffer[DelayCounter] = input_signal;
    DelayCounter++;
    if(DelayCounter >= Delay_Depth) DelayCounter = 0;
    input_signal = (Delay_Buffer[DelayCounter]+input_signal)>>1;
}

void distortionEffect() {
    distortion_value = 100 + (10 * effectStrength);
    if (input_signal > 2047 + distortion_value) input_signal= 2047 + distortion_value;
    if (input_signal < 2047 - distortion_value) input_signal= 2047 - distortion_value;
}

void echoEffect() {
    Delay_Depth = 100000 + (effectStrength * 50000);
    Echo_Buffer[DelayCounter]  = (input_signal + Echo_Buffer[DelayCounter])>>1;
    DelayCounter++;
    if(DelayCounter >= Delay_Depth) DelayCounter = 0;
    input_signal = (input_signal + (Echo_Buffer[DelayCounter]))>>1;
}

void fuzzEffect() {
    fuzz_value = 100 + (10 * effectStrength);
    if (input_signal > 2047 + fuzz_value) input_signal= 4095;
    if (input_signal < 2047 - fuzz_value) input_signal= 0;
}

void looperEffect() {
    if (effectStrength % 2 != 0) {
        recording=1;
        DelayCounter=0;
    } else {
        recording=0;
        record_length=DelayCounter;
        DelayCounter=0;
    }
    if (recording==1)
    {//start recording
        Delay_Buffer[DelayCounter] = input_signal;
        DelayCounter++;
        bcm2835_gpio_write(LED,!FOOT_SWITCH_val);
        //if(DelayCounter >= Delay_Depth) DelayCounter = 0;
    }
    else
    {//bypass mode
        input_signal = (Delay_Buffer[DelayCounter]+input_signal)>>1;
        DelayCounter++;
        if (DelayCounter>record_length)DelayCounter=0;
        bcm2835_gpio_write(LED,FOOT_SWITCH_val);
    }
}

void octaverEffect() {
    //save current reading
    Delay_Buffer[DelayWrite] = input_signal;

    //Increse/reset delay counter.
    DelayWrite++;
    if(DelayWrite >= Delay_Depth) DelayWrite = 0;

    input_signal = Delay_Buffer[DelayRead];

    if (octaver_value == 2) DelayRead = DelayRead + 2;
    if (octaver_value == 1) DelayRead = DelayRead + 1;
    if (octaver_value == 0)
    {
        divider++;
        if (divider>=2)
        {
            DelayRead = DelayRead + 1;
            divider=0;

        }
    }
    if(DelayRead >= Delay_Depth) DelayRead = 0;
}

void reverbEffect() {
    if (Delay_Depth1<DELAY_MAX)Delay_Depth1 = (10000 + (5000 * effectStrength)); //50000 adds 25ms approx.
    if (Delay_Depth2<DELAY_MAX)Delay_Depth2 = (5000 + (5000 * effectStrength)); //50000 adds 25ms approx.
    if (Delay_Depth3<DELAY_MAX)Delay_Depth2 = (2500 + (5000 * effectStrength)); //50000 adds 25ms approx.

    Echo_Buffer1[DelayCounter1]  = (input_signal + Echo_Buffer1[DelayCounter1])>>1;
    Echo_Buffer2[DelayCounter2]  = (input_signal + Echo_Buffer2[DelayCounter2])>>1;
    Echo_Buffer3[DelayCounter3]  = (input_signal + Echo_Buffer3[DelayCounter3])>>1;

    DelayCounter1++;
    if(DelayCounter1 >= Delay_Depth1) DelayCounter1 = 0;

    DelayCounter2++;
    if(DelayCounter2 >= Delay_Depth2) DelayCounter2 = 0;

    DelayCounter3++;
    if(DelayCounter3 >= Delay_Depth3) DelayCounter3 = 0;

    input_signal = (input_signal + (Echo_Buffer1[DelayCounter1])+(Echo_Buffer2[DelayCounter2])+(Echo_Buffer3[DelayCounter3]))>>2;
}

void tremeloEffect() {
    max_count = 1 * (effectStrength * 4);
    divider++;
    if (divider==max_count){ divider=0; sample++;}

    if(sample==999)sample=1;
    input_signal= (int)(((float)input_signal) * ((float)waveform[sample]/4095.0));
}

void callEffect() {
    if (effect == 7) {
        Delay_Depth = 50000;
    } else {
        Delay_Depth = 100000;
    }
    switch (effect) {
        case 0:
            if (lastEffect != effect) printf("Mode: Clean");
            break;
        case 1:
            if (lastEffect != effect) printf("Mode: BitCrusher");
            bitCrusherEffect();
            break;
        case 2:
            if (lastEffect != effect) printf("Mode: Booster");
            boosterEffect();
            break;
        case 3:
            if (lastEffect != effect) printf("Mode: Delay");
            delayEffect();
            break;
        case 4:
            if (lastEffect != effect) printf("Mode: Distortion");
            distortionEffect();
            break;
        case 5:
            if (lastEffect != effect) printf("Mode: Fuzz");
            fuzzEffect();
            break;
        case 6:
            if (lastEffect != effect) printf("Mode: Echo");
            echoEffect();
            break;
        case 7:
            if (lastEffect != effect) printf("Mode: Looper");
            looperEffect();
            break;
        case 8:
            if (lastEffect != effect) printf("Mode: Octaver");
            octaverEffect();
            break;
        case 9:
            if (lastEffect != effect) printf("Mode: Reverb");
            reverbEffect();
            break;
        case 10:
            if (lastEffect != effect) printf("Mode: Tremelo");
            tremeloEffect();
            break;
    }
    lastEffect = effect;
}

int main(int argc, char **argv)
{
    // Start the BCM2835 Library to access GPIO.
    if (!bcm2835_init())
    {
        printf("bcm2835_init failed. Are you running as root??\n");
        return 1;
    }
    // Start the SPI BUS.
    if (!bcm2835_spi_begin())
    {
        printf("bcm2835_spi_begin failed. Are you running as root??\n");
        return 1;
    }

    if (effect == 0) {
        printf("Mode: Clean");
    } else {
        printf("Mode Number: %d\n", effect);
    }

    printf("Effect Strength: %d\n", effectStrength);

    //define PWM
    bcm2835_gpio_fsel(18,BCM2835_GPIO_FSEL_ALT5 ); //PWM0 signal on GPIO18
    bcm2835_gpio_fsel(13,BCM2835_GPIO_FSEL_ALT0 ); //PWM1 signal on GPIO13
    bcm2835_pwm_set_clock(2); // Max clk frequency (19.2MHz/2 = 9.6MHz)
    bcm2835_pwm_set_mode(0,1 , 1); //channel 0, markspace mode, PWM enabled.
    bcm2835_pwm_set_range(0,64);   //channel 0, 64 is max range (6bits): 9.6MHz/64=150KHz switching PWM freq.
    bcm2835_pwm_set_mode(1, 1, 1); //channel 1, markspace mode, PWM enabled.
    bcm2835_pwm_set_range(1,64);   //channel 0, 64 is max range (6bits): 9.6MHz/64=150KHz switching PWM freq.

    //define SPI bus configuration
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); 	  // 4MHz clock with _64
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

    uint8_t mosi[10] = { 0x01, 0x00, 0x00 }; //12 bit ADC read 0x08 ch0, - 0c for ch1
    uint8_t miso[10] = { 0 };

    //Define GPIO pins configuration
    bcm2835_gpio_fsel(PUSH1, BCM2835_GPIO_FSEL_INPT); 			//PUSH1 button as input
    bcm2835_gpio_fsel(PUSH2, BCM2835_GPIO_FSEL_INPT); 			//PUSH2 button as input
    bcm2835_gpio_fsel(TOGGLE_SWITCH, BCM2835_GPIO_FSEL_INPT);	//TOGGLE_SWITCH as input
    bcm2835_gpio_fsel(FOOT_SWITCH, BCM2835_GPIO_FSEL_INPT); 	//FOOT_SWITCH as input
    bcm2835_gpio_fsel(LED, BCM2835_GPIO_FSEL_OUTP);				//LED as output

    bcm2835_gpio_set_pud(PUSH1, BCM2835_GPIO_PUD_UP);           //PUSH1 pull-up enabled
    bcm2835_gpio_set_pud(PUSH2, BCM2835_GPIO_PUD_UP);           //PUSH2 pull-up enabled
    bcm2835_gpio_set_pud(TOGGLE_SWITCH, BCM2835_GPIO_PUD_UP);   //TOGGLE_SWITCH pull-up enabled
    bcm2835_gpio_set_pud(FOOT_SWITCH, BCM2835_GPIO_PUD_UP);     //FOOT_SWITCH pull-up enabled

    while(1) //Main Loop
    {
        //read 12 bits ADC
        bcm2835_spi_transfernb(mosi, miso, 3);
        input_signal = miso[2] + ((miso[1] & 0x0F) << 8);

        //Read the PUSH buttons every 50000 times (0.25s) to save resources.
        readHardware();

        //**** CLEAN EFFECT ***///
        //Nothing to do, the input_signal goes directly to the PWM output.
        callEffect();

        //generate output PWM signal 6 bits
        bcm2835_pwm_set_data(1,input_signal & 0x3F);
        bcm2835_pwm_set_data(0,input_signal >> 6);
    }

    //close all and exit
    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}
