#include "declarations.h"

char *szFilePath;
char *szFileTitle1;

unsigned long numListQuads = 0;
unsigned long numFixed = 0;
Quad* Quads;
char szAppPath[MAX_PATH];
char szDataFilePath[MAX_PATH];

char szFileName[MAX_PATH] = "";
char szFileTitle[MAX_PATH] = "";

bool done = FALSE;

HANDLE hThread;

HWND hDialog = NULL;

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//INITCOMMONCONTROLSEX InitCtrlEx;

	//InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	//InitCtrlEx.dwICC  = ICC_PROGRESS_CLASS;
	//InitCommonControlsEx(&InitCtrlEx);

	// GET PATH TO APP DIRECTORY
	GetModuleFileName(GetModuleHandle(NULL), szAppPath, MAX_PATH);
	
	size_t pathSize = strlen(szAppPath);
	for (size_t strX = pathSize - 1;szAppPath[strX] != '\\';strX--)
	{
		szAppPath[strX] = NULL;
	}





	//OPEN AND SCAN DATA FILE
	FILE *liststream;
	
	sprintf(szDataFilePath,"%s%s",szAppPath,"fix.dat"); //MAKE FULL DATA FILE PATH

	//MessageBox(NULL, szDataFilePath, "AAA", MB_OK);
	if( (liststream  = fopen(szDataFilePath, "r" )) == NULL)//Double check if file exists or there is some other error
	{
		MessageBox(NULL,"Cannot open fix data file","Error",MB_OK | MB_ICONHAND);
		return 1; //File Error Code
	}
	char buffer[255];
	while(!feof(liststream))
	{

    fgets(buffer, 255, liststream);
	numListQuads++;
	}
	Quads = new Quad[numListQuads];
	fseek(liststream,0,0);
	unsigned long curQuad = 0;
	while(!feof(liststream))
	{
	//Quads[curQuad].quad[0] = ' ';
	fscanf(liststream, "-%c%c%c%c %d %d %d\n", &Quads[curQuad].quad[0],&Quads[curQuad].quad[1],&Quads[curQuad].quad[2],&Quads[curQuad].quad[3], &Quads[curQuad].id,&Quads[curQuad].fromProd,&Quads[curQuad].toProd);
	
	//char baf[255];
	//wsprintf(baf,"-%.4s- %d",Quads[curQuad].quad,numListQuads);
	//MessageBox(NULL,baf,baf,MB_OK);
	curQuad++;
	}
	fclose(liststream);
	//IF COMMAND LINE HAS A " IN IT THEN SKIP THE OPEN DIALOG BOX AND USE THE INFO IN THE PATH
	if (lpCmdLine[0] != '\"')
	{

		//Get user to select path
		OPENFILENAME ofn;
		

		ZeroMemory(&ofn, sizeof(ofn));

		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFilter = "3D Movies (*.3mm)\0*.3mm\0";
		ofn.lpstrFile = szFileName;
		ofn.lpstrFileTitle = szFileTitle;
		ofn.nMaxFile = MAX_PATH;
		ofn.nMaxFileTitle = MAX_PATH;
		//ofn.lpstrInitialDir = szMSKIDSPath;
		ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		ofn.lpstrDefExt = "";
		ofn.lpstrTitle = "Open 3DMM File";
		
		//Show open dialog end prog if cancel is selected
		if(GetOpenFileName(&ofn))
		{
			szFileTitle1 = ofn.lpstrFileTitle;
			szFilePath = ofn.lpstrFile;
		}
		else
		{
			//MessageBox(NULL,"Cancel!","Cancel",MB_OK | MB_ICONEXCLAMATION);
			return 0;
		}
			
	}
	else
	{
		//ZeroMemory(szFileName,MAX_PATH);
		int curSpot = 0;
		for (int strX = 1;lpCmdLine[strX] != '\"';strX++)
		{
			szFileName[curSpot] = lpCmdLine[strX];
			curSpot++;
		}
		//MessageBox(NULL,path,path,MB_OK);
		strcpy(szFileTitle,szFileName);
		szFileTitle1 = strrchr(szFileTitle, '\\') + 1;
		//MessageBox(NULL,szFileTitle,"",MB_OK);
		szFilePath = szFileName;
		//MessageBox(NULL,szFilePath,szFilePath,MB_OK);
	}
		//MessageBox(NULL,"WHY??","??",MB_OK);
		
		DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC)MainDlgProc);

		
		return 0;
}

