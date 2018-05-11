/*
* Simple app to read/write into custom IP in PL via /dev/uoi0 interface
* To compile for arm: make ARCH=arm CROSS_COMPILE=arm-xilinx-linux-gnueabi-
* Author: Tsotnep, Kjans
*/
  
//C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>     // For socket related functions
#include <netinet/in.h>     // For address struct
#include <string.h>         // For memset"
#include <arpa/inet.h>     // For address struct

#include <sys/types.h> 		//fifo
#include <sys/stat.h>		//fifo
#include <pthread.h>


#include "ZedboardOLED.c"

#include <unistd.h>


#define VOLUME_0_REG_0   *((unsigned *)(ptr + 0))
#define VOLUME_0_REG_1   *((unsigned *)(ptr + 4))
#define VOLUME_0_REG_2   *((unsigned *)(ptr + 8))
#define VOLUME_0_REG_3   *((unsigned *)(ptr + 12))

#define FILTER_0_REG_0   *((unsigned *)(ptr2 + 0))
#define FILTER_0_REG_1   *((unsigned *)(ptr2 + 4))
#define FILTER_0_REG_2   *((unsigned *)(ptr2 + 8))
#define FILTER_0_REG_3   *((unsigned *)(ptr2 + 12))
#define FILTER_0_REG_4   *((unsigned *)(ptr2 + 16))
#define FILTER_0_REG_5   *((unsigned *)(ptr2 + 20))
#define FILTER_0_REG_6   *((unsigned *)(ptr2 + 24))
#define FILTER_0_REG_7   *((unsigned *)(ptr2 + 28))
#define FILTER_0_REG_8   *((unsigned *)(ptr2 + 32))
#define FILTER_0_REG_9   *((unsigned *)(ptr2 + 36))
#define FILTER_0_REG_10   *((unsigned *)(ptr2 + 40))
#define FILTER_0_REG_11   *((unsigned *)(ptr2 + 44))
#define FILTER_0_REG_12   *((unsigned *)(ptr2 + 48))
#define FILTER_0_REG_13   *((unsigned *)(ptr2 + 52))
#define FILTER_0_REG_14   *((unsigned *)(ptr2 + 56))
#define FILTER_0_REG_15   *((unsigned *)(ptr2 + 60))
#define FILTER_0_REG_16   *((unsigned *)(ptr2 + 64))
#define FILTER_0_REG_17   *((unsigned *)(ptr2 + 68))
#define FILTER_0_REG_18   *((unsigned *)(ptr2 + 72))
#define FILTER_0_REG_19   *((unsigned *)(ptr2 + 76))

#define VOLUME_1_REG_0   *((unsigned *)(ptr3 + 0))
#define VOLUME_1_REG_1   *((unsigned *)(ptr3 + 4))
#define VOLUME_1_REG_2   *((unsigned *)(ptr3 + 8))
#define VOLUME_1_REG_3   *((unsigned *)(ptr3 + 12))

#define FILTER_1_REG_0   *((unsigned *)(ptr4 + 0))
#define FILTER_1_REG_1   *((unsigned *)(ptr4 + 4))
#define FILTER_1_REG_2   *((unsigned *)(ptr4 + 8))
#define FILTER_1_REG_3   *((unsigned *)(ptr4 + 12))
#define FILTER_1_REG_4   *((unsigned *)(ptr4 + 16))
#define FILTER_1_REG_5   *((unsigned *)(ptr4 + 20))
#define FILTER_1_REG_6   *((unsigned *)(ptr4 + 24))
#define FILTER_1_REG_7   *((unsigned *)(ptr4 + 28))
#define FILTER_1_REG_8   *((unsigned *)(ptr4 + 32))
#define FILTER_1_REG_9   *((unsigned *)(ptr4 + 36))
#define FILTER_1_REG_10   *((unsigned *)(ptr4 + 40))
#define FILTER_1_REG_11   *((unsigned *)(ptr4 + 44))
#define FILTER_1_REG_12   *((unsigned *)(ptr4 + 48))
#define FILTER_1_REG_13   *((unsigned *)(ptr4 + 52))
#define FILTER_1_REG_14   *((unsigned *)(ptr4 + 56))
#define FILTER_1_REG_15   *((unsigned *)(ptr4 + 60))
#define FILTER_1_REG_16   *((unsigned *)(ptr4 + 64))
#define FILTER_1_REG_17   *((unsigned *)(ptr4 + 68))
#define FILTER_1_REG_18   *((unsigned *)(ptr4 + 72))
#define FILTER_1_REG_19   *((unsigned *)(ptr4 + 76))

