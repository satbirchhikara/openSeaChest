//
// Do NOT modify or remove this copyright and license
//
// Copyright (c) 2014-2017 Seagate Technology LLC and/or its Affiliates, All Rights Reserved
//
// This software is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// ******************************************************************************************
// 
// 
// \file OpenSeaChest_Basic.c command line that performs various basic functions on a device.

//////////////////////
//  Included files  //
//////////////////////
#include <stdio.h>
#include <ctype.h>
#if defined (__unix__) || defined(__APPLE__) //using this definition because linux and unix compilers both define this. Apple does not define this, which is why it has it's own definition
#include <unistd.h>
#include <getopt.h>
#elif defined (_WIN32)
#include "getopt.h"
#else
#error "OS Not Defined or known"
#endif
#include "common.h"
#include "EULA.h"
#include "openseachest_util_options.h"
#include "drive_info.h"
#include "dst.h"
#include "smart.h"
#include "power_control.h"
#include "set_max_lba.h"
#include "firmware_download.h"
#include "seagate_operations.h"
#include "operations.h"
#include "host_erase.h"
#include "trim_unmap.h"
////////////////////////
//  Global Variables  //
////////////////////////
const char *util_name = "openSeaChest_Basics";

const char *buildVersion = "2.7.0";

////////////////////////////
//  functions to declare  //
////////////////////////////
static void utility_Usage(bool shortUsage);

