#include <libloader.h>
#include<iostream>
using namespace std;
namespace Geocosm 
{
   
   
    LIB_HANDLE LibLoader::LoadSharedLibrary(const char *dllname, int iMode)
    {
        return dlopen(dllname,RTLD_NOW);//RTLD_LAZY|RTLD_LOCAL|RTLD_DEEPBIND);
    }
    
    
    void* LibLoader::GetFunction(LIB_HANDLE libHandle, const char *fucnname)
    {
        return dlsym(libHandle,fucnname);
    }

   
    bool LibLoader::FreeSharedLibrary(LIB_HANDLE libHandle)
    {
        dlclose(libHandle);
        return true;
    }
   
    void LibLoader::GetLibList(std::vector<std::string>& list, const std::string& dir)
    {
        std::string searchStr;
        searchStr.append(".so");
        DIR* directory = opendir(dir.c_str());
        struct dirent* entry;
        
        while ((entry = readdir(directory)) != NULL) 
        {
            std::string temp = entry->d_name;
            if (temp.find(searchStr, 0) != std::string::npos)
            {
                std::string fullpath(dir);
                fullpath.append(temp);
                list.push_back(fullpath);
            }
        }
        closedir(directory); 
    }
}
