//#include "controls.h"
#include "bac_point.h"
#include "user_data.h"
#include "e2prom.h"
#include "bo.h"
#include "ao.h"
 
void control_input(void);
void control_output(void);
S16_T exec_program(S16_T current_prg, U8_T *prog_code);
void update_comport_health(void);
extern void check_output_priority_array(U8_T i,U8_T HOA);
extern void Set_AO_raw(uint8 i,float value);
void set_output_raw(uint8_t point,uint16_t value);
void sample_points( void );
//extern S16_T get_point_value( Point *point, S32_T *val_ptr );
extern U16_T Test[50];

uint8_t get_max_internal_output(void);
/*{
	return MAX_INS;
}*/

Con_aux				 			con_aux[MAX_CONS];

#define PID_SAMPLE_COUNT 20
#define PID_SAMPLE_TIME 10



void pid_controller( S16_T p_number )   // 10s 
{
 /* The setpoint and input point can only be local to the panel even
		though the structure would allow for points from the network segment
		to which the local panel is connected */
	S32_T op, oi, od, err, erp, out_sum;
/*  err = percent of error = ( input_value - setpoint_value ) * 100 /
					proportional_band	*/
//	U8_T sample_time = 10L;       /* seconds */
	Point *pt1 = NULL;
	S32_T value = 0;

	U16_T prop;
	S32_T l1;
	Str_controller_point *con;
	Con_aux *conx;
	static S32_T temp_input_value,temp_setpoint_value;
	
	controllers[p_number].sample_time = PID_SAMPLE_TIME;
	con = &controllers[p_number];
	

	if(con->auto_manual == 1)  // manual - 1 , 0 - auto
		return;

	conx = &con_aux[p_number];

#if 1
	//
	get_point_value( (Point*)&con->input, &con->input_value);
	get_point_value( (Point*)&con->setpoint, &con->setpoint_value );
	od = oi = op = 0;
//	con->proportional = 20;

	prop = con->prop_high;	
	prop <<= 8;
	prop += con->proportional;

	temp_input_value = (con->input_value);
	temp_setpoint_value = (con->setpoint_value); 

	err = temp_input_value - temp_setpoint_value;  /* absolute error */
		
	erp = 0L;

//	con->reset = 20;

/* proportional term*/
	if( prop > 0 )
		erp = 100L * err / prop;
	if( erp > 100000L ) erp = 100000L;
	if( con->action > 0)
		op = erp; /* + */
	else
		op = -erp; /* - */

	erp = 0L;
	
/* integral term	*/
	/* sample_time = 10s */
	l1 = ( conx->old_err + err ) * (con->sample_time / 2); /* 5 = sample_time / 2 */
	l1 += conx->error_area;
	if( conx->error_area >= 0 )
	{
		 if( l1 > 8388607L )
				l1 = 8388607L;
	}
	else
	{
		 if( l1 < -8388607L )
				l1 = -8388607L;
	}
	conx->error_area = l1;
	if( con->reset > 0 )
	{
		if( con->action > 0)  // fix iterm 
			oi = con->reset * conx->error_area;
		else
			oi -= con->reset * conx->error_area;

		if(con->repeats_per_min > 0)
			oi /= 60L;
		else
			oi /= 3600L;
	}
/* differential term	*/
	if( con->rate > 0)
	{
		od = conx->old_err * 100;
		od /= prop;
		od = erp - od;
		if(con->action > 0)
		{
/*			od = ( erp - conx->old_err * 100 / prop ) * con->rate / 600L;
			 600 = sample_time * 60  */
			od *= con->rate;
		}
		else
		{
/*			od = -con->rate * ( erp - conx->old_err
						* 100 / prop ) / 600L;  600 = sample_time * 60  */
			od *= ( -con->rate );
		}
		od /= (con->sample_time * 60); 	/* 600 = sample_time * 60  */
	}

	out_sum = op + con->bias + od / 100; //  od / 100 , because con->rate is 100x

	if( out_sum > 100000L ) out_sum = 100000L;
	if( out_sum < 0 ) out_sum = 0;
	if( con->reset > 0)
	{
		 out_sum += oi;
		 if( out_sum > 100000L )
		 {
				out_sum = 100000L;
		 }
		 if( out_sum < 0 )
		 {
			out_sum = 0;
		 }
	}
	conx->old_err = err;
	con->value = (out_sum);
#endif


}


