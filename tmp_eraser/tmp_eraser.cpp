#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <format>
#include <string_view>
#include <io.h>

static constexpr bool VERBOSE{ false };

void Print(auto thing)
{
   std::wcout << thing << L"\n";
}

void PrintError(auto thing)
{
   std::wcout << L"[ERROR] " << thing << L"\n";
}

__forceinline auto EnableUnicodeOutput() noexcept
{
   [[maybe_unused]] const auto o_O = _setmode(_fileno(stdout), 0x20000);
}

bool GetPathTmpSysDir(std::filesystem::path& pathTmpSysDir)
{
   std::array<wchar_t, 256> sTmpSysDir{};
   size_t nTmpSysDirPathLength{};
   [[maybe_unused]] const auto errCode = _wgetenv_s(&nTmpSysDirPathLength, sTmpSysDir.data(), sTmpSysDir.size(), L"TMP");
   if (0 == nTmpSysDirPathLength)
   {
      PrintError(L"Cannot read TMP environment variable");
      return false;
   }
   pathTmpSysDir.assign(sTmpSysDir.data());
   Print(sTmpSysDir.data());
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
   std::vector<std::wstring> vTmpFilePaths;
public:
   JunkEraser()
   {
      vTmpFilePaths.reserve(6969);
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
            vTmpFilePaths.push_back(entryTmpFile.path().wstring());
         }
      }
      if constexpr (VERBOSE)
      {
         for (const auto& sTmpFilePath : vTmpFilePaths)
         {
            Print(sTmpFilePath);
         }
      }
      Print(std::format(L"Total size: {} MB | in {} files", nTotalJunkSize/1'048'576, vTmpFilePaths.size()));
      return static_cast<bool>(vTmpFilePaths.size());
   }

   auto DeleteFoundFiles()
   {
      size_t nFilesDeleted = 0;
      for (const auto& sFilePath : vTmpFilePaths)
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

   JunkEraser eraser(pathTmpSysDir);

   char decision{};
   std::wcout << L"Delete? (y): ";
   std::cin >> decision;
   if (decision == 'y')
   {
      auto nFilesDeleted = eraser.DeleteFoundFiles();
      Print(std::format(L"{} files deleted.", nFilesDeleted));
   }
   return 0;
}