//-----------------------------------------------------------------------------
//
//  main()
//
//! \brief   Description:  main function that runs and decides what to do based on the passed in args
//
//  Entry:
//!   \param argc =
//!   \param argv =
//!
//  Exit:
//!   \return exitCode = error code returned by the application
//
//-----------------------------------------------------------------------------
int32_t main(int argc, char *argv[])
{
    /////////////////
    //  Variables  //
    /////////////////
    //common utility variables
    int                 ret = SUCCESS;
    eUtilExitCodes      exitCode = UTIL_EXIT_NO_ERROR;
    DEVICE_UTIL_VARS
    DEVICE_INFO_VAR
    SAT_INFO_VAR
    DATA_ERASE_VAR
    PARTIAL_DATA_ERASE_VAR
    LICENSE_VAR
    ECHO_COMMAND_LINE_VAR
    SCAN_FLAG_VAR
	AGRESSIVE_SCAN_FLAG_VAR
    SHOW_BANNER_VAR
    SHOW_HELP_VAR
    TEST_UNIT_READY_VAR
    SAT_12_BYTE_CDBS_VAR
    MODEL_MATCH_VARS
    FW_MATCH_VARS
    CHILD_MODEL_MATCH_VARS
    CHILD_FW_MATCH_VARS
    ONLY_SEAGATE_VAR
    POLL_VAR
    PROGRESS_VAR
    FORCE_DRIVE_TYPE_VARS
    ENABLE_LEGACY_PASSTHROUGH_VAR
    MAX_LBA_VARS
    //scan output flags
    SCAN_FLAGS_UTIL_VARS
    DISPLAY_LBA_VAR
    //utility specific
    SMART_CHECK_VAR
    SHORT_DST_VAR
    SMART_ATTRIBUTES_VARS
    ABORT_DST_VAR
    ABORT_IDD_VAR
    IDD_TEST_VARS
    CHECK_POWER_VAR
    SPIN_DOWN_VAR
    DOWNLOAD_FW_VARS
    SET_MAX_LBA_VARS
    RESTORE_MAX_LBA_VAR
    SET_PHY_SAS_PHY_IDENTIFIER_VAR
    SET_PHY_SPEED_VARS
    SET_PIN_11_VARS
    READ_LOOK_AHEAD_VARS
    WRITE_CACHE_VARS
    TRIM_UNMAP_VARS
    OVERWRITE_VARS
    HOURS_TIME_VAR
    MINUTES_TIME_VAR
    SECONDS_TIME_VAR
    PROVISION_VAR
    ACTIVATE_DEFERRED_FW_VAR
#if defined (ENABLE_CSMI)
    CSMI_FORCE_VARS
    CSMI_VERBOSE_VAR
#endif
    HIDE_LBA_COUNTER_VAR

    firmwareUpdateData dlOptions;

    int8_t  args = 0;
    uint8_t argIndex = 0;
    int32_t optionIndex = 0;

    //add -- options to this structure DO NOT ADD OPTIONAL ARGUMENTS! Optional arguments are a GNU extension and are not supported in Unix or some compilers- TJE
    struct option longopts[] = {
        //common command line options
        DEVICE_LONG_OPT,
        HELP_LONG_OPT,
        DEVICE_INFO_LONG_OPT,
        SAT_INFO_LONG_OPT,
        USB_CHILD_INFO_LONG_OPT,
        SCAN_LONG_OPT,
		AGRESSIVE_SCAN_LONG_OPT,
        SCAN_FLAGS_LONG_OPT,
        VERSION_LONG_OPT,
        VERBOSE_LONG_OPT,
        QUIET_LONG_OPT,
        LICENSE_LONG_OPT,
        ECHO_COMMAND_LIN_LONG_OPT,
        TEST_UNIT_READY_LONG_OPT,
        SAT_12_BYTE_CDBS_LONG_OPT,
        ONLY_SEAGATE_LONG_OPT,
        MODEL_MATCH_LONG_OPT,
        FW_MATCH_LONG_OPT,
        CHILD_MODEL_MATCH_LONG_OPT,
        CHILD_FW_MATCH_LONG_OPT,
        POLL_LONG_OPT,
        CONFIRM_LONG_OPT,
        PROGRESS_LONG_OPT,
        FORCE_DRIVE_TYPE_LONG_OPTS,
        ENABLE_LEGACY_PASSTHROUGH_LONG_OPT,
        //tool specific options go here
        SMART_CHECK_LONG_OPT,
        SHORT_DST_LONG_OPT,
        SMART_ATTRIBUTES_LONG_OPT,
        ABORT_DST_LONG_OPT,
        ABORT_IDD_LONG_OPT,
        IDD_TEST_LONG_OPT,
        CHECK_POWER_LONG_OPT,
        SPIN_DOWN_LONG_OPT,
        DOWNLOAD_FW_LONG_OPT,
        DOWNLOAD_FW_MODE_LONG_OPT,
        SET_MAX_LBA_LONG_OPT,
        PROVISION_LONG_OPT,
        RESTORE_MAX_LBA_LONG_OPT,
        SET_PHY_SPEED_LONG_OPT,
        SET_PHY_SAS_PHY_LONG_OPT,
        SET_PIN_11_LONG_OPT,
        READ_LOOK_AHEAD_LONG_OPT,
        WRITE_CACHE_LONG_OPT,
        OVERWRITE_LONG_OPTS,
        TRIM_LONG_OPTS,
        UNMAP_LONG_OPTS,
        HOURS_TIME_LONG_OPT,
        MINUTES_TIME_LONG_OPT,
        SECONDS_TIME_LONG_OPT,
        DISPLAY_LBA_LONG_OPT,
        ACTIVATE_DEFERRED_FW_LONG_OPT,
        HIDE_LBA_COUNTER_LONG_OPT,
#if defined (ENABLE_CSMI)
        CSMI_VERBOSE_LONG_OPT,
        CSMI_FORCE_LONG_OPTS,
#endif
        LONG_OPT_TERMINATOR
    };

    g_verbosity = VERBOSITY_DEFAULT;

    atexit(print_Final_newline);

    ////////////////////////
    //  Argument Parsing  //
    ////////////////////////
    if (argc < 2)
    {
        openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
        utility_Usage(true);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    //get options we know we need
    while (1) //changed to while 1 in order to parse multiple options when longs options are involved
    {
        args = getopt_long(argc, argv, "d:hisSF:Vv:q%:", longopts, &optionIndex);
        if (args == -1)
        {
            break;
        }
        //printf("Parsing arg <%u>\n", args);
        switch (args)
        {
        case 0:
            //parse long options that have no short option and required arguments here
            if (strcmp(longopts[optionIndex].name, CONFIRM_LONG_OPT_STRING) == 0)
            {
                if (strlen(optarg) == strlen(DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, DATA_ERASE_ACCEPT_STRING, strlen(DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    DATA_ERASE_FLAG = true;
                }
                else if (strlen(optarg) == strlen(PARTIAL_DATA_ERASE_ACCEPT_STRING) && strncmp(optarg, PARTIAL_DATA_ERASE_ACCEPT_STRING, strlen(PARTIAL_DATA_ERASE_ACCEPT_STRING)) == 0)
                {
                    PARTIAL_DATA_ERASE_FLAG = true;
                }
            }
            else if (strcmp(longopts[optionIndex].name, TRIM_RANGE_LONG_OPT_STRING) == 0 || strcmp(longopts[optionIndex].name, UNMAP_RANGE_LONG_OPT_STRING) == 0)
            {
                sscanf(optarg, "%"SCNu64, &TRIM_UNMAP_RANGE_FLAG);
            }
            else if (strcmp(longopts[optionIndex].name, TRIM_LONG_OPT_STRING) == 0 || strcmp(longopts[optionIndex].name, UNMAP_LONG_OPT_STRING) == 0)
            {
                RUN_TRIM_UNMAP_FLAG = true;
                sscanf(optarg, "%"SCNu64, &TRIM_UNMAP_START_FLAG);
                if (0 == sscanf(optarg, "%"SCNu64, &TRIM_UNMAP_START_FLAG))
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
                    }
                    else
                    {
                        RUN_TRIM_UNMAP_FLAG = false;
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, OVERWRITE_RANGE_LONG_OPT_STRING) == 0)
            {
                sscanf(optarg, "%"SCNu64, &OVERWRITE_RANGE_FLAG);
            }
            else if (strcmp(longopts[optionIndex].name, OVERWRITE_LONG_OPT_STRING) == 0)
            {
                RUN_OVERWRITE_FLAG = true;
                if (0 == sscanf(optarg, "%"SCNu64, &OVERWRITE_START_FLAG))
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
                    }
                    else
                    {
                        RUN_OVERWRITE_FLAG = false;
                    }
                }
            }
            else if (strcmp(longopts[optionIndex].name, HOURS_TIME_LONG_OPT_STRING) == 0)
            {
                HOURS_TIME_FLAG = (uint8_t)atoi(optarg);
            }
            else if (strcmp(longopts[optionIndex].name, MINUTES_TIME_LONG_OPT_STRING) == 0)
            {
                MINUTES_TIME_FLAG = (uint16_t)atoi(optarg);
            }
            else if (strcmp(longopts[optionIndex].name, SECONDS_TIME_LONG_OPT_STRING) == 0)
            {
                SECONDS_TIME_FLAG = (uint32_t)atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, DOWNLOAD_FW_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DOWNLOAD_FW_LONG_OPT_STRING))) == 0)
            {
                DOWNLOAD_FW_FLAG = true;
                sscanf(optarg, "%s", DOWNLOAD_FW_FILENAME_FLAG);
            }
            else if (strncmp(longopts[optionIndex].name, DOWNLOAD_FW_MODE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(DOWNLOAD_FW_MODE_LONG_OPT_STRING))) == 0)
            {
                USER_SET_DOWNLOAD_MODE = true;
                DOWNLOAD_FW_MODE = DL_FW_SEGMENTED;
                if (strncmp(optarg, "immediate", strlen(optarg)) == 0 || strncmp(optarg, "full", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_FULL;
                }
                else if (strncmp(optarg, "segmented", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_SEGMENTED;
                }
                else if (strncmp(optarg, "deferred", strlen(optarg)) == 0)
                {
                    DOWNLOAD_FW_MODE = DL_FW_DEFERRED;
                }
            }
            else if (strncmp(longopts[optionIndex].name, SET_MAX_LBA_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_MAX_LBA_LONG_OPT_STRING))) == 0)
            {
                SET_MAX_LBA_FLAG = true;
                sscanf(optarg, "%"SCNu64"", &SET_MAX_LBA_VALUE);
            }
            else if (strncmp(longopts[optionIndex].name, SET_PHY_SPEED_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_PHY_SPEED_LONG_OPT_STRING))) == 0)
            {
                SET_PHY_SPEED_FLAG = true;
                SET_PHY_SPEED_GEN = (uint8_t)atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, SET_PHY_SAS_PHY_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_PHY_SAS_PHY_LONG_OPT_STRING))) == 0)
            {
                SET_PHY_ALL_PHYS = false;
                SET_PHY_SAS_PHY_IDENTIFIER = (uint8_t)atoi(optarg);
            }
            else if (strncmp(longopts[optionIndex].name, SET_PIN_11_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SET_PIN_11_LONG_OPT_STRING))) == 0)
            {
                SET_PIN_11_FLAG = true;
                if (strcmp(optarg, "default") == 0)
                {
                    SET_PIN_11_DEFAULT = true;
                }
                else if (strcmp(optarg, "on") == 0)
                {
                    SET_PIN_11_MODE = true;
                }
                else if (strcmp(optarg, "off") == 0)
                {
                    SET_PIN_11_MODE = false;
                }
                else
                {
                    //invalid option sent
                    SET_PIN_11_FLAG = false;
                }
            }
            else if (strncmp(longopts[optionIndex].name, READ_LOOK_AHEAD_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(READ_LOOK_AHEAD_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    READ_LOOK_AHEAD_INFO = true;
                }
                else
                {
                    READ_LOOK_AHEAD_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        READ_LOOK_AHEAD_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        READ_LOOK_AHEAD_SETTING = false;
                    }
                    else
                    {
                        READ_LOOK_AHEAD_FLAG = false;
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, WRITE_CACHE_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(WRITE_CACHE_LONG_OPT_STRING))) == 0)
            {
                if (strcmp(optarg, "info") == 0)
                {
                    WRITE_CACHE_INFO = true;
                }
                else
                {
                    WRITE_CACHE_FLAG = true;
                    if (strcmp(optarg, "enable") == 0)
                    {
                        WRITE_CACHE_SETTING = true;
                    }
                    else if (strcmp(optarg, "disable") == 0)
                    {
                        WRITE_CACHE_SETTING = false;
                    }
                    else
                    {
                        WRITE_CACHE_FLAG = false;
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, PROVISION_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(PROVISION_LONG_OPT_STRING))) == 0)
            {
                SET_MAX_LBA_FLAG = true;
                sscanf(optarg, "%"SCNu64"", &SET_MAX_LBA_VALUE);
                //now, based on the new MaxLBA, set the TRIM/UNMAP start flag to get rid of the LBAs that will not be above the new maxLBA (the range will be set later)
                TRIM_UNMAP_START_FLAG = SET_MAX_LBA_VALUE + 1;
            }
            else if (strncmp(longopts[optionIndex].name, IDD_TEST_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(IDD_TEST_LONG_OPT_STRING))) == 0)
            {
                RUN_IDD_FLAG = true;
                if (strcmp(optarg, "short") == 0)
                {
                    IDD_TEST_FLAG = SEAGATE_IDD_SHORT;
                }
                else if (strcmp(optarg, "long") == 0)
                {
                    IDD_TEST_FLAG = SEAGATE_IDD_LONG;
                }
                else if (strcmp(optarg, "repair") == 0)
                {
                    IDD_TEST_FLAG = SEAGATE_IDD_LONG_WITH_REPAIR;
                }
                else
                {
                    //read in the argument as a hex value instead of an integer
                    uint32_t iddTestNumber = 0;
                    sscanf(optarg, "%"SCNx32, &iddTestNumber);
                    switch (iddTestNumber)
                    {
                    case 0x70:
                        IDD_TEST_FLAG = SEAGATE_IDD_SHORT;
                        break;
                    case 0x71:
                        IDD_TEST_FLAG = SEAGATE_IDD_LONG;
                        break;
                    case 0x72:
                        IDD_TEST_FLAG = SEAGATE_IDD_LONG_WITH_REPAIR;
                        break;
                    default:
                        RUN_IDD_FLAG = false;
                    }
                }
            }
            else if (strncmp(longopts[optionIndex].name, SMART_ATTRIBUTES_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(SMART_ATTRIBUTES_LONG_OPT_STRING))) == 0)
            {
                SMART_ATTRIBUTES_FLAG = true;
                if (strcmp(optarg, "raw") == 0)
                {
                    SMART_ATTRIBUTES_MODE_FLAG = SMART_ATTR_OUTPUT_RAW;
                }
                else if (strcmp(optarg, "analyzed") == 0)
                {
                    SMART_ATTRIBUTES_MODE_FLAG = SMART_ATTR_OUTPUT_ANALYZED;
                }
                else
                {
                    printf("Invalid argument to --%s: %s\n", SMART_ATTRIBUTES_LONG_OPT_STRING, optarg);
                    exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
                }
            }
            else if (strncmp(longopts[optionIndex].name, MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(MODEL_MATCH_LONG_OPT_STRING))) == 0)
            {
                MODEL_MATCH_FLAG = true;
                strncpy(MODEL_STRING_FLAG, optarg, M_Min(40, strlen(optarg)));
            }
            else if (strncmp(longopts[optionIndex].name, FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                FW_MATCH_FLAG = true;
                strncpy(FW_STRING_FLAG, optarg, M_Min(9, strlen(optarg)));
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_MODEL_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_MODEL_MATCH_LONG_OPT_STRING))) == 0)
            {
                CHILD_MODEL_MATCH_FLAG = true;
                strncpy(CHILD_MODEL_STRING_FLAG, optarg, M_Min(40, strlen(optarg)));
            }
            else if (strncmp(longopts[optionIndex].name, CHILD_FW_MATCH_LONG_OPT_STRING, M_Min(strlen(longopts[optionIndex].name), strlen(CHILD_FW_MATCH_LONG_OPT_STRING))) == 0)
            {
                CHILD_FW_MATCH_FLAG = true;
                strncpy(CHILD_FW_STRING_FLAG, optarg, M_Min(9, strlen(optarg)));
            }
            else if (strcmp(longopts[optionIndex].name, DISPLAY_LBA_LONG_OPT_STRING) == 0)
            {
                DISPLAY_LBA_FLAG = true;
                if (0 == sscanf(optarg, "%"SCNu64, &DISPLAY_LBA_THE_LBA))
                {
                    if (strcmp(optarg, "maxLBA") == 0)
                    {
                        USE_MAX_LBA = true;
                    }
                    else if (strcmp(optarg, "childMaxLBA") == 0)
                    {
                        USE_CHILD_MAX_LBA = true;
                    }
                    else
                    {
                    	DISPLAY_LBA_FLAG = false;
                    }
                }
            }
            break;
        case ':'://missing required argument
            exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
            switch (optopt)
            {
            case 0:
                break;
            case DEVICE_SHORT_OPT:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("You must specify a device handle\n");
                }
                return UTIL_EXIT_INVALID_DEVICE_HANDLE;
            case VERBOSE_SHORT_OPT:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("You must specify a verbosity level. 0 - 4 are the valid levels\n");
                }
                break;
            case SCAN_FLAGS_SHORT_OPT:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("You must specify which scan options flags you want to use.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Option %c requires an argument\n", optopt);
                }
                utility_Usage(true);
                return exitCode;
            }
            break;
        case DEVICE_SHORT_OPT: //device
            if (0 != parse_Device_Handle_Argument(optarg, &RUN_ON_ALL_DRIVES, &USER_PROVIDED_HANDLE, &DEVICE_LIST_COUNT, &HANDLE_LIST))
            {
                //Free any memory allocated so far, then exit.
                free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                exit(255);
            }
            break;
        case DEVICE_INFO_SHORT_OPT: //device information
            DEVICE_INFO_FLAG = true;
            break;
        case SCAN_SHORT_OPT: //scan
            SCAN_FLAG = true;
            break;
		case AGRESSIVE_SCAN_SHORT_OPT:
			AGRESSIVE_SCAN_FLAG = true;
			break;
        case VERSION_SHORT_OPT:
            SHOW_BANNER_FLAG = true;
            break;
        case VERBOSE_SHORT_OPT: //verbose
            if (optarg != NULL)
            {
                g_verbosity = atoi(optarg);
            }
            break;
        case QUIET_SHORT_OPT: //quiet mode
            g_verbosity = VERBOSITY_QUIET;
            break;
        case SCAN_FLAGS_SHORT_OPT://scan flags
            SCAN_FLAGS_SUBOPT_PARSING;
            break;
        case '?': //unknown option
            openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
            utility_Usage(false);
            exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
        case 'h': //help
            SHOW_HELP_FLAG = true;
            openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
            utility_Usage(false);
            exit(UTIL_EXIT_NO_ERROR);
        case PROGRESS_SHORT_OPT://progress [test]
            PROGRESS_CHAR = strdup(optarg);
            break;
        default:
            break;
        }
    }


    if (ECHO_COMMAND_LINE_FLAG)
    {
        uint64_t commandLineIter = 1;//start at 1 as starting at 0 means printing the directory info+ SeaChest.exe (or ./SeaChest)
        for (commandLineIter = 1; commandLineIter < argc; commandLineIter++)
        {
            if (strncmp(argv[commandLineIter], "--echoCommandLine", strlen(argv[commandLineIter])) == 0)
            {
                continue;
            }
            printf("%s ", argv[commandLineIter]);
        }
        printf("\n");
    }

    if (VERBOSITY_QUIET < g_verbosity)
    {
        openseachest_utility_Info(util_name, buildVersion, OPENSEA_TRANSPORT_VERSION);
    }

    if (SHOW_BANNER_FLAG)
    {
        utility_Full_Version_Info(util_name, buildVersion, OPENSEA_TRANSPORT_MAJOR_VERSION, OPENSEA_TRANSPORT_MINOR_VERSION, OPENSEA_TRANSPORT_PATCH_VERSION);
    }

    if (LICENSE_FLAG)
    {
        print_EULA_To_Screen(false, false);
    }

	if (SCAN_FLAG || AGRESSIVE_SCAN_FLAG)
    {
        unsigned int scanControl = DEFAULT_SCAN;
		if(AGRESSIVE_SCAN_FLAG)
		{
			scanControl |= AGRESSIVE_SCAN;
		}
        #if defined (__linux__)
        if (scanSD)
        {
            scanControl |= SD_HANDLES;
        }
        if (scanSDandSG)
        {
            scanControl |= SG_TO_SD;
        }
        #endif
        //set the drive types to show (if none are set, the lower level code assumes we need to show everything)
        if (scanATA)
        {
            scanControl |= ATA_DRIVES;
        }
        if (scanUSB)
        {
            scanControl |= USB_DRIVES;
        }
        if (scanSCSI)
        {
            scanControl |= SCSI_DRIVES;
        }
        if (scanNVMe)
        {
            scanControl |= NVME_DRIVES;
        }
        if (scanRAID)
        {
            scanControl |= RAID_DRIVES;
        }
        //set the interface types to show (if none are set, the lower level code assumes we need to show everything)
        if (scanInterfaceATA)
        {
            scanControl |= IDE_INTERFACE_DRIVES;
        }
        if (scanInterfaceUSB)
        {
            scanControl |= USB_INTERFACE_DRIVES;
        }
        if (scanInterfaceSCSI)
        {
            scanControl |= SCSI_INTERFACE_DRIVES;
        }
        if (scanInterfaceNVMe)
        {
            scanControl |= NVME_INTERFACE_DRIVES;
        }
        if (SAT_12_BYTE_CDBS_FLAG)
        {
            scanControl |= SAT_12_BYTE;
        }
#if defined (ENABLE_CSMI)
        if (scanIgnoreCSMI)
        {
            scanControl |= IGNORE_CSMI;
        }
        if (scanAllowDuplicateDevices)
        {
            scanControl |= ALLOW_DUPLICATE_DEVICE;
        }
#endif
        scan_And_Print_Devs(scanControl, NULL);
    }
    // Add to this if list anything that is suppose to be independent.
    // e.g. you can't say enumerate & then pull logs in the same command line.
    // SIMPLE IS BEAUTIFUL
	if (SCAN_FLAG || AGRESSIVE_SCAN_FLAG || SHOW_BANNER_FLAG || LICENSE_FLAG || SHOW_HELP_FLAG)
    {
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_NO_ERROR);
    }

    //print out errors for unknown arguments for remaining args that haven't been processed yet
    for (argIndex = optind; argIndex < argc; argIndex++)
    {
        if (VERBOSITY_QUIET < g_verbosity)
        {
            printf("Invalid argument: %s\n", argv[argIndex]);
        }
    }
    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        uint64_t flags = 0;