void check_weekly_routines(void)
{
 /* Override points can only be local to the panel even though the
		structure would allow for points from the network segment to which
		the local panel is connected */
	S8_T w, i, j;
	S32_T value;
	Str_weekly_routine_point *pw = NULL;
	Time_on_off *pt = NULL;
#if 0
	pw = &weekly_routines[0];
	for( i=0; i< MAX_WR; pw++, i++ )
	{
		w = Rtc.Clk.week - 1;
		if( w < 0 ) w = 6;
		if( pw->auto_manual == 0 )  // auto
		{		
			if( pw->override_2.point_type )
			{				
				get_point_value( (Point*)&pw->override_2, &value );
				
				pw->override_2_value = value?1:0;
				if( value )		w = 8;
			}
			else
			 	pw->override_2_value = 0;
		
			if(pw->override_1.point_type)
			{				
				get_point_value( (Point*)&pw->override_1, &value );
				pw->override_1_value = value?1:0;
				if( value )
					w = 7;
			}
			else
			 	pw->override_1_value = 0;
		
		
			pt = &wr_times[i][w].time[2*MAX_INTERVALS_PER_DAY-1];

			j = 2 * MAX_INTERVALS_PER_DAY - 1;
		/*		for( j=2*MAX_INTERVALS_PER_DAY-1; j>=0; j-- )*/
		/*	do
			{				
				pt->hours = 10 + j;
				pt->minutes = 25 + j;
				pt--;
				
			}
			while( --j >= 0 );
			pt = &wr_times[i][w].time[2*MAX_INTERVALS_PER_DAY-1];
			j = 2 * MAX_INTERVALS_PER_DAY - 1; 
		 */

			do
			{				
				//if( pt->hours || pt->minutes )
				if(wr_time_on_off[i][w][j] != 0xff)
				{	
					if( Rtc.Clk.hour > pt->hours ) break;
					if( Rtc.Clk.hour == pt->hours )
						if( Rtc.Clk.min >= pt->minutes )
							break;
				}
				pt--;
				
			}
			while( --j >= 0 );

			if(wr_time_on_off[i][w][j] == 0xff)
				pw->value = 0;
			else
				pw->value = wr_time_on_off[i][w][j];
			
#if 0			
			if( j < 0)		
				pw->value = 0;
			else
			{ 				
				if( j & 1 ) /* j % 2 */
				{
					pw->value = 0;//SetByteBit(&pw->flag,0,weekly_value,1);
				}
				else
				{
					pw->value = 1;//SetByteBit(&pw->flag,1,weekly_value,1);
				}    
			}
#endif
		}
	}
#endif
}


void check_annual_routines( void )
{
	S8_T i;
	S8_T mask;
	S8_T octet_index;
	Str_annual_routine_point *pr;

	pr = &annual_routines[0];
#if 0
	for( i=0; i<MAX_AR; i++, pr++ )
	{
   	if( pr->auto_manual == 0 )
	 	{
			mask = 0x01;
			/* Assume bit0 from octet0 = Jan 1st */
			/* octet_index = ora_current.day_of_year / 8;*/
			octet_index = (Rtc.Clk.day_of_year) >> 3;
			/* bit_index = ora_current.day_of_year % 8;*/    // ????????????????????????????
	/*		bit_index = ora_current.day_of_year & 0x07;*/
			mask = mask << ((Rtc.Clk.day_of_year) & 0x07 );
			
			if( ar_dates[i][octet_index] & mask )
			{
				pr->value = 1;
			}
			else
				pr->value = 0;
	   	}
	}
  //	misc_flags.check_ar=0;
#endif

}


#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)

extern U8_T tst_retry[MAX_ID];

U8_T   output_state_index[32] = {0};
U8_T   first_time_schedual[32] = {0};
U8_T   cycle_minutes_timeout;

U8_T ID_FLAG = 0;
//sbit ID_A_M = ID_FLAG^7; 
//sbit ID_OUTPUT = ID_FLAG^6;  
//sbit ID_STATE1 = ID_FLAG^5;
//sbit ID_STATE2 = ID_FLAG^4;
void SetBit(U8_T bit_number,U8_T *byte)
{
	U8_T mask; 
	mask = 0x01;
	mask = mask << bit_number;
	*byte = *byte | mask;
}

void ClearBit(U8_T bit_number,U8_T *byte)
{
	U8_T mask; 
	mask = 0x01;
	mask = mask << bit_number;
 	mask = ~mask;
	*byte = *byte & mask;
}

