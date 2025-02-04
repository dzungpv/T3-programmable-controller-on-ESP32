/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

/* Binary Output Objects - customize for your use */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bacdef.h"
#include "bacnet.h"
#include "bacdcode.h"
#include "bacenum.h"
#include "bacapp.h"
#include "rp.h"
#include "wp.h"
#include "bo.h"
#include "handlers.h"



#if BAC_COMMON

/* When all the priorities are level null, the present value returns */
/* the Relinquish Default value */
#define RELINQUISH_DEFAULT BINARY_INACTIVE
/* Here is our Priority Array.*/
//BACNET_BINARY_PV   far BO_Present_Value[MAX_BOS][BACNET_MAX_PRIORITY];
/* Writable out-of-service allows others to play with our Present Value */
/* without changing the physical output */
//static bool Binary_Output_Out_Of_Service_Array[MAX_BOS];
uint8_t  BOS;
/* These three arrays are used by the ReadPropertyMultiple handler */

static
#if ARM
const 
#endif
int Properties_Required[] = {
    PROP_OBJECT_IDENTIFIER,
    PROP_OBJECT_NAME,
    PROP_OBJECT_TYPE,
    PROP_PRESENT_VALUE,
    PROP_STATUS_FLAGS,
    PROP_EVENT_STATE,
    PROP_OUT_OF_SERVICE,
    PROP_POLARITY,
    PROP_PRIORITY_ARRAY,
    PROP_RELINQUISH_DEFAULT,
    -1
};

static
#if ARM
 const 
#endif
int Properties_Optional[] = {
    PROP_DESCRIPTION,
    PROP_ACTIVE_TEXT,
    PROP_INACTIVE_TEXT,
    -1
};

static
#if ARM
 const 
#endif
int Properties_Proprietary[] = {
    -1
};

void Binary_Output_Property_Lists(
    const int **pRequired,
    const int **pOptional,
    const int **pProprietary)
{
	
#if ASIX

	Properties_Required[0] = PROP_OBJECT_IDENTIFIER;
	Properties_Required[1] = PROP_OBJECT_NAME;
	Properties_Required[2] = PROP_OBJECT_TYPE;
	Properties_Required[3] = PROP_PRESENT_VALUE;
	Properties_Required[4] = PROP_STATUS_FLAGS;
	Properties_Required[5] = PROP_EVENT_STATE;
	Properties_Required[6] = PROP_UNITS;
	Properties_Required[7] = PROP_OUT_OF_SERVICE;
	Properties_Required[8] = PROP_POLARITY;
	Properties_Required[9] = PROP_PRIORITY_ARRAY;
	Properties_Required[10] = PROP_RELINQUISH_DEFAULT;
	Properties_Required[11] = -1;
	
	Properties_Optional[0] = PROP_DESCRIPTION;
	Properties_Optional[1] = PROP_ACTIVE_TEXT;
	Properties_Optional[2] = PROP_INACTIVE_TEXT;
	Properties_Optional[3] = -1;
	
	Properties_Proprietary[0] = -1;
#endif
    if (pRequired)
        *pRequired = Properties_Required;
    if (pOptional)
        *pOptional = Properties_Optional;
    if (pProprietary)
        *pProprietary = Properties_Proprietary;

    return;
}

//void Binary_Output_Init(
//    void)
//{
//    unsigned i, j;
////    static bool initialized = false;

////    if (!initialized) {
////        initialized = true;

//        /* initialize all the analog output priority arrays to NULL */
//        for (i = 0; i < MAX_BOS; i++) {
//            for (j = 0; j < BACNET_MAX_PRIORITY; j++) {
//                BO_Present_Value[i][j] = BINARY_NULL;
//            }
//        }
////    }

