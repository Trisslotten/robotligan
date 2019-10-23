#include "global_settings.hpp"

#include <fstream>
#include <iostream>

// Private:--------------------------------------------------------------------

// Public----------------------------------------------------------------------
GlobalSettings* GlobalSettings::Access() {
  // Return a pointer to this GlobalSettings object
  static GlobalSettings instance;
  return &instance;
}

void GlobalSettings::UpdateValuesFromFile() {
  // Create three strings to hold entire line, key and value
  std::string cur_str;
  std::string key_str;
  std::string val_str;

  // Clear the settings map
  this->settings_map_.clear();

  // Open a file stream
  std::ifstream settings_file("config/client_settings.txt");

  // If the file isn't able to be opened, write error
  if (!settings_file.is_open()) {
    this->WriteError("global_setting.cpp", "UpdateValuesFromFile()",
                     "Settings file cound not be opened");
    return;
  }

  // Read through the settings file
  while (settings_file) {
    // Read up to the end of the line. Save as the current line
    std::getline(settings_file, cur_str, '\n');

    // Check if the first char is '>'
    if (!(cur_str.size() == 0) && cur_str.at(0) == '>') {
      // If it is, read the string up the '=' delimiter. That is the key.
      key_str = cur_str.substr(0, cur_str.find('='));

      // Remove the '>' from the key
      key_str.erase(0, 1);

      // Remove the part holding the key from the strong, along with the
      // delimiter
      cur_str.erase(0, cur_str.find('=') + 1);

      // Save the remaining string as the value
      val_str = cur_str;

      // Save what has been read into the map
      this->settings_map_[key_str] = std::stof(val_str);
    }

	if (!(cur_str.size() == 0) && cur_str.at(0) == '#') {
      // If it is, read the string up the '=' delimiter. That is the key.
      key_str = cur_str.substr(0, cur_str.find('='));

      // Remove the '#' from the key
      key_str.erase(0, 1);

      // Remove the part holding the key from the strong, along with the
      // delimiter
      cur_str.erase(0, cur_str.find('=') + 1);

      // Save the remaining string as the value
      val_str = cur_str;

      // Save what has been read into the map
      this->strings_map_[key_str] = val_str;
    }
  }

  settings_file.close();
}

float GlobalSettings::ValueOf(std::string in_identifier) {
  // Check if the identifier exists
  if (this->settings_map_.find(in_identifier) == settings_map_.end()) {
    // If not, write error message to terminal
    // NTS: Should maybe write to a error log instead?
    this->WriteError("global_settings.cpp", "ValueOf()",
                     ("Unavailable value: " + in_identifier));
    return 0.0;
  }

  // If identifier exists return asociated value
  return this->settings_map_[in_identifier];
}

std::string GlobalSettings::StringValueOf(std::string in_identifier) {
  // Check if the identifier exists
  if (this->strings_map_.find(in_identifier) == strings_map_.end()) {
    // If not, write error message to terminal
    // NTS: Should maybe write to a error log instead?
    this->WriteError("global_settings.cpp", "StringValueOf()",
                     ("Unavailable value: " + in_identifier));
    return "";
  }

  // If identifier exists return asociated value
  return this->strings_map_[in_identifier];
}

void GlobalSettings::WriteError(std::string in_file_name,
                                std::string in_function_name,
                                std::string in_msg) {
  // TRUE:	Write to console
  // FALSE:	Write to file (/util/errorlog.txt)
  if (true) {
    std::cerr << "ERROR::" << in_file_name << "::" << in_function_name << "::"
              << "[" << in_msg << "]\n";
    return;
  }

  unsigned int file_name_size = (unsigned)strlen(in_file_name.c_str());
  unsigned int function_name_size = (unsigned)strlen(in_function_name.c_str());
  unsigned int msg_size = (unsigned)strlen(in_msg.c_str());

  std::ofstream error_file("util/errorlog.txt");

  error_file.write(in_file_name.c_str(), file_name_size);
  error_file.write("::", 2);
  error_file.write(in_function_name.c_str(), function_name_size);
  error_file.write("::", 2);
  error_file.write(in_msg.c_str(), msg_size);
  error_file.write("\n\n", 2);

  error_file.close();

  return;
}

void GlobalSettings::WriteMapToConsole() {
  std::cout << "GLOBAL SETTINGS:\n";
  for (const auto& m : this->settings_map_) {
    std::cout << m.first << "\t" << m.second << "\n";
  }
  std::cout << "NUMBER OF SETTINGS: " << this->settings_map_.size() << "\n";
  return;
}

void GlobalSettings::WriteValue(std::string name, float value) {
  std::string cur_str;
  std::string key_str;
  std::string val_str;

  // Open a file stream
  std::fstream settings_file("config/client_settings.txt");

  // If the file isn't able to be opened, write error
  if (!settings_file.is_open()) {
    this->WriteError("global_setting.cpp", "UpdateValuesFromFile()",
                     "Settings file cound not be opened");
    return;
  }

  std::string line_to_write = ">" + name + "=" + std::to_string(value) + "\n";
  bool found = false;
  // Read through the settings file
  std::string accum = "";
  while (std::getline(settings_file, cur_str)) {
    // Read up to the end of the line. Save as the current line
    bool replaced = false;
    

    // Check if the first char is '>'
    if (!(cur_str.size() == 0) && cur_str.at(0) == '>') {
      // If it is, read the string up the '=' delimiter. That is the key.
      key_str = cur_str.substr(0, cur_str.find('='));

      // Remove the '>' from the key
      key_str.erase(0, 1);
      if (key_str == name) {
        found = true;
        replaced = true;
        accum.append(line_to_write);
      }
    }
    if (!replaced) {
      accum.append(cur_str + "\n");
    }
  }
  if (!found) {
    accum.append(line_to_write);
  }
  settings_file.close();
  std::ofstream output_file("config/client_settings.txt");
  output_file.write(accum.c_str(), accum.size());
  
  output_file.close();
  UpdateValuesFromFile();
}

void GlobalSettings::StringWriteValue(std::string name, std::string value) {
  std::string cur_str;
  std::string key_str;
  std::string val_str;

  // Open a file stream
  std::fstream settings_file("config/client_settings.txt");

  // If the file isn't able to be opened, write error
  if (!settings_file.is_open()) {
    this->WriteError("global_setting.cpp", "UpdateValuesFromFile()",
                     "Settings file cound not be opened");
    return;
  }

  std::string line_to_write = "#" + name + "=" + (value) + "\n";
  bool found = false;
  // Read through the settings file
  std::string accum = "";
  while (std::getline(settings_file, cur_str)) {
    // Read up to the end of the line. Save as the current line
    bool replaced = false;

    // Check if the first char is '>'
    if (!(cur_str.size() == 0) && cur_str.at(0) == '#') {
      // If it is, read the string up the '=' delimiter. That is the key.
      key_str = cur_str.substr(0, cur_str.find('='));

      // Remove the '>' from the key
      key_str.erase(0, 1);
      if (key_str == name) {
        found = true;
        replaced = true;
        accum.append(line_to_write);
      }
    }
    if (!replaced) {
      accum.append(cur_str + "\n");
    }
  }
  if (!found) {
    accum.append(line_to_write);
  }
  settings_file.close();
  std::ofstream output_file("config/client_settings.txt");
  output_file.write(accum.c_str(), accum.size());

  output_file.close();
  UpdateValuesFromFile();
}