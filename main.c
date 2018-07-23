/*
 * XIMC_command.c
 *
 * Created: 18.07.2018 16:31:25
 * Author : tarabrina_n
 */ 

#define F_CPU 8000000L

#include <avr/io.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <util/delay.h>

//for command move (move to the exact Position and uPosition)
struct Request_Move
{
	uint32_t CMD; // name of the command ('move')
	int32_t	Position; //desired position
	int16_t uPosition; //position in micro steps (the range of valid values: -255... +255)
	uint8_t Reserved[6]; //6 bytes reserved
	uint16_t CRC; //Checksum
};

//for command gpos (get position) (answer - 26 bytes)
struct Request_Gpos
{
	uint32_t CMD; //Command
	int32_t	Position; //The position of the whole steps in the engine
	int16_t	uPosition; //Micro step position
	int64_t	EncPosition; //Encoder position
	uint8_t	Reserved[6]; //Reserved (6 bytes)
	uint16_t CRC; //Check sum
};

//UART3 and UART0 initializing
static void UARTInit(void)
{
	UCSR3A = 0; //double speed mode is off
	UCSR3B = (1 << RXEN3)|(1 << TXEN3); //permitting receive interrupts, receiving and transmission
	UCSR3C = (1 << UCSZ31)|(1 << UCSZ30)|(1 << USBS3); //8 bits, non-parity, 2 stop bite
	UBRR3H = 0;
	UBRR3L = 8; //baud rate 115200 bps

	UCSR0A = 0; //double speed mode is off
	UCSR0B = (1 << RXEN0)|(1 << TXEN0); //permitting receive interrupts, receiving and transmission
	UCSR0C = (1 << UCSZ01)|(1 << UCSZ00)|(1 << USBS0); //8 bits, non-parity, 2 stop bite
	UBRR0H = 0;
	UBRR0L = 8; //baud rate 115200 bps
}

//UART0 transmit handler
static void UART0_Transmit(uint8_t data)
{
	while (!(UCSR0A & (1<<UDRE0))); //waiting for data to transmit
	UDR0 = data;
}

//UART3 transmit handler
static void UART3_Transmit(uint8_t data)
{
	while (!(UCSR3A & (1<<UDRE3))); //waiting for data to transmit
	UDR3 = data;
}

//UART0 receive handler
unsigned char UART0_Receive(void)
{
	while (!(UCSR0A & (1 << RXC0))){}; //waiting for data
	return UDR0; //receive and return data received
}

//UART3 receive handler
unsigned char UART3_Receive(void)
{
	while (!(UCSR3A & (1 << RXC3))){}; //waiting for data
	return UDR3; //receive and return data received
}


//Calculating check sum CRC
//pbuf - buffer of data for calculate
//n - size of buffer
unsigned short CRC16(uint8_t *pbuf, unsigned short n)
{
	unsigned short crc, i, j, carry_flag, a;
	crc = 0xffff;
	for (i = 4; i < (n + 4); i++)
	{
		crc = crc ^ pbuf[i];
		for (j = 0; j < 8; j++)
		{
			a = crc;
			carry_flag = a & 0x0001;
			crc = crc >> 1;
			if (carry_flag == 1) crc = crc ^ 0xa001;
		}
	}
	return crc;
}

//Sending the command 'move' to the UART0
static void Command_Move(struct Request_Move move)
{
	uint8_t *buf_to_send = (uint8_t *)&move; //make the array from structure 'move' for transmitting

	//transmit command "move" (UART3) [18 bytes]
	for (int i = 0; i < sizeof(struct Request_Move); i++)
	{
		UART3_Transmit(buf_to_send[i]);
	}

	//transmit (UART0) the answer (UART3) from XIMC  ('move' [4 bytes])
	for (int i = 0; i < 4; i++)
	{
		//buf_to_read[i]=(uint8_t)UART_Receive();
		UART0_Transmit((uint8_t)UART3_Receive());
	}
}