U8_T GetBit(U8_T bit_number,U8_T *value)
{
	U8_T mask;
	U8_T octet_index;
	mask = 0x01;
	octet_index = bit_number >> 3;
	mask = mask << (bit_number & 0x07);
	return (U8_T)(*(value + octet_index) & mask) ? 1 : 0;
}


// mode - 0: auto, 1: manual
U8_T SendSchedualData(U8_T mode)
{

	U8_T buf[40];
	U16_T crc_val;
	U16_T length;
	U16_T crc_check;
	U8_T port;
	static U8_T retry = 0;
	static U8_T i = 0;
	STR_Sch_To_T8 * str;
	do
	{
		str = &Sch_To_T8[i];
		if(str->f_send_schdule_data != 0) 
			break;
		
	}
	while(i++ < 100);
	
	if(i >= 100) { i = 0; return 0;}
//	if(str->f_send_schdule_data == 0) 
//	return 0;
	if(retry++ >= 3) 
	{// generate a alarm, sync time error
		
		retry = 0;
		return 0;
	}

	port = get_port_by_id(str->schedule_id);
	if(port == 0) 
	{  // wrong id
	    return 0;
	}
	port = port - 1;

	if(str->en_device_schdule == 0)
	{
		// disable schedule , REG565
		buf[0] = str->schedule_id;
		buf[1] = 0x06;
		buf[2] = 0x02;
		buf[3] = 0x35;  // 565
		buf[4] = 0;
		buf[5] = 0;
		
		crc_val = crc16(buf, 6);
		buf[6] = HIGH_BYTE(crc_val);
		buf[7] = LOW_BYTE(crc_val);

		uart_init_send_com(port);

		uart_send_string(buf,8,port);
		set_subnet_parameters(RECEIVE, 8,port);
		// send successful if receive the reply
		if(length = wait_subnet_response(200,port))
		{
			crc_check = crc16(subnet_response_buf, length - 2);
			if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
			{  
				tst_retry[buf[0]] = 0;
				scan_db_time_to_live[buf[0]] = 100;
				retry = 0;
			}
			else
			{
				tst_retry[buf[0]]++;
				return 0;
			}
		}
		else
		{
			tst_retry[buf[0]]++;
			return 0;
		}
		set_subnet_parameters(SEND, 0,port);
		str->en_device_schdule = 1;
	}
	
		// occ or unocc control
		buf[0] = str->schedule_id;
		buf[1] = 0x06;
		buf[2] = 1;
		buf[3] = 17;  // 273
		buf[4] = 0;
		if(str->schedule_output == 0)
			buf[5] = 0;
		else	
			buf[5] = 4;
		
		crc_val = crc16(buf, 6);
		buf[6] = HIGH_BYTE(crc_val);
		buf[7] = LOW_BYTE(crc_val);

		uart_init_send_com(port);

		uart_send_string(buf,8,port);
		
		set_subnet_parameters(RECEIVE, 8,port);
		// send successful if receive the reply
		if(length = wait_subnet_response(200,port))
		{
			crc_check = crc16(subnet_response_buf, length - 2);
			if(crc_check == subnet_response_buf[length - 2] * 256 + subnet_response_buf[length - 1])
			{  
				retry = 0;
			}
			else
			{
				 return 0;
			}
		}
		else
		{
			 return 0;
		}
		set_subnet_parameters(SEND, 0,port);
	
	
		if(str->schedule_output != 0) 
		{
			if(GetBit(str->schedule_id - 1, output_state_index) == 0)
			{
				SetBit((str->schedule_id - 1) & 0x07, &output_state_index[(str->schedule_id - 1) >> 3]);
				//ID_CHANGED = 0;
				ID_FLAG &= 0xbf;
			}
		}
		else 
		{
			if(GetBit(str->schedule_id - 1 , output_state_index) == 1)
			{
				ClearBit((str->schedule_id - 1) & 0x07, &output_state_index[(str->schedule_id - 1) >> 3]);
				//ID_CHANGED = 0;
				ID_FLAG &= 0xbf;
			}
		}

	str->f_send_schdule_data = 0;

	return 1;
}