LRESULT CALLBACK MainDlgProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{

	switch(Msg)
	{
	case WM_INITDIALOG:
		{
		
		char szMainTitle[] = "3D Movie Expansion Fixer - ";
		char szTitle[255];
		wsprintf(szTitle,"%s%s",szMainTitle,szFileTitle1);

		//HWND hProgress;
		//GetDlgItem(hWnd,IDC_PROGRESS1);


		SetWindowText(hWnd,szTitle);
		//free(szTitle);
		
		hDialog = hWnd;
		//MessageBox(NULL,szFilePath,szFilePath,MB_OK);
		DWORD threadid;

		hThread = CreateThread(0, 0, MyThreadProc, 0, 0, &threadid);
		
		

		break;
		}
	case WM_PAINT:
		break;
	case WM_CLOSE:

		break;
	case WM_QUIT:
		
		break;
	}
	return FALSE;
}

DWORD APIENTRY MyThreadProc(void *pData)
{
	//MessageBox(NULL,szFilePath,szFilePath,MB_OK);
	while(hDialog == NULL)
	{
		//MessageBox(NULL,szFilePath,szFilePath,MB_OK);
	}
	
	switch (FixFile())
	{
	case 0:
		char szBuff[300];
		sprintf(szBuff,"Fix Complete\n\nFile: %s\nNumber of problems found and fixed: %d",szFileTitle1,numFixed);
		MessageBox(hDialog,szBuff,"3D Movie Expansion Fixer", MB_OK | MB_ICONINFORMATION);
		CloseHandle(hThread);
		EndDialog(hDialog,0);

		break;
	case 1:
		MessageBox(hDialog,"Cannot open 3DMM file. Make sure the file is not already open and is not read only.","Error",MB_OK | MB_ICONHAND);
		CloseHandle(hThread);
		EndDialog(hDialog,0);
		break;
	case 2:
		MessageBox(hDialog,"Not a valid 3DMM file","Error",MB_OK | MB_ICONHAND);
		CloseHandle(hThread);
		EndDialog(hDialog,0);
		break;
	}
	return 0;
}

