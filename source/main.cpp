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
#include "data.h"
#include "menu.h"

#include "svchax/svchax.h"
#include "json/json.h"

static const u16 top = 0x140;
static bool bSvcHaxAvailable = true;
static bool bInstallMode = false;
static std::string regionFilter = "off";

std::string upper(std::string s)
{
  std::string ups;
  
  for(unsigned int i = 0; i < s.size(); i++)
  {
    ups.push_back(std::toupper(s[i]));
  }
  
  return ups;
}

struct display_item {
  int ld;
  int index;
};

bool compareByLD(const display_item &a, const display_item &b)
{
    return a.ld < b.ld;
}

bool FileExists (std::string name){
    struct stat buffer;
    return (stat (name.c_str(), &buffer) == 0);
}

std::string u32_to_hex_string(u32 i)
{
    std::stringstream stream;
    stream << std::setfill ('0') << std::setw(sizeof(u32)*2) << std::hex << i;
    return stream.str();
}

int mkpath(std::string s,mode_t mode)
{
    size_t pre=0,pos;
    std::string dir;
    int mdret = 0;

    if(s[s.size()-1]!='/'){
        // force trailing / so we can handle everything in loop
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

char parse_hex(char c)
{
    if ('0' <= c && c <= '9') return c - '0';
    if ('A' <= c && c <= 'F') return c - 'A' + 10;
    if ('a' <= c && c <= 'f') return c - 'a' + 10;
    std::abort();
}

char* parse_string(const std::string & s)
{
    char* buffer = new char[s.size() / 2];
    for (std::size_t i = 0; i != s.size() / 2; ++i)
        buffer[i] = 16 * parse_hex(s[2 * i]) + parse_hex(s[2 * i + 1]);
    return buffer;
}

void CreateTicket(std::string titleId, std::string encTitleKey, char* titleVersion, std::string outputFullPath)
{
    std::ofstream ofs;

    ofs.open(outputFullPath, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
    ofs.write(tikTemp, 0xA50);
    ofs.close();

    ofs.open(outputFullPath, std::ofstream::out | std::ofstream::in | std::ofstream::binary);

    //write version
    ofs.seekp(top+0xA6, std::ios::beg);
    ofs.write(titleVersion, 0x2);

    //write title id
    ofs.seekp(top+0x9C, std::ios::beg);
    ofs.write(parse_string(titleId), 0x8);

    //write key
    ofs.seekp(top+0x7F, std::ios::beg);
    ofs.write(parse_string(encTitleKey), 0x10);

    ofs.close();
}



void removeForbiddenChar(std::string* s)
{
    std::string::iterator it;
    std::string illegalChars = "\\/:?\"<>|";
    for (it = s->begin() ; it < s->end() ; ++it){
        bool found = illegalChars.find(*it) != std::string::npos;
        if(found)
        {
            *it = ' ';
        }
    }
}

std::istream& GetLine(std::istream& is, std::string& t)
{
    t.clear();
    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        switch (c) {
            case '\n':
              return is;
            case '\r':
              if (sb->sgetc() == '\n')
                sb->sbumpc();
              return is;
            case  EOF:
              if (t.empty())
                is.setstate(std::ios::eofbit);
              return is;
            default:
              t += (char)c;
        }
    }
}

std::string ToHex(const std::string& s)
{
    std::ostringstream ret;
    for (std::string::size_type i = 0; i < s.length(); ++i)
    {
        int z = s[i]&0xff;
        ret << std::hex << std::setfill('0') << std::setw(2) << z;
    }
    return ret.str();
}

int levenshtein_distance(const std::string &s1, const std::string &s2)
{
    // To change the type this function manipulates and returns, change
    // the return type and the types of the two variables below.
    int s1len = s1.size();
    int s2len = s2.size();
    
    auto column_start = (decltype(s1len))1;
    
    auto column = new decltype(s1len)[s1len + 1];
    std::iota(column + column_start, column + s1len + 1, column_start);
    
    for (auto x = column_start; x <= s2len; x++) {
        column[0] = x;
        auto last_diagonal = x - column_start;
        for (auto y = column_start; y <= s1len; y++) {
            auto old_diagonal = column[y];
            auto possibilities = {
                column[y] + 1,
                column[y - 1] + 1,
                last_diagonal + (s1[y - 1] == s2[x - 1]? 0 : 1)
            };
            column[y] = std::min(possibilities);
            last_diagonal = old_diagonal;
        }
    }
    auto result = column[s1len];
    delete[] column;
    return result;
}


void action_about()
{
    consoleClear();
	
	printf(CONSOLE_RED "TIKdevil by Kyraminol\n");
    printf(CONSOLE_BLUE "CIAngel by cearp and Drakia\n" CONSOLE_RESET);
    printf("Download encTitleKey.bin,\n");
    printf("generate tickets right on the 3DS\n");
    printf("and (soon) install them.\n");
    wait_key_specific("\nPress A to continue.\n", KEY_A);
}

void action_download()
{
	consoleClear();
	mkpath("/TIKdevil/", 0777);

	if (FileExists("/TIKdevil/encTitleKeys.bin")){
		printf("File exists... we will overwrite it!\n");
	}

	printf("Downloading encTitleKeys.bin...\n");
	FILE *oh = fopen("/TIKdevil/encTitleKeys.bin", "wb");
	Result res = DownloadFile("http://3ds.nfshost.com/downloadenc", oh, true);
	fclose(oh);
	if (res != 0)
	{
		printf("Could not download file.\n");
	}
	else
	{
		printf("Downloaded OK!\n");
	}
}


void action_make_full(){
	consoleClear();
	mkpath("/TIKdevil/tickets/", 0777);
	char titleVersion[2] = {0x00, 0x00};
	int countGenerated = 0;
	std::ifstream keyfile("/TIKdevil/encTitleKeys.bin", std::ifstream::binary);
	keyfile.seekg(0x10, std::ios::beg);
	std::vector<char> buffer (0x20,0);
	printf("Generating all tickets from encTitleKeys.\n");
	printf(CONSOLE_RED "This can take up to 30 minutes.\n" CONSOLE_RESET);
	while(keyfile.read(buffer.data(), buffer.size()))
	{
		std::string titleId = "";
		std::string encTitleKey = "";

		for (u16 i=0x8; i<0x10; i++)
		{
			titleId = titleId + buffer[i];
		}
		for (u16 i=0x10; i<0x20; i++)
		{
			encTitleKey = encTitleKey + buffer[i];
		}

		titleId = ToHex(titleId);
		encTitleKey = ToHex(encTitleKey);

		countGenerated++;
		CreateTicket(titleId, encTitleKey, titleVersion, "/TIKdevil/tickets/" + titleId + ".tik");
	}
	consoleClear();
	printf("Tickets saved to sd:/TIKdevil/tickets/");
	printf("Total Title Keys parsed: %d\n\n", countGenerated);
	wait_key_specific("Press A to return.", KEY_A);	
}

void action_make_delta(){
	consoleClear();
	mkpath("/TIKdevil/tickets/", 0777);
	char titleVersion[2] = {0x00, 0x00};
	int countGenerated = 0;
	int countTotal = 0;
	std::ifstream keyfile("/TIKdevil/encTitleKeys.bin", std::ifstream::binary);
	keyfile.seekg(0x10, std::ios::beg);
	std::vector<char> buffer (0x20,0);
	printf(CONSOLE_RED "Parsing encTitleKeys.bin (can take some time).\n\n" CONSOLE_RESET);
	while(keyfile.read(buffer.data(), buffer.size()))
	{
		std::string titleId = "";
		std::string encTitleKey = "";

		for (u16 i=0x8; i<0x10; i++)
		{
			titleId = titleId + buffer[i];
		}
		for (u16 i=0x10; i<0x20; i++)
		{
			encTitleKey = encTitleKey + buffer[i];
		}

		titleId = ToHex(titleId);
		encTitleKey = ToHex(encTitleKey);

		if (! FileExists("/TIKdevil/tickets/" + titleId + ".tik")){
			countGenerated++;
			consoleClear();
			printf(CONSOLE_RED "Parsing encTitleKeys.bin (can take some time).\n\n" CONSOLE_RESET);
			printf("Generating Ticket for title id: %s..", titleId.c_str());
			CreateTicket(titleId, encTitleKey, titleVersion, "/TIKdevil/tickets/" + titleId + ".tik");
			printf("done!\n");
			printf("%d tickets generated so far.\n", countGenerated);
		}
		countTotal++;
	}
	consoleClear();
	printf("Tickets saved to sd:/TIKdevil/tickets/");
	printf("New Tickets generated: %d\n", countGenerated);
	printf("Total Title Keys parsed: %d\n\n", countTotal);
	wait_key_specific("Press A to return.", KEY_A);	
}


/* Menu functions */
void menu_main()
{
    const char *options[] = {
		"Download & Full Generate (recommended 1st time)",
		"Download & Delta Generate (updating)",
		"Download encTitleKeys.bin",
		"Generate Tickets [FULL]",
		"Generate Tickets [DELTA]",
        "About TIKdevil",
        "Exit"
    };
    char footer[37];

    while (true)
    {
        // We have to update the footer every draw, incase the user switches install mode
        sprintf(footer, "Original CIAngel by cearp and Drakia");

        int result = menu_draw("TIKdevil by Kyraminol", footer, 0, sizeof(options) / sizeof(char*), options);

        switch (result)
        {
			case 0:
				action_download();
				action_make_full();
			break;
			case 1:
                action_download();
				action_make_delta();
            break;
            case 2:
                action_download();
            break;
            case 3:
                action_make_full();
            break;
			case 4:
                action_make_delta();
            break;
            case 5:
                action_about();
            break;
			case 6:
                return;
            break;
        }

        clear_screen(GFX_BOTTOM);
    }
}

int main(int argc, const char* argv[])
{
    /* Sadly svchax crashes too much, so only allow install mode when running as a CIA
    // Trigger svchax so we can install CIAs
    if(argc > 0) {
        svchax_init(true);
        if(!__ctr_svchax || !__ctr_svchax_srv) {
            bSvcHaxAvailable = false;
            //printf("Failed to acquire kernel access. Install mode disabled.\n");
        }
    }
    */
    
    // argc is 0 when running as a CIA, and 1 when running as a 3dsx
    if (argc > 0)
    {
        bSvcHaxAvailable = false;
    }

    u32 *soc_sharedmem, soc_sharedmem_size = 0x100000;
    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);

    httpcInit(0);
    soc_sharedmem = (u32 *)memalign(0x1000, soc_sharedmem_size);
    socInit(soc_sharedmem, soc_sharedmem_size);
    sslcInit(0);

    if (bSvcHaxAvailable)
    {
        amInit();
        AM_InitializeExternalTitleDatabase(false);
    }

    init_menu(GFX_TOP);
    menu_main();

    if (bSvcHaxAvailable)
    {
        amExit();
    }

    gfxExit();
    httpcExit();
    socExit();
    sslcExit();
}
