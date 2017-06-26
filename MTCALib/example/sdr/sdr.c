#include <mtca.h>
#include <stdlib.h>
#include <string.h>

#include "sdr.h"

struct sdr_record_list *sdr_list_head = NULL;
struct sdr_record_list *sdr_list_tail = NULL;
struct sdr_info_s sdr_info_g;
int sdr_info_init = 0;

int get_sdr_info(struct ipmi_intf *intf){
  struct ipmi_rs *rsp;
  
  if(sdr_info_init == 0){
    sdr_info_init = 1;
    
    rsp = send_ipmi_cmd(intf, 0x0a, 0x22, NULL, 0);	  //Reserve SDR repository
  	if(rsp == NULL){
      intf->close(intf);
  		return -1;
  	}else{
  		if(rsp->ccode){
        //printf("1 ccode : %x \n", rsp->ccode);
        intf->close(intf);
  			return -1;
  	  }
    }
    
    sdr_info_g.resID[0] = rsp->data[0];
    sdr_info_g.resID[1] = rsp->data[1];
    sdr_info_g.nextID[0] = 0;
    sdr_info_g.nextID[1] = 0;
    sdr_info_g.ID[0] = 0;
    sdr_info_g.ID[1] = 0;
    sdr_info_g.recordtype = 0;
  }
  
  return 0;
}

int ipmi_sdr_get_next_header(struct ipmi_intf *intf){
	struct sdr_get_rs *header;
  struct ipmi_rs *rsp;
  unsigned char data[10];
  
  //printf("Next header start \n");
  
	if (sdr_info_g.nextID[0] == 0xff && sdr_info_g.nextID[1] == 0xff){
      //printf("Last record \n");
      return 1;
  }
  
  //printf("prepare data \n");
  
  data[0] = sdr_info_g.resID[0];
  data[1] = sdr_info_g.resID[1];
  data[2] = sdr_info_g.nextID[0];
  data[3] = sdr_info_g.nextID[1];
  data[4] = 0;
  data[5] = 5;
  
  //printf("Data ready \n");
  //if(intf==NULL)  printf("intf null \n");
  
  rsp = send_ipmi_cmd(intf, 0x0a, 0x23, data, 6);	  //GET SDR
  if(rsp == NULL){
    printf("rsp = NULL \n");
    intf->close(intf);
    return -1;
 	}else{
  	if(rsp->ccode){
      //printf("2 ccode : 0x%x \n",rsp->ccode);
      intf->close(intf);
  		return -2;
 	  }
  }
  
  //printf("Header received \n");
  sdr_info_g.ID[0] = sdr_info_g.nextID[0];
  sdr_info_g.ID[1] = sdr_info_g.nextID[1];
  sdr_info_g.recordtype = rsp->data[5];
   
  sdr_info_g.nextID[0] = rsp->data[0];
  sdr_info_g.nextID[1] = rsp->data[1];
  
  sdr_info_g.length = rsp->data[6];
    
  //printf("OK \n");
	return 0;
}

struct sdr_record_list * get_sdr_bynumtype(struct ipmi_intf *intf, unsigned char type, unsigned char num){

	struct sdr_record_list *sdrr;
  unsigned char found = 0;
  
  /*Check new sensors */   
  if(get_sdr_info(intf))  return NULL;    //Get resID
  
  sdr_info_g.nextID[0] = 0;  //Init first sens ID = 0
  sdr_info_g.nextID[1] = 0;
  
  while (!ipmi_sdr_get_next_header(intf)){ 
  
    if(sdr_info_g.recordtype != 0x01 && sdr_info_g.recordtype != 0x02)  continue;
    
		sdrr = malloc(sizeof (struct sdr_record_list));
		if (sdrr == NULL) {
			break;
		}
		memset(sdrr, 0, sizeof (struct sdr_record_list));
   
		sdrr->id = sdr_info_g.ID[0] | ((unsigned short)(sdr_info_g.ID[1]) << 8 & 0xFF00) ;
		sdrr->type = sdr_info_g.recordtype;

		if(ipmi_sdr_get_record(intf, sdrr)){
      if(sdrr != NULL){
        free(sdrr);
        sdrr = NULL;
      }
      continue;
    }
   
		switch (sdr_info_g.recordtype){
			case SDR_RECORD_TYPE_FULL_SENSOR:
			case SDR_RECORD_TYPE_COMPACT_SENSOR:
				if (sdrr->sensor_num == num && sdrr->sensor_type == type)
					found = 1;
				break;
        
			case SDR_RECORD_TYPE_EVENTONLY_SENSOR:
				if (sdrr->sensor_num == num && sdrr->sensor_type == type)
					found = 1;
				break;
        
			default:
				if (sdrr != NULL) {
					free(sdrr);
					sdrr = NULL;
				}
				continue;
		}
   
		if (found){
			return sdrr;
    }
	}
 
