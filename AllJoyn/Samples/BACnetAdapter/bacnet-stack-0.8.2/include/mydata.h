/* Sample data structure for confirmed private transfer
   We have a simple data structure which can be written to
   and read from by sending a confirmed private transfer
   request with the appropriate parameters.
   */

#define MY_MAX_STR 32
#define MY_MAX_BLOCK 8

#define MY_SVC_READ 0
#define MY_SVC_WRITE 1

#define MY_ERR_OK			0
#define MY_ERR_BAD_INDEX	1

typedef struct MyData {
    uint8_t cMyByte1;
    uint8_t cMyByte2;
    float fMyReal;
    int8_t sMyString[MY_MAX_STR + 1];   /* A little extra for the nul */
} DATABLOCK;