#if defined (ENABLE_CSMI)
        flags |= GET_DEVICE_FUNCS_IGNORE_CSMI;
#endif
        if (SUCCESS != get_Device_Count(&DEVICE_LIST_COUNT, flags))
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Unable to get number of devices\n");
            }
            exit(UTIL_EXIT_OPERATION_FAILURE);
        }
    }
    else if (DEVICE_LIST_COUNT == 0)
    {
        if (VERBOSITY_QUIET < g_verbosity)
        {
            printf("You must specify one or more target devices with the --%s option to run this command.\n", DEVICE_LONG_OPT_STRING);
            utility_Usage(true);
            printf("Use -h option for detailed description\n\n");
        }
        exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
    }
    
    if ((FORCE_SCSI_FLAG && FORCE_ATA_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_DMA_FLAG)
        || (FORCE_ATA_PIO_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_ATA_DMA_FLAG && FORCE_ATA_UDMA_FLAG)
        || (FORCE_SCSI_FLAG && (FORCE_ATA_PIO_FLAG || FORCE_ATA_DMA_FLAG || FORCE_ATA_UDMA_FLAG))//We may need to remove this. At least when software SAT gets used. (currently only Windows ATA passthrough and FreeBSD ATA passthrough)
        )
    {
        printf("\nError: Only one force flag can be used at a time.\n");
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }
    //need to make sure this is set when we are asked for SAT Info
    if (SAT_INFO_FLAG)
    {
        DEVICE_INFO_FLAG = goTrue;
    }
    //check that we were given at least one test to perform...if not, show the help and exit
    if (!(DEVICE_INFO_FLAG
        || TEST_UNIT_READY_FLAG
        || SMART_CHECK_FLAG
        || SHORT_DST_FLAG
        || SMART_ATTRIBUTES_FLAG
        || ABORT_DST_FLAG
        || ABORT_IDD_FLAG
        || RUN_IDD_FLAG
        || CHECK_POWER_FLAG
        || SPIN_DOWN_FLAG
        || DOWNLOAD_FW_FLAG
        || RESTORE_MAX_LBA_FLAG
        || SET_MAX_LBA_FLAG
        || SET_PHY_SPEED_FLAG
        || SET_PIN_11_FLAG
        || WRITE_CACHE_FLAG
        || READ_LOOK_AHEAD_FLAG
        || READ_LOOK_AHEAD_INFO
        || WRITE_CACHE_INFO
        || PROVISION_FLAG
        || RUN_OVERWRITE_FLAG
        || RUN_TRIM_UNMAP_FLAG
        || (PROGRESS_CHAR != NULL)
        || DISPLAY_LBA_FLAG
        //check for other tool specific options here
        ))
    {
        utility_Usage(true);
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_ERROR_IN_COMMAND_LINE);
    }

    uint64_t flags = 0;
    DEVICE_LIST = (tDevice*)calloc(DEVICE_LIST_COUNT * sizeof(tDevice), sizeof(tDevice));
    if (!DEVICE_LIST)
    {
        if (VERBOSITY_QUIET < g_verbosity)
        {
            printf("Unable to allocate memory\n");
        }
        free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
        exit(UTIL_EXIT_OPERATION_FAILURE);
    }
    versionBlock version;
    memset(&version, 0, sizeof(versionBlock));
    version.version = DEVICE_BLOCK_VERSION;
    version.size = sizeof(tDevice);

    if (TEST_UNIT_READY_FLAG || CHECK_POWER_FLAG)
    {
        flags = DO_NOT_WAKE_DRIVE;
    }

    //set flags that can be passed down in get device regarding forcing specific ATA modes.
    if (FORCE_ATA_PIO_FLAG)
    {
        flags |= FORCE_ATA_PIO_ONLY;
    }

    if (FORCE_ATA_DMA_FLAG)
    {
        flags |= FORCE_ATA_DMA_SAT_MODE;
    }

    if (FORCE_ATA_UDMA_FLAG)
    {
        flags |= FORCE_ATA_UDMA_SAT_MODE;
    }

