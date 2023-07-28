#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>

#define Log(text) std::wcout << text << L"\n"
#define Error(text) std::wcerr << L"ERROR | " << text << L"\n"

bool delete_file(const std::wstring& full_path, bool display_logs = false)
{
   std::filesystem::path file_path(full_path);
   std::error_code ec;
   if (bool not_removed = !std::filesystem::remove(file_path, ec))
   {
      if (display_logs)
      {
         std::string msg = ec.message();
         std::wstring wstr_message(msg.begin(), msg.end());
         Error(wstr_message << L" | " << full_path);
      }
      return false;
   }
   if(display_logs) Log(L"Removed: " << full_path);
   return true;
}

size_t delete_all(const std::vector<std::wstring>& file_paths)
{
   size_t count = 0;
   for (const auto& file : file_paths)
   {
      if (delete_file(file))
      {
         count++;
      }
   }
   return count;
}

bool get_tmp_dir(std::filesystem::path& directory_path)
{
   wchar_t tmp_path[128];
   size_t len;
   [[maybe_unused]] auto fuck = _wgetenv_s(&len, tmp_path, L"TMP");
   if (0 == len)
   {
      Error("Cannot read TMP environment variable");
      return false;
   }
   directory_path = tmp_path;
   if (!std::filesystem::exists(directory_path) || !std::filesystem::is_directory(directory_path))
   {
      Error("Invalid TMP path");
      return false;
   }
   return true;
}

size_t find_cl_files(std::vector<std::wstring>& file_paths, const std::filesystem::path& directory_path, bool display_logs = false)
{
   for (const auto& entry : std::filesystem::directory_iterator(directory_path))
   {
      if (entry.is_regular_file() && entry.path().filename().string().starts_with("_CL_"))
      {
         file_paths.push_back(entry.path().wstring());
      }
   }
   if (display_logs)
   {
      for (const auto& file_path : file_paths)
      {
         Log(file_path);
      }
   }
   Log("Total files: " << file_paths.size());
   return file_paths.size();
}

int main()
{
   std::filesystem::path directory_path;
   if (!get_tmp_dir(directory_path))
      return 1;

   std::vector<std::wstring> file_paths;
   file_paths.reserve(4000);
   if (!find_cl_files(file_paths, directory_path))
      return 1;

   char decision;
   std::cout << "Delete? (y): ";
   std::cin >> decision;
   if (decision != 'y')
      return 0;

   size_t count = delete_all(file_paths);
   Log(count << " files deleted.");
   return 0;
}