void CheckIdRoutines(void)
{
	U8_T i;
	U8_T wr1_value = 0;
	U8_T output_value = 0;
	U8_T temp_bit;
	U8_T wr1_valid;
	U8_T id_index;
	
	for(i = 0; i < MAX_ID; i++)
	{
		if((ID_Config[i].Str.id > 0) && (ID_Config[i].Str.schedule > 0))
		{
			id_index = ID_Config[i].Str.id - 1;
			if(ID_Config[i].all[0] != 0xff)
			{
				ID_FLAG = ID_Config[i].Str.flag;
				wr1_valid = 0;

				if(ID_Config[i].Str.schedule > 0 && ID_Config[i].Str.schedule <= MAX_WR)
				{
					wr1_value = weekly_routines[ID_Config[i].Str.schedule - 1].value;
					wr1_valid = 1;
					
				}

				//if(!ID_A_M)  
				if(!(ID_FLAG & 0x80)) // AUTO
				{
#if 1			
					if(wr1_valid)
					{	
						Sch_To_T8[i].schedule_output = wr1_value;
						// sync ID[].flag = 
						if(wr1_value)
							ID_Config[i].Str.flag |= 0x40;
						else
							ID_Config[i].Str.flag &= 0xbf;
						
						output_value = GetBit(id_index, output_state_index); 
						temp_bit = GetBit(id_index, first_time_schedual);
						if(output_value != (wr1_value) || temp_bit == 0)
						{ 
						
							// wait send_schdule_data
							Sch_To_T8[i].schedule_id = id_index + 1;
							Sch_To_T8[i].schedule_index = ID_Config[i].Str.schedule;
							//Sch_To_T8[i].schedule_output = wr1_value;
							
							Sch_To_T8[i].f_send_schdule_data = 1;
							if(temp_bit == 0)
							{
								Sch_To_T8[i].f_schedule_sync = 1;
								Sch_To_T8[i].count_send_schedule = 0;
								Sch_To_T8[i].f_time_sync = 1;
								SetBit(id_index & 0x07, &first_time_schedual[id_index >> 3]);
							}							
						
						}
						else if(cycle_minutes_timeout == 1)
						{  // ????????????
							//SendSchedualData(id, wr1_value | wr2_value);
							
							Sch_To_T8[i].schedule_id = id_index + 1;						
							Sch_To_T8[i].schedule_output = wr1_value;
							Sch_To_T8[i].f_send_schdule_data = 1;  
							if(id_index == MAX_ID - 1)
								cycle_minutes_timeout = 0;
						}
					}
#endif
				}
				else // MAN
				{

					if(ID_FLAG != 0xff)  
					{
					#if 1
						temp_bit = GetBit(id_index, first_time_schedual);
						
						if(((ID_FLAG & 0x40) ? 1 : 0) != GetBit(id_index,output_state_index) || temp_bit == 0)
						{ 
							if(temp_bit == 0)
								SetBit(id_index & 0x07, &first_time_schedual[id_index >> 3]);
													 
						//	SendSchedualData(id, ID_FLAG & 0x40);
							
							Sch_To_T8[i].schedule_id = id_index + 1;						
							Sch_To_T8[i].schedule_output = (ID_FLAG & 0x40) ? 1 : 0;
							Sch_To_T8[i].f_send_schdule_data = 2;
						}
						else if(cycle_minutes_timeout == 1)
						{
	//						test0 = 9;
	//						SendSchedualData(id, ID_FLAG & 0x40);
						
							Sch_To_T8[i].schedule_id = id_index;						
							Sch_To_T8[i].schedule_output = (ID_FLAG & 0x40) ? 1 : 0;
							Sch_To_T8[i].f_send_schdule_data = 2;
							if(id_index == (MAX_ID - 1))
								cycle_minutes_timeout = 0;
						}
					#endif
					}
				}
			} // check if the correspond ID has been used
		} // for 
	}
} // main function

#endif


void check_graphic_element(void)
{
	U8_T i;
	for(i = 0;i < MAX_GRPS;i++)
	{
		control_groups[i].element_count = 0;
	}
	
	for(i = 0;i < MAX_ELEMENTS;i++)
	{		
		if(group_data[i].reg.label_status == 0)
				break;
		
		if(group_data[i].reg.label_status == 1)  // current element is valid
		{
			if(group_data[i].reg.nScreen_index < MAX_GRPS)
			{
				control_groups[group_data[i].reg.nScreen_index].element_count++;
			}
		}			
	}
}

