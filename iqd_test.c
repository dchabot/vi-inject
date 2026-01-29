/* Copyright 2025 Osprey DCS
 * All rights reserved
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include "test3.h"
#include "NiFpga.h"


static void test_status(NiFpga_Status sts) {
    if(NiFpga_IsError(sts)) {
        syslog(LOG_USER, "ERR - NiFpga_Status = %d\n", sts);
    }
}

static NiFpga_Session sess = 0;

void lib_init(void) {
    openlog(NULL, LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_USER, "%s loaded\n", __FILE__);
}

void lib_fini(void) {
    syslog(LOG_USER, "%s unloading\n", __FILE__);
    closelog();
}

int init(char* bitfile, char *resource) {
    /* data extraction
    -------------------
    * from MAD email 08/25/25:
        * Use of IQD with one-shot trigger source.
            1. Open session with FPGA without reset without bit file load
            2. Poll FPGA_RUNNING until reading 1
                * future paranoia.  Disarm ACQ.
            3. Setup FIFO to card eg. "IQD2.6-Host"
                * future paranoia.  Flush/drain FIFO.
            4. Set "ACQ_Record Size" = 512 (valid values 100 - 512)
            5. Set ACQ_Arm = 0
            6. Set ACQ_Arm = 1 (action on 0->1 edge)
            7. read ACQ_Armed_RB == 1 to confirm
            8. read initial value of "ACQ_Record Index_RB"
            9. Set TRIG_Once = 0
            10. Set TRIG_Once = 1 (action on 0->1 edge)
            11. read "ACQ_Record Index_RB", confirm increment by 1
            12. read "ACQ_Sample Index_RB" confirm 511 (Size-1)
            13. Read 4x512xU64 from FIFO

        * Change to internal trigger source
            1. Instead of read "FPGA_Clock_Hz_RB" and set "TRIG_INT Clock_Div"
               according to the desired trigger rate.  (rate = Clock_Hz / Div)
            2. set "TRIG_INT Enable" = 1
    */

    int rc = 0;

    if(bitfile == NULL || resource == NULL) {
        syslog(LOG_USER, "bitfile and resource args must be non-null\n");
        closelog();
        return 1;
    }
    else {
        syslog(LOG_USER, "bitfile = %s, resource = %s\n", bitfile, resource);
    }

    NiFpga_Status sts = 0;

    sts = NiFpga_Initialize();
    test_status(sts);
    sts = NiFpga_Open(bitfile, NULL, resource,
                      NiFpga_OpenAttribute_NoRun | NiFpga_OpenAttribute_IgnoreSignatureArgument,
                      &sess);
    test_status(sts);

    uint32_t reg_offs = 0;
    sts = NiFpga_FindRegister(sess, "FPGA_RUNNING", &reg_offs);
    test_status(sts);
    uint8_t reg_val = 0;
    sts =  NiFpga_ReadU8(sess, reg_offs, &reg_val);
    test_status(sts);

    uint32_t loop_cnt = 0;
    while(reg_val != 1) {
        syslog(LOG_USER, "Waiting for FPGA...\n");
        if(++loop_cnt >= 10) {
            syslog(LOG_USER, "Timed out waiting for FPGA\n");
            rc = -1;
            goto bail;
        }
        sleep(1);
        sts =  NiFpga_ReadU8(sess, reg_offs, &reg_val);
        test_status(sts);
    }
    syslog(LOG_USER, "FPGA is running\n");

    sts = NiFpga_FindRegister(sess, "FPGA_SW Version_RB", &reg_offs);
    test_status(sts);
    sts = NiFpga_ReadU8(sess, reg_offs, &reg_val);
    syslog(LOG_USER, "FPGA_SW Version_RB = %x\n", reg_val);

    uint32_t fifo_num = 0;
    const char *fifo_name = "IQD1.6-Host";
    syslog(LOG_USER, "Fifo name = %s\n", fifo_name);
    sts = NiFpga_FindFifo(sess, fifo_name, &fifo_num);
    test_status(sts);

    size_t depth_req = 512, *depth_act = 0;
    sts = NiFpga_ConfigureFifo2(sess, fifo_num, depth_req, depth_act);
    test_status(sts);
    if(depth_req != *depth_act) {
        syslog(LOG_USER, "requested depth = %lu, actual depth = %lu\n", depth_req, *depth_act);
        rc = -2;
        goto bail;
    }

    sts = NiFpga_FindRegister(sess, "ACQ_Record Size", &reg_offs);
    test_status(sts);
    sts = NiFpga_WriteU16(sess, reg_offs, 512);
    test_status(sts);

    /* toggle Acquisition Armed bit */
    sts = NiFpga_FindRegister(sess, "ACQ_Arm", &reg_offs);
    test_status(sts);
    NiFpga_Bool ni_bool = 0;
    sts = NiFpga_WriteBool(sess, reg_offs, ni_bool);
    test_status(sts);
    ni_bool = 1;
    sts = NiFpga_WriteBool(sess, reg_offs, ni_bool);
    test_status(sts);

    sts = NiFpga_FindRegister(sess, "ACQ_Armed_RB", &reg_offs);
    test_status(sts);
    sts = NiFpga_ReadBool(sess, reg_offs, &ni_bool);
    test_status(sts);
    syslog(LOG_USER, "ACQ_Armed_RB = %d\n", ni_bool);
    if(ni_bool == 0) {
        syslog(LOG_USER, "Could not arm acquisition\n");
        rc = -3;
        goto bail;
    }

    sts = NiFpga_FindRegister(sess, "ACQ_Record Index_RB", &reg_offs);
    uint16_t ni_u16 = 0;
    sts = NiFpga_ReadU16(sess, reg_offs, &ni_u16);
    test_status(sts);
    syslog(LOG_USER, "ACQ_Record Index_RB initial value = %hd\n", ni_u16);

    sts = NiFpga_FindRegister(sess, "Trig_Once", &reg_offs);
    test_status(sts);
    ni_bool = 1;
    sts = NiFpga_WriteBool(sess, reg_offs, ni_bool);
    test_status(sts);

bail:
    sts = NiFpga_StopFifo(sess, fifo_num);
    syslog(LOG_USER, "NiFpga_Status = %d\n", sts);
    sts = NiFpga_ReleaseFifoElements(sess, fifo_num, *depth_act);
    syslog(LOG_USER, "NiFpga_Status = %d\n", sts);
    sts = NiFpga_Close(sess, NiFpga_CloseAttribute_NoResetIfLastSession);
    syslog(LOG_USER, "NiFpga_Status = %d\n", sts);
    sts = NiFpga_Finalize();
    syslog(LOG_USER, "NiFpga_Status = %d\n", sts);

    return rc;

}
