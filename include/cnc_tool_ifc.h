/*
Copyright (C)
ISG Industrielle Steuerungstechnik GmbH
https://www.isg-stuttgart.de
*/

/** \file
 * @brief TODO-DOC: Module description missing
*/

#ifndef CNC_TOOL_IFC_INC
#define CNC_TOOL_IFC_INC

/*
+----------------------------------------------------------------------------+
| cnc_tool_ifc.h                                        | Copyright    ISG   |
|                                                       |                    |
|                                                       | Gropiusplatz 10    |
|                                                       | 70563 Stuttgart    |
+-------------------------------------------------------+--------------------+
| CNC SDK external tool management functions and data structures for tool    |
| data exchange of CNC.                                                      |
+----------------------------------------------------------------------------+
*/

/*
+----------------------------------------------------------------------------+
| Defines                                                                    |
+----------------------------------------------------------------------------+
*/
#define CNCT_SDA_AXMAX                                 32
#define CNCT_KOPF_VERSATZ_MAX                          70
#define CNCT_FILE_NAME_L_EXT                          256
#define CNCT_WZV_PARAM_ANZ                             20
#define CNCT_CSG_MAX_NBR_GOBJECT                        5
#define CNCT_MAX_NBR_ADD_INFO                           2
#define CNCT_ORI_VECTOR_MAX                             3

/*
+----------------------------------------------------------------------------+
| Format of data interchange between tool manager <-> CNC                    |
+----------------------------------------------------------------------------+
*/
/*
+----------------------------------------------------------------------------+
| ToolInfoReadWrite                                                          |
| CNC -> (CNC_TOOL_ID)              -> ToolManagement                        |
| CNC <- (acknowldedge CNC_TOOL_ID) <- ToolManagement                        |
+----------------------------------------------------------------------------+
*/
typedef struct _cnc_tool_id                                                           /* Structure for the identification of the tools in the tool management        */
{                                                                                     /*-----------------------------------------------------------------------------*/
  int32_t                         basic;                                              /* Basic tool number                                                           */
  int32_t                         sister;                                             /* Tool number of sister tool                                                  */
  int32_t                         variant;                                            /* Variant number                                                              */
  uint8_t                         sister_valid;                                       /* Valid flag for sister tool number                                           */
  uint8_t                         variant_valid;                                      /* Valid flag for variant number                                               */
  uint8_t                         res1;                                               /* Reserved 1 - alignment bit                                                  */
  uint8_t                         res2;                                               /* Reserved 2 - alignment bit                                                  */
                                                                                      /*-----------------------------------------------------------------------------*/
} CNC_TOOL_ID;

/*
+----------------------------------------------------------------------------+
| ToolLifeDataReadWrite                                                      |
| CNC -> (CNC_TOOL_DATA_IN) -> ToolManagement                                |
| CNC <- (<empty>)          <- ToolManagement                                |
+----------------------------------------------------------------------------+
*/
typedef struct _cnc_tool_data_in                                                       /* Structure for tool life data                                                */
{                                                                                     /*-----------------------------------------------------------------------------*/
  CNC_TOOL_ID                     tool_id;                                            /* Tool identification                                                         */
  double                          time_used;                                          /* Time the tool was in use                                                    */
  double                          dist_used;                                          /* Distance the tool was in use                                                */
                                                                                      /*-----------------------------------------------------------------------------*/
} CNC_TOOL_DATA_IN;

/*
+----------------------------------------------------------------------------+
| ToolDataReadWrite                                                          |
| CNC -> (CNC_TOOL_REQUEST_IN) -> ToolManagement                             |
| CNC <- (CNC_TOOL_DESC)       <- ToolManagement                             |
+----------------------------------------------------------------------------+
*/
typedef struct _cnc_tool_request_in                                                   /* Structure to read tool data of tool management                              */
{                                                                                     /*-----------------------------------------------------------------------------*/
  CNC_TOOL_ID                     id;                                                 /* Tool identification                                                         */
  uint32_t                        act_t_nr;                                           /* Tool number - actual T number                                               */
  CNC_TOOL_ID                     last_d_nr;                                          /* Last D number                                                               */
  double                          param[CNCT_WZV_PARAM_ANZ];                          /* Tool parameter                                                              */
  uint16_t                        log_ax_nr_spdl;                                     /* Relation to the correct spindle                                             */
  uint8_t                         f_transp_data_access;                               /* Flag for transport data access                                              */
                                                                                      /*-----------------------------------------------------------------------------*/
} CNC_TOOL_REQUEST_IN;