//    return;
//}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need validate that the */
/* given instance exists */
bool Binary_Output_Valid_Instance(
    uint32_t object_instance)
{
    if ((object_instance < MAX_BOS + OBJECT_BASE)/* && (object_instance >= OBJECT_BASE)*/)
        return true;

    return false;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then count how many you have */
unsigned Binary_Output_Count(
    void)
{
	 return BOS;
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the instance */
/* that correlates to the correct index */
uint32_t Binary_Output_Index_To_Instance(
    unsigned index)
{
#ifdef T3_CON
		return BO_Index_To_Instance[index] + OBJECT_BASE;
#else
    return index + OBJECT_BASE;
#endif
}

/* we simply have 0-n object instances.  Yours might be */
/* more complex, and then you need to return the index */
/* that correlates to the correct instance number */
unsigned Binary_Output_Instance_To_Index(
    uint32_t object_instance)
{
#ifdef T3_CON
		return BO_Instance_To_Index[object_instance - OBJECT_BASE];
#else
    return object_instance - OBJECT_BASE;
#endif
}

#if EXTERNAL_IO
uint8_t Get_BOx_by_index(uint8_t index,uint8_t *bo_index);

BACNET_BINARY_PV Binary_Output_Present_Value(
    uint32_t object_instance)
{
    BACNET_BINARY_PV far value = RELINQUISH_DEFAULT;
    uint8_t index = 0;
    unsigned i = 0;

		
		Get_BOx_by_index(object_instance,&index);
	// index is box
		value = Get_Output_Relinguish(BO,index);
	
    if (index < MAX_BOS) {
//		 value = BO_Present_Value[index];
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (Get_bacnet_value_from_buf(BO,i,index) != BINARY_NULL) {

                value = Get_bacnet_value_from_buf(BO,i,index);

                break;
            }
        }
    }

    return value;
}
#endif


BACNET_BINARY_PV Binary_Output_Present_Value1(
    uint32_t object_instance)
{
    BACNET_BINARY_PV far value = RELINQUISH_DEFAULT;
    uint8_t index = 0;
    unsigned i = 0;

		index = object_instance;

		value = Get_Output_Relinguish(BO,index);
    if (index < MAX_BOS) {
//		 value = BO_Present_Value[index];
        for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
            if (Get_bacnet_value_from_buf(BO,i,index) != BINARY_NULL) {

                value = Get_bacnet_value_from_buf(BO,i,index);

                break;
            }
        }
    }

    return value;
}



/* note: the object name must be unique within this device */
bool Binary_Output_Object_Name(
    uint32_t object_instance,
    BACNET_CHARACTER_STRING * object_name)
{
    bool status = false;
		unsigned index = 0;

    index = Binary_Output_Instance_To_Index(object_instance);
	
    if (object_instance < MAX_BOS) {
        status = characterstring_init_ansi(object_name, get_label(BO,index));
    }

    return status;
}



