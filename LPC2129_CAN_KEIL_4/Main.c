#include<lpc21xx.h>
#include<stdio.h>


unsigned char data[9]={0,0,0,0,1,0,0,0,0};
unsigned char a[9]={0,0,0,0,0,0,0,0,0};

int id;

void uart_init(void)
			{
						PINSEL0  =  (0x01 << 0) | (0x01 << 2);  						 // Selecting UART0 Tx and Rx P0.0 and P0.1;
						U0LCR= (0x3 << 0) | (0x1 << 7);                      // Uart Line Control Register
						U0DLL=0XC3;                          								 // Uart Baud Rate Register
						U0DLM=0X00;                         								 // Uart Baud Rate Register
						U0LCR=(0x3 << 0);                          					 // Uart Line Control Register
			} 
void can_init(char d[])
			{
						PINSEL1 |= ((0x01 << 14) | (0x01 <<16));
						AFMR=0X02; 																					 // All receiver messages are accepted
						C2MOD=0X01;  																				 // Set CAN controller into reset   
						C2IER=0X00; 																		  	 // Disable all interrupts 		 																				 
						C2BTR= ((d[3] << 0) | (d[2] << 16) | (d[1] << 20));  // Baud rate value to 500Kbps with Baudrate prescaler = 5,Sync Segment=1,TS1=8,TS2=3
						C2MOD=0X00;  																				 // Release CAN controller       
			}


void uart_tx(char a[])                                           // Function to Send data to GUI
			{	
						int j=0;
						while(j!=9)
									{
											U0THR=a[j];                                // Received CCP Frame from Slave is sent to GUI
											j++;
											while((U0LSR&0X20)!=0X20);                 // Wait till transmission is completed 
									}
			}


 
void can_tx(char data[])																				 // Function to Send data to Slave	
			{
						C2TFI1=(0x8<<16);                                    // Data length code = 8, RTR =0,11 bit Identifier;
						C2TID1=id;                                         // Identifier is set to 34(dec)
						C2TDA1=(data[1]) | (data[2]<<8) | (data[3]<<16) | (data[4]<<24);             // 4 bytes of data is added
						C2TDB1=(data[5]) | (data[6]<<8) | (data[7]<<16) | (data[8]<<24);             // 4  bytes of data is added
						C2CMR=0X21;                                          // Transmission request and Transmitter buffer 1 is selected;
						while((C2SR & 0x08 )!= 0x08);                        // wait till transmission is completed
	
			}


 
void uart_rx()                                                   // Function thats receives data from GUI
			{	
						int i=0;
						while(i!=9)
									{
										while(!(U0LSR&0X01));                        // Check if data is present in Rx Buffer
															{
																data[i]= U0RBR;
																i++;
															}
									}
			}



void can_rx()                                                    // Function that receives data from Slave
			{

						int i,j=0,k=4;

						while (!(C2GSR & 0x01));                            // Checking if data is present in Rx buffer
						if(C2RID==112)                                      // Checking the Identifier of Slave
									{
										for(i=0;i<4;i++)
															{
																a[j]=((C2RDA >> (i*8)) & 255); //Storing the Buffer value into array
																j++;
															}
										for(i=0;i<4;i++)
															{
																a[k]=((C2RDB >> (i*8)) & 255);  //Storing the Buffer value into array
																k++;
															}
									}

						C2CMR=0x04;                                         // Clears the Rx buffer
			} 	




int main()
			{
						uart_init();				                               //Uart Initialization
						
					
		while(1)
								 {
										uart_rx();			                           // Uart Receive Function (Data received from GUI is Stored)
									 if(data[0]=='B')
									 {
										 can_init(data);
										 id=data[7]+data[6]*100;
										 uart_tx(data);
									 }
									 else if(data[0]=='T')
									 {
										can_tx(data);		                           // CAN Transmit Function	(Sending the CCP frame to Slave)
										can_rx();				                           // CAN Receive Function	(Receiving the CCP frame from Slave)
										uart_tx(a);			                           // Uart Transmit Function (Sending received CCP frame to GUI)
								   }
							 }
					
      }

