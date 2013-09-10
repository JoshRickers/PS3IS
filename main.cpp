#include <NoRSX.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <fstream>
#include <iostream>
#include <string.h>
#include <vector>
#include <sysmodule/sysmodule.h>
#include <io/pad.h>

using namespace std;
static NoRSX *GFX;

string detectUSB()
{
	int fd;
	string USB = "";
	int usbNum = 0;
	char path [256];
	
	for(usbNum = 0; usbNum < 11; usbNum++)
	{
		sprintf(path, "/dev_usb00%d/", usbNum);
		if(sysLv2FsOpenDir(path, &fd) == 0)
		{
			USB = string(path);
		}
	}
	
	return USB;
}

void searchDir(string rootDir, vector<string> &fileNames)
{	
	sysFSDirent entry;
	s32 fd;
	u64 read;
	string filePath;
	
	if(sysLv2FsOpenDir(rootDir.c_str(), &fd) == 0)
	{
		sysLv2FsOpenDir(rootDir.c_str(), &fd);
		while(!sysLv2FsReadDir(fd,&entry,&read) && strlen(entry.d_name)>0)
		{
			if(entry.d_name[0] != '.')
			{
				if(entry.d_type == 0x01)
				{
					GFX->Flip();
					filePath.erase();
					filePath = rootDir + string(entry.d_name) + "/";
					searchDir(filePath, fileNames);					
				}
				else
				{
					if(string(entry.d_name).substr(string(entry.d_name).length() - 3, 3) == "jpg")
					{
						fileNames.push_back(rootDir.substr(0, rootDir.size()) + string(entry.d_name));
					}
				}
			}
		}
		sysLv2FsCloseDir(fd);
	}
}

void copyFiles(string destDir, vector<string> files)
{
	string destFilePath = "";
	string origFilePath = "";
	for(std::vector<string>::const_iterator i = files.begin(); i != files.end(); i++)
	{
		origFilePath = *i;
		destFilePath = destDir + origFilePath.substr(origFilePath.find_last_of("/") + 1);
		ifstream origFile(origFilePath.c_str());
		ofstream destFile(destFilePath.c_str());
		destFile << origFile.rdbuf();
	}
}

s32 main(s32 argc, const char* argv[])
{
	padInfo padinfo;
	padData paddata;
	ioPadInit(7);
	
	GFX = new NoRSX();
	Font F(JPN, GFX);
	Bitmap BMap(GFX);
	NoRSX_Bitmap Precalculated_Layer;
	BMap.GenerateBitmap(&Precalculated_Layer);
	
	vector<string> files;
	int debug = 0;
	string USBStatus = "";
	string HDDStatus = "";
	string USBFiles = "";
	string HDDFiles = "";
	string test = "";
	int ImagesFound = 160;
	
	F.PrintfToBitmap(100,80,&Precalculated_Layer,COLOR_RED,15,"PS3 Image Search, Press X to start or START to quit");
	
	GFX->AppStart();
	while(GFX->GetAppStatus())
	{
		ioPadGetInfo(&padinfo);
		for(int i = 0; i < MAX_PORT_NUM; i++)
		{
			if(padinfo.status[i])
			{
				ioPadGetData(i, &paddata);
				if(paddata.BTN_START)
				{
					GFX->AppExit();
				}
				if(paddata.BTN_CROSS)
				{
					HDDStatus = "/dev_hdd0/Forensic/";
					searchDir("/dev_hdd0/Forensic/", files);
					for(std::vector<string>::const_iterator i = files.begin(); i != files.end(); ++i)
					{
						BMap.DrawBitmap(&Precalculated_Layer);
						HDDFiles = "File: " + *i;
						F.PrintfToBitmap(600,ImagesFound,&Precalculated_Layer,COLOR_WHITE,15,"HDD Files: %s", HDDFiles.c_str());
						GFX->Flip();
						ImagesFound = ImagesFound + 20;
					}
					
					USBStatus = "No USB";
					USBFiles = "";
					if(detectUSB() != "")
					{
						USBStatus = detectUSB();
						copyFiles(USBStatus, files);
					}
				}
			}
		}
		
		BMap.DrawBitmap(&Precalculated_Layer);
		F.Printf(100,100,COLOR_WHITE,15,"USB Status: %s", USBStatus.c_str());
		F.Printf(100,120,COLOR_WHITE,15,"USB Files: %s", USBFiles.c_str());
		F.Printf(100,140,COLOR_WHITE,15,"HDD Status: %s", HDDStatus.c_str());
		F.Printf(100,160,COLOR_WHITE,15,"HDD Files: %s", HDDFiles.c_str());
		F.Printf(100,180,COLOR_WHITE,15,"Images Found: %d", ImagesFound);
		F.Printf(100,200,COLOR_RED,15,"DEBUG: %d", debug);
		GFX->Flip();
		debug++;
	}
	
	BMap.ClearBitmap(&Precalculated_Layer);
	GFX->NoRSX_Exit();
	ioPadEnd();
	return 0;
}
