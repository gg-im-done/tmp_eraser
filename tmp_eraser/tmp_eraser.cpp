#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <format>
#include <io.h>

static constexpr bool VERBOSE{ false };

auto Print(auto thing)
{
   std::wcout << thing << L"\n";
}

[[nodiscard]] bool DeleteFile(const std::wstring& sFilePath)
{
   static std::error_code err;
   if (std::filesystem::path pathFile(sFilePath); !std::filesystem::remove(pathFile, err))
   {
      if (VERBOSE)
      {
         auto str = err.message();
         std::wstring sError(str.begin(), str.end());
         Print(std::format(L"[ERROR] {} | {}", sError, sFilePath));
      }
      return false;
   }
   if (VERBOSE)
      Print(std::format(L"Removed: {}", sFilePath));
   return true;
}

size_t DeleteFiles(const std::vector<std::wstring>& vTmpFilePaths)
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

bool GetPathTmpSysDir(std::filesystem::path& pathTmpSysDir)
{
   std::array<wchar_t, 256> sTmpSysDir;
   size_t nTmpSysDirPathLength = 0;
   [[maybe_unused]] auto errCode = _wgetenv_s(&nTmpSysDirPathLength, sTmpSysDir.data(), sTmpSysDir.size(), L"TMP");
   if (0 == nTmpSysDirPathLength)
   {
      Print(L"[ERROR] Cannot read TMP environment variable");
      return false;
   }
   pathTmpSysDir = std::wstring(sTmpSysDir.data());
   if (!std::filesystem::exists(pathTmpSysDir) || !std::filesystem::is_directory(pathTmpSysDir))
   {
      Print(L"[ERROR] Invalid TMP path");
      return false;
   }
   return true;
}

size_t FildTmpFiles(std::vector<std::wstring>& vTmpFilePaths, const std::filesystem::path& pathDirectory)
{
   uint64_t nTotalJunkSize{};
   for (const auto& entryTmpFile : std::filesystem::directory_iterator(pathDirectory))
   {
      if (entryTmpFile.is_regular_file() && entryTmpFile.path().filename().string().starts_with("_CL_"))
      {
         nTotalJunkSize += entryTmpFile.file_size();
         vTmpFilePaths.push_back(entryTmpFile.path().wstring());
      }
   }
   if (VERBOSE)
   {
      for (const auto& sTmpFilePath : vTmpFilePaths)
      {
         Print(sTmpFilePath);
      }
   }
   Print(std::format(L"Total size: {} MB | in {} files", std::to_wstring(nTotalJunkSize/1'048'576), std::to_wstring(vTmpFilePaths.size())));
   return vTmpFilePaths.size();
}

int main()
{
   [[maybe_unused]] auto o_O = _setmode(_fileno(stdout), 0x20000);

   std::filesystem::path pathTmpSysDir;
   if (!GetPathTmpSysDir(pathTmpSysDir))
      return 1;

   std::vector<std::wstring> vTmpFilePaths;
   vTmpFilePaths.reserve(6000);
   if (!FildTmpFiles(vTmpFilePaths, pathTmpSysDir))
   {
      Print(L"No CL files found, quitting...");
      return 2;
   }

   std::wcout << L"Delete? (y): ";
   char decision{};
   std::cin >> decision;
   if (decision == 'y')
   {
      auto nFilesDeleted = DeleteFiles(vTmpFilePaths);
      Print(std::format(L"{} files deleted.", std::to_wstring(nFilesDeleted)));
   }
   return 0;
}