aalibInit();
aalibLoad("res/loop.ogg",7,0);//Masayoshi Minoshima - Moonlives
aalibPlay(7);
aalibSetAutoloop(7,1);
printf(aalibGetStatus(7)+"\n")
while(1){
	aalibGetStatus(7)
}