#define AXI_TO_AUDIO_REG_0   *((unsigned *)(ptr5 + 0)) //we use this
#define AXI_TO_AUDIO_REG_1   *((unsigned *)(ptr5 + 4))
#define AXI_TO_AUDIO_REG_2   *((unsigned *)(ptr5 + 8))
#define AXI_TO_AUDIO_REG_3   *((unsigned *)(ptr5 + 12))

#define PMOD_REG_0   *((unsigned *)(ptr6 + 0))
#define PMOD_REG_1   *((unsigned *)(ptr6 + 4))
#define PMOD_REG_2   *((unsigned *)(ptr6 + 8))
#define PMOD_REG_3   *((unsigned *)(ptr6 + 12)) //we only use this, first 4 bits

#define MENULENGTH 8

int udp_client_setup(char *broadcast_address, int broadcast_port);
int udp_client_recv(unsigned *buffer,int buffer_size );
void *send_audio_function(void *arg);
void *pmod_function(void *arg);
void *recv_function(void *arg);

struct sockaddr_in receiving_address;
int client_socket; 
socklen_t addr_size;
int fd5;
int fd6;
void *ptr5; //AXI_TO_AUDIO
void *ptr6; //PMOD_CONTROLLER

int rotary = 0;
int menuUp = 0;
int menuDown = 0;