/* return apdu len, or BACNET_STATUS_ERROR on error */
// READ
int Binary_Output_Encode_Property_APDU
	(uint8_t * apdu,
    uint32_t object_instance,
    BACNET_PROPERTY_ID property,
    uint32_t array_index,
    BACNET_ERROR_CLASS * error_class,
    BACNET_ERROR_CODE * error_code)
{
    int far len = 0;
    int far apdu_len = 0;   /* return value */
    BACNET_BIT_STRING far bit_string;
    BACNET_CHARACTER_STRING far char_string;
    BACNET_BINARY_PV far present_value = BINARY_INACTIVE;
    BACNET_POLARITY far polarity = POLARITY_NORMAL;
    unsigned far object_index = 0;
    unsigned far i = 0;
    bool far state = false;
//    uint8_t *apdu = NULL;

//    if ((rpdata == NULL) || (rpdata->application_data == NULL) ||
//        (rpdata->application_data_len == 0)) {
//        return 0;
//    }

  //  apdu = rpdata->application_data;
	
		object_index =	Binary_Output_Instance_To_Index(object_instance);
    switch (property) {
        case PROP_OBJECT_IDENTIFIER:
            apdu_len = encode_application_object_id(&apdu[0], OBJECT_BINARY_OUTPUT,object_index);
            break;
            /* note: Name and Description don't have to be the same.
               You could make Description writable and different */
        case PROP_OBJECT_NAME:
						characterstring_init_ansi(&char_string,get_label(BO,object_index));
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
						break;
        case PROP_DESCRIPTION:
						characterstring_init_ansi(&char_string,get_description(BO,object_index));
            apdu_len = encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_OBJECT_TYPE:
            apdu_len = encode_application_enumerated(&apdu[0], OBJECT_BINARY_OUTPUT);
            break;
        case PROP_PRESENT_VALUE:
						present_value = Binary_Output_Present_Value1(object_index);
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
            break;
        case PROP_STATUS_FLAGS:
            /* note: see the details in the standard on how to use these */
            bitstring_init(&bit_string);
            bitstring_set_bit(&bit_string, STATUS_FLAG_IN_ALARM, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_FAULT, false);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OVERRIDDEN, true);
            bitstring_set_bit(&bit_string, STATUS_FLAG_OUT_OF_SERVICE, true);
            apdu_len = encode_application_bitstring(&apdu[0], &bit_string);
            break;
        case PROP_EVENT_STATE:
            /* note: see the details in the standard on how to use this */
            apdu_len =
                encode_application_enumerated(&apdu[0], EVENT_STATE_NORMAL);
            break;
        case PROP_OUT_OF_SERVICE: 
					// when it is true, the vaule is not changed by system, equal MANUAL
				  // when it is false, equal AUTO

					//  get auto_manual status
						
            state = Get_Out_Of_Service(BO,object_index); //Binary_Output_Out_Of_Service_Array[object_index];
            apdu_len = encode_application_boolean(&apdu[0], state);

            break;
        case PROP_POLARITY:
            apdu_len = encode_application_enumerated(&apdu[0], 
							Binary_Output_Polarity(object_index));
            break;

        case PROP_PRIORITY_ARRAY:
            /* Array element zero is the number of elements in the array */
            if (array_index == 0)
                apdu_len =
                    encode_application_unsigned(&apdu[0], BACNET_MAX_PRIORITY);
            /* if no index was specified, then try to encode the entire list */
            /* into one packet. */
            else if (array_index == BACNET_ARRAY_ALL) { 
                for (i = 0; i < BACNET_MAX_PRIORITY; i++) {
                    /* FIXME: check if we have room before adding it to APDU */
                    if (Get_bacnet_value_from_buf(BO,i,object_index)/*BO_Present_Value[object_index][i]*/ == BINARY_NULL)
                        len = encode_application_null(&apdu[apdu_len]);
                    else {
                        present_value = Get_bacnet_value_from_buf(BO,i,object_index);//BO_Present_Value[object_index][i];
                        len =
                            encode_application_enumerated(&apdu[apdu_len],
                            present_value);
                    }
                    /* add it if we have room */
                    if ((apdu_len + len) < MAX_APDU)
                        apdu_len += len;
                    else {
                        *error_class = ERROR_CLASS_SERVICES;
                        *error_code = ERROR_CODE_NO_SPACE_FOR_OBJECT;
                        apdu_len = BACNET_STATUS_ERROR;
                        break;
                    }
                }
            } else { 
                if (array_index <= BACNET_MAX_PRIORITY) { 
                    if (Get_bacnet_value_from_buf(BO,array_index - 1,object_index)
											/*BO_Present_Value[object_index][array_index - 1]*/ == BINARY_NULL)
                        apdu_len = encode_application_null(&apdu[apdu_len]);
                    else {
                        present_value = Get_bacnet_value_from_buf(BO,array_index - 1,object_index)
											/*BO_Present_Value[object_index][array_index - 1]*/;
                        apdu_len =
                            encode_application_enumerated(&apdu[apdu_len],
                            present_value);
                    }
                } else {
                    *error_class = ERROR_CLASS_PROPERTY;
                    *error_code = ERROR_CODE_INVALID_ARRAY_INDEX;
                    apdu_len = BACNET_STATUS_ERROR;
                }
            }

            break;

        case PROP_RELINQUISH_DEFAULT:
            present_value = Get_Output_Relinguish(BO,object_index);//RELINQUISH_DEFAULT;
            apdu_len = encode_application_enumerated(&apdu[0], present_value);
            break;
        case PROP_ACTIVE_TEXT:
            characterstring_init_ansi(&char_string, "on");
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
        case PROP_INACTIVE_TEXT:
            characterstring_init_ansi(&char_string, "off");
            apdu_len =
                encode_application_character_string(&apdu[0], &char_string);
            break;
				 case PROP_UNITS:
						apdu_len = encode_application_enumerated(&apdu[0], get_range(BO,object_index));

            break;
        default:
            *error_class = ERROR_CLASS_PROPERTY;
            *error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            apdu_len = BACNET_STATUS_ERROR;
            break;
    }
    /*  only array properties can have array options */
    if ((apdu_len >= 0) && (property != PROP_PRIORITY_ARRAY) &&
        (array_index != BACNET_ARRAY_ALL)) {
        *error_class = ERROR_CLASS_PROPERTY;
        *error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        apdu_len = BACNET_STATUS_ERROR;
    }
    return apdu_len;
}



