# gdb setup for J-Link - start JLinkGDBServer first
target remote localhost:2331
monitor reset
monitor speed 5
monitor speed auto
monitor long 0xffffff60 0x00320100
monitor long 0xfffffd44 0xa0008000
monitor long 0xfffffc20 0xa0000601
monitor sleep 100
monitor long 0xfffffc2c 0x00480a0e
monitor sleep 200
monitor long 0xfffffc30 0x7
monitor sleep 100
monitor long 0xfffffd08 0xa5000401
monitor sleep 100

