#include <conio.h>
//~
import <string_view>;
import <filesystem>;
import <expected>;
import <vector>;
import <string>;
import <print>;
import <map>;

namespace cfg::log
{
	static constexpr bool VERBOSE{ false };
}

/**
* @brief A class that stores environment variables
* 
* @details
*    - The environment variable is found by the constructor or by the FindVariable() method
*    - The environment variable is stored in a map
*    - The map is indexed by the environment variable name
*    - The environment variable value is a path (std::filesystem::path) to a directory
*/
class EnvironmentVariableBank
{
   std::map<std::string, std::filesystem::path> m_envVars;
public:
	enum class ErrorType : char
	{
		NOT_CONTAINED,
		NOT_EXISTING,
		NOT_DIRECTORY
	};

	static constexpr void CheckUnexpected(ErrorType err)
	{
		switch (err)
		{
		case EnvironmentVariableBank::ErrorType::NOT_CONTAINED:
			std::println("ERROR | Environment variable does not exist in The Bank");
			break;
		case EnvironmentVariableBank::ErrorType::NOT_EXISTING:
			std::println("ERROR | Environment variable does not exist in the system");
			break;
		case EnvironmentVariableBank::ErrorType::NOT_DIRECTORY:
			std::println("ERROR | Environment variable is not a directory");
			break;
		default:
			std::println("ERROR | UNKNOWN ERROR");
			break;
		}
	}

   EnvironmentVariableBank(std::string_view sName)
   {
      FindVariable(sName);
   }

   bool FindVariable(std::string_view sName)
   {
      const auto cstrEnvVar = std::getenv(sName.data());
      if (!cstrEnvVar)
      {
         std::println("ERROR | Environment variable '{}' not found", sName);
         return false;
      }
      const std::string sEnvironmentVariableValue(cstrEnvVar);
      const std::filesystem::path pathTmpSysDir(sEnvironmentVariableValue);
      m_envVars[sName.data()] = pathTmpSysDir;
      return true;
   }

   [[nodiscard]] auto GetDirectory(const std::string& sName) -> std::expected<std::filesystem::path, ErrorType>
   {
		if (!m_envVars.contains(sName))
			return std::unexpected(ErrorType::NOT_CONTAINED);
		else if (!std::filesystem::exists(m_envVars[sName]))
			return std::unexpected(ErrorType::NOT_EXISTING);
		else if (!std::filesystem::is_directory(m_envVars[sName]))
			return std::unexpected(ErrorType::NOT_DIRECTORY);

      return std::filesystem::path(m_envVars[sName]);
   }
};

/**
* @brief Erases junk files from the temporary directory using the <filesystem> library
* 
* @details
*    - Junk files are files that start with "_CL_" and are located in the temporary directory
*    - The temporary directory is specified by the environment variable "TMP"
*    - The environment variable "TMP" is found by the EnvironmentVariableBank
* 
* @note
*   - The temporary directory is usually located at "C:\Users\%USERNAME%\AppData\Local\Temp"
*/
class JunkEraser
{
public:
   [[nodiscard]] static bool DeleteFile(std::string_view sFilePath)
   {
      static std::error_code err;
      if (std::filesystem::path pathFile(sFilePath); !std::filesystem::remove(pathFile, err))
      {
         if constexpr (cfg::log::VERBOSE) std::println("ERROR | {} | {}", err.message(), sFilePath);
         return false;
      }
      if constexpr (cfg::log::VERBOSE) std::println("Removed: {}", sFilePath);
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
         std::println("No junk files found in [{}]", directory.string());
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
      if constexpr (cfg::log::VERBOSE)
      {
         for (const auto& sTmpFilePath : m_files)
         {
            std::println("{}", sTmpFilePath);
         }
         std::print("--------------------------------------------------------------------------------\n");
      }
		if (m_files.size())
			std::println("Total size: {} MB | in {} files", nTotalJunkSize/1'048'576, m_files.size());
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

void WeAreDone([[maybe_unused]]const long double o_O = 6e-9l)
{
	std::print(R"(
   _______________                        |*\_/*|________
  |  ___________  |     .-.     .-.      ||_/-\_|______  |
  | |           | |    .****. .****.     | |           | |
  | |   0   0   | |    .*****.*****.     | |   0   0   | |
  | |     -     | |     .*********.      | |     -     | |
  | |   \___/   | |      .*******.       | |   \___/   | |
  | |___     ___| |       .*****.        | |___________| |
  |_____|\_/|_____|        .***.         |_______________|
    _|__|/ \|_|_.............*.............._|________|_
   / ********** \                          / ********** \
 /  ************  \                      /  ************  \
--------------------                    --------------------
)");
}

int main()
{
   EnvironmentVariableBank bank("TMP");
   const auto directoryTmp = bank.GetDirectory("TMP");
	if (directoryTmp.has_value())
	{
		JunkEraser eraser(directoryTmp.value());
		if (eraser.GetFoundFilesCount())
		{
			std::println("Delete? (y)");
			const auto choice = static_cast<char>(_getch());
			if (choice == 'y')
			{
			   const auto nFilesDeleted = eraser.DeleteFoundFiles();
			   std::println("{} files deleted.", nFilesDeleted);
			}
		}
	}
	else
	{
		EnvironmentVariableBank::CheckUnexpected(directoryTmp.error());
	}

	WeAreDone();
	_getch();
}