/* returns true if successful */
// WRITE
bool Binary_Output_Write_Property(
    BACNET_WRITE_PROPERTY_DATA * wp_data)
{
    bool far status = false;        /* return value */
    unsigned int far object_index = 0;
    unsigned int far priority = 0;
    BACNET_BINARY_PV far level = BINARY_NULL;
    int far len = 0;
    BACNET_APPLICATION_DATA_VALUE far value;

    /* decode the some of the request */
    len =
        bacapp_decode_application_data(wp_data->application_data,
        wp_data->application_data_len, &value);
    /* FIXME: len < application_data_len: more data? */
    if (len < 0) {
        /* error while decoding - a value larger than we can handle */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
        return false;
    }
    if ((wp_data->object_property != PROP_PRIORITY_ARRAY) &&
        (wp_data->array_index != BACNET_ARRAY_ALL)) {
        /*  only array properties can have array options */
        wp_data->error_class = ERROR_CLASS_PROPERTY;
        wp_data->error_code = ERROR_CODE_PROPERTY_IS_NOT_AN_ARRAY;
        return false;
    }
		
		object_index = Binary_Output_Instance_To_Index(wp_data->object_instance);
		
    switch (wp_data->object_property) {
        case PROP_PRESENT_VALUE:
            if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
                priority = wp_data->priority;
                /* Command priority 6 is reserved for use by Minimum On/Off
                   algorithm and may not be used for other purposes in any
                   object. */
                if (priority && (priority <= BACNET_MAX_PRIORITY) &&
                    (priority != 6 /* reserved */ ) &&
                    (value.type.Enumerated <= MAX_BINARY_PV)) {
                    level = (BACNET_BINARY_PV) value.type.Enumerated;
                    priority--;

										wirte_bacnet_value_to_buf(BO,priority,object_index,level);

                    /* Note: you could set the physical output here if we
                       are the highest priority.
                       However, if Out of Service is TRUE, then don't set the
                       physical output.  This comment may apply to the
                       main loop (i.e. check out of service before changing output) */
                    status = true;
                } else if (priority == 6) {
                    /* Command priority 6 is reserved for use by Minimum On/Off
                       algorithm and may not be used for other purposes in any
                       object. */
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
                } else {
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            } else {
                status =
                    WPValidateArgType(&value, BACNET_APPLICATION_TAG_NULL,
                    &wp_data->error_class, &wp_data->error_code);
                if (status) {
                    level = BINARY_NULL;

                    priority = wp_data->priority;
                    if (priority && (priority <= BACNET_MAX_PRIORITY)) {
                        priority--;
                        //BO_Present_Value[object_index][priority] = level;
											wirte_bacnet_value_to_buf(BO,priority,object_index,level);
                        /* Note: you could set the physical output here to the next
                           highest priority, or to the relinquish default if no
                           priorities are set.
                           However, if Out of Service is TRUE, then don't set the
                           physical output.  This comment may apply to the
                           main loop (i.e. check out of service before changing output) */
                    } else {
                        status = false;
                        wp_data->error_class = ERROR_CLASS_PROPERTY;
                        wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                    }
                }
            }
            break;
        case PROP_OUT_OF_SERVICE:
            status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_BOOLEAN,
                &wp_data->error_class, &wp_data->error_code);
            if (status) {
                
//                Binary_Output_Out_Of_Service_Array[object_index] =
//                    value.type.Boolean;
//   save  auto_manual
							write_Out_Of_Service(BO,object_index,value.type.Boolean);
							
            }
            break;
				case PROP_OBJECT_NAME:
				if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {

					write_bacnet_name_to_buf(BO,wp_data->priority,object_index,value.type.Character_String.value);
                status = true;
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }						
				break;
				case PROP_DESCRIPTION:
						if (value.tag == BACNET_APPLICATION_TAG_CHARACTER_STRING) {
								write_bacnet_description_to_buf(BO,wp_data->priority,object_index,value.type.Character_String.value);
                status = true;
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }						
				break;
				case PROP_UNITS:
				if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {
				
					write_bacnet_unit_to_buf(BO,wp_data->priority,object_index,value.type.Enumerated);

                status = true;
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }					
				break;
				case PROP_RELINQUISH_DEFAULT:
					if (value.tag == BACNET_APPLICATION_TAG_ENUMERATED) {						
					write_Output_Relinguish(BO,object_index,value.type.Enumerated);
                status = true;
            } else {
                wp_data->error_class = ERROR_CLASS_PROPERTY;
                wp_data->error_code = ERROR_CODE_INVALID_DATA_TYPE;
            }						 
					break;
				case PROP_POLARITY:
					status =
                WPValidateArgType(&value, BACNET_APPLICATION_TAG_ENUMERATED,
                &wp_data->error_class, &wp_data->error_code);
            if (status) { 
                if (value.type.Enumerated < MAX_POLARITY) {  
                    Binary_Output_Polarity_Set(wp_data->object_instance,
                        (BACNET_POLARITY) value.type.Enumerated);
                } else {
                    status = false;
                    wp_data->error_class = ERROR_CLASS_PROPERTY;
                    wp_data->error_code = ERROR_CODE_VALUE_OUT_OF_RANGE;
                }
            }
							break;
        case PROP_OBJECT_IDENTIFIER:        
        case PROP_OBJECT_TYPE:
        case PROP_STATUS_FLAGS:
        case PROP_RELIABILITY:
        case PROP_EVENT_STATE:
        
        case PROP_PRIORITY_ARRAY:
        case PROP_ACTIVE_TEXT:
        case PROP_INACTIVE_TEXT:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_WRITE_ACCESS_DENIED;
            break;
        default:
            wp_data->error_class = ERROR_CLASS_PROPERTY;
            wp_data->error_code = ERROR_CODE_UNKNOWN_PROPERTY;
            break;
    }

    return status;
}