#if defined (ENABLE_CSMI)
    if (CSMI_VERBOSE_FLAG)
    {
        flags |= CSMI_FLAG_VERBOSE;//This may be temporary....or it may last longer, but for now this highest bit is what we'll set for this option...
    }
    if (CSMI_FORCE_USE_PORT_FLAG)
    {
        flags |= CSMI_FLAG_USE_PORT;
    }
    if (CSMI_FORCE_IGNORE_PORT_FLAG)
    {
        flags |= CSMI_FLAG_IGNORE_PORT;
    }
#endif

    if (RUN_ON_ALL_DRIVES && !USER_PROVIDED_HANDLE)
    {
        //TODO? check for this flag ENABLE_LEGACY_PASSTHROUGH_FLAG. Not sure it is needed here and may not be desirable.
#if defined (ENABLE_CSMI)
        flags |= GET_DEVICE_FUNCS_IGNORE_CSMI;//TODO: Remove this flag so that CSMI devices can be part of running on all drives. This is not allowed now because of issues with running the same operation on the same drive with both PD? and SCSI?:? handles.
#endif
        if (DEVICE_LIST_COUNT)
        {
            ret = get_Device_List(DEVICE_LIST, DEVICE_LIST_COUNT * sizeof(tDevice), version, flags);
        }
        if (SUCCESS != ret)
        {
            if (ret == WARN_NOT_ALL_DEVICES_ENUMERATED)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("WARN: Not all devices enumerated correctly\n");
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Unable to get device list\n");
                }
                exit(UTIL_EXIT_OPERATION_FAILURE);
            }
        }
    }
    else
    {
        /*need to go through the handle list and attempt to open each handle.*/
        for (uint16_t handleIter = 0; handleIter < DEVICE_LIST_COUNT; ++handleIter)
        {
            /*Initializing is necessary*/
            deviceList[handleIter].sanity.size = sizeof(tDevice);
            deviceList[handleIter].sanity.version = DEVICE_BLOCK_VERSION;
#if !defined(_WIN32)
            deviceList[handleIter].os_info.fd = -1;
#else
            deviceList[handleIter].os_info.fd = INVALID_HANDLE_VALUE;
#endif
            deviceList[handleIter].dFlags = flags;

            if (ENABLE_LEGACY_PASSTHROUGH_FLAG)
            {
                deviceList[handleIter].drive_info.ata_Options.enableLegacyPassthroughDetectionThroughTrialAndError = true;
            }

            /*get the device for the specified handle*/
#if defined(_DEBUG)
            printf("Attempting to open handle \"%s\"\n", HANDLE_LIST[handleIter]);
#endif
            ret = get_Device(HANDLE_LIST[handleIter], &deviceList[handleIter]);
#if !defined(_WIN32)
            if ((deviceList[handleIter].os_info.fd < 0) || (ret == FAILURE || ret == PERMISSION_DENIED))
#else
            if ((deviceList[handleIter].os_info.fd == INVALID_HANDLE_VALUE) || (ret == FAILURE || ret == PERMISSION_DENIED))
#endif
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Error: Could not open handle to %s\n", HANDLE_LIST[handleIter]);
                }
                free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
                exit(UTIL_EXIT_INVALID_DEVICE_HANDLE);
            }
        }
    }
    free_Handle_List(&HANDLE_LIST, DEVICE_LIST_COUNT);
    for (uint32_t deviceIter = 0; deviceIter < DEVICE_LIST_COUNT; ++deviceIter)
    {
        if (ONLY_SEAGATE_FLAG)
        {
            if (is_Seagate_Family(&deviceList[deviceIter]) == NON_SEAGATE)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("%s - This drive (%s) is not a Seagate drive.\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification);
                }
                continue;
            }
        }

        if (SAT_12_BYTE_CDBS_FLAG)
        {
            //set SAT12 for this device if requested
            deviceList[deviceIter].drive_info.ata_Options.use12ByteSATCDBs = true;
        }

        //check for model number match
        if (MODEL_MATCH_FLAG)
        {
            if (strncmp(MODEL_STRING_FLAG, deviceList[deviceIter].drive_info.product_identification, M_Min(strlen(MODEL_STRING_FLAG), strlen(deviceList[deviceIter].drive_info.product_identification))))
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("%s - This drive (%s) does not match the input model number: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification, MODEL_STRING_FLAG);
                }
                continue;
            }
        }
        //check for fw match
        if (FW_MATCH_FLAG)
        {
            if (strcmp(FW_STRING_FLAG, deviceList[deviceIter].drive_info.product_revision))
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input firmware revision: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_revision, FW_STRING_FLAG);
                }
                continue;
            }
        }

        //check for child model number match
        if (CHILD_MODEL_MATCH_FLAG)
        {
            if (strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN) == 0 || strncmp(CHILD_MODEL_STRING_FLAG, deviceList[deviceIter].drive_info.bridge_info.childDriveMN, M_Min(strlen(CHILD_MODEL_STRING_FLAG), strlen(deviceList[deviceIter].drive_info.bridge_info.childDriveMN))))
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("%s - This drive (%s) does not match the input child model number: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.bridge_info.childDriveMN, CHILD_MODEL_STRING_FLAG);
                }
                continue;
            }
        }
        //check for child fw match
        if (CHILD_FW_MATCH_FLAG)
        {
            if (strcmp(CHILD_FW_STRING_FLAG, deviceList[deviceIter].drive_info.bridge_info.childDriveFW))
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("%s - This drive's firmware (%s) does not match the input child firmware revision: %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.bridge_info.childDriveFW, CHILD_FW_STRING_FLAG);
                }
                continue;
            }
        }
        
        if (FORCE_SCSI_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tForcing SCSI Drive\n");
            }
            deviceList[deviceIter].drive_info.drive_type = SCSI_DRIVE;
        }
        
        if (FORCE_ATA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tForcing ATA Drive\n");
            }
            deviceList[deviceIter].drive_info.drive_type = ATA_DRIVE;
        }

        if (FORCE_ATA_PIO_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tAttempting to force ATA Drive commands in PIO Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaSupported = false;
            deviceList[deviceIter].drive_info.ata_Options.dmaMode = ATA_DMA_MODE_NO_DMA;
            deviceList[deviceIter].drive_info.ata_Options.downloadMicrocodeDMASupported = false;
            deviceList[deviceIter].drive_info.ata_Options.readBufferDMASupported = false;
            deviceList[deviceIter].drive_info.ata_Options.readLogWriteLogDMASupported = false;
            deviceList[deviceIter].drive_info.ata_Options.writeBufferDMASupported = false;
        }

        if (FORCE_ATA_DMA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tAttempting to force ATA Drive commands in DMA Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaMode = ATA_DMA_MODE_DMA;
        }

        if (FORCE_ATA_UDMA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("\tAttempting to force ATA Drive commands in UDMA Mode\n");
            }
            deviceList[deviceIter].drive_info.ata_Options.dmaMode = ATA_DMA_MODE_UDMA;
        }

        if (VERBOSITY_QUIET < g_verbosity)
        {
			printf("\n%s - %s - %s - %s\n", deviceList[deviceIter].os_info.name, deviceList[deviceIter].drive_info.product_identification, deviceList[deviceIter].drive_info.serialNumber, print_drive_type(&deviceList[deviceIter]));
        }

        //now start looking at what operations are going to be performed and kick them off
        if (DEVICE_INFO_FLAG)
        {
            if (SUCCESS != print_Drive_Information(&deviceList[deviceIter], SAT_INFO_FLAG))
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("ERROR: failed to get device information\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (TEST_UNIT_READY_FLAG)
        {
            scsiStatus returnedStatus = { 0 };
            ret = scsi_Test_Unit_Ready(&deviceList[deviceIter], &returnedStatus);
            if ((ret == SUCCESS) && (returnedStatus.senseKey == SENSE_KEY_NO_ERROR))
            {
                printf("READY\n");
            }
            else
            {
                eVerbosityLevels tempVerbosity = g_verbosity;
                printf("NOT READY\n");
                g_verbosity = VERBOSITY_COMMAND_NAMES;//the function below will print out a sense data translation, but only it we are at this verbosity or higher which is why it's set before this call.
                check_Sense_Key_ASC_ASCQ_And_FRU(&deviceList[deviceIter], returnedStatus.senseKey, returnedStatus.acq, returnedStatus.ascq, returnedStatus.fru);
                g_verbosity = tempVerbosity;//restore it back to what it was now that this is done.
            }
        }

        if (DISPLAY_LBA_FLAG)
        {
            uint8_t *displaySector = (uint8_t*)calloc(deviceList[deviceIter].drive_info.deviceBlockSize * sizeof(uint8_t), sizeof(uint8_t));
            if (!displaySector)
            {
                perror("Could not allocate memory to read LBA.");
                exit(UTIL_EXIT_OPERATION_FAILURE);
            }
            if (USE_MAX_LBA)
            {
                DISPLAY_LBA_THE_LBA = deviceList[deviceIter].drive_info.deviceMaxLba;
            }
            else if (USE_CHILD_MAX_LBA)
            {
                DISPLAY_LBA_THE_LBA = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
            }
            if (SUCCESS == read_LBA(&deviceList[deviceIter], DISPLAY_LBA_THE_LBA, false, displaySector, deviceList[deviceIter].drive_info.deviceBlockSize))
            {
                printf("\nContents of LBA %"PRIu64":\n", DISPLAY_LBA_THE_LBA);
                print_Data_Buffer(displaySector, deviceList[deviceIter].drive_info.deviceBlockSize, true);
            }
            else
            {
                printf("Error Reading LBA %"PRIu64" for display\n", DISPLAY_LBA_THE_LBA);
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
            safe_Free(displaySector);
        }

        if (SPIN_DOWN_FLAG)
        {
            switch (spin_down_drive(&deviceList[deviceIter], false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully sent command to spin down device. Please wait 15 seconds for it to finish spinning down.\n");
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Spin down not supported by this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to spin down device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        //this option must come after --transition power so that these two options can be combined on the command line and produce the correct end result
        if (CHECK_POWER_FLAG)
        {
            switch (print_Current_Power_Mode(&deviceList[deviceIter]))
            {
            case SUCCESS:
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Checking current power mode not supported on this device.\n");
                }
                break;
            default:
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed checking current power mode.\n");
                }
                break;
            }
        }

        if (SMART_CHECK_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("SMART Check\n");
            }
            ret = run_SMART_Check(&deviceList[deviceIter], NULL);
            if (FAILURE == ret)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("SMART has been tripped!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
            else if (SUCCESS == ret)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("SMART Check Passed!\n");
                }
            }
            else if (IN_PROGRESS == ret)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("SMART Warning condition detected!\n");
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Unable to run SMART Check!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SMART_ATTRIBUTES_FLAG)
        {
            switch (print_SMART_Attributes(&deviceList[deviceIter], SMART_ATTRIBUTES_MODE_FLAG))
            {
            case SUCCESS:
                //nothing to print here since if it was successful, the attributes will be printed to the screen
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Showing SMART attributes is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("A failure occured while trying to get SMART attributes\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (ABORT_DST_FLAG)
        {
            int abortResult = UNKNOWN;
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Aborting DST\n");
            }
            abortResult = abort_DST(&deviceList[deviceIter]);
            switch (abortResult)
            {
            case UNKNOWN:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Unknown Error occurred while trying to abort DST\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case SUCCESS:
            case ABORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully aborted DST.\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Aborting DST is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Abort DST Failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        
        if (ABORT_IDD_FLAG)
        {
            int abortResult = UNKNOWN;
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Aborting IDD\n");
            }
            abortResult = abort_DST(&deviceList[deviceIter]);//calls into the same code to do the abort - TJE
            switch (abortResult)
            {
            case UNKNOWN:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Unknown Error occurred while trying to abort IDD\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            case SUCCESS:
            case ABORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully aborted IDD.\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Aborting IDD is not supported on this device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Abort IDD Failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SHORT_DST_FLAG)
        {
            int32_t DSTResult = UNKNOWN;
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Short DST\n");
            }
            DSTResult = run_DST(&deviceList[deviceIter], 1, POLL_FLAG, false);
            switch (DSTResult)
            {
            case UNKNOWN:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Unknown Error occurred while trying to start DST\n");
                }
                break;
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (POLL_FLAG)
                    {
                        printf("Short DST Passed!\n");
                    }
                    else
                    {
                        printf("Short DST started!\n");
                        printf("use --progress dst -d %s to monitor Drive Self Test progress\n", deviceHandleExample);
                        printf("use --abortDST -d %s to stop Drive Self Test\n", deviceHandleExample);
                    }
                }
                break;
            case IN_PROGRESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("A self test is currently in progress.\n");
                }
                break;
            case ABORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Short DST aborted!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Short DST is not supported on this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Short DST Failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (RUN_IDD_FLAG)
        {
            int32_t IDDResult = UNKNOWN;
            if (VERBOSITY_QUIET < g_verbosity)
            {
                uint64_t iddTimeSeconds = 0;
                uint8_t hours = 0, minutes = 0, seconds = 0;
                get_Approximate_IDD_Time(&deviceList[deviceIter], IDD_TEST_FLAG, &iddTimeSeconds);
                if (iddTimeSeconds == UINT64_MAX)
                {
                    printf("A time estimate is not available for this IDD operation");
                }
                else
                {
                    printf("The In Drive Diagnostics (IDD) test will take approximately ");
                    convert_Seconds_To_Displayable_Time(iddTimeSeconds, NULL, NULL, &hours, &minutes, &seconds);
                    print_Time_To_Screen(NULL, NULL, &hours, &minutes, &seconds);
                }
                printf("\n");
            }
            IDDResult = run_IDD(&deviceList[deviceIter], IDD_TEST_FLAG, POLL_FLAG);
            switch (IDDResult)
            {
            case UNKNOWN:
                //IDD was not run
                break;
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (POLL_FLAG || IDD_TEST_FLAG == SEAGATE_IDD_SHORT)//short test is run in captive mode, so polling doesn't make sense
                    {
                        printf("IDD - ");
                        switch(IDD_TEST_FLAG)
                        {
                        case SEAGATE_IDD_SHORT:
                            printf("short");
                            break;
                        case SEAGATE_IDD_LONG:
                            printf("long");
                            break;
                        case SEAGATE_IDD_LONG_WITH_REPAIR:
                            printf("long with repair");
                            break;
                        }
                        printf(" - completed without error!\n");
                    }
                    else
                    {
                        printf("IDD - ");
                        switch(IDD_TEST_FLAG)
                        {
                        case SEAGATE_IDD_SHORT:
                            printf("short");
                            break;
                        case SEAGATE_IDD_LONG:
                            printf("long");
                            break;
                        case SEAGATE_IDD_LONG_WITH_REPAIR:
                            printf("long with repair");
                            break;
                        }
                        printf(" - has been started.\n");
                        printf("use --progress idd -d %s to monitor IDD progress\n", deviceHandleExample);
                        printf("use --abortIDD -d %s to stop IDD\n", deviceHandleExample);
                    }
                }
                break;
            case IN_PROGRESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("A self test is currently in progress.\n");
                }
                break;
            case ABORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("IDD aborted!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("IDD not supported\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("IDD Failed!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (DOWNLOAD_FW_FLAG)
        {
            FILE *firmwareFilePtr = NULL;
            bool fileOpenedSuccessfully = true;//assume true in case of activate command
			firmwareUpdateData dlOptions; 
			memset(&dlOptions, 0, sizeof(firmwareUpdateData));
            if (DOWNLOAD_FW_MODE != DL_FW_ACTIVATE)
            {
                //open the file and send the download
                if ((firmwareFilePtr = fopen(DOWNLOAD_FW_FILENAME_FLAG, "rb")) == NULL)
                {
                    fileOpenedSuccessfully = false;
                }
            }
            if (DOWNLOAD_FW_MODE == DL_FW_ACTIVATE)
            {
                //this shouldn't fall into this code path anymore...
                fileOpenedSuccessfully = false;
            }
            if (fileOpenedSuccessfully)
            {
                long firmwareFileSize = get_File_Size(firmwareFilePtr);
                uint8_t *firmwareMem = (uint8_t*)calloc(firmwareFileSize * sizeof(uint8_t), sizeof(uint8_t));
                if (firmwareMem)
                {
                    supportedDLModes supportedFWDLModes;
                    memset(&supportedFWDLModes, 0, sizeof(supportedDLModes));
                    if (SUCCESS == get_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes))
                    {
                        if (!USER_SET_DOWNLOAD_MODE)
                        {
                            //This line is commented out as we want to wait a little longer before letting deferred be a default when supported.
                            /*
                            DOWNLOAD_FW_MODE = supportedFWDLModes.recommendedDownloadMode;
                            /*/
                            if (!supportedFWDLModes.deferred)
                            {
                                DOWNLOAD_FW_MODE = supportedFWDLModes.recommendedDownloadMode;
                            }
                            else
                            {
                                if (supportedFWDLModes.segmented)
                                {
                                    DOWNLOAD_FW_MODE = DL_FW_SEGMENTED;
                                }
                                else
                                {
                                    DOWNLOAD_FW_MODE = DL_FW_FULL;
                                }
                            }
                            //*/
                        }
                    }
                    fread(firmwareMem, sizeof(uint8_t), firmwareFileSize, firmwareFilePtr);

					dlOptions.useDMA = deviceList[deviceIter].drive_info.ata_Options.downloadMicrocodeDMASupported;
					dlOptions.dlMode = DOWNLOAD_FW_MODE;					
					dlOptions.segmentSize = 0;
					dlOptions.firmwareFileMem = firmwareMem;
					dlOptions.firmwareMemoryLength = firmwareFileSize;
					switch (firmware_Download(&deviceList[deviceIter], &dlOptions) )
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Firmware Download successful\n");
                            if (DOWNLOAD_FW_MODE == DL_FW_DEFERRED)
                            {
                                printf("Firmware download complete. Reboot or run the --%s command to finish installing the firmware.\n", ACTIVATE_DEFERRED_FW_LONG_OPT_STRING);
                            }
                            else if (supportedFWDLModes.seagateDeferredPowerCycleActivate && DOWNLOAD_FW_MODE == DL_FW_SEGMENTED)
                            {
                                printf("This drive requires a full power cycle to activate the new code.\n");
                            }
                            else
                            {
                                fill_Drive_Info_Data(&deviceList[deviceIter]);
                                printf("New firmware version is %s\n", deviceList[deviceIter].drive_info.product_revision);
                            }
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Firmware Download not supported\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Firmware Download failed\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                    safe_Free(firmwareMem);
                }
                else
                {
                    perror("failed to allocate memory");
                    exit(255);
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    perror("fopen");
                    printf("Couldn't open file %s\n", DOWNLOAD_FW_FILENAME_FLAG);
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
            }
        }

        if (ACTIVATE_DEFERRED_FW_FLAG)
        {
            supportedDLModes supportedFWDLModes;
            memset(&supportedFWDLModes, 0, sizeof(supportedDLModes));
            get_Supported_FWDL_Modes(&deviceList[deviceIter], &supportedFWDLModes);
            if (supportedFWDLModes.deferred || supportedFWDLModes.scsiInfoPossiblyIncomplete)
            {
                memset(&dlOptions, 0, sizeof(firmwareUpdateData));
                dlOptions.useDMA = deviceList[deviceIter].drive_info.ata_Options.downloadMicrocodeDMASupported;
                dlOptions.dlMode = DL_FW_ACTIVATE;
                dlOptions.segmentSize = 0;
                dlOptions.firmwareFileMem = NULL;
                dlOptions.firmwareMemoryLength = 0;
                ret = firmware_Download(&deviceList[deviceIter], &dlOptions);
                switch (ret)
                {
                case SUCCESS:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Firmware activate successful\n");
                        fill_Drive_Info_Data(&deviceList[deviceIter]);
                        printf("New firmware version is %s\n", deviceList[deviceIter].drive_info.product_revision);
                    }
                    break;
                case NOT_SUPPORTED:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Firmware activate not supported\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                    break;
                default:
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("Firmware activate failed\n");
                    }
                    exitCode = UTIL_EXIT_OPERATION_FAILURE;
                    break;
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("This drive does not support the activate command.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
            }
        }

        if (SET_PHY_SPEED_FLAG)
        {
            switch (set_phy_speed(&deviceList[deviceIter], SET_PHY_SPEED_GEN, SET_PHY_ALL_PHYS, SET_PHY_SAS_PHY_IDENTIFIER))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully set the PHY speed. Please power cycle the device to complete this change.\n");
                }
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Operation not supported by this device.\n");
                }
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to set the PHY speed of the device.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (SET_PIN_11_FLAG)
        {
            switch (change_Pin11(&deviceList[deviceIter], SET_PIN_11_DEFAULT, SET_PIN_11_MODE))
            {
            case SUCCESS:
                exitCode = 0;
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully changed pin 11 behavior!\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Changin pin 11 behavior is not supported on this device or this device type.\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to change pin 11 behavior!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (WRITE_CACHE_INFO)
        {
            if (is_Write_Cache_Enabled(&deviceList[deviceIter]))
            {
                printf("Write Cache is Enabled\n");
            }
            else
            {
                printf("Write Cache is Disabled\n");
            }
        }

        if (WRITE_CACHE_FLAG)
        {
            switch (set_Write_Cache(&deviceList[deviceIter], WRITE_CACHE_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Write cache successfully enabled!\n");
                    }
                    else
                    {
                        printf("Write cache successfully disabled!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Enabling Write cache not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling Write cache not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (WRITE_CACHE_SETTING)
                    {
                        printf("Failed to enable Write cache!\n");
                    }
                    else
                    {
                        printf("Failed to disable Write cache!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (READ_LOOK_AHEAD_INFO)
        {
            if (is_Read_Look_Ahead_Enabled(&deviceList[deviceIter]))
            {
                printf("Read Look Ahead is Enabled.\n");
            }
            else
            {
                printf("Read Look Ahead is Disabled.\n");
            }
        }

        if (READ_LOOK_AHEAD_FLAG)
        {
            switch (set_Read_Look_Ahead(&deviceList[deviceIter], READ_LOOK_AHEAD_SETTING))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Read look-ahead successfully enabled!\n");
                    }
                    else
                    {
                        printf("Read look-ahead successfully disabled!\n");
                    }
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Enabling read look-ahead not supported on this device.\n");
                    }
                    else
                    {
                        printf("Disabling read look-ahead not supported on this device.\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    if (READ_LOOK_AHEAD_SETTING)
                    {
                        printf("Failed to enable read look-ahead!\n");
                    }
                    else
                    {
                        printf("Failed to disable read look-ahead!\n");
                    }
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (PROVISION_FLAG)
        {
            //this means we are performing a TRIM/UNMAP operation, then setting the maxlba. Since we've already set the trim start location, time to set the range
            TRIM_UNMAP_RANGE_FLAG = deviceList[deviceIter].drive_info.deviceMaxLba - TRIM_UNMAP_START_FLAG;
            RUN_TRIM_UNMAP_FLAG = true;
        }

        if (RUN_TRIM_UNMAP_FLAG)
        {
            uint64_t localStartLBA = TRIM_UNMAP_START_FLAG;
            uint64_t localRange = TRIM_UNMAP_RANGE_FLAG;
            if (USE_MAX_LBA)
            {
                localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG > 1)
                {
                    localRange = 1;
                }
            }
            else if (USE_CHILD_MAX_LBA)
            {
                localStartLBA = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG > 1)
                {
                    localRange = 1;
                }
            }
            if (localStartLBA != UINT64_MAX)
            {
                if (localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                }
                if (TRIM_UNMAP_RANGE_FLAG == 0 || TRIM_UNMAP_RANGE_FLAG == UINT64_MAX || (localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba)
                {
                    localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
                }
                if (PARTIAL_DATA_ERASE_FLAG)
                {
                    switch (trim_Unmap_Range(&deviceList[deviceIter], localStartLBA, localRange))
                    {
                    case SUCCESS:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Successfully trimmed/unmapped LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Trim/Unmap is not supported on this drive type.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Failed to trim/unmap LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    if (VERBOSITY_QUIET < g_verbosity)
                    {
                        printf("\n");
                        printf("You must add the flag:\n\"%s\" \n", PARTIAL_DATA_ERASE_ACCEPT_STRING);
                        printf("to the command line arguments to run a trim/unmap operation.\n\n");
                        printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample, TRIM_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, PARTIAL_DATA_ERASE_ACCEPT_STRING);
                    }
                }
            }
            else
            {
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("An invalid start LBA has been entered. Please enter a valid value.\n");
                }
            }
        }

        if (RUN_OVERWRITE_FLAG)
        {
            if (PARTIAL_DATA_ERASE_FLAG)
            {
                //check the time
                uint64_t overwriteSeconds = SECONDS_TIME_FLAG + (MINUTES_TIME_FLAG * 60) + (HOURS_TIME_FLAG * 3600);
                //determine if it's timed or a range
                if (overwriteSeconds == 0)
                {
                    int overwriteRet = UNKNOWN;
                    uint64_t localStartLBA = OVERWRITE_START_FLAG;
                    uint64_t localRange = OVERWRITE_RANGE_FLAG;
                    if (USE_MAX_LBA)
                    {
                        localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                        if (OVERWRITE_RANGE_FLAG == 0 || OVERWRITE_RANGE_FLAG > 1)
                        {
                            localRange = 1;
                        }
                    }
                    else if (USE_CHILD_MAX_LBA)
                    {
                        localStartLBA = deviceList[deviceIter].drive_info.bridge_info.childDeviceMaxLba;
                        if (OVERWRITE_RANGE_FLAG == 0 || OVERWRITE_RANGE_FLAG > 1)
                        {
                            localRange = 1;
                        }
                    }
                    if (localStartLBA > deviceList[deviceIter].drive_info.deviceMaxLba)
                    {
                        localStartLBA = deviceList[deviceIter].drive_info.deviceMaxLba;
                    }
                    //range based overwrite
                    if ((localStartLBA + localRange) > deviceList[deviceIter].drive_info.deviceMaxLba || localRange == UINT64_MAX || localRange == 0)
                    {
                        localRange = deviceList[deviceIter].drive_info.deviceMaxLba - localStartLBA + 1;
                    }
                    overwriteRet = erase_Range(&deviceList[deviceIter], localStartLBA, localStartLBA + localRange, NULL, 0, HIDE_LBA_COUNTER);
                    switch (overwriteRet)
                    {
                    case SUCCESS:
                        exitCode = 0;
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Successfully overwrote LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
                        }
                        break;
                    case NOT_SUPPORTED:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Erase Range is not supported on this drive type at this time.\n");
                        }
                        exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                        break;
                    default:
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("Failed to erase LBAs %"PRIu64" to %"PRIu64"\n", localStartLBA, localStartLBA + localRange - 1);
                        }
                        exitCode = UTIL_EXIT_OPERATION_FAILURE;
                        break;
                    }
                }
                else
                {
                    if (overwriteSeconds > 0)
                    {
                        int overwriteRet = UNKNOWN;
                        overwriteRet = erase_Time(&deviceList[deviceIter], OVERWRITE_START_FLAG, overwriteSeconds, NULL, 0, HIDE_LBA_COUNTER);
                        switch (overwriteRet)
                        {
                        case SUCCESS:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Successfully overwrote LBAs!\n");
                            }
                            break;
                        case NOT_SUPPORTED:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Overwrite Time is not supported on this drive type at this time.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                            break;
                        default:
                            if (VERBOSITY_QUIET < g_verbosity)
                            {
                                printf("Failed to overwrite for the entered amount of time.\n");
                            }
                            exitCode = UTIL_EXIT_OPERATION_FAILURE;
                            break;
                        }
                    }
                    else
                    {
                        if (VERBOSITY_QUIET < g_verbosity)
                        {
                            printf("You must specify a time to perform an overwrite for.\n");
                        }
                        exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                    }
                }
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("\n");
                    printf("You must add the flag:\n\"%s\" \n", PARTIAL_DATA_ERASE_ACCEPT_STRING);
                    printf("to the command line arguments to run an overwrite operation.\n\n");
                    printf("e.g.: %s -d %s --%s 0 --%s %s\n\n", util_name, deviceHandleExample, OVERWRITE_LONG_OPT_STRING, CONFIRM_LONG_OPT_STRING, PARTIAL_DATA_ERASE_ACCEPT_STRING);
                }
            }
        }

        if (SET_MAX_LBA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Setting MaxLBA to %"PRIu64"\n", SET_MAX_LBA_VALUE);
            }
            switch (set_Max_LBA(&deviceList[deviceIter], SET_MAX_LBA_VALUE, false))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully set the max LBA to %"PRIu64"\n", SET_MAX_LBA_VALUE);
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Setting the max LBA is not supported by this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to set the max LBA!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
        if (RESTORE_MAX_LBA_FLAG)
        {
            if (VERBOSITY_QUIET < g_verbosity)
            {
                printf("Restoring max LBA\n");
            }
            switch (set_Max_LBA(&deviceList[deviceIter], 0, true))
            {
            case SUCCESS:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Successfully restored the max LBA\n");
                }
                break;
            case NOT_SUPPORTED:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Restoring the max LBA is not supported by this device\n");
                }
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Failed to restore the max LBA!\n");
                }
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }

        if (PROGRESS_CHAR != NULL)
        {
            int result = UNKNOWN;
            //first take whatever was entered in progressTest and convert it to uppercase to do fewer string comparisons
            convert_String_To_Upper_Case(progressTest);
            //do some string comparisons to figure out what we are checking for progress on
            if (strcmp(progressTest, "DST") == 0 ||
                strcmp(progressTest, "SHORTDST") == 0 ||
                strcmp(progressTest, "LONGDST") == 0)
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Getting DST progress.\n");
                }
                result = print_DST_Progress(&deviceList[deviceIter]);
            }
            else if (strcmp(progressTest, "IDD") == 0)
            {
                uint8_t iddStatus = 0;
                char iddStatusString[MAX_DST_STATUS_STRING_LENGTH + 1] = { 0 };
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Getting IDD progress.\n");
                }
                result = get_IDD_Status(&deviceList[deviceIter], &iddStatus);
                translate_DST_Status_To_String(iddStatus, iddStatusString, false, false);
                printf("%s\n", iddStatusString);
            }
            else
            {
                if (VERBOSITY_QUIET < g_verbosity)
                {
                    printf("Unknown test \"%s\" for requested progress\n", progressTest);
                }
            }
            switch (result)
            {
            case UNKNOWN:
                exitCode = UTIL_EXIT_ERROR_IN_COMMAND_LINE;
                break;
            case SUCCESS:
            case IN_PROGRESS:
            case ABORTED:
                exitCode = UTIL_EXIT_OPERATION_ABORTED;
                break;
            case NOT_SUPPORTED:
                exitCode = UTIL_EXIT_OPERATION_NOT_SUPPORTED;
                break;
            default:
                exitCode = UTIL_EXIT_OPERATION_FAILURE;
                break;
            }
        }
    }
    exit(exitCode);
}