int FixFile()
{
	FILE *stream3dmm;
	//MessageBox(NULL,szFilePath,szFilePath,MB_OK);
	if( (stream3dmm  = fopen( szFilePath, "r+b" )) == NULL )//Double check if file exists or there is some other error
	{
		return 1; //File Error Code
	}
	fseek(stream3dmm,0,0);
	char header[4];
	fread(header,sizeof(char),4,stream3dmm);
	//header[4] = '\0';
	if (!(header[0]=='C' && header[1]=='H' && header[2]=='N' && header[3]=='2')){
		return 2;//File Format Error Code
	}
	unsigned long indxoffset;
	unsigned long indxlen;
	unsigned long numquads;
	unsigned long dicStart;
	fseek(stream3dmm,20,0);
	//GET INDEX OFFSET AND LENGTH
	fread(&indxoffset,1,4,stream3dmm);
	fread(&indxlen,1,4,stream3dmm);
	fseek(stream3dmm,indxoffset + 4,0);
	//GET NUM OF QUADS AND Directory STARTING POINT
	fread(&numquads,1,4,stream3dmm);
	fread(&dicStart,1,4,stream3dmm);

	//char cool[255];
	//sprintf(cool,"%d",numquads);
	//MessageBox(hDialog,cool,cool,MB_OK);

	SendDlgItemMessage(hDialog,IDC_PROGRESS1, PBM_SETRANGE32, 0, numquads);
	SendDlgItemMessage(hDialog,IDC_PROGRESS1, PBM_SETSTEP, 1,0); 


	DirectoryIndexQuad* dmmDirectory;
	dmmDirectory = new DirectoryIndexQuad[numquads];
	fseek(stream3dmm,indxoffset + 20 + dicStart,0);
	for (unsigned long x = 0; x <= numquads - 1; x++)
	{
		fread(&dmmDirectory[x].offset,1,4,stream3dmm);
		fread(&dmmDirectory[x].length,1,4,stream3dmm);	
	}

	for (x = 0; x <= numquads - 1; x++)
	{
		fseek(stream3dmm,indxoffset + 20 + dmmDirectory[x].offset,0);
		char QUAD[4];
		unsigned long QuadID;
		unsigned long SectionOffset;
		unsigned long SectionSize;
		char mode;
		fread(&QUAD,1,4,stream3dmm);
		fread(&QuadID,1,4,stream3dmm);
		fread(&SectionOffset,1,4,stream3dmm);
		fread(&mode,sizeof(char),1,stream3dmm);
		unsigned char buffer[3]; 
		fread(buffer,3,1,stream3dmm); 
		SectionSize =(buffer[2]<<16)|(buffer[1]<<8)|buffer[0];
		//SEND TO CORRECT FIX FUNCTION IF NEEDED
		//NOTE: GGFR AND GGST ARE VERY SIMILAR SO THE SAME FUNCTION IS USED FOR BOTH
		if (QUAD[3] == 'G' && QUAD[2] == 'G' && QUAD[1] == 'F' && QUAD[0] == 'R') //for SCENES
		{
			FixGGFR(SectionOffset,SectionSize,stream3dmm);
		}
		else if (QUAD[3] == 'G' && QUAD[2] == 'G' && QUAD[1] == 'S' && QUAD[0] == 'T') //for SOUNDS
		{
			FixGGFR(SectionOffset,SectionSize,stream3dmm);
		}
		else if (QUAD[3] == 'A' && QUAD[2] == 'C' && QUAD[1] == 'T' && QUAD[0] == 'R') //for ACTORS/PROPS
		{
			FixACTR(SectionOffset,SectionSize,stream3dmm);
		}
		else if (QUAD[3] == 'G' && QUAD[2] == 'G' && QUAD[1] == 'A' && QUAD[0] == 'E') //for TEXTURES
		{
			//char buff[255];
			//sprintf(buff,"%.4s - %d - %d",QUAD,QuadID,SectionOffset);
			//
			//MessageBox(NULL,buff,buff,MB_OK);
			FixGGAE(SectionOffset,SectionSize,stream3dmm);
		}
		else if (QUAD[3] == 'T' && QUAD[2] == 'D' && QUAD[1] == 'T' && QUAD[0] == ' ') //for 3D WORD FONTS
		{
			FixTDT(SectionOffset,SectionSize,stream3dmm);
		}
		SendDlgItemMessage(hDialog,IDC_PROGRESS1, PBM_STEPIT, 0,0);
	}


	return 0;
}
int FixGGAE(unsigned long SectionOffset, unsigned long SectionLength, FILE *buffer)
{

	fseek(buffer,SectionOffset + 4,0);
	unsigned long num;
	unsigned long dicStart;
	//GET NUM OF QUADS AND Directory STARTING POINT
	fread(&num,1,4,buffer);
	if (num == 0)
	{
	return 0;
	}

	fread(&dicStart,1,4,buffer);

	DirectoryIndexQuad* dmmDirectory;
	dmmDirectory = new DirectoryIndexQuad[num];
	fseek(buffer,SectionOffset + 20 + dicStart,0);
	for (unsigned long x = 0; x <= num - 1; x++)
	{
		fread(&dmmDirectory[x].offset,1,4,buffer);
		fread(&dmmDirectory[x].length,1,4,buffer);

	}

	for (x = 0; x <= num - 1; x++)
	{
		//SEEK TO BEGINNING OF SECTION
		fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset,0);
		unsigned long type;
		fread(&type,1,4,buffer);
		char QUAD[4];
		unsigned long id;
		unsigned long PRODUCT;
		unsigned long QuadsNum;
		//if (SectionOffset == 744)
		//{
		//	char TEST[255];
		//	wsprintf(TEST,"%d",type);
		//	MessageBox(NULL,TEST,TEST,MB_OK);
		//}
		switch (type)
		{
		case 2://WORD  TEXTURES?
			
			fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 40,0);
			
			fread(&QUAD,sizeof(char),4,buffer);
			fread(&id,1,4,buffer);

			fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 32,0);

			
			fread(&PRODUCT,1,4,buffer);
			if (!VerifyQuad(QUAD,id,&QuadsNum))
			{
				
				break;
			}
			if (PRODUCT != Quads[QuadsNum].fromProd)
			{
				break;
			}
			fseek(buffer, -4,1);
			
			
			
			//unsigned long PRODUCT;
			//fread(&PRODUCT,1,4,buffer);

			//char TEST[255];
			//wsprintf(TEST,"%.4s - %d - %d", QUAD , id, PRODUCT );
			//MessageBox(NULL,TEST,TEST,MB_OK);

			if (!fwrite(&Quads[QuadsNum].toProd,1,4,buffer))
			{
				MessageBox(NULL,"??","??",MB_OK);
			}
			numFixed++;
			
			//unsigned long PRODUCT;
			//fread(&PRODUCT,1,4,buffer);
		break;
		case 6://ACTOR SOUNDS?
			fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 56,0);
			
			fread(&QUAD,sizeof(char),4,buffer);
			fread(&id,1,4,buffer);

			//char buff[255];
			//sprintf(buff,"%.4s - %d - %d",QUAD,id,SectionOffset + 20 + dmmDirectory[x].offset + 40);
			//
			//MessageBox(NULL,buff,buff,MB_OK);

			fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 48,0);

			fread(&PRODUCT,1,4,buffer);
			if (!VerifyQuad(QUAD,id,&QuadsNum))
			{
				//MessageBox(NULL,"NOT VERIFIED","NO",MB_OK);
				break;
			}
			if (PRODUCT != Quads[QuadsNum].fromProd)
			{
				break;
			}
			fseek(buffer, -4,1);
			
			
			
			//unsigned long PRODUCT;
			//fread(&PRODUCT,1,4,buffer);

			//char TEST[255];
			//wsprintf(TEST,"%.4s - %d - %d", QUAD , id, PRODUCT );
			//MessageBox(NULL,TEST,TEST,MB_OK);

			if (!fwrite(&Quads[QuadsNum].toProd,1,4,buffer))
			{
				MessageBox(NULL,"??","??",MB_OK);
			}
			numFixed++;
			
			//unsigned long PRODUCT;
			//fread(&PRODUCT,1,4,buffer);
		break;
		}

	}

	//MessageBox(NULL,"AA","AA",MB_OK);
	free(dmmDirectory);
	return 0;
}
int FixGGFR(unsigned long SectionOffset, unsigned long SectionLength, FILE *buffer)
{
	fseek(buffer,SectionOffset + 4,0);
	unsigned long num;
	unsigned long dicStart;
	//GET NUM OF QUADS AND Directory STARTING POINT
	fread(&num,1,4,buffer);
	if (num == 0)
	{
	return 0;
	}

	fread(&dicStart,1,4,buffer);

	DirectoryIndexQuad* dmmDirectory;
	dmmDirectory = new DirectoryIndexQuad[num];
	fseek(buffer,SectionOffset + 20 + dicStart,0);
	for (unsigned long x = 0; x <= num - 1; x++)
	{
		fread(&dmmDirectory[x].offset,1,4,buffer);
		fread(&dmmDirectory[x].length,1,4,buffer);

	}
	for (x = 0; x <= num - 1; x++)
	{
		//SEEK TO BEGINNING OF SECTION SKIPPING FRAME NUMBER with + 4
		fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 4,0);
		unsigned long type;
		fread(&type,1,4,buffer);
		char QUAD[4];
		unsigned long id;
		unsigned long PRODUCT;
		unsigned long QuadsNum;
		switch (type)
		{
		case 1://SOUND
			fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 36,0);
			
			fread(&QUAD,sizeof(char),4,buffer);
			fread(&id,1,4,buffer);

			fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 28,0);

			fread(&PRODUCT,1,4,buffer);

			if (!VerifyQuad(QUAD,id,&QuadsNum))
			{
				break;
			}
			if (PRODUCT != Quads[QuadsNum].fromProd)
			{
				break;
			}
			fseek(buffer, -4,1);
			
			
			
			if (!fwrite(&Quads[QuadsNum].toProd,1,4,buffer))
			{
				MessageBox(NULL,"??","??",MB_OK);
			}
			numFixed++;
			
			//unsigned long PRODUCT;
			//fread(&PRODUCT,1,4,buffer);
		break;
		case 4://SCENES
			fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 16,0);
			
			fread(&QUAD,sizeof(char),4,buffer);
			fread(&id,1,4,buffer);

			fseek(buffer,SectionOffset + 20 + dmmDirectory[x].offset + 8,0);
			
			
			fread(&PRODUCT,1,4,buffer);
			if (!VerifyQuad(QUAD,id,&QuadsNum))
			{
				break;
			}
			if (PRODUCT != Quads[QuadsNum].fromProd)
			{
				break;
			}
			fseek(buffer, -4,1);
			
			
			//unsigned long PRODUCT;
			//fread(&PRODUCT,1,4,buffer);

			//char TEST[255];
			//wsprintf(TEST,"%d", PRODUCT );
			//MessageBox(NULL,TEST,TEST,MB_OK);

			if (!fwrite(&Quads[QuadsNum].toProd,1,4,buffer))
			{
				MessageBox(NULL,"??","??",MB_OK);
			}
			numFixed++;

		break;
		}

	}

	//MessageBox(NULL,"AA","AA",MB_OK);
	free(dmmDirectory);
	return 0;
}
int FixACTR(unsigned long SectionOffset, unsigned long SectionLength, FILE *buffer)



