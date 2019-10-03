#ifndef PLAYER_SETTINGS_HPP_
#define PLAYER_SETTINGS_HPP_

#include <map>
#include <string>

class PlayerSettings {
 private:
  // Map for all the values
  std::map<std::string, float> settings_map_;

  // NTS:	Constructor and destructor
  //		both private members
  PlayerSettings(){};
  ~PlayerSettings();

 public:
  // NTS:	Delete copy constructor
  //		and assignment operator
  PlayerSettings(PlayerSettings&) = delete;
  void operator=(PlayerSettings const&) = delete;

  static PlayerSettings* Access();
  void UpdateValuesFromFile();
  float ValueOf(std::string in_identifier);
  void SetValue(std::string key, float value);
  void WriteError(std::string in_file_name, std::string in_function_name,
                  std::string in_msg);
  void WriteMapToConsole();
};

#endif  // !PLAYER_SETTINGS_HPP_
