
/* scans for enumlog command */
/* put this inside ProcessIO function */
i = numBytesRead;
while(i--){ 
	switch(USB_In_Buffer[i]){
		case 'e':
			validChar = 1; break;
		case 'n':
			if(validChar != 1)
				validChar = 0;
			else validChar = 2;	break;
		case 'u':
			if(validChar != 2)
				validChar = 0;
			else validChar = 3;	break;
		case 'm':
			if(validChar != 3)
				validChar = 0;
			else validChar = 4;	break;
		case 'l':
			if(validChar != 4)
				validChar = 0;
			else validChar = 5;	break;
		case 'o':
			if(validChar != 5)
				validChar = 0;
			else validChar = 6;	break;
		case 'g':						
			if(validChar != 6)
				validChar = 0;
			else validChar = 7;	break;
		case 0x0D:						
			if(validChar != 7)
				validChar = 0;
			else validChar = 8;	break;
		default:
			validChar = 0;
	}
 }
if (validChar == 8)
	PRINT LOG FILE
