#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <malloc.h>
#include <regex>
#include <streambuf>
#include <cerrno>

#include <typeinfo>
#include <cmath>
#include <numeric>
#include <iterator>
#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <3ds.h>

#include "utils.h"
#include "launcheshop.h"
#include "data.h"
#include "json/json.h"

static const u16 top = 0x140;
static std::string region = "ALL";
int sourceDataType;
Json::Value sourceData;

int mkpath(std::string s,mode_t mode){
    size_t pre=0,pos;
    std::string dir;
    int mdret = 0;
    if(s[s.size()-1]!='/'){
        s+='/';
    }
    while((pos=s.find_first_of('/',pre))!=std::string::npos){
        dir=s.substr(0,pos++);
        pre=pos;
        if(dir.size()==0) continue; // if leading / first time is 0 length
        if((mdret=mkdir(dir.c_str(),mode)) && errno!=EEXIST){
            return mdret;
        }
    }
    return mdret;
}

char parse_hex(char c){
    if ('0' <= c && c <= '9') return c - '0';
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    std::abort();
}

char* parse_string(const std::string & s){
    char* buffer = new char[s.size() / 2];
    for (std::size_t i = 0; i != s.size() / 2; ++i)
        buffer[i] = 16 * parse_hex(s[2 * i]) + parse_hex(s[2 * i + 1]);
    return buffer;
}

std::string GetTicket(std::string titleId, std::string encTitleKey, char* titleVersion){
    std::ostringstream ofs;
    ofs.write(tikTemp, 0xA50);
    ofs.seekp(top+0xA6, std::ios::beg);
    ofs.write(titleVersion, 0x2);
    ofs.seekp(top+0x9C, std::ios::beg);
    ofs.write(parse_string(titleId), 0x8);
    ofs.seekp(top+0x7F, std::ios::beg);
    ofs.write(parse_string(encTitleKey), 0x10);
    return ofs.str();
}

void load_JSON_data() {
    printf(" loading horns.json...\n");
    std::ifstream ifs("/TIKdevil/horns.json");
    Json::Reader reader;
    Json::Value obj;
    reader.parse(ifs, obj);
    sourceData = obj; // array of characters
    
    if(sourceData[0]["titleID"].isString()) {
      sourceDataType = JSON_TYPE_ONLINE;
    } else if (sourceData[0]["titleid"].isString()) {
      sourceDataType = JSON_TYPE_HORNS;
    }
}

int util_compare_u64(const void* e1, const void* e2) {
    u64 id1 = *(u64*) e1;
    u64 id2 = *(u64*) e2;

    return id1 > id2 ? 1 : id1 < id2 ? -1 : 0;
}

std::vector<std::string> util_get_installed_tickets(){
	std::vector<std::string> vTickets;
	Result res = 0;
    u32 ticketCount = 0;
    if(R_SUCCEEDED(res = AM_GetTicketCount(&ticketCount))) {
        u64* ticketIDs = (u64*) calloc(ticketCount, sizeof(u64));
        if(ticketIDs != NULL) {
            if(R_SUCCEEDED(res = AM_GetTicketList(&ticketCount, ticketCount, 0, ticketIDs))) {
                qsort(ticketIDs, ticketCount, sizeof(u64), util_compare_u64);
				char cur[34];
                for(u32 i = 0; i < ticketCount && R_SUCCEEDED(res); i++) {
					sprintf(cur,"%016llX", ticketIDs[i]);
					vTickets.push_back(cur);
				}
			}
		}
	}
	return vTickets;
}

bool is_ticket_installed(std::vector<std::string> &vNANDTiks, std::string &titleId){
    for(unsigned int foo =0; foo < vNANDTiks.size(); foo++)
    {
        std::string curTik = vNANDTiks.at(foo);
        std::transform(curTik.begin(), curTik.end(), curTik.begin(), ::tolower);
        if(titleId == curTik) 
        {
            return true;				
        }
    }
    return false;
}

