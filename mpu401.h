/* mpu401.h 	header file */
/* mpu-401 command and message summary */

/* mpu marks */

#define	NOP		0xf8		/* no operation */
#define	MES_END		0xf9		/*  measure end */
#define	DATA_END	0xfc		/*  end data */

/* mpu messages */

#define	REQ_T1	 	0xf0	/* track data request track 1 */
#define	REQ_T2 		0xf1	
#define	REQ_T3		0xf2
#define	REQ_T4		0xf3
#define	REQ_T5		0xf4
#define	REQ_T6		0xf5
#define	REQ_T7		0xf6
#define	REQ_T8		0xf7 	/* track data request track 8 */
#define	TIME_OUT 	0xf8	/* timing overflow */
#define	CONDUCT	 	0xf9 	/* conductor data request */
#define	ALL_END		0xfc	/* all end */
#define	CLOCK_OUT 	0xfd	/* clock to host */
#define	ACK 	 	0xfe	/* acknowledge */
#define	SYS_MES	 	0xff 	/* system message */

/* mpu commands */
/* full control summary is given below, but only most common are defined */

/* command	midi	play	record	comment */

/*
00	-	-	-	not used
01	stop
02	start
03	continue
04	-	stop
05	stop	stop		stop play
06	start	stop
07	cont.	stop
08	-	start
09	stop	start
0a	start	start		start play
0b	cont.	start		continue play
0c-0f				not used

10	-	-	stop	        
11	stop	-	stop	stop record
12	start	-	stop
13	cont.   -	stop
14	-	stop	stop
15	stop	stop	stop	stop over-dub
16	start	stop	stop
17	cont.	stop	stop
18 	-	start	stop
19	stop	start	stop
1a	start	start	stop		
1b	cont.	start	stop	
1c-1f				not used

20	-	-	standby record standby
21	stop	-	standby 
22	start	-	start	start record
23	cont.   -	start	continue record
24	-	stop	standby
25	stop	stop	standby
26	start	stop	start
27	cont.	stop	start
28 	-	start	standby
29	stop	start	standby
2a	start	start	start	start over-dub
2b	cont.	start	start	start over-dub
2c-2f				not used
*/

#define	STOP_PLAY	0x05
#define	START_PLAY	0x0a
#define	STOP_REC	0x11
#define	START_REC	0x22
#define STOP_OVDUB	0x15
#define START_OVDUB	0x2A

#define	N0_ALL_OFF 	0x30 	/* all notes off */
#define	NO_RTIME 	0x32 	/* no real time */
#define	THRU_OFF_CHAN	0x33 	/* thru : off on channels */
#define	WITH_TIME 	0x34 	/* with timing byte : on */
#define	MODE_THRU 	0x35 	/* mode mes : on */
#define	EXCL_THRU 	0x36 	/* exclusive thru : on */
#define	COM_THRU 	0x38 	/* common to host : on */
#define	REAL_THRU 	0x39 	/* real time to host : on */
#define	UART	 	0x3f 	/* uart mode */

/* channel reference table numbers are normally computed */

/*
40 - 4f sets channel reference table a
50 - 5f b
60 - 6f c
70 - 7f d
*/

