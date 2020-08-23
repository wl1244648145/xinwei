#ifndef __KERNEL_BASIC_DEFINE_H
#define __KERNEL_BASIC_DEFINE_H
/**************************************************************************//**
 @Description   General Handle
*//***************************************************************************/
typedef void *      t_Handle;   /**< handle, used as object's descriptor */

#define uint8_t        unsigned char
#define uint16_t       unsigned short int
#define uint32_t       unsigned int
#define uint64_t       unsigned long long int
#define int8_t         char
#define int16_t        short int
#define int32_t        int
#define int64_t        long long int
#define bool           int
#define uintptr_t      unsigned long

/**************************************************************************//**
 @Description   Error Code.

                The high word of the error code is the code of the software
                module (driver). The low word is the error type (e_ErrorType).
                To get the values from the error code, use GET_ERROR_TYPE()
                and GET_ERROR_MODULE().
*//***************************************************************************/
typedef uint32_t    t_Error;

typedef struct t_Device {
    int         id; /**< the device id */
    int         fd; /**< the device file-descritpor */
    t_Handle    h_UserPriv;
} t_Device;



/**************************************************************************//**
 @Description   A callback for enquing frame onto a QM queue.

 @Param[in]     h_App           - User's application descriptor.
 @Param[in]     p_Fd            - Frame descriptor for the frame.

 @Return        E_OK on success; Error code otherwise.
 *//***************************************************************************/
typedef t_Error (t_FmPcdQmEnqueueCallback) (t_Handle h_QmArg, void *p_Fd);

/**************************************************************************//**
 @Description   A structure for Host-Command
                When using Host command for PCD functionalities, a dedicated port
                must be used. If this routine is called for a PCD in a single partition
                environment, or it is the Master partition in a Multi partition
                environment, The port will be initialized by the PCD driver
                initialization routine.
 *//***************************************************************************/
typedef struct t_FmPcdHcParams {
    uintptr_t                   portBaseAddr;       /**< Host-Command Port Virtual Address of
                                                         memory mapped registers.*/
    uint8_t                     portId;             /**< Host-Command Port Id (0-6 relative
                                                         to Host-Command/Offline parsing ports) */
    uint16_t                    liodnBase;          /**< Irrelevant for P4080 rev 1. LIODN base for this port, to be
                                                         used together with LIODN offset. */
    uint32_t                    errFqid;            /**< Host-Command Port Error Queue Id. */
    uint32_t                    confFqid;           /**< Host-Command Port Confirmation queue Id. */
    uint32_t                    qmChannel;          /**< Host-Command port - QM-channel dedicated to
                                                         this port will be used by the FM for dequeue. */
    t_FmPcdQmEnqueueCallback    *f_QmEnqueue;       /**< Call back routine for enquing a frame to the QM */
    t_Handle                    h_QmArg;            /**< A handle of the QM module */
} t_FmPcdHcParams;



/**************************************************************************//**
 @Description   PCD interrupts
*//***************************************************************************/
typedef enum e_FmPcdExceptions {
    e_FM_PCD_KG_EXCEPTION_DOUBLE_ECC,                   /**< Keygen ECC error */
    e_FM_PCD_PLCR_EXCEPTION_DOUBLE_ECC,                 /**< Read Buffer ECC error */
    e_FM_PCD_KG_EXCEPTION_KEYSIZE_OVERFLOW,             /**< Write Buffer ECC error on system side */
    e_FM_PCD_PLCR_EXCEPTION_INIT_ENTRY_ERROR,           /**< Write Buffer ECC error on FM side */
    e_FM_PCD_PLCR_EXCEPTION_PRAM_SELF_INIT_COMPLETE,    /**< Self init complete */
    e_FM_PCD_PLCR_EXCEPTION_ATOMIC_ACTION_COMPLETE,     /**< Atomic action complete */
    e_FM_PCD_PRS_EXCEPTION_DOUBLE_ECC,                  /**< Parser ECC error */
    e_FM_PCD_PRS_EXCEPTION_SINGLE_ECC                   /**< Parser single ECC */
} e_FmPcdExceptions;


/**************************************************************************//**
 @Description   Exceptions user callback routine, will be called upon an
                exception passing the exception identification.

 @Param[in]     h_App      - User's application descriptor.
 @Param[in]     exception  - The exception.
  *//***************************************************************************/