int main(int argc, char *argv[])
{
	pthread_t thread;
	pthread_t pmod_thread;
	pthread_t recv_thread;
	int i;
	
	char menuBuf[16];
	int cursorPos = 0;
	int menuPos = 0;
	int setting = 0;
	char menuitem[MENULENGTH][12] = {
			"menuitem 1  ",
			"menuitem 2  ",
			"menuitem 3  ",
			"menuitem 4  ",
			"menuitem 5  ",
			"menuitem 6  ",
			"menuitem 7  ",
			"menuitem 8  "};

    if (*argv[1] == 'p') {
        printf("::::START_USAGE::::\n");
        printf("EXAMPLE : %s Right_vol(512) Left_vol(512) High_pass(0 on, 1 off) Band_pass Low_pass \n", argv[0]);
        printf("::::END_USAGE::::\n");
    }
    else {
  
        //take inputs from user
        unsigned Right = atoi(argv[1]);
        unsigned Left = atoi(argv[2]);
        unsigned High = atoi(argv[3]);
        unsigned Band = atoi(argv[4]);
        unsigned Low = atoi(argv[5]);

        //VOLUME 0
        int fd = open ("/dev/uio2", O_RDWR);
        if (fd < 1) { perror(argv[0]); return -1; }
        printf("fd init done\n");
        //FILTER 0
        int fd2 = open ("/dev/uio1", O_RDWR);
        if (fd2 < 1) { perror(argv[0]); return -1; }
        printf("fd2 init done\n");
        //VOLUME 1
        int fd3 = open ("/dev/uio4", O_RDWR);
        if (fd3 < 1) { perror(argv[0]); return -1; }
        printf("fd3 init done\n");
        //FILTER 1
        int fd4 = open ("/dev/uio3", O_RDWR);
        if (fd4 < 1) { perror(argv[0]); return -1; }
        printf("fd4 init done\n");
        //open dev/uio0 AXI_TO_AUDIO
        fd5 = open ("/dev/uio0", O_RDWR);
        if (fd5 < 1) { perror(argv[0]); return -1; }
        printf("fd5 init done\n");
        //open dev/uio6 PMOD_CONTROLLER_0
        fd6 = open ("/dev/uio6", O_RDWR);
        if (fd6 < 1) { perror(argv[0]); return -1; }
        printf("fd6 init done\n");
        //open dev/uio5 ZEDBOARDOLED_0
        int fd7 = open ("/dev/uio5", O_RDWR);
        if (fd7 < 1) { perror(argv[0]); return -1; }
        printf("fd7 init done\n");
        
		mkfifo("/tmp/myfifo", 0644);
  
        unsigned pageSize = sysconf(_SC_PAGESIZE);
        void *ptr; //VOL 0
        ptr = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, pageSize*0);
        printf("ptr init done\n");
        void *ptr2; //FILTER 0
        ptr2 = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, pageSize*0);
		printf("ptr2 init done\n");
		void *ptr3; //VOL 1
        ptr3 = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd3, pageSize*0);
        printf("ptr3 init done\n");
        void *ptr4; //FILTER 1
        ptr4 = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd4, pageSize*0);
        printf("ptr4 init done\n");
        //void *ptr5; //AXI_TO_AUDIO
        ptr5 = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd5, pageSize*0);
        printf("ptr5 init done\n");
        //void *ptr6; //PMOD_CONTROLLER
        ptr6 = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd6, pageSize*0);
        printf("ptr6 init done\n");
        void *ptr7; //ZEDBOARDOLED_0
        ptr7 = mmap(NULL, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd7, pageSize*0);
        printf("ptr7 init done\n");
		
        //write into registers
        VOLUME_1_REG_0  = VOLUME_0_REG_0  = Right;
        VOLUME_1_REG_1  = VOLUME_0_REG_1  = Left;
        FILTER_1_REG_0  = FILTER_0_REG_0  = 0x00002CB6;
        FILTER_1_REG_1  = FILTER_0_REG_1  = 0x0000596C;
        FILTER_1_REG_2  = FILTER_0_REG_2  = 0x00002CB6;
        FILTER_1_REG_3  = FILTER_0_REG_3  = 0x8097A63A;
        FILTER_1_REG_4  = FILTER_0_REG_4  = 0x3F690C9D;
        FILTER_1_REG_5  = FILTER_0_REG_5  = 0x074D9236;
        FILTER_1_REG_6  = FILTER_0_REG_6  = 0x00000000;
        FILTER_1_REG_7  = FILTER_0_REG_7  = 0xF8B26DCA;
        FILTER_1_REG_8  = FILTER_0_REG_8  = 0x9464B81B;
        FILTER_1_REG_9  = FILTER_0_REG_9  = 0x3164DB93;
        FILTER_1_REG_10 = FILTER_0_REG_10 = 0x12BEC333;
        FILTER_1_REG_11 = FILTER_0_REG_11 = 0xDA82799A;
        FILTER_1_REG_12 = FILTER_0_REG_12 = 0x12BEC333;
        FILTER_1_REG_13 = FILTER_0_REG_13 = 0x00000000;
        FILTER_1_REG_14 = FILTER_0_REG_14 = 0x0AFB0CCC;
        FILTER_1_REG_15 = FILTER_0_REG_15 = 0;
        FILTER_1_REG_16 = FILTER_0_REG_16 = 1;
        FILTER_1_REG_17 = FILTER_0_REG_17 = High;
        FILTER_1_REG_18 = FILTER_0_REG_18 = Band;
        FILTER_1_REG_19 = FILTER_0_REG_19 = Low;
        
		int iret1 = pthread_create(&thread, NULL, send_audio_function, NULL);
		if(iret1)
		{
			printf("Error - pthread_create() return code: %d\n",iret1);
		}
		
		int iret2 = pthread_create(&pmod_thread, NULL, pmod_function, NULL);
		if(iret2)
		{
			printf("Error - pthread_create() return code: %d\n",iret2);
		}
		
		int iret3 = pthread_create(&recv_thread, NULL, recv_function, NULL);
		if(iret3)
		{
			printf("Error - pthread_create() return code: %d\n",iret3);
		}
		
		
		while(1)
        {
			if (menuDown)
			{
				menuDown = 0;
				if(cursorPos<3)
					cursorPos++;
				else if (menuPos < MENULENGTH-4)
					menuPos++;
			}
			else if (menuUp)
			{
				menuUp = 0;
				if(cursorPos>1)
					cursorPos--;
				else if (menuPos>0)
					menuPos--;
			}
			for(i=0;i<4;i++);
			{
				if(i==cursorPos)
					menuBuf[0] = 62;
				else
					menuBuf[0] = 32;
				sprintf(&menuBuf[1], "%-12s%3d",menuitem[menuPos+i], setting);
				oled_print_message(&menuBuf[0], 0, ptr7);
			}
		}
		
		//join
		pthread_join(thread, NULL);
		pthread_join(pmod_thread, NULL);
		pthread_join(recv_thread, NULL);
        //unmap
        munmap(ptr, pageSize);
        munmap(ptr2, pageSize);
        munmap(ptr3, pageSize);
        munmap(ptr4, pageSize);
        munmap(ptr5, pageSize);
        munmap(ptr6, pageSize);
        munmap(ptr7, pageSize);
        //close
        fclose(stdout);
    }
    return 0;
}

