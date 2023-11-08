#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <format>
#include <string_view>

#ifdef _WIN32
#include <io.h>
__forceinline auto EnableUnicodeOutput() noexcept
{
   [[maybe_unused]] const auto o_O = _setmode(_fileno(stdout), 0x20000);
}
#else
void EnableUnicodeOutput() noexcept
{
   // Linux
}
#endif

static constexpr bool VERBOSE{ false };

void Print(auto thing)
{
   std::wcout << thing << L"\n";
}

void PrintError(auto thing)
{
   std::wcout << L"[ERROR] " << thing << L"\n";
}

bool GetPathTmpSysDir(std::filesystem::path& pathTmpSysDir)
{
   std::string sTmpEnvVar(std::getenv("TMP"));
   pathTmpSysDir = sTmpEnvVar;
   Print(sTmpEnvVar.c_str());
   if (!std::filesystem::exists(pathTmpSysDir) || !std::filesystem::is_directory(pathTmpSysDir))
   {
      PrintError(L"Invalid TMP path");
      return false;
   }
   return true;
}

[[nodiscard]] bool DeleteFile(std::wstring_view sFilePath)
{
   static std::error_code err;
   if (std::filesystem::path pathFile(sFilePath); !std::filesystem::remove(pathFile, err))
   {
      if constexpr (VERBOSE)
      {
         auto str = err.message();
         PrintError(std::format(L"{} | {}", std::wstring(str.begin(), str.end()), sFilePath));
      }
      return false;
   }
   if constexpr (VERBOSE)
   {
      Print(std::format(L"Removed: {}", sFilePath));
   }
   return true;
}

class JunkEraser
{   
   std::vector<std::wstring> m_files;
public:
   JunkEraser()
   {
      m_files.reserve(6969);
   }
   explicit JunkEraser(const std::filesystem::path& directory) : JunkEraser()
   {
      if (!FindFiles(directory))
      {
         Print(std::format(L"No CL files found in {}", directory.wstring()));
      }
   }

   bool FindFiles(const std::filesystem::path& directory)
   {
      uint64_t nTotalJunkSize{};
      for (const auto& entryTmpFile : std::filesystem::directory_iterator(directory))
      {
         if (entryTmpFile.is_regular_file() && entryTmpFile.path().filename().string().starts_with("_CL_"))
         {
            nTotalJunkSize += entryTmpFile.file_size();
            m_files.push_back(entryTmpFile.path().wstring());
         }
      }
      if constexpr (VERBOSE)
      {
         for (const auto& sTmpFilePath : m_files)
         {
            Print(sTmpFilePath);
         }
      }
      Print(std::format(L"Total size: {} MB | in {} files", nTotalJunkSize/1'048'576, m_files.size()));
      return static_cast<bool>(m_files.size());
   }

   auto DeleteFoundFiles()
   {
      size_t nFilesDeleted = 0;
      for (const auto& sFilePath : m_files)
      {
         if (DeleteFile(sFilePath))
         {
            nFilesDeleted++;
         }
      }
      return nFilesDeleted;
   }
};

int main()
{
   EnableUnicodeOutput();

   std::filesystem::path pathTmpSysDir;
   if (!GetPathTmpSysDir(pathTmpSysDir))
   {
      return 1;
   }

   JunkEraser eraser;
   if (!eraser.FindFiles(pathTmpSysDir))
   {
      return 0;
   }

   char choice{};
   std::wcout << L"Delete? (y): ";
   std::cin >> choice;
   if (choice == 'y')
   {
      auto nFilesDeleted = eraser.DeleteFoundFiles();
      Print(std::format(L"{} files deleted.", nFilesDeleted));
   }
   return 0;
}