typedef void (t_FmPcdExceptionCallback) (t_Handle h_App, e_FmPcdExceptions exception);


/**************************************************************************//**
 @Description   Exceptions user callback routine, will be called upon an exception
                passing the exception identification.

 @Param[in]     h_App           - User's application descriptor.
 @Param[in]     exception       - The exception.
 @Param[in]     index           - id of the relevant source (may be scheme or profile id).
 *//***************************************************************************/
typedef void (t_FmPcdIdExceptionCallback) ( t_Handle           h_App,
                                            e_FmPcdExceptions  exception,
                                            uint16_t           index);


/**************************************************************************//**
 @Description   The main structure for PCD initialization
 *//***************************************************************************/
typedef struct t_FmPcdParams {
    bool                        prsSupport;             /**< TRUE if Parser will be used for any
                                                             of the FM ports */
    bool                        ccSupport;              /**< TRUE if Coarse Classification will be used for any
                                                             of the FM ports */
    bool                        kgSupport;              /**< TRUE if Keygen will be used for any
                                                             of the FM ports */
    bool                        plcrSupport;            /**< TRUE if Policer will be used for any
                                                             of the FM ports */
    t_Handle                    h_Fm;                   /**< A handle to the FM module */
    uint8_t                     numOfSchemes;           /**< Number of schemes dedicated to this partition. */
    bool                        useHostCommand;         /**< Optional for single partition, Mandatory for Multi partition */
    t_FmPcdHcParams             hc;                     /**< Relevant only if useHostCommand=TRUE.
                                                             Host Command parameters. */

    t_FmPcdExceptionCallback    *f_Exception;           /**< Relevant for master (or single) partition only: Callback routine
                                                             to be called of PCD exception */
    t_FmPcdIdExceptionCallback  *f_ExceptionId;         /**< Relevant for master (or single) partition only: Callback routine
                                                             to be used for a single scheme and
                                                             profile exceptions */
    t_Handle                    h_App;                  /**< Relevant for master (or single) partition only: A handle to an
                                                             application layer object; This handle will
                                                             be passed by the driver upon calling the above callbacks */
} t_FmPcdParams;



/**************************************************************************//**
 @Collection    Definitions of coarse classification
                parameters as required by keygen (when coarse classification
                is the next engine after this scheme).
*//***************************************************************************/
#define FM_PCD_MAX_NUM_OF_CC_NODES          255
#define FM_PCD_MAX_NUM_OF_CC_TREES          8
#define FM_PCD_MAX_NUM_OF_CC_GROUPS         16
#define FM_PCD_MAX_NUM_OF_CC_UNITS          4
#define FM_PCD_MAX_NUM_OF_KEYS              256
#define FM_PCD_MAX_SIZE_OF_KEY              56
#define FM_PCD_MAX_NUM_OF_CC_ENTRIES_IN_GRP 16





#define INTG_MAX_NUM_OF_FM          2

/* Ports defines */
#define FM_MAX_NUM_OF_1G_RX_PORTS   5
#define FM_MAX_NUM_OF_10G_RX_PORTS  1
#define FM_MAX_NUM_OF_RX_PORTS      (FM_MAX_NUM_OF_10G_RX_PORTS+FM_MAX_NUM_OF_1G_RX_PORTS)
#define FM_MAX_NUM_OF_1G_TX_PORTS   5
#define FM_MAX_NUM_OF_10G_TX_PORTS  1
#define FM_MAX_NUM_OF_TX_PORTS      (FM_MAX_NUM_OF_10G_TX_PORTS+FM_MAX_NUM_OF_1G_TX_PORTS)
#define FM_MAX_NUM_OF_OH_PORTS      7
#define FM_MAX_NUM_OF_1G_MACS       (FM_MAX_NUM_OF_1G_RX_PORTS)
#define FM_MAX_NUM_OF_10G_MACS      (FM_MAX_NUM_OF_10G_RX_PORTS)
#define FM_MAX_NUM_OF_MACS          (FM_MAX_NUM_OF_1G_MACS+FM_MAX_NUM_OF_10G_MACS)



#define NCSW_IOC_TYPE_BASE          0xe0    /**< defines the IOCTL type for all
                                                 the NCSW Linux module commands */
                                                 
#define FM_IOC_TYPE_BASE            (NCSW_IOC_TYPE_BASE+1)
#define FMT_IOC_TYPE_BASE           (NCSW_IOC_TYPE_BASE+3)










#endif
