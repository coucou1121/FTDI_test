#include "rts_cts_check.h"

RTS_CTS_Check::RTS_CTS_Check()
{

}


/*
  Simple example to read a large amount of data from a BM device.
  Device must have bitbang capabilities to enable this to work

  To build use the following gcc statement
  (assuming you have the d2xx library in the /usr/local/lib directory).
  compilation:
  gcc -o rts_cts_check RTS_CTS_Check.c -ldl -lpthread libftd2xx.a
  run:
  sudo nice -n -20 ./rts_cts_check
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ftd2xx.h"

#define SLEEP_TIME    1
#define DEBUG         0
#define DISPLAY_VALUE 0
#define DISPLAY_ERROR 0
#define DISPLAY_COUNTER_BUFFER 0
#define SHIFT         5
#define FRAME_SIZE    8         //byte
#define BUF_SIZE      512        //bit
//#define BAUD_RATE    256000      // speed of the USB port transfer
#define BAUD_RATE  1000000   //  speed of the USB port transfer
#define SIZE_OF_FILE  100000    // size if the file for reccording.
#define COUNTER_FIND 10            // Number of time we find successively the counter

//------------------------------------------------------------------------------------------------
//Global variables
//------------------------------------------------------------------------------------------------
char * pcBufRead;
DWORD dwBytesRead;
FILE * fh;

//FT_DEVICE ftDevice;
int iport;

FT_HANDLE ftHandle;
FT_DEVICE ftDevice;
FT_STATUS ftStatus;

DWORD 	dwBytesWritten, dwBytesRead;
DWORD	dwRxSize = 0;

//application
unsigned long i,j,k,l,m;
char rowValue;
char rowCounter;
char rowCounterFind;
char counterValue, memoCounterValue, counterError;
char txtFile[20] = "targetX.bin";
int counterValueHigh, counterValueLow;
long int sz;
int dataSend = 0xff;
int dataReceved = 0x00;


//FT_Read(ftHandle, pcBufRead, BUF_SIZE, &dwBytesRead);
//param@1: ftHandle             Handle of the device.
//param@2: lpBuffer             Pointer to the buffer that receives the data from the device.
//param@3: dwBytesToRead        Number of bytes to be read from the device.
//param@4: lpdwBytesReturned    Pointer to a variable of type DWORD which receives
//                              the number of bytes read from the device.
//start reading
// strcat(txtFile,".bin ");
// while(1)
//  {
//------------------------------------------------------------------------------------------------
//functions
//------------------------------------------------------------------------------------------------
int openSerialCommunication()
{

  ftStatus = FT_Open(iport, &ftHandle);
  if(ftStatus != FT_OK)
  {
      /*
     This can fail if the ftdi_sio driver is loaded
     use lsmod to check this and rmmod ftdi_sio to remove
     also rmmod usbserial
      */
      printf("FT_Open(%d) failed\n", iport);
      return 1;
  }
  else
  {
    printf("device open: \tok\n");
    return 0;
  }
}

int readDeviceInfo()
{
  DWORD deviceID;
  char SerialNumber[16];
  char Description[64];

  ftStatus = FT_GetDeviceInfo(ftHandle,
                  &ftDevice,
                  &deviceID,
                  SerialNumber,
                  Description,
                  NULL);
  if(ftStatus != FT_OK)
    {
      /*
     This can fail if the ftdi_sio driver is loaded
     use lsmod to check this and rmmod ftdi_sio to remove
     also rmmod usbserial
      */
      printf("readDeviceInfo(%d) failed\n", iport);
     }
  else
    {
      printf("ID: \t\t%d\n", deviceID);
      printf("Serial Number: \t%s \n", SerialNumber);
      printf("Description: \t%s \n", Description);
    }
   return((int)ftStatus);
}

