/*
 * GameChat.h:
 *  Game HUD component for drawing messages.
 *  These messages include things like "A monster has appeared!" and chat messages.
 */

#ifndef GAMECHAT_H_
#define GAMECHAT_H_

#include <SDL.h>
#include <string>
#include <vector>

#include <lcommon/SerializeBuffer.h>
#include <ldraw/Colour.h>
#include <ldraw/ldrawfwd.h>

#include "TextField.h"

#include "lanarts_defines.h"

class GameState;

/*Handle key repeating, in steps*/
const int INITIAL_REPEAT_MS = 40;
const int NEXT_REPEAT_MS = 5;
const int NEXT_BACKSPACE_MS = 3;


/*Represents a coloured message in chat*/
struct ChatMessage {
	std::string sender, message;
	Colour sender_colour, message_colour;
	int exact_copies;
	ChatMessage(const std::string& sender = std::string(),
			const std::string& message = std::string(),
			const Colour& sender_colour = Colour(255, 255, 255),
			const Colour& message_colour = Colour(255, 255, 255)) :
			sender(sender), message(message), sender_colour(sender_colour), message_colour(
					message_colour), exact_copies(1) {
	}
	bool operator==(const ChatMessage& cm) const {
		/*Used to determine if message should be added, or 'exact_copies' incremented*/
		return sender == cm.sender && message == cm.message
				&& sender_colour == cm.sender_colour
				&& message_colour == cm.message_colour;
	}
	void draw(const ldraw::Font& font, float alpha, Pos pos) const;
	bool empty() const;
	void serialize(SerializeBuffer& serializer);
	void deserialize(SerializeBuffer& serializer);
};

class GameChat {
public:
	GameChat();

	void step(GameState* gs);
	void draw(GameState* gs) const;
	void clear();

	void add_message(const ChatMessage& cm);
	void add_message(const std::string& msg,
			const Colour& colour = Colour(255, 255, 255));

	bool is_typing_message();
	/*Returns whether has handled event completely or not*/
	bool handle_event(GameState* gs, SDL_Event *event);
	void toggle_chat(GameState* gs);

private:
	ChatMessage get_field_as_chat_message(GameState* gs,
			bool include_username) const;
	bool handle_special_commands(GameState* gs, const std::string& command);

	void reset_typed_message();
	void draw_player_chat(GameState* gs) const;
	TextField typing_field;

	std::vector<ChatMessage> messages;
	bool show_chat, is_typing;
	float fade_out, fade_out_rate;
};

#endif /* GAMECHAT_H_ */
