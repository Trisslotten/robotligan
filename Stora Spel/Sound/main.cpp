 #include "include/slob/sound_engine.hpp"

int main(void) {
	// Create Sound Engine object
	SoundEngine audio_handler;

	// Initialize FMOD
	audio_handler.Init();
    
	// Create the channel group
    FMOD::ChannelGroup *channel_group = nullptr;

	// Create the sound
	FMOD::Sound *sound = nullptr;
	audio_handler.GetSystem()->createSound("assets/sound/crowd.mp3",
                                               FMOD_LOOP_NORMAL, nullptr, &sound);
	// Play the sound
    FMOD::Channel *channel = nullptr;
        audio_handler.GetSystem()->playSound(sound, channel_group, false,
                                             &channel);

	// Assign channel to channel group
    channel->setChannelGroup(channel_group);


}