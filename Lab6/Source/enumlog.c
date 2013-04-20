/* GLOBALS */		
BOOL key[8] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
BOOL flag = FALSE;

/* scans for enumlog command */
/* put this inside ProcessIO function */
/* we could use a function for the repetitive parts */

			i = numBytesRead;
			while(i--){ 
				switch(USB_In_Buffer[i]){
					case 'e':
						if(flag == TRUE) 
							resetKey();
						key[0] = TRUE;
						flag = TRUE;	break;
					case 'n':
						if(key[1] == TRUE){ 
							flag = FALSE;
							resetKey();
						}
						else if(flag == TRUE)
							key[1] = TRUE;	break;
					case 'u':
						if(key[2] == TRUE){ 
							flag = FALSE;
							resetKey();
						}
						else if(flag == TRUE)
							key[2] = TRUE;	break;
					case 'm':
						if(key[3] == TRUE){ 
							flag = FALSE;
							resetKey();
						}
						else if(flag == TRUE)
							key[3] = TRUE;	break;
					case 'l':
						if(key[4] == TRUE){ 
							flag = FALSE;
							resetKey();
						}
						else if(flag == TRUE)
							key[4] = TRUE;	break;
					case 'o':
						if(key[5] == TRUE){ 
							flag = FALSE;
							resetKey();
						}
						else if(flag == TRUE)
							key[5] = TRUE;	break;
					case 'g':						
						if(key[6] == TRUE){ 
							flag = FALSE;
							resetKey();
						}
						else if(flag == TRUE)
							key[6] = TRUE;	break;
					case 0x0D:						
						if(key[7] == TRUE){ 
							flag = FALSE;
							resetKey();
						}
						else if(flag == TRUE)
							key[7] = TRUE;	break;
					default:
						flag = FALSE;
						resetKey();
				}
 			}
			if (key[7] == TRUE)
			    // PRINT LOG FILE