{
	fseek(buffer,SectionOffset + 36,0);
		char QUAD[4];
		unsigned long id;
			fread(&QUAD,sizeof(char),4,buffer);
			fread(&id,1,4,buffer);


			
			fseek(buffer,SectionOffset + 28,0);
			unsigned long PRODUCT;
			fread(&PRODUCT,1,4,buffer);
			unsigned long QuadsNum;
			if (!VerifyQuad(QUAD,id,&QuadsNum))
			{
				return 0;
			}
			if (PRODUCT != Quads[QuadsNum].fromProd)
			{
				return 0;
			}
			fseek(buffer, -4,1);
			//char TEST[255];
			//wsprintf(TEST,"%d", PRODUCT );
			//MessageBox(NULL,TEST,TEST,MB_OK);

			if (!fwrite(&Quads[QuadsNum].toProd,1,4,buffer))
			{
				MessageBox(NULL,"??","??",MB_OK);
			}
			numFixed++;
	return 0;
}
int FixTDT(unsigned long SectionOffset, unsigned long SectionLength, FILE *buffer)
{
	fseek(buffer,SectionOffset + 16,0);
		char QUAD[4];
		unsigned long id;
			fread(&QUAD,sizeof(char),4,buffer);
			fread(&id,1,4,buffer);
			fseek(buffer,SectionOffset + 8,0);
			unsigned long PRODUCT;
			fread(&PRODUCT,1,4,buffer);
			unsigned long QuadsNum;
			if (!VerifyQuad(QUAD,id,&QuadsNum))
			{
				return 0;
			}
			if (PRODUCT != Quads[QuadsNum].fromProd)
			{
				return 0;
			}
			fseek(buffer, -4,1);
			//char TEST[255];
			//wsprintf(TEST,"%d", PRODUCT );
			//MessageBox(NULL,TEST,TEST,MB_OK);

			if (!fwrite(&Quads[QuadsNum].toProd,1,4,buffer))
			{
				MessageBox(NULL,"??","??",MB_OK);
			}
			numFixed++;

	return 0;
}
bool VerifyQuad(char Quad[4],unsigned long ID,unsigned long *retQuadsArrNum)
{
	for (unsigned long curQuad = 0;curQuad<numListQuads;curQuad++)
	{
		if ((Quads[curQuad].quad[0] == Quad[0] && Quads[curQuad].quad[1] == Quad[1] && Quads[curQuad].quad[2] == Quad[2] && Quads[curQuad].quad[3] == Quad[3]) && Quads[curQuad].id == ID )
		{
			*retQuadsArrNum = curQuad;
			//MessageBox (NULL,"LOOKIE!","FDSA",MB_OK);
			return TRUE;
		}
	}
	return FALSE;
}