//1a 00 74 00 00 13 00 0c 00 01 0a 00 09 9c 02 12 9d a0 86 01 00 fe 00
//10  VAR1 = 100  TST1-SETPOINT = 100
//const U8_T code test_prg_code[30] = {
//0x1d,0x00,0x74,0x00,0x00,0x16,0x00,0x0f,0x00,0x01,
//0x0a,0x00,0x09,0x9E,0x55,0xaa,0x0f,0x02,0x12,0x9d,0xa0,0x86,0x01,0x00,0xfe};


//24 00 74 00 00 1d 00 16 00 01 0a 00 09 9c 00 03 9d a0 86 01 00 01 14 00 09 9c 01 03 9c 02 03 fe 00
// 10  VAR1 = 100
//20  VAR2 = VAR3
//const U8_T test_prg_code[30] = {
//0x1a,0x00,0x74,0x00,0x00,0x13,0x00,0x0c,0x00,0x01,
//0x0a,0x00,0x09,0x9c,0x02,0x12,0x9d,0xa0,0x86,0x01,0x00,0xfe};
//const U8_T test_prg_code[50] = {
//0x24,0x00 ,0x74 ,0x00 ,0x00 ,0x1d ,0x00 ,0x16 ,0x00 ,0x01 ,0x0a ,0x00 ,0x09 ,0x9c ,0x02, 
//0x12 ,0x9d ,0xa0 ,0x86 ,0x01 ,0x00 ,0x01 ,0x14 ,0x00 ,0x09 ,0x9c ,0x01 ,0x03 ,0x9c ,0x05,0x12 ,0xfe ,0x00
//};


void Ethernet_Debug_Task();
U8_T count_10s = 0;
U8_T count_reset_zigbee = 0;
U16_T last_reset_zigbee;
U8_T count_1min = 0;	
U8_T count_3s = 0;
U8_T count_1s = 0;
U8_T current_day;
//U8_T count_pid[MAX_CONS] = 0;
//uint8_t flag_load_prg;
//uint8_t count_load_prg;
void Check_All_WR(void);
U8_T check_whehter_running_code(void);
extern U8_T flag_writing_code;
extern U8_T count_wring_code;
extern U8_T max_dos;
extern U8_T max_aos;
extern U16_T output_raw[MAX_OUTS];

#define		SW_OFF 	 0
#define 	SW_HAND	 2
#define		SW_AUTO	 1
void vTaskDelay( const uint32_t xTicksToDelay );	

#if 0
void Bacnet_Control(void) 
{
	U16_T i,j;
	U8_T decom;
	static U8_T count_wait_sample;

	count_reset_zigbee = 0;
//	count_1s = 0;
	count_10s = 0;
	count_1min = 0;	
	count_3s = 0;
	current_day = 0;
//	memset(count_pid,0,MAX_CONS);
//	count_wait_sample = 0;
	check_graphic_element();
	for(i = 0;i < MAX_OUTS;i++)
	{		
		check_output_priority_array(i,0);
#if OUTPUT_DEATMASTER
			clear_dead_master();
#endif	
	}

	flag_writing_code = 1;
	count_wring_code = 5;
	Check_All_WR();
	for(;;)
  {
		
		
		//vTaskDelayUntil( &xLastWakeTime,500 );
		/* deal with exec_program roution per 1s */	
			
		control_input();


#ifdef ETHERNET_DEBUG
		Ethernet_Debug_Task();
#endif

#if (ARM_MINI || ARM_TSTAT_WIFI)
		Check_spd_count_led();
#endif
		
		/*if((run_time > 60) && (reboot_counter != 0))
		{
			reboot_counter = 0;
			E2prom_Write_Byte(EEP_REBOOT_COUNTER,0);
		}*/		
		
		//ptr = &programs[0];
		if(check_whehter_running_code() == 1)
		for( i = 0; i < MAX_PRGS; i++/*, ptr++*/ )
		{
			if( programs[i].bytes )
			{
				if(programs[i].on_off	== 1)  
				{ 
					uint32_t t1 = 0;
					uint32_t t2 = 0;
					// add checking running time of program			

#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )					
					t1 = uip_timer;
#endif
					exec_program( i, prg_code[i]);
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI )							
					t2 = uip_timer;
					programs[i].costtime = (t2 - t1) + 1;
#endif					

				}
				else
					programs[i].costtime = 0;
			}
			else
					programs[i].costtime = 0;
			
		}

		control_output();
// check whether external IO are on line
		/*for(i = 0;i < sub_no;i++)
		{
			if(current_online[sub_map[i].id / 8] & (1 << (sub_map[i].id % 8)))			
			{
				decom = 0;
			}
			else  // off line
			{
				decom = 1;
			}
			
			for(j = 0; j < sub_map[i].do_len;j++)
			{
					outputs[sub_map[i].do_start + j].decom = decom;
			}
			for(j = 0; j < sub_map[i].ao_len;j++)
			{
					outputs[sub_map[i].ao_start + j].decom = decom;
			}
		}*/			
		count_10s++;  // 1s
		if(count_10s >= PID_SAMPLE_COUNT) // 500MS * PID_SAMPLE_COUNT == PID_SAMPLE_TIME 10S
		{
	   // dealwith controller roution per 1 sec	
			for(i = 0;i < MAX_CONS; i++)
			{					
				pid_controller( i );
			}
			count_10s = 0;
#if (ARM_MINI || ARM_TSTAT_WIFI)
			Store_Pulse_Counter(0);
			calculate_RPM();
#endif			
		}	 
		// dealwith check_weekly_routines per 1 min

		if(count_1min < 5) 	count_1min++;
		else
		{		
			count_1min = 0;
			check_weekly_routines();
			check_annual_routines();			
		}
#if (ARM_MINI || ARM_CM5 || ARM_TSTAT_WIFI)		
		CheckIdRoutines();
#endif
	
#if (ARM_MINI || ASIX_MINI)
		if(Modbus.com_config[1] == 7)
		{
			if(count_reset_zigbee < 30) 	count_reset_zigbee++;
			else
			{		
				last_reset_zigbee++;
				count_reset_zigbee = 0;
			
	//  only for big minipanel, com1 must select rs232 jumper
				
				Check_Zigbee();
		
			}	

		}
	
#endif

		// count monitor task
		if(just_load)
			just_load = 0;
		
		//if( count_1s++  >= 1)
		{
			miliseclast_cur = miliseclast;
			miliseclast = 0;		
    }
//		else
//			count_1s = 0;
		


		
	//	taskYIELD();
	}		
	
}
#endif

