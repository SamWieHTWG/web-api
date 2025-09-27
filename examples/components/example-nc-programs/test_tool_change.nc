%TestToolChange
N010 V.P.TOOLNR=1
N020 $FOR P1=1,3,1
N030   G00 G90 X0
N040   #TOOL PREP [V.P.TOOLNR] (info to tool management)
N050   G01 X10 F1000
N060   #TOOL DATA [V.P.TOOLNR] (tool change)
N070   G01 X20
N080   M6 TV.P.TOOLNR (tool change)
N090   G01 X30
N100   #TOOL LIFE READ[V.P.TOOLNR] (send tool wear data to tool management)
N110   G01 X40
N120   V.P.TOOLNR=V.P.TOOLNR+1
N130 $ENDFOR
N140 M30
