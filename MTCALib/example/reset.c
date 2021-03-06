#include <mtca.h>    //Include used for mTCA functions (IPMI over Ethernet)
#include <stdio.h>

#define DEFAULT_DELAY 20  //In tens of ms -> 20 = 200 ms
#define RESET_CMD     2
#define RELOAD_CMD    3

int reset_generator(unsigned char *ip, unsigned char *username, unsigned char *password, unsigned char amc_slot, unsigned char cmd);

void print_help(){
  printf("\n== Reset generator (AMC40) \n");
  printf("\t- Reset FPGA: ./resetGen reset <mch IP> <IPMI username> <IPMI password> <AMC slot [1:12]>\n");
  printf("\t- Reload FPGA: ./resetGen reload <mch IP> <IPMI username> <IPMI password> <AMC slot [1:12]>\n");
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
                                                           
  if(!strcmp(argv[1],"reset")){
    for(;i > 0; i--, amc_slot++)    reset_generator(argv[2], argv[3], argv[4], amc_slot, RESET_CMD);
    
  }else if(!strcmp(argv[1],"reload")){
    for(;i > 0; i--, amc_slot++)    reset_generator(argv[2], argv[3], argv[4], amc_slot, RELOAD_CMD);
    
  }else{
    print_help();
  }
  
  return 0;
}

int reset_generator(unsigned char *ip, unsigned char *username, unsigned char *password, unsigned char amc_slot, unsigned char cmd){

  unsigned char data[] = {0x00, 0xA1, 0x2E, DEFAULT_DELAY};	//Command data: manufacturer ID and delay (pulse length)
   
  struct ipmi_intf *intf = open_lan_session(ip,                //IP
                          									username,          //Username
                          									password,          //Password
                          									(0x70+(2*amc_slot)),	  //No target specified (Default: MCH)
                          									0x82,								    //No transit addr specified (Default: 0) 
                          									7,								      //No target channel specified (Default: 0)
                          									0);
  
  struct ipmi_rs *rsp = send_ipmi_cmd(intf, 0x2e, cmd, data, 4);	  //Send LOCAL_RESET_FPGA_CMD or LOCAL_RELOAD_FGPA_CMD (OEM Cmd)
  
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