import <iostream>;
import <string>;
import <vector>;
import <filesystem>;
import <print>;
import <string_view>;
import <map>;

static constexpr bool VERBOSE{ true };



class EnvironmentVariableBank
{
   std::map<std::string, std::filesystem::path> m_envVars;
public:
   EnvironmentVariableBank(std::string_view sName)
   {
      FindVariable(sName);
   }

   bool FindVariable(std::string_view sName)
   {
      auto cstrEnvVar = std::getenv(sName.data());
      if (!cstrEnvVar)
      {
         std::print("ERROR | Environment variable '{}' not found\n", sName);
         return false;
      }
      std::string sEnvironmentVariableValue(cstrEnvVar);
      std::filesystem::path pathTmpSysDir(sEnvironmentVariableValue);
      m_envVars[sName.data()] = pathTmpSysDir;
      return true;
   }

   [[nodiscard]] auto GetDirectory(const std::string& sName)
   {
      if (m_envVars.contains(sName))
      {
         if (std::filesystem::exists(m_envVars[sName]) & std::filesystem::is_directory(m_envVars[sName]))
         {
            return m_envVars[sName];
         }
         std::print("ERROR | GetDirectory: '{}'", sName);
      }
      return std::filesystem::path();
   }
};



class JunkEraser
{
public:
   [[nodiscard]] static bool DeleteFile(std::string_view sFilePath)
   {
      static std::error_code err;
      if (std::filesystem::path pathFile(sFilePath); !std::filesystem::remove(pathFile, err))
      {
         if constexpr (VERBOSE) std::print("ERROR | {} | {}\n", err.message(), sFilePath);
         return false;
      }
      if constexpr (VERBOSE) std::print("Removed: {}\n", sFilePath);
      return true;
   }
private:
   std::vector<std::string> m_files;
public:
   JunkEraser()
   {
      m_files.reserve(6969);
   }
   explicit JunkEraser(const std::filesystem::path& directory) : JunkEraser()
   {
      if (!FindFiles(directory))
      {
         std::print("No CL files found in {}\n", directory.string());
      }
   }

   size_t FindFiles(const std::filesystem::path& directory)
   {
      uint64_t nTotalJunkSize{};
      if (directory.empty())
      {
         return false;
      }
      for (const auto& entryTmpFile : std::filesystem::directory_iterator(directory))
      {
         if (entryTmpFile.is_regular_file() && entryTmpFile.path().filename().string().starts_with("_CL_"))
         {
            nTotalJunkSize += entryTmpFile.file_size();
            m_files.push_back(entryTmpFile.path().string());
         }
      }
      if constexpr (VERBOSE)
      {
         for (const auto& sTmpFilePath : m_files)
         {
            std::print("{}\n", sTmpFilePath);
         }
         std::print("--------------------------------------------------------------------------------\n");
      }
      std::print("Total size: {} MB | in {} files\n", nTotalJunkSize/1'048'576, m_files.size());
      return m_files.size();
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

   [[nodiscard]] auto GetFoundFilesCount() const noexcept
   {
      return m_files.size();
   }
};



int main()
{
   EnvironmentVariableBank bank("TMP");
   JunkEraser eraser(bank.GetDirectory("TMP"));
   if (eraser.GetFoundFilesCount() == 0)
   {
      return 0;
   }
   std::print("Delete? (y): ");
   char choice{};
   std::cin >> choice;
   if (choice == 'y')
   {
      auto nFilesDeleted = eraser.DeleteFoundFiles();
      std::print("{} files deleted.\n", nFilesDeleted);
      std::cin >> choice;
   }
   return 0;
}
