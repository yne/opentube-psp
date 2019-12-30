typedef struct{//le premier est a droite dans les bits
	char unsigned F;
	char unsigned L;
	char unsigned V;
	char unsigned version;
	
	char unsigned video:1;
	char unsigned res2:1;
	char unsigned audio:1;
	char unsigned res1:5;
	
	int  unsigned size;
}Header;
typedef struct{
	int unsigned length:24;
	int unsigned type:5;
	int unsigned res:2;
	int unsigned filter:1;
	
	int unsigned ts:24;
	int unsigned tsEx:8;
	char unsigned id[3];
	//(A/V)TAG
	//enc header
	//filter param
	//DATA
}Tag;
typedef struct{
	char unsigned ch:1;
	char unsigned bps:1;
	char unsigned khz:2;
	char unsigned fmt:4;
	//char aac;
}ATag;
typedef struct{
	char unsigned codec:4;
	char unsigned type:4;
	//if(codec==7)
	//	char AVCtype:1;
	//	char ch:1;
}VTag;
typedef struct{
	char unsigned ver;
	char unsigned AVCprofile;
	char unsigned AVCprofileCompatibility;
	char unsigned AVClevel;
	char unsigned len:2;
	char unsigned res:6;//111111
	char unsigned num:5;
	char unsigned res:3;//111
	
}DCR;//AVCDecoderConfigurationRecord 