#include <mtca.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "../sdr/sdr.h"

void scan_event(unsigned char sensor_type, unsigned char sensor_number, unsigned char evType, unsigned char *evData);
int getSensorName(unsigned char sensor_number, unsigned char sensor_type, unsigned char *name, unsigned char maxlen);

int main(){
  int len, i;
  unsigned char buf[25];
  
  int timestamp;
  unsigned char time[40];
  
  sel_init("137.138.63.19","admin","admin");
  
  while(1){
    len = get_event(buf, 25);
    
    if(len > 0){
      timestamp = (buf[3] | (((unsigned int)buf[4] << 8) & 0xFF00) | (((unsigned int)buf[5] << 16) & 0xFF0000) | (((unsigned int)buf[6] << 24) & 0xFF000000));
      strftime(time, sizeof(time), "%d/%m/%y %H:%M:%S", localtime((const time_t *)&timestamp));
    
      printf("(%s) 0x%02x 0x%02x : %s of ", time, buf[10], buf[11], (buf[12] & 0x80)? "De-assertion":"Assertion");
      
      scan_event(buf[10],buf[11],buf[12]&0x7F,&buf[13]);
    }
    
    usleep(100000); //Wait for 100ms 
  }
  return 0;
}

void scan_event(unsigned char sensor_type, unsigned char sensor_number, unsigned char evType, unsigned char *evData){
  int len;
  struct sdr_record_list * sdr;
  struct ipmi_intf *intf = open_lan_session("137.138.63.19", 
									"admin", 
									"admin", 
									0,			//No target specified (Default: MCH)
									0,								//No transit addr specified (Default: 0) 
									0,									//No target channel specified (Default: 0)
									0);
  
                                  
  switch(evType){
    case 0x01:  
      switch(evData[0]){
        case 0x00:  printf("Lower non critical - going low"); break;
        case 0x01:  printf("Lower non critical - going high"); break;
        case 0x02:  printf("Lower critical - going low"); break;
        case 0x03:  printf("Lower critical - going high"); break;
        case 0x04:  printf("Lower non recoverable - going low"); break;
        case 0x05:  printf("Lower non recorverable - going high"); break;
        case 0x06:  printf("Upper non critical - going low"); break;
        case 0x07:  printf("Upper non critical - going high"); break;
        case 0x08:  printf("Upper critical - going low"); break;
        case 0x09:  printf("Upper critical - going high"); break;
        case 0x0A:  printf("Upper non recoverable - going low"); break;
        case 0x0B:  printf("Upper non recoverable - going high"); break;
      }
      break;
      
    case 0x02:  
      switch(evData[0]){
        case 0x00:  printf("Transition to Idle"); break;
        case 0x01:  printf("Transition to Active"); break;
        case 0x02:  printf("Transition to busy"); break;
      }
      break;
      
    case 0x03:  
      switch(evData[0]){
        case 0x00:  printf("State deasserted"); break;
        case 0x01:  printf("State asserted"); break;
      }
      break;
      
    case 0x04:  
      switch(evData[0]){
        case 0x00:  printf("Predictive failure deasserted"); break;
        case 0x01:  printf("Predictive failure asserted"); break;
      }
      break;
      
    case 0x05:  
      switch(evData[0]){
        case 0x00:  printf("Limit not exceeded"); break;
        case 0x01:  printf("Limit exceeded"); break;
      }
      break;
      
    case 0x06:  
      switch(evData[0]){
        case 0x00:  printf("Performance met"); break;
        case 0x01:  printf("Performance lags"); break;
      }
      break;
      
    case 0x07:  
      switch(evData[0]){
        case 0x00:  printf("Transition to OK"); break;
        case 0x01:  printf("Transition to Non critical from OK"); break;
        case 0x02:  printf("Transition to Critical from less severe"); break;
        case 0x03:  printf("Transition to Non-recoverable from less severe"); break;
        case 0x04:  printf("Transition to Non-Critical from more severe"); break;
        case 0x05:  printf("Transition to Critical from Non-recoverable"); break;
        case 0x06:  printf("Transition to Non-recoverable"); break;
        case 0x07:  printf("Monitor"); break;
        case 0x08:  printf("Informational"); break;
      }
      break;
      
    case 0x08:  
      switch(evData[0]){
        case 0x00:  printf("Device removed / absent"); break;
        case 0x01:  printf("Device inserted / present"); break;
      }
      break;
      
    case 0x09:  
      switch(evData[0]){
        case 0x00:  printf("Device disabled"); break;
        case 0x01:  printf("Device enabled"); break;
      }
      break;
      
    case 0x0A:  
      switch(evData[0]){
        case 0x00:  printf("Transition to running"); break;
        case 0x01:  printf("Transition to in test"); break;
        case 0x02:  printf("Transition to power off"); break;
        case 0x03:  printf("Transition to on line"); break;
        case 0x04:  printf("Transition to off line"); break;
        case 0x05:  printf("Transition to off duty"); break;
        case 0x06:  printf("Transition to degraded"); break;
        case 0x07:  printf("Transition to power save"); break;
        case 0x08:  printf("Install error"); break;
      }
      break;
      
    case 0x0B:  
      switch(evData[0]){
        case 0x00:  printf("Full redundancy has been regaigned"); break;
        case 0x01:  printf("Redundancy lost"); break;
        case 0x02:  printf("Redundancy degraded"); break;
        case 0x03:  printf("Non redundant: Sufficient ressources from redundant"); break;
        case 0x04:  printf("Non redundant: Sufficient ressources from insufficient ressources"); break;
        case 0x05:  printf("Non redundant: Insufficient ressources"); break;
        case 0x06:  printf("Redundancy degraded from fully redundant"); break;
        case 0x07:  printf("Redundancy degraded from non-redundant"); break;
      }
      break;
      
    case 0x0C:  
      switch(evData[0]){
        case 0x00:  printf("D0 power state"); break;
        case 0x01:  printf("D1 power state"); break;
        case 0x02:  printf("D2 power state"); break;
        case 0x03:  printf("D3 power state"); break;
      }
      break;
      
    case 0x6F: 
      switch(sensor_type){
        case 0x00: printf("Reserved"); break;
        case 0x01: printf("Temperature"); break;
        case 0x02: printf("Voltage"); break;
        case 0x03: printf("Current"); break;
        case 0x04: printf("Fan"); break;
        
        case 0x05:
          switch(evData[1]){
            case 0x00: printf("General chassis intrusion"); break;
            case 0x01: printf("Drive bay intrusion"); break;
            case 0x02: printf("I/O card area intrusion"); break;
            case 0x03: printf("Processor area intrusion"); break;
            case 0x04: printf("LAN leash lost (system is unplugged from LAN)"); break;
            case 0x05: printf("Unahthorized dock/undock"); break;
            case 0x06: printf("FAN area intrusion (supports detection of hot plug fan tampering"); break;
          }
          break;
          
        case 0x06:
          switch(evData[1]){
            case 0x00: printf("Secure Mode (Front Pannel Lockout) violation attempt"); break;
            case 0x01: printf("Pre-boot password violation - user password"); break;
            case 0x02: printf("Pre-boot password violation attempt - setup password"); break;
            case 0x03: printf("Pre-boot password violation - network boot password"); break;
            case 0x04: printf("Other pre-boot password violation"); break;
            case 0x05: printf("Out of band password violation"); break;
          }
          break;
          
        case 0x07:
          switch(evData[1]){
            case 0x00: printf("IERR"); break;
            case 0x01: printf("Thermal trip"); break;
            case 0x02: printf("FRB1/BIST failure"); break;
            case 0x03: printf("FRB2/Hang POST failure"); break;
            case 0x04: printf("FRB3/Processor startup/init failure"); break;
            case 0x05: printf("Configuration error"); break;
            case 0x06: printf("SM Bios Uncorrectable CPU-complex error"); break;          
            case 0x07: printf("Processor presence detected"); break;          
            case 0x08: printf("Processor disabled"); break;          
            case 0x09: printf("Terminator presence detected"); break;      
          }
          break;
          
        case 0x08:
          switch(evData[1]){
            case 0x00: printf("Presence detected"); break;
            case 0x01: printf("Power supply failure detected"); break;
            case 0x02: printf("Predictive failure"); break;
            case 0x03: printf("Power supply AC lost"); break;
            case 0x04: printf("AC lost or out-of-range"); break;
            case 0x05: printf("AC out-of-range, but present"); break;
          }
          break;
          
        case 0x09:
          switch(evData[1]){
            case 0x00: printf("Power off / power down"); break;
            case 0x01: printf("Power cycle"); break;
            case 0x02: printf("240VA power down"); break;
            case 0x03: printf("Interlock power down"); break;
            case 0x04: printf("AC lost"); break;
            case 0x05: printf("Soft power control failure"); break;
            case 0x06: printf("Power unit failure detected"); break;
            case 0x07: printf("Predictive failure"); break;                    
          }
          break;
        
	case 0x0B:
	  switch(evData[0]){
	    case 0x00: printf("Fully redundant regained");
	    case 0x01: printf("Redundancy lost");
	  }
        case 0xF0:
          switch(evData[0] & 0x0F){
            case 0:  printf("M0 - FRU not installed state"); break;
            case 1:  printf("M1 - FRU inactive"); break;
            case 2:  printf("M2 - FRU Activation request"); break;
            case 3:  printf("M3 - FRU Activation in progress"); break;
            case 4:  printf("M4 - FRU Active"); break;
            case 5:  printf("M5 - FRU Deactivation request"); break;
            case 6:  printf("M6 - FRU Deactivation in progress"); break;
            case 7:  printf("M7 - FRU Communication lost"); break;
            case 8:  printf("Reserved"); break;
          }
          
          switch(evData[1] & 0xF0){
            case 0:  printf(" (Normal state change)"); break;
            case 1:  printf(" (Change commanded by shelf manager with Set FRU activation)"); break;
            case 2:  printf(" (Due to operator changing handle switch)"); break;
            case 3:  printf(" (Due to FRU programmatic action)"); break;
            case 4:  printf(" (Communication lost or regaigned)"); break;
            case 5:  printf(" (Communication lost or regained - locally detected)"); break;
            case 6:  printf(" (Surprise state change du to extraction)"); break;
            case 7:  printf(" (State change due to provided information)"); break;
            case 8:  printf(" (Invalid hardware address detected)"); break;
            case 9:  printf(" (Unexpected deactivation)"); break;
            case 0x0a: printf(" (Surprise state change due to power failure)"); break;
            case 0x0f: printf(" (Cause unknown)"); break;
            default: printf(" (reserved)");
          }
          
          printf("for FRU %d",evData[2]);
          break;

        case 0xF3:
	        printf("Power channel notification (");
          switch(evData[0] & 0x01){
            case 0:  
          		if(evData[1] & 0x08)	printf("Redundant PM is providing Payload current / ");
          		if(!(evData[1] & 0x04))	printf("Payload power is not good / ");
          		if(!(evData[1] & 0x02)) printf("Management power is not good / ");
          		if(evData[1] & 0x01)	printf("Primary)");
          		else			printf("Redundant)");
          		break;
            case 1:   
          		if(evData[1] & 0x40)	printf("Power ON is asserted / ");
          		if(evData[1] & 0x20)	printf("PP overcurrent detected / ");
          		if(evData[1] & 0x10) 	printf("PP Enabled / ");
          		if(evData[1] & 0x08)	printf("ENABLE# is asserted / ");
          		if(evData[1] & 0x04)	printf("MP overcurrent detected / ");
          		if(evData[1] & 0x02) 	printf("MP Enabled / ");
          		if(evData[1] & 0x01)	printf("PS1# is asserted / ");
          		printf("\b\b\b)");
          		break;
          }
                   
          printf("for channel %d",evData[2]);
          break;  

        default: printf("evType 0x6F / code {0x%02x, 0x%02x, 0x%02x}",evData[1], evData[2], evData[3]);
      }
      break;
    
    default:printf("evType {0x%02x}",evType);
  }
  
  sdr = get_sdr_bynumtype(intf, sensor_type, sensor_number);
  if(sdr != NULL){
    printf(" for sensor <%s> (",sdr->sensName);
    switch(sdr->entityID){
      case 0xC1: printf("AMC "); break;
      case 0xC2: printf("MCH "); break;
      case 0x0A: printf("PM "); break;
      case 0x1E: printf("CU "); break;
      default: printf("Unknown "); break;
    } 
    printf("%d) \n",sdr->entityInst - 0x60);
  }else
    printf("\n");
    
  intf->close(intf);
  //printf(" event ");
  
  //len = getSensorName(sensor_number, sensor_type, sensorName, 24);
  
  //if(len > 0){
  //  if(len  >= 23)  sensorName[24] = 0;
  //  else sensorName[len] = 0;
    
  //  printf("(%s sensor)\n", sensorName); 
  //}
  /*
  printf("\tSensor type: 0x%x \n", sensor_type);
  printf("\tSensor numer: 0x%x \n", sensor_number);
  printf("\tEv Type: 0x%x \n", evType);
  printf("\tData 1: 0x%x \n", evData[1]);
  printf("\tData 2: 0x%x \n", evData[2]);
  printf("\tData 3: 0x%x \n", evData[3]);*/
}
