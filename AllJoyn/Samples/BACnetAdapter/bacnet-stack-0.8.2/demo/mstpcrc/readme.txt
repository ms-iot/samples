BACnet MS/TP CRC Calculator

This tool receives MS/TP bytes and generates a CRC for those bytes

mstpcrc [options] <00 00 00 00...>
perform MS/TP CRC on data bytes.
options:
[-x] interprete the arguments as ascii hex (default)
[-d] interprete the argument as ascii decimal
[-8] calculate the MS/TP 8-bit Header CRC (default)
[-16] calculate the MS/TP 16-bit Data CRC

Here is a sample of the tool running (use CTRL-C to quit):
D:\code\bacnet-stack\demo\mstpcrc>mstpcrc 06 ff 01 00 15
0x06
0xFF
0x01
0x00
0x15
0x8E Header CRC
