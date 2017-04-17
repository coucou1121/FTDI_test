#include "ftdi_Function.h"

FTDI_Function::FTDI_Function()
{

    /*
        Simple example to open a maximum of 4 devices - write some data then read it back.
        Shows one method of using list devices also.
        Assumes the devices have a loopback connector on them and they also have a serial number

        To build use the following gcc statement
        (assuming you have the static d2xx library in the /usr/local/lib directory
        and you have created a symbolic link to it in the current dir).
        gcc -o static_link main.c -ldl -lpthread libftd2xx.a
    */

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include "ftd2xx.h"

    #define BUF_SIZE 0x10
    #define SLEEP_TIME 1
    #define BAUD_RATE    1000000      // speed of the USB port transfer

    int main(int argc, char *argv[])
    {
      FT_HANDLE ftHandle;
      FT_DEVICE ftDevice;
      FT_STATUS ftStatus;
      DWORD deviceID;
      char SerialNumber[16];
      char Description[64];

      char * 	pcBufRead = NULL;
      DWORD	dwRxSize = 0;
      DWORD 	dwBytesWritten, dwBytesRead;

      //  char 	cBufWrite[BUF_SIZE] = "coucou";
      char*	cBufWrite = "coucou";

      int	iNumDevs;
      int	i,j,k;
      int	iDevicesOpen;

      j=0;

      if(argc > 1)
        {
          sscanf(argv[1], "%d", &iNumDevs);
        }
      else
        {
          iNumDevs = 0;
        }

      //open serial communication
      if ((ftStatus = FT_Open(iNumDevs, &ftHandle)) != FT_OK)
        {
          // FT_Open failed
          /*
         This can fail if the ftdi_sio driver is loaded
         use lsmod to check this and rmmod ftdi_sio to remove
         also rmmod usbserial
          */
          printf("Failes to open device\n");
        }
      else
        {
          // FT_Open OK, use ftHandle to access device
          ftStatus = FT_GetDeviceInfo(ftHandle,
                      &ftDevice,
                      &deviceID,
                      SerialNumber,
                      Description,
                      NULL);
          puts("device open:");
          printf("ID: \t\t%d\n", deviceID);
          printf("Serial Number: \t%s \n", SerialNumber);
          printf("Description: \t%s \n", Description);
          sleep(SLEEP_TIME);
        }


      /*Reset the device for test*/
      //sends a reset command to the device.
      //param@1: fthandle      Handle of the device
      // FT_ResetDevice(ftHandle);
      ftStatus = FT_ResetDevice(ftHandle);
      if ((ftStatus = FT_ResetDevice(ftHandle)) != FT_OK)
        {
        // FT_ResetDevice failed
        printf("Error: FT_ResetDevice returned %d \n", (int)ftStatus);
      }
      else
        {
        // FT_ResetDevice  OK
          printf("Reset device : \tok\n");
          sleep(SLEEP_TIME);
      }


      /* Setup */
      //BAUD RATE
      //set the baud rate for the device
      //param@1: fthandle      Handle of the device
      //param@2: dwBaudRate    Baud rate
      if((ftStatus = FT_SetBaudRate(ftHandle, BAUD_RATE)) != FT_OK)
        {
          printf("Error: FT_SetBaudRate returned %d \n", (int)ftStatus);
        }
      else
        {
          printf("Baud rate : \t%d\n", BAUD_RATE);
          sleep(SLEEP_TIME);
        }

      //FLOW CONTROL
      //set the flow contole for the device
      //param@1: Handle of the device
      //param@2 : FT_FLOW_NONE       no control
      //          FT_FLOW_RTS_CTS    hardware conrol
      //          FT_FLOW_DTR_DSR    ... conrol
      //          FT_FLOW_XON_XOFF   caracter conrol

      if ((ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_RTS_CTS, 0, 0)) != FT_OK)
        {
           printf("Error: FT_SetFlowControl returned %d \n", (int)ftStatus);
        }
      else
        {
          printf("Flow Controle: \tFT_FLOW_RTS_CTS\n");
          sleep(SLEEP_TIME);
        }

      //SET RTS
      //sets the Request To Send (RTS) control signal
      //param@1: Handle of the device
      if ((ftStatus = FT_SetRts(ftHandle)) != FT_OK)
        {
          // FT_SetRts failed
          printf("Error: FT_SetRts returned %d \n", (int)ftStatus);
        }
        else
          {
        printf("RTS: \t\tset\n");
        sleep(SLEEP_TIME);
          }

      /* Write */
       if((ftStatus = FT_Write(ftHandle, cBufWrite, BUF_SIZE, &dwBytesWritten)) != FT_OK)
        {
          printf("Error: FT_Write returned %d\n", (int)ftStatus);
          // break;
        }
       else
         {
            printf("start Write\n");
        sleep(SLEEP_TIME);
         }


      // printf("start read\n");
      /*Read*/
      dwRxSize = 0;
      while ((dwRxSize < BUF_SIZE) && (ftStatus == FT_OK))
        {
          ftStatus = FT_GetQueueStatus(ftHandle, &dwRxSize);
        }
      printf("queue finshed\n");
      if(ftStatus == FT_OK)
        {
          pcBufRead = (char*)realloc(pcBufRead, dwRxSize);
          if((ftStatus = FT_Read(ftHandle, pcBufRead, dwRxSize, &dwBytesRead)) != FT_OK)
        {
          printf("Error: FT_Read returned %d\n", (int)ftStatus);
        }
          else
        {
          j=(int)dwBytesRead;
          printf("FT_Read read %d bytes\n", j);
          printf("%s \n",pcBufRead);
          sleep(SLEEP_TIME);
        }
        }
      else
        {
          printf("Error: FT_GetQueueStatus returned %d\n", (int)ftStatus);
        }

       //RESET RTS
       //clear the Request To Send (RTS) control signal
       //param@1: Handle of the device
       if ((ftStatus = FT_ClrRts(ftHandle)) != FT_OK)
         {
           // FT_SetRts failed
           printf("Error: FT_ClrRts returned %d \n", (int)ftStatus);
         }
       else
         {
           printf("RTS: \t\treset\n");
           sleep(SLEEP_TIME);
         }

      //second loop//
      cBufWrite = "the end";
     //SET RTS
      if ((ftStatus = FT_SetRts(ftHandle)) != FT_OK)
        {
          // FT_SetRts failed
          printf("Error: FT_SetRts returned %d \n", (int)ftStatus);
        }
        else
          {
        printf("RTS: \t\tset\n");
        sleep(SLEEP_TIME);
          }

      /* Write */
       if((ftStatus = FT_Write(ftHandle, cBufWrite, BUF_SIZE, &dwBytesWritten)) != FT_OK)
        {
          printf("Error: FT_Write returned %d\n", (int)ftStatus);
          // break;
        }
       else
         {
            printf("start Write\n");
        sleep(SLEEP_TIME);
         }



      // printf("start read\n");
      /*Read*/
      dwRxSize = 0;
      while ((dwRxSize < BUF_SIZE) && (ftStatus == FT_OK))
        {
          ftStatus = FT_GetQueueStatus(ftHandle, &dwRxSize);
        }
      printf("queue finshed\n");
      if(ftStatus == FT_OK)
        {
          pcBufRead = (char*)realloc(pcBufRead, dwRxSize);
          if((ftStatus = FT_Read(ftHandle, pcBufRead, dwRxSize, &dwBytesRead)) != FT_OK)
        {
          printf("Error: FT_Read returned %d\n", (int)ftStatus);
        }
          else
        {
          j=(int)dwBytesRead;
          printf("FT_Read read %d bytes\n", j);
          printf("%s \n",pcBufRead);
          sleep(SLEEP_TIME);
        }
        }
      else
        {
          printf("Error: FT_GetQueueStatus returned %d\n", (int)ftStatus);
        }

        //RESET RTS
      if ((ftStatus = FT_ClrRts(ftHandle)) != FT_OK)
        {
          // FT_SetRts failed
          printf("Error: FT_ClrRts returned %d \n", (int)ftStatus);
        }
        else
          {
        printf("RTS: \t\treset\n");
        sleep(SLEEP_TIME);
          }


    /* Cleanup */
    if ((ftStatus = FT_Close(ftHandle)) != FT_OK)
      {
        printf("Error: FT_Close returned %d\n", (int)ftStatus);
      }
     else
       {
         printf("device Closed\n");
       }

    if(pcBufRead)
      {
        free(pcBufRead);
      }
    return 0;
    }

}