void check_trendlog_1s(unsigned char count)
{
	//static U16_T count_wait_sample = 0;
	static U8_T count_1s = 0;
	// wait 5 second, after input value is ready

	//if(count_wait_sample >= 100)
	{
		//count_wait_sample = 0;

		//if(count_1s++ % count == 0)
		{
			count_1s = 0;
			// tbd: sample

		sample_points();	
#if BAC_TRENDLOG
		trend_log_timer(0);
#endif
		//update_comport_health();
		}
		
	}
	//else
	//	count_wait_sample++;
}

void Set_AO_raw(uint8 i,float value)
{		
	switch( outputs[i].range )
	{
		case V0_10:				
			set_output_raw(i,value / 10);
			break;
		case P0_100_Open:
		case P0_100_Close:
		case P0_100:
		case P0_100_PWM:	
			set_output_raw(i,value / 100);
			break;
		case P0_20psi:
		case I_0_20ma:	
			set_output_raw(i,value / 20);
			break;
		case P0_100_2_10V:
			set_output_raw(i,200 + value * 8 / 1000);
			break;
		default:
			set_output_raw(i,value / 1000);
			break; 				 			
		
	}	
}

void check_output_priority_HOA(U8_T i)
{	
	// check swtich status
		if(outputs[i].switch_status == SW_OFF)
		{	
#if OUTPUT_DEATMASTER
			clear_dead_master();
#endif						
			if(outputs[i].digital_analog == 0)
			{									
				if(( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
				||(outputs[i].range >= custom_digital1 // customer digital unit
				&& outputs[i].range <= custom_digital8
				&& digi_units[outputs[i].range - custom_digital1].direct == 1))
				{ // inverse
					output_priority[i][6] = 1;	

					if(i < max_dos)
						outputs[i].control = Binary_Output_Present_Value(i) ? 0 : 1;
					else
						outputs[i].control = Analog_Output_Present_Value(i) ? 0 : 1;
				}
				else
				{
					output_priority[i][6] = 0;	
					if(i < max_dos)
						outputs[i].control = Binary_Output_Present_Value(i) ? 1 : 0;
					else
						outputs[i].control = Analog_Output_Present_Value(i) ? 1 : 0;
					
				}				
				if(i < max_dos)
				{
					outputs[i].value = (Binary_Output_Present_Value(i) * 1000);					
				}
				else
				{
					outputs[i].value = (Analog_Output_Present_Value(i) * 1000);					
				}
				
				if(outputs[i].control) 
					set_output_raw(i,1000);
				else 
					set_output_raw(i,0);	
			}
			else
			{
				output_priority[i][6] = 0;
				outputs[i].control = 0;
				
				if(i < max_dos)
				{
					outputs[i].value = (Binary_Output_Present_Value(i) * 1000);					
					set_output_raw(i,Binary_Output_Present_Value(i) * 1000);
				}
				else
				{
					outputs[i].value = (Analog_Output_Present_Value(i) * 1000);					
					Set_AO_raw(i,(outputs[i].value));			
				}
			}
		}
		else if(outputs[i].switch_status == SW_HAND)
		{
#if OUTPUT_DEATMASTER
			clear_dead_master();
#endif			
			if(outputs[i].digital_analog == 0)
			{
				if(( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
					||(outputs[i].range >= custom_digital1 // customer digital unit
					&& outputs[i].range <= custom_digital8
					&& digi_units[outputs[i].range - custom_digital1].direct == 1))
				{// inverse
					output_priority[i][6] = 0;	
					if(i < max_dos)
						outputs[i].control = Binary_Output_Present_Value(i) ? 0 : 1;	
					else
						outputs[i].control = Analog_Output_Present_Value(i) ? 0 : 1;
				}
				else
				{
					output_priority[i][6] = 1;
					if(i < max_dos)					
						outputs[i].control = Binary_Output_Present_Value(i) ? 1 : 0;	
					else
						outputs[i].control = Analog_Output_Present_Value(i) ? 1 : 0;	
				}	
				
				if(i < max_dos)
					outputs[i].value = (Binary_Output_Present_Value(i) * 1000);
				else
					outputs[i].value = (Analog_Output_Present_Value(i) * 1000);

				if(outputs[i].control) 
					set_output_raw(i,1000);
				else 
					set_output_raw(i,0);	
			
			}
			else
			{
				switch( outputs[i].range )
				{
					case V0_10:	
						output_priority[i][6] = 10;
						break;
					case P0_100_Open:
					case P0_100_Close:
					case P0_100:
					case P0_100_PWM:	
					case P0_100_2_10V:
						output_priority[i][6] = 100;
						break;
					case P0_20psi:
					case I_0_20ma:	
						output_priority[i][6] = 20;
						break;
					default:
						output_priority[i][6] = (outputs[i].value) / 1000;
						break; 				 			
					
				}						
				
				if(i < max_dos)
					outputs[i].value = (Binary_Output_Present_Value(i) * 1000);
				else
					outputs[i].value = (Analog_Output_Present_Value(i) * 1000);

				Set_AO_raw(i,(outputs[i].value));					
			}
		}
		else if(outputs[i].switch_status == SW_AUTO)// auto
		{//Test[16]++;
			output_priority[i][6] = 0xff;	
			check_output_priority_array(i,1);
		}
		else
		{
			// error status
//			Test[30]++;
		}

}



// CHECK A/M
// HOA: 1 -> �ı�HOA�����level7ֵ����
//			0 -> modbus����bancent���ֱ�Ӹı� leve7 ֵ�ı�
void check_output_priority_array(U8_T i,U8_T HOA)
{	
	if(i >= max_dos + max_aos)
	{
		output_priority[i][7] = 0xff;	
	}
	else
	{
		if(outputs[i].auto_manual == 0)  // auto , pirotry_array level 8 is NULL
		{
			output_priority[i][7] = 0xff;	
			if(outputs[i].digital_analog == 0)
			{	// digital
				//if(i < max_dos)
				{
					outputs[i].value = (Binary_Output_Present_Value(i) * 1000); 

					if(( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
					||(outputs[i].range >= custom_digital1 // customer digital unit
					&& outputs[i].range <= custom_digital8
					&& digi_units[outputs[i].range - custom_digital1].direct == 1))
					{// inverse
						outputs[i].control = Binary_Output_Present_Value(i) ? 0 : 1;	
					}	
					else
					{
						outputs[i].control = Binary_Output_Present_Value(i) ? 1 : 0;
					}
				}
				
				// when OUT13-OUT24 are used for DO
				if(outputs[i].control) 
				{					
					set_output_raw(i,1000);
				}
				else 
				{
					set_output_raw(i,0);	
				}
			}
			else
			{
				if(i < get_max_internal_output())
				{	
					outputs[i].value = (Analog_Output_Present_Value(i) * 1000);
					// set output_raw
					Set_AO_raw(i,(outputs[i].value));
				}
				else
				{
					output_raw[i] = (outputs[i].value);
				}					
			}

		} // else manual
		else
		{		
#if OUTPUT_DEATMASTER
			clear_dead_master();
#endif			
			if(outputs[i].digital_analog == 0)
			{	// digital
				if(HOA == 0)
				{
					if(outputs[i].control == 1)
						output_priority[i][7] = 1;
					else
						output_priority[i][7] = 0;
				}
				if(i < max_dos)
					outputs[i].value = (Binary_Output_Present_Value(i) * 1000); 
				else
					outputs[i].value = (Analog_Output_Present_Value(i) * 1000);
				
				if(( outputs[i].range >= ON_OFF && outputs[i].range <= HIGH_LOW )
					||(outputs[i].range >= custom_digital1 // customer digital unit
					&& outputs[i].range <= custom_digital8
					&& digi_units[outputs[i].range - custom_digital1].direct == 1))
				{// inverse
					outputs[i].control = Binary_Output_Present_Value(i) ? 0 : 1;	
				}	
				else
				{
					outputs[i].control = Binary_Output_Present_Value(i) ? 1 : 0;
				}
				
				
				if(outputs[i].control) 
					set_output_raw(i,1000);
				else 
					set_output_raw(i,0);	
			}		
			else
			{  // analog
//				if(i < max_dos)
//				{
//					if(HOA == 0)
//						output_priority[i][7] = (float)(outputs[i].value) / 1000;
//					outputs[i].value = (Binary_Output_Present_Value(i) * 1000); 
//					set_output_raw(i,Binary_Output_Present_Value(i) * 1000);
//				}
//				else
				{
					if(HOA == 0)
						output_priority[i][7] = (float)(outputs[i].value) / 1000;
					if(i < get_max_internal_output())
					{	
						outputs[i].value = (Analog_Output_Present_Value(i) * 1000);
						// set output_raw
						Set_AO_raw(i,(outputs[i].value));
					}
					else
					{
						output_raw[i] = (outputs[i].value);
					}
				}					
			}
		}	
	}	
	
}


// dont check A/M
void check_output_priority_array_without_AM(U8_T i)
{	
	
	if(i >= max_dos + max_aos)
	{
		output_priority[i][7] = 0xff;	
	}
	else
	{		
		if(outputs[i].digital_analog == 0)
		{	// digital
			if(i < max_dos)
			{
				outputs[i].value = (Binary_Output_Present_Value(i) * 1000);
			}				
			else
			{
				outputs[i].value = (Analog_Output_Present_Value(i) * 1000);
			}
				
			if(outputs[i].control) 
				set_output_raw(i,1000);
			else 
				set_output_raw(i,0);	
		}		
		else
		{  // analog
			if(i < max_dos)
			{
				outputs[i].value = (Binary_Output_Present_Value(i) * 1000); 
				set_output_raw(i,Binary_Output_Present_Value(i) * 1000);
			}
			else
			{
				if(i < get_max_internal_output())
				{
					outputs[i].value = (Analog_Output_Present_Value(i) * 1000);
					// set output_raw
					Set_AO_raw(i,(outputs[i].value));
				}
				else
				{
					output_raw[i] = (outputs[i].value);
				}
			}	
			
		}

	}	
	
}

U8_T   output_pri_live[MAX_OUTS];


void Check_Program_Output_Pri_valid(void)
{
	U8_T i;
	for(i = 0;i < MAX_OUTS;i++)
	{
		if(output_priority[i][9] != 0xff)
		{
			if(output_pri_live[i] > 0)
				output_pri_live[i]--;
			else
				output_priority[i][9] = 0xff;
		}
	}
	
}



#if OUTPUT_DEATMASTER
U32_T   count_dead_master = 0;	

void clear_dead_master(void)
{
	count_dead_master = 0;	
}


void output_dead_master(void)
{
	U8_T i,j;
	if(Modbus.dead_master != 0)  
	{
		if(count_dead_master++ > Modbus.dead_master  * 60)
		{
			// go to relinquish
			count_dead_master = 0;
			for(i = 0;i < MAX_OUTS;i++)
			{
				for(j = 0;j < 16;j++)
					output_priority[i][j] = 0xff;
				check_output_priority_array(i,0);
			}
		}
			
	}
}

#endif


