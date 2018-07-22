/*
 * Music.cpp:
 *  Represents a piece of music, only one piece of music can be playing at a time.
 */

#include <SDL_mixer.h>

#include "Music.h"
#include "Sound.h"
#include <lcommon/sdl_headless_support.h>

/* Controls the smart pointer deletion */
static void __freemusic_callback(void* mm) {
	Mix_FreeMusic((Mix_Music*)mm);
}

namespace lsound {

	Music::Music() {
	}

	Music::~Music() {
	}

	Music::Music(const std::string& filename) {
		init(filename);
	}

	void Music::init(const std::string& filename) {
#ifdef EMSCRIPTEN
		Mix_Music* mm = NULL;
#else
		Mix_Music* mm = Mix_LoadMUS(filename.c_str());
#endif
		clear();
		if (mm) {
			_music = smartptr<Mix_Music>(mm, __freemusic_callback);
		} else {
                    //printf("Music Error %s\n",Mix_GetError());
                }
	}

	void Music::play() const {
		if (_music) {
			Mix_PlayMusic(_music.get(), 0);
		} else {
                    //printf("Music Error %s\n",Mix_GetError());
                }
	}
	void Music::clear() {
		_music = smartptr<Mix_Music>();
	}

	bool Music::empty() const {
		return _music.empty();
	}

	void Music::loop() const {
		if (_music) {
			Mix_PlayMusic(_music.get(), -1);
		} else {
                        //printf("Music Error %s\n",Mix_GetError());
                }
	}

	Sound load_music(const std::string& filename) {
		return Sound(new Music(filename));
	}
}
