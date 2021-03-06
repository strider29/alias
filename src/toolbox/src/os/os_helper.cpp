#include <toolbox/os/os_helper.h>


namespace toolbox {

namespace OSHelper {

int
execute(const std::string& command, const std::string& arguments, bool detached)
{
  // TODO: maybe this should be fixed for windows
  std::stringstream ss;
  ss << command << " " << arguments;
  if (detached) {
    ss << " &";
  }
  const std::string cmd = ss.str();
  return std::system(cmd.c_str()) == 0;
}

int
killByName(const std::string& name)
{
  return execute("killall -e", name);
}

bool
deleteFolder(const std::string& folder)
{
  return std::filesystem::remove_all(folder);
}

std::string
getUniqueSystemID(void)
{
  return "TODO_IMPLEMENT_THIS";
}


}

}
