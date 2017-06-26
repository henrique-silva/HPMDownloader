#include <mtca.h>    //Include used for mTCA functions (IPMI over Ethernet)
#include <stdio.h>

#define DEFAULT_DELAY 20  //In tens of ms -> 20 = 200 ms
#define FRONT     		 0
#define AMC_CONNECTOR    1

int jtagcmd_generator(unsigned char *ip, unsigned char *username, unsigned char *password, unsigned char amc_slot, unsigned char cmd);

void print_help(){
  printf("\n== Reset generator (AMC40) \n");
  printf("\t- Reset FPGA: ./jtagSwitch front <mch IP> <IPMI username> <IPMI password> <AMC slot [1:12]>\n");
  printf("\t- Reload FPGA: ./jtagSwitch AMC_connector <mch IP> <IPMI username> <IPMI password> <AMC slot [1:12]>\n");
  printf("\n\t\tAll slots- set \"all\" for <AMC slot [1:12]> parameter\n");
  printf("\n\t\tNAT- default username and password: \"admin\"\n");
  printf("\nBy Julian Mendez <julian.mendez@cern.ch> \n\n");
}

int main(int argc, unsigned char *argv[]){
  
  
  unsigned i=0;
  unsigned amc_slot;
  
  if(argc != 6){
    print_help();
    return -1;
  }
  
  if(!strcmp(argv[5],"all")){
    i = 12;
    amc_slot=1;
  }else{
    i = 1;
    amc_slot = atoi(argv[5]);
  }
                                                           
  if(!strcmp(argv[1],"front")){
    for(;i > 0; i--, amc_slot++)    jtagcmd_generator(argv[2], argv[3], argv[4], amc_slot, FRONT);
    
  }else if(!strcmp(argv[1],"AMC_connector")){
    for(;i > 0; i--, amc_slot++)    jtagcmd_generator(argv[2], argv[3], argv[4], amc_slot, AMC_CONNECTOR);
    
  }else{
    print_help();
  }
  
  return 0;
}

int jtagcmd_generator(unsigned char *ip, unsigned char *username, unsigned char *password, unsigned char amc_slot, unsigned char cmd){

  unsigned char data[] = {0x00, 0xA1, 0x2E, cmd};
   
  struct ipmi_intf *intf = open_lan_session(ip,                //IP
                          									username,          //Username
                          									password,          //Password
                          									(0x70+(2*amc_slot)),	  //No target specified (Default: MCH)
                          									0x82,								    //No transit addr specified (Default: 0) 
                          									7,								      //No target channel specified (Default: 0)
                          									0);
  
  struct ipmi_rs *rsp = send_ipmi_cmd(intf, 0x2e, 0x01, data, 4);	  //Send JTAG_CTRL_SET_CMD (OEM Cmd)
  
	if(rsp == NULL){
    intf->close(intf);
    printf("Reset AMC[%d] = Command failed (rsp = NULL)\n", amc_slot);
		return -1;
	}else{
		if(rsp->ccode){
      printf("Reset AMC[%d] = Command failed (comp code = 0x%02x)\n", amc_slot, rsp->ccode);
      intf->close(intf);
			return -1;
	  }
  }      
  
  printf("Reset AMC[%d] = Command success \n", amc_slot);                                                                 
  intf->close(intf);
}