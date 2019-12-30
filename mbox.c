	typedef struct{
		SceKernelMsgPacket header;
		char text[8];
	}MyMessage;
	int mbxid = sceKernelCreateMbx("lua.result", 0, NULL);
	MyMessage msg={{0},"Hello"};
	sceKernelSendMbx(mbxid,(void*)&msg);
	printf("sent !\n");
	MyMessage*pmsg=NULL;unsigned timeout=0;
	int res=sceKernelPollMbx(mbxid,(void*)&pmsg);
	printf("%i\n",res);
	if(res>=0){
		printf("received : ");
		sceKernelReceiveMbx(mbxid,(void*)&pmsg,&timeout);
		if(pmsg)printf(pmsg->text);
		else printf("<error>");
		printf("\n");
	}
	sceKernelDeleteMbx(mbxid);