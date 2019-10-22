#include "Chat.hpp"
#include "util/input.hpp"
#include <GLFW\glfw3.h>
#include <sstream>
#include <iostream>
#include "shared/shared.hpp"

void Chat::AddMessage(std::string name, std::string text, unsigned int message_from) {
  if (text.size() == 0) return;
  std::vector<std::string> result;
  std::istringstream iss(text);
  for (std::string s; iss >> s;) result.push_back(s);

  RowMessage row;
  //std::string row;
  row.name = name;
  row.message_from = message_from;
  row.offset = name.size() * 6;
  for (int i = 0; i < result.size(); ++i) {
    int space_left = row_length_ - (row.message.size() + row.name.size());
  
    if (space_left >= result[i].size()) {
        row.message += result[i] + " ";
    } else {
      std::string left_over = result[i];
      if (result[i].size() > 10) {
        row.message += result[i].substr(0, space_left);
        left_over = result[i].erase(0, space_left);
      } 
     
      messages_.push_back(row);
      row.name = "";
      row.offset = 0.f;

      while (left_over.size() > row_length_) {
        row.message = left_over.substr(0, row_length_);
        messages_.push_back(row);
        left_over.erase(0, row_length_);
      }
      
      if (left_over.size() > 0)
		row.message = left_over + " ";
    }
  }
  if (row.message.size() > 0) messages_.push_back(row);
}
//void Chat::AddToCurrentMessage(std::string text) { current_message_ += text; }

void Chat::Update(float dt) {
  if (send_message_ == true) {
    //AddMessage(current_message_);
    current_message_ = "";
    send_message_ = false;
  }
  if (close_chat_ == true) {
    close_timer_ -= dt;
    if (close_timer_ < 0) {
      show_chat_ = false;
      close_chat_ = false;
    }
  } else {
    current_message_ += Input::GetCharacters();

    if (current_message_.size() > 200)
      current_message_ = current_message_.substr(0, 200);
    // std::cout << messages_.size() << std::endl;
    if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE)) {
      if (current_message_.size() > 0) current_message_.pop_back();
    }
  }
 }

void Chat::SubmitText(glob::Font2DHandle font) {
   int index = messages_.size() - 5;
  if (index < 0) index = 0;
  int counter = 0;

  for (index; index < messages_.size(); ++index) {
    glm::vec4 color(1.f,1.f,1.f,1.f);

	if (messages_[index].name.size() > 0) {
      glm::vec4 name_color;
      if (messages_[index].message_from == TEAM_RED) {
        name_color = glm::vec4(1, 0, 0, 1);
      } else if (messages_[index].message_from == TEAM_BLUE) {
		  name_color = glm::vec4(0, 0, 1, 1);
	  } else {
		  name_color = glm::vec4(1,1,0,1);
	  }
      glob::Submit(font, draw_pos_ + glm::vec2(0, -20.f * counter), 20, messages_[index].name, name_color);
	}
    glob::Submit(font, draw_pos_ + glm::vec2(messages_[index].offset,- 20.f * counter), 20, messages_[index].message, color);
    counter++;
  
  }
  std::string temp = current_message_; 
  if (current_message_.size() > row_length_) {
    temp = current_message_.substr(current_message_.size() - row_length_,
                                   row_length_);
  }

  

  glob::Submit(font, draw_pos_ + glm::vec2(0, -20.f * 5), 20, temp, glm::vec4(1, 1, 1, 1));
 }

 void Chat::SetShowChat() {
   show_chat_ = true;
   close_chat_ = false;
   close_timer_ = 1000.f;
 }

void Chat::SetSendMessage(bool send) { send_message_ = send; }

void Chat::CloseChat() {
  close_timer_ = 5.0f;
  close_chat_ = true;
}

std::string Chat::GetCurrentMessage() { return current_message_; }

bool Chat::IsVisable() { return show_chat_; }

bool Chat::IsClosing() { return close_chat_;}

bool Chat::IsTakingChatInput() { return show_chat_ && !close_chat_; }