#ifdef TEST
#include <assert.h>
#include <string.h>
#include "ctest.h"

bool WPValidateArgType(
    BACNET_APPLICATION_DATA_VALUE * pValue,
    uint8_t ucExpectedTag,
    BACNET_ERROR_CLASS * pErrorClass,
    BACNET_ERROR_CODE * pErrorCode)
{
    pValue = pValue;
    ucExpectedTag = ucExpectedTag;
    pErrorClass = pErrorClass;
    pErrorCode = pErrorCode;

    return false;
}

void testBinaryOutput(
    Test * pTest)
{
    uint8_t apdu[MAX_APDU] = { 0 };
    int len = 0;
    uint32_t len_value = 0;
    uint8_t tag_number = 0;
    uint16_t decoded_type = 0;
    uint32_t decoded_instance = 0;
    BACNET_READ_PROPERTY_DATA rpdata;

    Binary_Output_Init();
    rpdata.application_data = &apdu[0];
    rpdata.application_data_len = sizeof(apdu);
    rpdata.object_type = OBJECT_BINARY_OUTPUT;
    rpdata.object_instance = 1;
    rpdata.object_property = PROP_OBJECT_IDENTIFIER;
    rpdata.array_index = BACNET_ARRAY_ALL;
    len = Binary_Output_Read_Property(&rpdata);
    ct_test(pTest, len != 0);
    len = decode_tag_number_and_value(&apdu[0], &tag_number, &len_value);
    ct_test(pTest, tag_number == BACNET_APPLICATION_TAG_OBJECT_ID);
    len = decode_object_id(&apdu[len], &decoded_type, &decoded_instance);
    ct_test(pTest, decoded_type == rpdata.object_type);
    ct_test(pTest, decoded_instance == rpdata.object_instance);

    return;
}

#ifdef TEST_BINARY_OUTPUT
int main(
    void)
{
    Test *pTest;
    bool rc;

    pTest = ct_create("BACnet Binary Output", NULL);
    /* individual tests */
    rc = ct_addTestFunction(pTest, testBinaryOutput);
    assert(rc);

    ct_setStream(pTest, stdout);
    ct_run(pTest);
    (void) ct_report(pTest);
    ct_destroy(pTest);

    return 0;
}
#endif /* TEST_BINARY_INPUT */
#endif /* TEST */


#endif