int setUSBparameter()
{
// the USB request transfer siye
//param@1: fthandle Handle of the device
//param@2: DWORD    Transfer siye for USB IN request
//param@3: DWORD    Transfer siye for USB OUT request

  ftStatus = FT_SetUSBParameters(ftHandle, 256, 128);
  if(ftStatus != FT_OK)
    {
      printf("Error: FT_SetBaudRate returned %d \n");
    }
  else
    {
      printf("USB : \t\tset\n", BAUD_RATE);
    }
  return((int)ftStatus);
}

int setBaudRate()
{
  //set the baud rate for the device
  //param@1: fthandle      Handle of the device
  //param@2: dwBaudRate    Baud rate
  ftStatus = FT_SetBaudRate(ftHandle, BAUD_RATE);
  if(ftStatus != FT_OK)
    {
      printf("Error: FT_SetBaudRate returned %d \n", (int)ftStatus);
    }
  else
    {
      printf("Baud rate : \t%d\n", BAUD_RATE);
    }
  return((int)ftStatus);
}

int setFlowControl()
{
  //set the flow contole for the device
  //param@1: Handle of the device
  //param@2 : FT_FLOW_NONE       no control
  //          FT_FLOW_RTS_CTS    hardware conrol
  //          FT_FLOW_DTR_DSR    ... conrol
  //          FT_FLOW_XON_XOFF   caracter conrol
  ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0, 0);

  if (ftStatus != FT_OK)
    {
      printf("Error: FT_SetFlowControl returned %d \n");
    }
  else
    {
      printf("Flow Controle: \tFT_FLOW_RTS_CTS\n");
    }
   return((int)ftStatus);
}

int setDataCaracteristique()
{
  // Set the data caract[riatique for the device
  //param@1: fthandle     Handle of the device
  //param@2: uWordLenght  Number of bits per word,  must be FT_BITS_8 or FT_BITS_7
  //param@3: uStopBits    Number of stop bits, muste be FT_STOP_BITS_1 or FT_STOP_BITS_2
  //param@4: uParitz      Paritz, muste be FT_PARTIY_NONE, FT_PARITY_ODD, FT_PARITY_EVEN or FT_PARITY_SPACE
  ftStatus= FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);

  if (ftStatus != FT_OK)
    {
      // FT_SetRts failed
      printf("Error: FT_SetDataCharacteristic returned %d \n");
    }
  else
    {
      printf("data cara: \tset\n");
    }
  return((int)ftStatus);
}

int setRTS()
{
  //sets the Request To Send (RTS) control signal
  //param@1: Handle of the device
  ftStatus = FT_SetRts(ftHandle);
  if (ftStatus != FT_OK)
    {
      // FT_SetRts failed
      printf("Error: FT_SetRts returned %d \n");
    }
    else
      {
    printf("RTS: \t\tset\n");
      }
  return((int)ftStatus);
}

int resetRTS()
{
  //This function clears the Request To Send (RTS) control signal.
   //param@1: Handle of the device
  ftStatus = FT_ClrRts(ftHandle);
   if (ftStatus != FT_OK)
     {
       // FT_SetRts failed
       printf("Error: FT_ClrRts returned %d \n");
     }
   else
     {
       printf("RTS: \t\treset\n");
    }
   return((int)ftStatus);
}

int resetdevice()
{
  //sends a reset command to the device.
  //param@1: fthandle      Handle of the device
  // FT_ResetDevice(ftHandle);
  ftStatus = FT_ResetDevice(ftHandle);
  if ((ftStatus = FT_ResetDevice(ftHandle)) != FT_OK)
    {
    // FT_ResetDevice failed
      printf("Error: FT_ResetDevice returned %d \n");
  }
  else
    {
    // FT_ResetDevice  OK
      printf("Reset device : \tok\n");
  }
   return((int)ftStatus);
}