  return NULL;
}

int ipmi_sdr_get_record(struct ipmi_intf *intf, struct sdr_record_list *sdr){

  struct ipmi_rs *rsp;
  unsigned char data[10];
  unsigned char offset, startstr;
  unsigned char maxlen, strlength;
  unsigned char i, itt = 0;
  
  data[0] = sdr_info_g.resID[0];
  data[1] = sdr_info_g.resID[1];
  data[2] = sdr_info_g.ID[0];
  data[3] = sdr_info_g.ID[1];
  data[4] = 0;
  data[5] = 0;
  
  //Get sensor string
  switch(sdr_info_g.recordtype){
    case 0x01:
      startstr = 48;
      if(sdr_info_g.length > 43)
        strlength = sdr_info_g.length - 43;
      else
        strlength = 0;
      break;
    case 0x02:
      startstr = 32;
      if(sdr_info_g.length > 27)
        strlength = sdr_info_g.length - 27;
      else
        strlength = 0;
      break;
    default: 
      //printf("Record: neither full nor compact \n");
      return -1;
  }
  
  maxlen = strlength;
  offset = 0;
  
  do{ 
    data[4] = startstr+offset;
    data[5] = maxlen;
        
    rsp = send_ipmi_cmd(intf, 0x0a, 0x23, data, 6);	  //GET SDR
  	if(rsp == NULL){
      intf->close(intf);
  		return -1;
  	}else{
      if(rsp->ccode == 0xca){
        maxlen--;
      }else if(rsp->ccode){
        //printf("3 ccode : %x \n", rsp->ccode);
        intf->close(intf);
  			return -2;
  	  }else{
        for(i=0; i < rsp->data_len - 2; i++, offset++){
          if(offset < 64)
            sdr->sensName[offset] = rsp->data[2+i];
        }
      }
    }
  }while(offset < (strlength-1));
  
  if(maxlen < 64)
    sdr->sensName[maxlen] = 0;
  else
    sdr->sensName[63] = 0;
    
  //Get sensor type
  switch(sdr_info_g.recordtype){
    case 0x01:
    case 0x02:
      data[4] = 12;
      data[5] = 1;
      break;
    default: return -1;
  }
      
  rsp = send_ipmi_cmd(intf, 0x0a, 0x23, data, 6);	  //GET SDR
	if(rsp == NULL){
    intf->close(intf);
		return -1;
	}else{
		if(rsp->ccode){
      //printf("4 ccode : %x \n", rsp->ccode);
      intf->close(intf);
			return -2;
	  }
  }
  
  sdr->sensor_type = rsp->data[2];
  
  //Get sensor number
  switch(sdr_info_g.recordtype){
    case 0x01:
    case 0x02:
      data[4] = 7;
      data[5] = 1;
      break;
    default: return -1;
  }
      
  rsp = send_ipmi_cmd(intf, 0x0a, 0x23, data, 6);	  //GET SDR
	if(rsp == NULL){
    intf->close(intf);
		return -1;
	}else{
		if(rsp->ccode){
      //printf("5 ccode : %x \n", rsp->ccode);
      intf->close(intf);
			return -2;
	  }
  }
  sdr->sensor_num = rsp->data[2];
  
  //Get entity ID and instance
  switch(sdr_info_g.recordtype){
    case 0x01:
    case 0x02:
      data[4] = 8;
      data[5] = 2;
      break;
    default: return -1;
  }
      
  rsp = send_ipmi_cmd(intf, 0x0a, 0x23, data, 6);	  //GET SDR
	if(rsp == NULL){
    intf->close(intf);
		return -1;
	}else{
		if(rsp->ccode){
      //printf("5 ccode : %x \n", rsp->ccode);
      intf->close(intf);
			return -2;
	  }
  }
  
  sdr->entityID = rsp->data[2];
  sdr->entityInst = rsp->data[3];
  
  return 0;
}