/*
+----------------------------------------------------------------------------+
| CNC Tool data description                                                  |
+----------------------------------------------------------------------------+
*/
typedef struct _cnc_tool_desc
{                                                                                     /*-----------------------------------------------------------------------------*/
  CNC_TOOL_ID                     tool_id;                                            /* Tool identification                                                         */
  double                          orientation_vector[CNCT_ORI_VECTOR_MAX];            /* Vector of tool orientation                                                  */
  int32_t                         laenge;                                             /* Tool length                                                                 */
  int32_t                         radius;                                             /* Tool radius                                                                 */
  int32_t                         radius_path2;                                       /* Tool radius on 2. path                                                      */
  int32_t                         ax_versatz[CNCT_SDA_AXMAX];                         /* Axis offsets                                                                */
  int32_t                         axis_offset_org[(CNCT_SDA_AXMAX)];                  /* Original axis offsets                                                 */
  uint16_t                        typ;                                                /* Tool type - turning or milling tool                                         */
  uint16_t                        srk_lage;                                           /* Tool length for cutter radius compensation                                  */
  int32_t                         kopf_versatz[CNCT_KOPF_VERSATZ_MAX];                /* Tool head offsets (e.g. of 5 axes machine)                                  */
  uint16_t                        kin_id;                                             /* Tool specific kinematik ID                                                  */
  uint8_t                         tool_fixed;                                         /* Flag if tool is fixed                                                       */
  double                          param[CNCT_WZV_PARAM_ANZ];                          /* Additional tool parameter                                                   */
  uint16_t                        mass_einheit;                                       /* Measurement unit                                                            */
  uint8_t                         gueltig;                                            /* Flag if tool is valid                                                       */
  int8_t                          res1;                                               /* Reserved 1 - alignment bit                                                  */
  uint16_t                        log_ax_nr_spdl;                                     /* Relation to the correct spindle                                             */
  uint32_t                        add_info[CNCT_MAX_NBR_ADD_INFO];                    /* Additive information that is sent to PLC at tool change                     */
  double                          vb_min;                                             /* Minimum speed                                                               */
  double                          vb_max;                                             /* Maximum speed                                                               */
  double                          a_max;                                              /* Maximum acceleration                                                        */
  double                          wear_const;
  double                          ext_discret_limit;                                  /* External discrete limit                                                     */
  double                          disc_min_radius;                                    /* Minimum disc radius with wear consideration                                 */
  double                          disc_min_width;                                     /* Minimum disc width with wear consideration                                  */
  double                          disc_tilt_angle;                                    /* Tilt angle of the grinding disc                                             */
  int32_t                         gear_ratio_num;                                     /* Tool gear ratio numerator                                                   */
  int32_t                         gear_ratio_denom;                                   /* Tool gear ratio denominator                                                 */
  uint8_t                         gear_inv_direction;                                 /* Tool gear: Inverse movement direction.                                      */
  uint8_t                         gear_inv_direction_no_stop;                         /* Allow dir. change in movement if drive rotating direction is not changed.   */

} CNC_TOOL_DESC;

/*
+----------------------------------------------------------------------------+
| Function prototypes                                                        |
+----------------------------------------------------------------------------+
*/
#ifdef __cplusplus
extern "C" {
#endif

  /* Access functions to external tool management */
  uint64_t                        cnc_get_sizeof_cnc_tool_desc                (void);
  int32_t                         cnc_register_tool_change_info(int32_t(*p_function)(const CNC_TOOL_DESC* pData));
  int32_t                         cnc_register_read_tool_data  (int32_t(*p_function)(CNC_TOOL_DESC* pData));
  int32_t                         cnc_register_write_tool_data (int32_t(*p_function) (const CNC_TOOL_DATA_IN* pData));

#ifdef __cplusplus
}
#endif

#endif /* CNC_TOOL_IFC_INC */
