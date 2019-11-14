#ifndef GLOBAL_SETTINGS_HPP_
#define GLOBAL_SETTINGS_HPP_

#include <map>
#include <string>

//#define __FILENAME__ (strchr(__FILE__, '\\') ? strchr(__FILE__. '\\') + 1 : __FILE__)

class GlobalSettings {
 private:
  // Map for all the values
  std::map<std::string, float> settings_map_;
  std::map<std::string, std::string> strings_map_;

  // NTS:	Constructor and destructor
  //		both private members
  GlobalSettings(){};
  ~GlobalSettings(){};

 public:
  // NTS:	Delete copy constructor
  //		and assignment operator
  GlobalSettings(GlobalSettings&) = delete;
  void operator=(GlobalSettings const&) = delete;

  static GlobalSettings* Access();
  void UpdateValuesFromFile();
  float ValueOf(std::string in_identifier);
  std::string StringValueOf(std::string in_identifier);
  void WriteError(std::string in_file_name, std::string in_function_name,
                  std::string in_msg);
  void WriteMapToConsole();
  void WriteValue(std::string name, float value);
  void StringWriteValue(std::string name, std::string value);
};

#endif  // !GLOBAL_SETTINGS_HPP_
