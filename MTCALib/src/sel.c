#include <mtca.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdio.h>

unsigned char ghostname[25];
unsigned char gusername[25];
unsigned char gpassword[25];

unsigned char reservationID[2];

int sel_init(unsigned char *hostname, unsigned char *username, unsigned char *password){
  int timestamp;
  unsigned char time[25];
  struct ipmi_rs *rsp;
  
  struct ipmi_intf *intf = 
    open_lan_session(hostname, 
									   username, 
									   password, 
									   0,			          //No target specified (Default: MCH)
									   0,								//No transit addr specified (Default: 0) 
									   0,								//No target channel specified (Default: 0)
									   0);
    do{                            
      rsp = send_ipmi_cmd(intf, 0x0a, 0x40, NULL, 0);	  //GET SEL INFO
    	if(rsp == NULL){
        intf->close(intf);
    		return -1;
    	}else{
    		if(rsp->ccode && rsp->ccode != 0x81){
    			printf("[INFO] \t {GET_SEL_INFO} \t\t Completion Code : 0x%02x \n", rsp->ccode);
          intf->close(intf);
    			return -2;
    	  }
        else if(rsp->ccode == 0x81){
          printf("Erease in progress \n");
        }
        usleep(1000);  //1ms sleep
      }
    }while(rsp->ccode == 0x81);  //SEL erase in progress //JM: Watchdog should be added
    
    printf("SEL Version : 0%02x \n",rsp->data[0]);
    printf("Number of log entries : %d \n",(rsp->data[1] | ((unsigned short)rsp->data[2] << 8)));
    printf("Free space (byte) : %d \n",(rsp->data[3] | ((unsigned short)rsp->data[4] << 8)));
    
    timestamp = (rsp->data[5] | (((unsigned int)rsp->data[6] << 8) & 0xFF00) | (((unsigned int)rsp->data[7] << 16) & 0xFF0000) | (((unsigned int)rsp->data[8] << 24) & 0xFF000000));
    strftime(time, sizeof(time), "%d/%m/%y %H:%M", localtime((const time_t *)&timestamp));
    
    printf("Most recent addition timestamp : %s \n",time);
    
    timestamp = (rsp->data[9] | (((unsigned int)rsp->data[10] << 8) & 0xFF00) | (((unsigned int)rsp->data[11] << 16) & 0xFF0000) | (((unsigned int)rsp->data[12] << 24) & 0xFF000000));
    strftime(time, sizeof(time), "%d/%m/%y %H:%M", localtime((const time_t *)&timestamp));
    
    printf("Most recent erease timestamp : %s \n",time);
    
    
    printf("Overflow flag : %s \n",(rsp->data[13] & 0x80)?"TRUE":"FALSE");
    printf("Delete supported : %s \n",(rsp->data[13] & 0x08)?"TRUE":"FALSE");
    printf("Partial add supported : %s \n",(rsp->data[13] & 0x04)?"TRUE":"FALSE");
    printf("Reserve SEL supported: %s \n",(rsp->data[13] & 0x02)?"TRUE":"FALSE");
    printf("Get SEL allocation information supported : %s \n",(rsp->data[13] & 0x01)?"TRUE":"FALSE");
    
    do{                            
      rsp = send_ipmi_cmd(intf, 0x0a, 0x42, NULL, 0);	  //GET SEL INFO
    	if(rsp == NULL){
        intf->close(intf);
    		return -1;
    	}else{
    		if(rsp->ccode && rsp->ccode != 0x81){
    			printf("[INFO] \t {RESERVE_SEL} \t\t Completion Code : 0x%02x \n", rsp->ccode);
          intf->close(intf);
    			return -2;
    	  }
        else if(rsp->ccode == 0x81){
          printf("Erease in progress \n");
        }
        usleep(1000);  //1ms sleep
      }
    }while(rsp->ccode == 0x81);  //SEL erase in progress //JM: Watchdog should be added
    
    printf("Reservation ID : 0x%x \n",(rsp->data[0] | ((unsigned short)rsp->data[1] << 8)));
    reservationID[0] = rsp->data[0]; 
    reservationID[1] = rsp->data[1];
  
    intf->close(intf);
    
    strncpy(ghostname, hostname, 25);
    strncpy(gusername, username, 25);
    strncpy(gpassword, password, 25);
    
    return 0;
}

int get_event(unsigned char *buf, unsigned char maxlen, unsigned short *entry_nb){
  struct ipmi_rs *rsp;
  unsigned char data[6];
  unsigned int i;
   
  struct ipmi_intf *intf = 
    open_lan_session(ghostname, 
									   gusername, 
									   gpassword, 
									   0,			          //No target specified (Default: MCH)
									   0,								//No transit addr specified (Default: 0) 
									   0,								//No target channel specified (Default: 0)
									   0);
      
  data[0] = reservationID[0];
  data[1] = reservationID[1];
  data[2] = (*entry_nb & 0xFF);  //Get first entry
  data[3] = (*entry_nb & 0xFF00) >> 8;
  data[4] = 0x00;  //No offset into record
  data[5] = 0xFF;  //Read entire record
          
  rsp = send_ipmi_cmd(intf, 0x0a, 0x43, data, 6);	  //GET SEL INFO
	if(rsp == NULL){
    intf->close(intf);
    //printf("Connection error \n");
		return -1;
	}else{
		if(rsp->ccode && rsp->ccode != 0x81){
			//printf("[INFO] \t {GET_SEL_ENTRY} \t\t Completion Code : 0x%02x \n", rsp->ccode);
      intf->close(intf);
			return -2;
	  }
		if(rsp->ccode == 0x81){
			//printf("[INFO] \t {GET_SEL_ENTRY} \t\t Completion Code : 0x%02x (Erase in progress)\n", rsp->ccode);
      intf->close(intf);
			return -3;
	  }
  }
  
  for(i=2; i < (maxlen+2) && i < rsp->data_len; i++)
    buf[i-2] = rsp->data[i];
  
  if(rsp->data[0] != 0xFF || rsp->data[1] != 0xFF){
    *entry_nb = rsp->data[0] | (((unsigned short)rsp->data[1]) << 8);
  }
  
    
  intf->close(intf);
  
  return i;
}