#define	INT_CLOCK 	0x80 	/* internal clock */
#define	FSK_CLOCK 	0x81 	/* fsk clock */
#define	MIDI_CLOCK 	0x82 	/* midi clock */
#define	MET_ON_WOUT 	0x83 	/* metronome : on - w/o accents */
#define	MET_OFF	 	0x84	/* metronome : off */
#define	MET_ON_WITH 	0x85 	/* metronome : on - with accents */
#define	BEND_OFF 	0x86 	/* bender : off */
#define	BEND_ON	 	0x87 	/* bender : on */
#define	THRU_OFF 	0x88	/* midi thru : off */
#define	THRU_ON	 	0x89 	/* midi thru : on */
#define	DSTOP_OFF 	0x8a 	/* data in stop mode : off */
#define	DSTOP_ON 	0x8b 	/* data in stop mode : on */
#define	MEAS_OFF 	0x8c 	/* send measure end : off */
#define	MEAS_ON  	0x8d 	/* send measure end : on */
#define	COND_OFF 	0x8e 	/* conductor : off */
#define	COND_ON	 	0x8f 	/* conductor : on */
#define	REAL_OFF 	0x90 	/* real time affecton : off */
#define	REAL_ON	 	0x91 	/* real time affection : on */
#define	FSK_INT	 	0x92 	/* fsk to internal */
#define	FSK_MIDI 	0x93 	/* fsk to midi */
#define	CLOCK_OFF 	0x94 	/* clock to host : off */
#define	CLOCK_ON 	0x95 	/* clock to host : on */
#define	EXCL_OFF 	0x96 	/* exclusive to host : off */
#define	EXCL_ON  	0x97 	/* exclusive to host : on */

#define	CHANA_OFF	0x98 	/* reference table a : off */
#define	CHANA_ON 	0x99 	/* a : on */
#define	CHANB_OFF 	0x9a 	/* b : off */
#define	CHANB_ON 	0x9b 	/* b : on */
#define	CHANC_OFF 	0x9c 	/* c : off */
#define	CHANC_ON 	0x9d 	/* c : on */
#define	CHAND_OFF 	0x9e 	/* d : off */
#define	CHAND_ON 	0x9f 	/* d : on */

/* reading data */

#define	REQ_CNT0	0xa0 	/* request play counter of track 1 */
#define	REQ_CNT1	0xa1
#define	REQ_CNT2	0xa2
#define	REQ_CNT3	0xa3
#define	REQ_CNT4	0xa4
#define	REQ_CNT5	0xa5
#define	REQ_CNT6	0xa6
#define	REQ_CNT7	0xa7

#define	REQ_REC_CNT 	0xab 	/* request record counter */
#define	REQ_VER	 	0xac 	/* request version */
#define	REQ_REV  	0xad 	/* request revision */
#define	REQ_TEMPO 	0xaf 	/* request tempo */

#define	RES_RTEMPO	0xb1	/* resets relative tempo */
#define	CLEAR_PCOUNT	0xb8	/* clear play counters */
#define	CLEAR_PMAP	0xb9	/* clear play map - all notes off */
#define	CLEAR_REC	0xba	/* clear record counter */

#define	TB_48		0xc2 	/* 48 timebase */
#define	TB_72		0xc3
#define	TB_96		0xc4
#define	TB_120		0xc5
#define	TB_144		0xc6
#define	TB_168		0xc7
#define	TB_192		0xc8

#define	WSD0 		0xd0	/* want to send data on track 1 */
#define	WSD1		0xd1
#define	WSD2		0xd2
#define	WSD3		0xd3
#define	WSD4		0xd4
#define	WSD5		0xd0
#define	WSD6		0xd6
#define	WSD7		0xd7

#define	WS_SYS		0xdf	/* want to send system message */

/* set conditions and values (follow by 1 byte data) */

#define	SET_TEMPO 	0xe0 	/* set tempo */
#define	SET_RTEMPO 	0xe1 	/* relative tempo */
#define	SET_GRAD 	0xe2 	/* graduation */
#define	MIDI_METRO 	0xe4 	/* midi clocks per metronome beep */
#define	METRO_MEAS 	0xe6 	/* metro/meas */
#define	INT_HOST 	0xe7 	/* int * 4 / clock to host */
#define	ACT_TRACK 	0xec 	/* active tracks on/off */
#define	SEND_PCOUNT 	0xed 	/* send play counter on/off */
#define	CHAN_ON1 	0xee 	/* acceptable channels 1 - 8 on/off */
#define	CHAN_ON2 	0xef 	/* acceptable channels 9 - 16 on/off */

#define	RESET		0xff

