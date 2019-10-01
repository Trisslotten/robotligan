#include "Chat.hpp"
#include "util/input.hpp"
#include <GLFW\glfw3.h>
#include <sstream>
#include <iostream>

void Chat::AddMessage(std::string text) {
  if (text.size() == 0) return;
  std::vector<std::string> result;
  std::istringstream iss(text);
  for (std::string s; iss >> s;) result.push_back(s);

  std::string row;
  for (int i = 0; i < result.size(); ++i) {
    int diff = row.size() + result[i].size() - row_length_;
    if (diff <= 0) {
        row += result[i] + " ";
    } else {
      if (result[i].size() > 10) {
        row += result[i].substr(0, diff % row_length_);
	  }
      messages_.push_back(row);
      std::string left_over = result[i].erase(0, diff % row_length_);
      while (left_over.size() > row_length_) {
        row = left_over.substr(0, row_length_);
        messages_.push_back(row);
        left_over.erase(0, row_length_);
	  }
      row = "";
      if (left_over.size() > 0)
		row = left_over + " ";
    }
  }
  if (row.size() > 0)
  messages_.push_back(row);
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
	glob::Submit(font, glm::vec2(50.f, 700.f - 20.f * counter), 20,
                 messages_[index], glm::vec4(0, 1, 1, 1));
    counter++;
  
  }
  std::string temp = current_message_; 
  if (current_message_.size() > row_length_) {
    temp = current_message_.substr(current_message_.size() - row_length_,
                                   row_length_);
  }

  glob::Submit(font, glm::vec2(50.f, 700.f - 20.f * 5), 20, temp, glm::vec4(0, 1, 1, 1));
 }

 void Chat::SetShowChat() {
   show_chat_ = true;
   close_chat_ = false;
   close_timer_ = 1000.f;
 }

void Chat::SetSendMessage(bool send) { send_message_ = send; }

void Chat::CloseChat() {
  close_timer_ = 2.0f;
  close_chat_ = true;
}

std::string Chat::GetCurrentMessage() { return current_message_; }

bool Chat::IsVisable() { return show_chat_; }

bool Chat::IsClosing() { return close_chat_;}

bool Chat::IsTakingChatInput() { return show_chat_ && !close_chat_; }