//-----------------------------------------------------------------------------
//
//  Utility_usage()
//
//! \brief   Description:  Dump the utility usage information
//
//  Entry:
//!   \param NONE
//!
//  Exit:
//!   \return VOID
//
//-----------------------------------------------------------------------------
void utility_Usage(bool shortUsage)
{
    //everything needs a help option right?
    printf("Usage\n");
    printf("=====\n");
    printf("\t %s [-d %s] {arguments} {options}\n\n", util_name, deviceHandleName);

    printf("Examples\n");
    printf("========\n");
    //example usage
    printf("\t%s --scan\n", util_name);
    printf("\t%s -d %s -i\n", util_name, deviceHandleExample);
    //return codes
    printf("\nReturn codes\n");
    printf("============\n");
    print_SeaChest_Util_Exit_Codes(0, NULL, util_name);

    //utility options - alphabetized
    printf("\nUtility Options\n");
    printf("===============\n");
#if defined (ENABLE_CSMI)
    print_CSMI_Force_Flags_Help(shortUsage);
    print_CSMI_Verbose_Help(shortUsage);
#endif
    print_Echo_Command_Line_Help(shortUsage);
    print_Enable_Legacy_USB_Passthrough_Help(shortUsage);
    print_Force_ATA_Help(shortUsage);
    print_Force_ATA_DMA_Help(shortUsage);
    print_Force_ATA_PIO_Help(shortUsage);
    print_Force_ATA_UDMA_Help(shortUsage);
    print_Force_SCSI_Help(shortUsage);
    print_Help_Help(shortUsage);
    print_Hide_LBA_Counter_Help(shortUsage);
    print_Time_Hours_Help(shortUsage);
    print_License_Help(shortUsage);
    print_Model_Match_Help(shortUsage);
    print_Time_Minutes_Help(shortUsage);
    print_Firmware_Revision_Match_Help(shortUsage);
    print_Only_Seagate_Help(shortUsage);
    print_Quiet_Help(shortUsage, util_name);
    print_SAT_12_Byte_CDB_Help(shortUsage);
    print_Time_Seconds_Help(shortUsage);
    print_Verbose_Help(shortUsage);
    print_Version_Help(shortUsage, util_name);

    //the test options
    printf("\nUtility arguments\n");
    printf("=================\n");
    //Common (across utilities) - alphabetized
    print_Scan_Help(shortUsage, deviceHandleExample);
    print_Scan_Flags_Help(shortUsage);
    print_Agressive_Scan_Help(shortUsage);
    print_Device_Help(shortUsage, deviceHandleExample);
    print_Device_Information_Help(shortUsage);
    print_SAT_Info_Help(shortUsage);
    print_Test_Unit_Ready_Help(shortUsage);
    //utility tests/operations go here - alphabetized
    print_Check_Power_Mode_Help(shortUsage);
    print_Display_LBA_Help(shortUsage);
    print_Firmware_Activate_Help(shortUsage);
    print_Firmware_Download_Help(shortUsage);
    print_Firmware_Download_Mode_Help(shortUsage);
    print_Short_DST_Help(shortUsage);
    print_IDD_Help(shortUsage);
    print_Poll_Help(shortUsage);
    print_Progress_Help(shortUsage, "dst, idd");
    print_Abort_DST_Help(shortUsage);
    print_Abort_IDD_Help(shortUsage);
    print_Phy_Speed_Help(shortUsage);
    print_Read_Look_Ahead_Help(shortUsage);
    print_Set_Max_LBA_Help(shortUsage);
    print_Spindown_Help(shortUsage);
    print_SMART_Check_Help(shortUsage);
    print_Restore_Max_LBA_Help(shortUsage);
    print_Write_Cache_Help(shortUsage);

    //SATA Only Options
    printf("\n\tSATA Only:\n\t=========\n");
    print_SMART_Attributes_Help(shortUsage);

    //SAS Only Options
    printf("\n\tSAS Only:\n\t=========\n");
    print_Set_Pin_11_Help(shortUsage);
    print_SAS_Phy_Help(shortUsage);


    //data destructive commands - alphabetized
    printf("\nData Destructive Commands\n");
    printf("===========================\n");
    //utility data destructive tests/operations go here
    print_Overwrite_Help(shortUsage);
    print_Overwrite_Range_Help(shortUsage);
    print_Provision_Help(shortUsage);
    print_Trim_Unmap_Help(shortUsage);
    print_Trim_Unmap_Range_Help(shortUsage);


}