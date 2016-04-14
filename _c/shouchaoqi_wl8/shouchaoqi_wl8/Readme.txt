-------- PROJECT GENERATOR --------
PROJECT NAME :	shouchaoqi_wl8
PROJECT DIRECTORY :	E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8
CPU SERIES :	RX600
CPU TYPE :	RX62T
TOOLCHAIN NAME :	Renesas RX Standard Toolchain
TOOLCHAIN VERSION :	1.2.0.0
GENERATION FILES :
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\dbsct.c
        Setting of B,R Section
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\typedefine.h
        Aliases of Integer Type
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\sbrk.c
        Program of sbrk
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\iodefine.h
        Definition of I/O Register
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\intprg.c
        Interrupt Program
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\vecttbl.c
        Initialize of Vector Table
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\vect.h
        Definition of Vector
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\resetprg.c
        Reset Program
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\shouchaoqi_wl8.c
        Main Program
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\sbrk.h
        Header file of sbrk file
    E:\rx_program\shouchaoqi_wl8\shouchaoqi_wl8\stacksct.h
        Setting of Stack area
START ADDRESS OF SECTION :
 H'1000	B_1,R_1,B_2,R_2,B,R,SU,SI
 H'FFFF8000	PResetPRG
 H'FFFF8100	C_1,C_2,C,C$*,D_1,D_2,D,P,PIntPRG,W*,L
 H'FFFFFFD0	FIXEDVECT

* When the user program is executed,
* the interrupt mask has been masked.
* 
* Program start 0xFFFF8000.
* RAM start 0x1000.

SELECT TARGET :
    RX600 E1/E20 SYSTEM
DATE & TIME : 2012-10-21 12:13:46