int freeTxRxBuffer()
{
  //This function purges receive and transmit buffers in the device.
  // Purge both Rx and Tx buffers
  //param@1: fthandle       Handle of the device
 ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX);
  if (ftStatus != FT_OK)
    {
    // FT_ResetDevice failed
      printf("Error: FT_ResetDevice returned %d \n");
  }
  else
    {
    // FT_ResetDevice  OK
      printf("Rx,Tx buffer: \tclean\n");
  }
   return((int)ftStatus);
}

int writeData()
{
  do
    {
      setRTS();
    }
  while(ftStatus != FT_OK);

   ftStatus = FT_Write(ftHandle, &dataSend, BUF_SIZE, &dwBytesWritten);

  /* Write */
  if(ftStatus != FT_OK)
    {
      printf("Error: FT_Write returned %d\n");
      // break;
    }
  else
    {
      printf("data Write : %x\n", dataSend);
    }

  resetRTS();
  return((int)ftStatus);
}

int readDataOneChar()
{
  /*Read*/
  dwRxSize = 1;
  printf("queue finshed\n");
  if(ftStatus == FT_OK)
    {
      ftStatus = FT_Read(ftHandle, &dataReceved, dwRxSize, &dwBytesRead);
    //     pcBufRead = (char*)realloc(pcBufRead, dwRxSize);
    if(ftStatus != FT_OK)
      {
        printf("Error: FT_Read returned %d\n");
      }
    else
      {
        printf("data read : %x\n", dataReceved);
      }
    }
  else
    {
      printf("Error: FT_GetQueueStatus returned %d\n", (int)ftStatus);
    }
  return((int)ftStatus);
}

int main(int argc, char *argv[])
{
    i,j,k=0;
  l = COUNTER_FIND;
  rowValue = 0;
  rowCounter = 0;
  rowCounterFind = 0;
  counterValue, memoCounterValue, counterError = 0;


//------------------------------------------------------------------------------------------------
//FTDI function
//------------------------------------------------------------------------------------------------


//set the Data Terminal Ready(DTR) control signal
//param@1: fthandle Handle of the device
FT_SetDtr(ftHandle);



// Set the read and write timeouts for the device
//param@1: fthandle       Handle of the device
//param@2: dwReadTimeout  Read timeout in miliseconds
//param@3: dwWriteTimeout Write timeout in miliseconds
FT_SetTimeouts(ftHandle, 1000, 1000);

//Enable different chips mode
//param@1: fthandle   Handle of the device
//param@2: ucMask     Read data sheet
//param@3: ucMode     Read data sheet	working mode 0x02 but counter not all time in the same place
//                                                 0x10 but counter not all time in the same place
//                                                 0x20 but counter not all time in the same place
//                                                 0x40 but counter not all time in the same place
FT_SetBitMode(ftHandle, 0xFF, 0x02);



 if(argc > 1)
   {
     sscanf(argv[1], "%d", &iport);
   }
 else
   {
     iport = 0;
   }

 //open serial communication
 openSerialCommunication();
 sleep(SLEEP_TIME);

 //read the device informations
 readDeviceInfo();
 sleep(SLEEP_TIME);

 //set the USB parameter
 setUSBparameter();
 sleep(SLEEP_TIME);

 //set the baud rate
 setBaudRate();
 sleep(SLEEP_TIME);

 //set flow control
 setFlowControl();
 sleep(SLEEP_TIME);

 //set the data caracteristique
 setDataCaracteristique();
 sleep(SLEEP_TIME);

 //empty the buffer
 freeTxRxBuffer();
 sleep(SLEEP_TIME);

 //reset the device
 //resetdevice();
 // sleep(SLEEP_TIME);

 //set RTS
setRTS();
 sleep(SLEEP_TIME);

 //reset RTS
  resetRTS();
 sleep(SLEEP_TIME);

 //write data
 //writeData();
 //sleep(SLEEP_TIME);

 //write data
 //readDataOneChar();
 // sleep(SLEEP_TIME);

 while(1)
   {
      readDataOneChar();
   }

return 0;
}
