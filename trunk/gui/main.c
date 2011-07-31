
	
	printchar(fontID, __DATE__" \x03 "__TIME__);//on vram
	sceIoWrite(2,__DATE__" "__TIME__"\n",11+1+8+1);//on cout