bool action_missing_tickets(std::vector<std::string> &vEncTitleKey, std::vector<std::string> &vTitleID, int &n, std::string regionFilter){
	// Set up the reading of json
	if (check_JSON(true))
	{
		load_JSON_data();
	} else
		return false;
	
	printf(" Checking for already installed tiks...\n\n");
  
	n = 0;
	
	std::vector<std::string> vNANDTiks = util_get_installed_tickets();
	
	const char*  ctitleId = nullptr;
	const char*  cencTitleKey = nullptr;
	const char*  ctitleName = nullptr;
	
	std::string titleType;
	std::string titleId;
	std::string encTitleKey;
	std::string titleRegion;
	bool isNotSystemTitle;
	for (unsigned int i = 0; i < sourceData.size(); i++) {
		// Check that the encTitleKey isn't null
		if (sourceData[i]["encTitleKey"].isNull())
		{
			continue;
		}
		ctitleId = sourceData[i]["titleID"].asCString();
		cencTitleKey = sourceData[i]["encTitleKey"].asCString();
		ctitleName = sourceData[i]["name"].asCString();
		
        titleId = ctitleId;
		titleRegion = sourceData[i]["region"].asString();
		titleType = sourceData[i]["titleID"].asString().substr(4,4);
		
		isNotSystemTitle = (titleType == ESHOP_GAMEAPP or titleType == ESHOP_DLC or titleType == ESHOP_DSIWARE);
		
        // a specific region is selected
        if(ctitleId != NULL and cencTitleKey != NULL and ctitleName != NULL and isNotSystemTitle == true and (regionFilter == "ALL" || titleRegion == regionFilter || titleRegion == "ALL" || titleRegion == ""))
        {
            // add it if it isn't a system title and the region matches
            
            
            if(is_ticket_installed(vNANDTiks, titleId)==false)
            {
                n++;
                
                encTitleKey = cencTitleKey;
                encTitleKey.erase(remove_if(encTitleKey.begin(), encTitleKey.end(), isspace), encTitleKey.end());
                
                vTitleID.push_back(titleId);
                vEncTitleKey.push_back(encTitleKey);
            }
        }
	}
	
	printf(" Missing tickets: %d\n\n", n);
    return true;
}

void action_install(std::vector<std::string> vEncTitleKey,std::vector<std::string> vTitleID, int index){
	printf(" Installing missing tickets...\n\n");
	char titleVersion[2] = {0x00, 0x00};
	Handle hTik;
	u32 writtenbyte;
	int instlastPrint = 0;
	for (unsigned int i =0; i < vTitleID.size(); i++)
	{
		
		AM_InstallTicketBegin(&hTik);
		std::string curr = GetTicket(vTitleID.at(i), vEncTitleKey.at(i), titleVersion);
		FSFILE_Write(hTik, &writtenbyte, 0, curr.c_str(), 0x150000, 0);
		AM_InstallTicketFinish(hTik);
		
		int instprogress = i*100/index;
		if((instprogress%10==0 and instprogress>instlastPrint)or(instprogress==0 and instlastPrint==0)){
			printf("%d%% ", instprogress);
			instlastPrint = instprogress+1;
		}
	}
	printf(" 100%%\n\n Done!\n\n");
}


void action_about(gfxScreen_t screen){
    PrintConsole infoConsole;
    PrintConsole* currentConsole = consoleSelect(&infoConsole);
    consoleInit(screen, &infoConsole);
    
    consoleClear();
	
	printf(CONSOLE_RED "\n tikShop " VERSION_STRING " by DanTheMan827\n\n" CONSOLE_RESET);
    printf("  Generate missing tickets,\n");
    printf("  install, and launch the eshop!\n\n\n");
    printf(CONSOLE_BLUE " Special thanks to:\n\n" CONSOLE_RESET);
	printf("  Kyraminol, cearp, Drakia, steveice10,\n");
    printf("  and Mmcx125.\n");
    printf("\n\n Commit: " REVISION_STRING "\n");
    
    consoleSelect(currentConsole);
}

void select_oneclick(){
	consoleClear();
	std::vector<std::string> Keys;
	std::vector<std::string> IDs;
	int n;
    
    if(action_missing_tickets(Keys, IDs, n, region) == true){
        action_install(Keys, IDs, n);

        if(launch_eshop()){
            while(true){
                wait_key();
            }
        }
    }
    
    printf("Press any key to exit.");
    wait_key();
}

int main(int argc, const char* argv[]){
    u32 *soc_sharedmem, soc_sharedmem_size = 0x100000;
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    httpcInit(0);
    soc_sharedmem = (u32 *)memalign(0x1000, soc_sharedmem_size);
    socInit(soc_sharedmem, soc_sharedmem_size);
    sslcInit(0);
	amInit();
    acInit();
    cfguInit();
    AM_InitializeExternalTitleDatabase(false);
    
    // Make sure the TIKdevil directory exists on the SD card
    mkpath("/TIKdevil/", 0777);
    
    // Load the region from system secure info
    region = GetSystemRegion();
    
    action_about(GFX_BOTTOM);
    select_oneclick();
	
    cfguExit();
    acExit();
	amExit();
    gfxExit();
    httpcExit();
    socExit();
    sslcExit();
}
