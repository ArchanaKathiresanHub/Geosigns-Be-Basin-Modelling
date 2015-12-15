#include <libloader.h>

namespace Geocosm
{


   LIB_HANDLE LibLoader::LoadSharedLibrary(const char *dllname, int iMode)
   {
#ifdef _WIN32
      return LoadLibrary(dllname);
#else
      return dlopen(dllname,RTLD_NOW);//RTLD_LAZY|RTLD_LOCAL|RTLD_DEEPBIND);
#endif
   }


   void* LibLoader::GetFunction(LIB_HANDLE libHandle, const char *fucnname)
   {
#ifdef _WIN32
      return GetProcAddress(libHandle,  (LPCSTR)fucnname);
#else
      return dlsym(libHandle,fucnname);
#endif
   }


   bool LibLoader::FreeSharedLibrary(LIB_HANDLE libHandle)
   {
#ifdef _WIN32
      FreeLibrary(libHandle);
#else
      dlclose(libHandle);
#endif
      return true;
   }

#if _WIN32
   void LibLoader::GetLibList(std::vector<std::string>& list, const std::string& dir)
   {
      WIN32_FIND_DATA FindFileData;
      HANDLE hFind;
      std::string searchStr(dir);
      searchStr.append("*.dll");
      hFind = FindFirstFile(searchStr.c_str(), &FindFileData);

      list.clear();
      if (hFind == INVALID_HANDLE_VALUE) 
      {
         return;
      }
      do {
         std::string filestr(dir);
         filestr.append(FindFileData.cFileName);
         list.push_back(filestr);
      }while (FindNextFile(hFind, &FindFileData) != 0);

      FindClose(hFind);
   }
#else
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
#endif

}
