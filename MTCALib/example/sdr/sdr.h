#ifndef SDR_H
#define SDR_H

#define SDR_RECORD_TYPE_FULL_SENSOR		0x01
#define SDR_RECORD_TYPE_COMPACT_SENSOR		0x02
#define SDR_RECORD_TYPE_EVENTONLY_SENSOR	0x03

struct sdr_info_s {
  unsigned char ID[2];
  unsigned char recordtype;
  unsigned char length;  
  unsigned char resID[2];
  unsigned char nextID[2];
};

struct sdr_record_list {
	unsigned short id;
	unsigned char type;
  unsigned char sensor_num;
  unsigned char sensor_type;
  unsigned char sensName[64];
  unsigned char entityID;
  unsigned char entityInst;
	struct sdr_record_list *next;
};

int ipmi_sdr_get_record(struct ipmi_intf *intf, struct sdr_record_list *sdr);
struct sdr_record_list * get_sdr_bynumtype(struct ipmi_intf *intf, unsigned char type, unsigned char num);
int ipmi_sdr_get_next_header(struct ipmi_intf *intf);
int get_sdr_info(struct ipmi_intf *intf);

#endif