(----------------------------------------------------------------------------)
( FILE NAME: sub_ch1_prog_start.nc                        DATE: Oct 29, 2021 )
( VERSION: 3.01.0100                                           Copyright ISG )
(----------------------------------------------------------------------------)
( NC-Code for program start.                                                 )
(----------------------------------------------------------------------------)
( HISTORY:                                                                   )
( Oct 29, 2021 Created                                                       )
(=================== End of NC-program header ===============================)

%SubProgStart

( Set slope type )
( N010 #SLOPE[TYPE=HSC]

( Activate kinematic transformation )
( N020 #TRAFO ON

( G129: Weighting of G00 velocity in % )
( G231: Weighting of G00 acceleration in % )
( G233: Weighting of G00 ramp time in % )
( N030 G129 = 100
( N031 G231=100
( N032 G233=100

( G130: Axis specific weighting of acceleration in % )
( G131: Axis group specific weighting of acceleration in % )
( N040 G130 X100 Y100 Z100 A100 B100 C100
( N041 G131 = 100

( G132: Axis specific weighting of ramp time to reach max. acceleration in % )
( G133: Axis group specific weighting of ramp time to reach max. acceleration in % )
( N050 G132 X100 Y100 Z100 A100 B100 C100 Y1=100
( N051 G133 = 100

( Axis group specific weighting of geometric ramp time in % )
( N060 G134 = 100

( Set feed axes )
( N070 #FGROUP [X,Y,Z,A,B]

( Set reduced speed limit )
( N080 #VECTOR LIMIT ON [VEL=12500]

N100 M29
