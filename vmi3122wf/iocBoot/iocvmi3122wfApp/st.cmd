# Example vxWorks startup file

< cdCommands
 
< ../nfsCommands

cd appbin

# testLib includes iocCore 
ld < testLib

cd top
dbLoadDatabase("dbd/vmi3122wf.dbd")

dbLoadRecords("db/vmi3122wf.db")

cd startup

# configure vmi3122 as waveform recorder 
#Vmi3122Config(numcards, offset, buffer size, scan rate, irq_level, ivec_base)
Vmi3122Config(1,0xc00000,1024,100000,3,0x80)

iocInit