//Sending the command "gpos" to the UART0 and get answer
uint8_t * Command_Gpos(struct Request_Gpos gpos, uint8_t *buf_to_read)
{
	uint8_t *buf_to_send = (uint8_t *)&gpos;

	//transmit command "gpos" (UART3) [4 bytes]
	for (int i = 0; i < 4; i++)
	{
		UART3_Transmit(buf_to_send[i]);
	}

	//get the answer (UART3) from XIMC  (struct Request_Gpos [26 bytes])
	//uint8_t buf_to_read[26];
	for (int i = 0; i < 26; i++)
	{
		buf_to_read[i]=(uint8_t)UART3_Receive();
		//UART0_Transmit((uint8_t)UART3_Receive());
	}

	//transmit the answer back (UART0)
	for (int i = 0; i < 26; i++)
	{
		UART0_Transmit(buf_to_read[i]);
	}

	return buf_to_read;
}

//Sending the command 'left' to the UART0
static void Command_Left(void)
{
	uint8_t buf_to_send[4] = { 'l', 'e', 'f', 't' };

	//transmit command "left" (UART3) [4 bytes]
	for (int i = 0; i < 4; i++)
	{
		UART3_Transmit(buf_to_send[i]);
	}

	//transmit (UART0) the answer from XIMC (UART3) ('left' [4 bytes])
	for (int i = 0; i < 4; i++)
	{
		UART0_Transmit((uint8_t)UART3_Receive());
	}
}

//Sending the command 'stop' to the UART0
static void Command_Stop(void)
{
	uint8_t buf_to_send[4] = { 's', 't', 'o', 'p' };

	//transmit command "stop" (UART3) [4 bytes]
	for (int i = 0; i < 4; i++)
	{
		UART3_Transmit(buf_to_send[i]);
	}

	//transmit (UART0) the answer from XIMC (UART3) ('stop' [4 bytes])
	for (int i = 0; i < 4; i++)
	{
		UART0_Transmit((uint8_t)UART3_Receive());
	}
}



int main(void)
{
	//Initializing UART
	UARTInit();

	uint8_t cmd_move[4] = { 'm', 'o', 'v', 'e' };
	uint8_t cmd_gpos[4] = { 'g', 'p', 'o', 's' };
	uint8_t buf_to_read[26];

	//Define the structure to send (command "gpos")
	struct Request_Gpos gpos;
	memcpy((void *)&gpos.CMD, (const void *)cmd_gpos, 4);

	//take the answer from the command "gpos" and put it in the array
	memcpy((void *)&buf_to_read, (const void *)Command_Gpos(gpos, buf_to_read), 26);

	//make the structure "gpos" from array
	memcpy((void *)&gpos.Position, (const void *)&buf_to_read[4], 4);
	memcpy((void *)&gpos.uPosition, (const void *)&buf_to_read[8], 2);
	memcpy((void *)&gpos.EncPosition, (const void *)&buf_to_read[10], 8);
	memcpy((void *)&gpos.Reserved, (const void *)&buf_to_read[18], 6);
	memcpy((void *)&gpos.CRC, (const void *)&buf_to_read[24], 2);

	_delay_ms(2000);

	
	//Move left for 2 sec
	Command_Left(); 
	_delay_ms(3000);

	//Stop
	Command_Stop();
	_delay_ms(2000);

	//Define the structure to send command 'move'
	struct Request_Move move;
	memcpy((void *)&move.CMD, (const void *)cmd_move, 4);  //command 'move'
	move.Position = gpos.Position; //Position
	move.uPosition = gpos.uPosition; //u Position (-255 ... +255)
	memset((void *)move.Reserved, 0, 6);

	
	uint8_t *pbuf1 = (uint8_t *)&move;//make array from structure to send

	move.CRC = CRC16(pbuf1, 12); //calculate check sum

	Command_Move(move); 
	_delay_ms(2000);

    while (1) 
    {
		
    }
}

