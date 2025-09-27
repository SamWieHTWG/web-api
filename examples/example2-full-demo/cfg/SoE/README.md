# Configuration: SERCOS over EtherCAT

This configuration includes the connection of a SERCOS drive. In contrast to the standard configuration, the drive interface has therefore changed.

## Description

The following parameters have been adjusted in each axis configuration:

```
kenngr.antr_typ                                       2
antr.sercos.telegramm_typ                             7
antr.sercos.ethercat                                  1
antr.sercos.at[0].ident_len                           2
antr.sercos.at[0].nc_ref                              STATUS_WORD
antr.sercos.at[1].ident_len                           4
antr.sercos.at[1].nc_ref                              POSITION_FEEDBACK_VALUE_1
antr.sercos.at[2].ident_len                           2
antr.sercos.at[2].nc_ref                              BUS_STATE
antr.sercos.mdt[0].ident_len                          2
antr.sercos.mdt[0].nc_ref                             CONTROL_WORD
antr.sercos.mdt[1].ident_len                          4
antr.sercos.mdt[1].nc_ref                             POSITION_COMMAND_VALUE
```

These parameters describe the type and telegram structure of the drive interface. A SERCOS over EtherCAT drive has been configured here. The control telegram contains the control word and the target position. The status word, position feedback and the status of the bus are configured in the feedback telegram.
