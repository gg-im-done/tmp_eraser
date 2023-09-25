#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>
#include <io.h>

#define Log(text) std::wcout << text << L"\n"
#define Error(text) std::wcerr << L"ERROR | " << text << L"\n"

using Path = std::filesystem::path;
using String = std::wstring;

bool delete_file(const String& full_path, bool show_log = false)
{
   static std::error_code err;
   Path file_path(full_path);
   if (bool not_removed = !std::filesystem::remove(file_path, err))
   {
      if (show_log)
      {
         auto error_str = err.message();
         String error_text(error_str.begin(), error_str.end());
         Error(error_text << L" | " << full_path);
      }
      return false;
   }
   if(show_log) Log(L"Removed: " << full_path);
   return true;
}

size_t delete_all(const std::vector<String>& path_strings)
{
   size_t count = 0;
   for (const auto& file_full_path_string : path_strings)
   {
      if (delete_file(file_full_path_string))
      {
         count++;
      }
   }
   return count;
}

bool get_tmp_dir(Path& tmp_dir)
{
	wchar_t tmp_path[256] = { 0 };
   size_t len = 0;
   [[maybe_unused]] auto fuck = _wgetenv_s(&len, tmp_path, L"TMP");
   if (0 == len)
   {
      Error(L"Cannot read TMP environment variable");
      return false;
   }
   tmp_dir = tmp_path;
   if (!std::filesystem::exists(tmp_dir) || !std::filesystem::is_directory(tmp_dir))
   {
      Error(L"Invalid TMP path");
      return false;
   }
   return true;
}

size_t find_cl_files(std::vector<String>& path_strings, const Path& tmp_dir, bool show_log = false)
{
   uint64_t total_size{};
   for (const auto& entry : std::filesystem::directory_iterator(tmp_dir))
   {
      if (entry.is_regular_file() && entry.path().filename().string().starts_with("_CL_"))
      {
         total_size += entry.file_size();
         path_strings.push_back(entry.path().wstring());
      }
   }
   if (show_log)
   {
      for (const auto& file_path : path_strings)
      {
         Log(file_path);
      }
   }
   Log(L"Total size: " << std::setprecision(4) << (total_size / 1024.0) / 1024.0 << L" Mb");
   Log(L"Total files: " << path_strings.size());
   return path_strings.size();
}

int main()
{
   [[maybe_unused]] auto o_O = _setmode(_fileno(stdout), 0x20000);

   Path tmp_dir;
   if (!get_tmp_dir(tmp_dir))
      return 1;

   std::vector<String> path_strings;
   path_strings.reserve(6000);
   if (!find_cl_files(path_strings, tmp_dir))
   {
      Log(L"no CL files found, quitting...");
      return 2;
   }

   std::wcout << L"Delete? (y): ";
   char decision{ 0 };
   std::cin >> decision;
   if (decision == 'y')
   {
      auto count = delete_all(path_strings);
      Log(count << L" files deleted.");
   }
   else
   {
      Log(L"\nOk. Quitting... \\o/");
   }
   return 0;
}