#ifndef _COMM_HELPER_H_
#define	_COMM_HELPER_H_

typedef struct
{
	int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
	unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
	unsigned max_size;            //! NAL Unit Buffer size
	int forbidden_bit;            //! Should always be FALSE
	int nal_reference_idc;        //! NALU_PRIORITY_xxxx
	int nal_unit_type;            //! NALU_TYPE_xxxx    
	char *buf;                    //! contains the first byte followed by the EBSP
	unsigned short lost_packets;  //! true, if packet loss is detected
} NALU_t;

int exec_cmd(const char *cmd);
int h264_nalu_startbit2 (unsigned char *buf);
int h264_nalu_startbit3 (unsigned char *buf);
int h264_nalu_annexb_get(unsigned char *frame, int length, NALU_t *nalu);

#endif

