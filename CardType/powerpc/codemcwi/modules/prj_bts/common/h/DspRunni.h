#ifndef __DSP_RUNNING_H
#define __DSP_RUNNING_H

typedef enum
{
    DSP_RESET = 0xAA,   // initial value provided by FPGA
    DSP_BOOTING = 0x55, // DSP bootloader running indicator 
    DSP_RUNNING = 0x88  // application running state
}DSP_RUNNING_STATE;

#ifdef LARGE_BTS
extern DSP_RUNNING_STATE DspRunningState[M_NumberOfDSP+M_NUM_FEP];
#else
extern DSP_RUNNING_STATE DspRunningState;
#endif
// defined in L1TddIf.cpp

#endif