int udp_client_setup(char *broadcast_address, int broadcast_port){

    /* Create UDP socket */
    client_socket = socket(PF_INET, SOCK_DGRAM, 0);

    int broadcast_enabled = 1;

    /* Enable broadcast */
    if (setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, (void *) &broadcast_enabled,sizeof(broadcast_enabled)) < 0){
        perror ("Error while enabling broadcast");
        return 1;
    }

    /* Configure settings in address struct */
    receiving_address.sin_family = AF_INET;
    receiving_address.sin_port = htons(broadcast_port);
    receiving_address.sin_addr.s_addr = inet_addr(broadcast_address);
    memset(receiving_address.sin_zero, '\0', sizeof receiving_address.sin_zero);  
    
    /* Connect to the server (broadcast) */
    bind(client_socket, (struct sockaddr *) &receiving_address, sizeof(receiving_address));
    
    /* Initialize size variable to be used later on */
    addr_size = sizeof receiving_address;

    return 0;
}

int udp_client_recv(unsigned *buffer,int buffer_size ){
    if((recvfrom(client_socket,buffer,buffer_size,0,(struct sockaddr *)&receiving_address,&addr_size)) != -1 )
        return 0;
    else
        return 1; 
}

void *send_audio_function(void *arg)
{
	short int buf[512];
	int fd;
	int IRQEnable = 1; 
	int i;
	//write(fd5, &IRQEnable, sizeof(IRQEnable));
	write(fd5, &IRQEnable, sizeof(IRQEnable));
	printf("audio sample Interrupt Enabled\n");
	if ((fd = open("/tmp/myfifo", O_RDONLY)) < 1)
		printf("fifo read open error");
	else
		printf("FIFO READ open\n");

	while (1)
	{
		read(fd, buf, 1024);
		for (i=0;i<512;i++)
		{
			read(fd5, &IRQEnable, sizeof(IRQEnable));
			IRQEnable = 1;
			write(fd5, &IRQEnable, sizeof(IRQEnable));
			AXI_TO_AUDIO_REG_0 = (int)buf[i];
		}
	}
}

void *pmod_function(void *arg)
{
	int position = 0;
	int IRQEnable = 1;
	int pmod_data;
	write(fd6, &IRQEnable, sizeof(IRQEnable));
	printf(" Pmod Interrupt Enabled\n");
	
	while(1)
	{
		read(fd6, &IRQEnable, sizeof(IRQEnable));
		pmod_data = PMOD_REG_3;
		printf("Rotary event detected: %d", &pmod_data);
		IRQEnable = 1;
		write(fd6, &IRQEnable, sizeof(IRQEnable));
	}
}

void *recv_function(void *arg)
{
	short int buffer[512];
	int fd_fifo;
	
	if ((fd_fifo = open("/tmp/myfifo", O_WRONLY)) < 0)
		printf("fifo write open error\n");
	else
		printf("fifo write open\n");
	
	if(udp_client_setup("10.255.255.255", 7891))
		printf("Connection error\n");
	else
		printf("Stream connected\n");
	
	while(1) //get stream and send to axi_to_audio
	{
		udp_client_recv((unsigned int*)&buffer, 1024);
		write(fd_fifo, buffer, 1024);
	}
}
