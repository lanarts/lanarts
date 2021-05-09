/*
 * shapes_example.cpp:
 *  Exercises the shape drawing methods
 */

#include <SDL.h>

#include <lua.hpp>

#include <lcommon/Timer.h>
#include <lcommon/lua_lcommon.h>
#include <lcommon/math_util.h>

#include <luawrap/LuaValue.h>
#include <luawrap/calls.h>
#include <luawrap/functions.h>

#include "lua_ldraw.h"

#include "Colour.h"

#include "colour_constants.h"

#include "display.h"
#include "draw.h"
#include "Image.h"
#include "Animation.h"
#include "DirectionalDrawable.h"
#include "Font.h"

static bool handle_event(SDL_Event* event) {
	ldraw::process_event(event);
	SDL_Keycode keycode = event->key.keysym.sym;
	SDL_Keymod keymod = (SDL_Keymod) event->key.keysym.mod;

	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN: {
		break;
	}
	case SDL_QUIT: {
		return false;
	}
	case SDL_KEYDOWN: {
		if (keycode == SDLK_RETURN || keycode == SDLK_ESCAPE) {
			return false;
		}
		if (keycode == SDLK_F1) {
			ldraw::display_set_fullscreen(!ldraw::display_is_fullscreen());
		}
	}
		break;
	}
	return true;
}

typedef void (*DrawFunc)();

static void draw_loop(DrawFunc draw_func) {
	ldraw::Font fpsfont("sample.ttf", 40);
	Timer timer;
	int frames = 0;
	while (1) {
		frames += 1;
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (!handle_event(&event)) {
				return; // Exit draw loop
			}
		}
		ldraw::display_draw_start();
		draw_func();
		double seconds = timer.get_microseconds() / 1000.0 / 1000.0;
		fpsfont.drawf(ldraw::DrawOptions(COL_GOLD).origin(ldraw::RIGHT_BOTTOM),
				PosF(400, 400), "%d", int(frames / seconds));
		ldraw::display_draw_finish();
		SDL_Delay(5);
	}
}

static void draw_shapes() {
	ldraw::draw_rectangle(COL_LIGHT_RED, BBoxF(100, 100, 300, 300));
	ldraw::draw_circle(COL_LIGHT_BLUE, PosF(200, 200), 100);
}

ldraw::Image image;

static void draw_images() {
	BBox box(0, 0, 10, 10);
	FOR_EACH_BBOX(box, x, y) {
		image.draw(PosF(x * 40, y * 40));
	}
}

ldraw::Font font;

static void draw_text() {
	using namespace ldraw;

	font.draw(COL_WHITE, Pos(0, 0), "Hello World!");
	int w = font.draw(COL_BLUE, Pos(0, 100), "Hello ");
	font.draw(COL_RED, Pos(w, 100), "World!");

	font.draw_wrapped(DrawOptions().origin(CENTER).colour(COL_PALE_RED),
			Pos(200, 200), 250,
			"This text is wrapped because it's sort of long.");
}

ldraw::Drawable arrow;

static void draw_directional() {
	using namespace ldraw;
	draw_rectangle(COL_BABY_BLUE, BBoxF(0, 0, 400, 400));
	PosF center(200, 200);
	BBox box(0, 0, 10, 10);
	FOR_EACH_BBOX(box, x, y) {
		PosF pos(x * 40 + 20, y * 40 + 20);
		float dir = compute_direction(pos, center);
		arrow.draw(DrawOptions().angle(dir).origin(CENTER), pos);
	}
}

ldraw::Drawable animation;
float frame = 0.0f;

static void draw_animation() {
	using namespace ldraw;
	PosF center(200, 200);
	frame += 1.0f;
	animation.draw(DrawOptions().origin(CENTER).frame(frame), center);
}

static lua_State* L;

static void draw_script() {
	ldraw::Font fpsfont("sample.ttf", 40);

	luawrap::globals(L)["draw"].push();
	luawrap::call<void>(L);

	fpsfont.drawf(ldraw::DrawOptions(COL_GOLD).origin(ldraw::LEFT_BOTTOM),
			PosF(0, 400), "Lua");
}

static void draw_script_from_drawable() {
	using namespace ldraw;
	using namespace luawrap;

	ldraw::Font fpsfont("sample.ttf", 40);
	lua_getglobal(L, "drawable");
	Drawable drawable = get<Drawable>(L, -1);
	drawable.draw(PosF());

	fpsfont.drawf(ldraw::DrawOptions(COL_GOLD).origin(ldraw::LEFT_BOTTOM),
			PosF(0, 400), "LuaDrawable");
}

static void setup_lua_state() {
	using namespace ldraw;

	L = lua_open();
	luaL_openlibs(L);

	LuaValue globals = luawrap::globals(L);

	lua_register_ldraw(L, globals);
	lua_register_lcommon(L);

}

static void draw_luascript(lua_State* L, const char* file) {
	luawrap::dofile(L, file);
	draw_loop(draw_script);
	if (lua_tostring(L,-1)) {
		printf("%s\n", lua_tostring(L,-1));
	}
}

int main(int argc, char** argv) {
	using namespace ldraw;

	display_initialize(__FILE__, Size(400, 400), false);
	image.initialize("sample.png");
	font.initialize("sample.ttf", 20);

	std::vector<Image> arr_images = image_split(Image("arrows.png"),
			SizeF(32, 32));
	arrow = Drawable(new DirectionalDrawable(arr_images, FLOAT_PI / 2));

	std::vector<Image> anim_images = image_split(Image("animation.png"),
			SizeF(480.0f / 6, 120));
	animation = Drawable(new Animation(anim_images, 0.1f));

	draw_loop(draw_shapes);
	draw_loop(draw_images);
	draw_loop(draw_text);
	draw_loop(draw_directional);
	draw_loop(draw_animation);

	setup_lua_state();

	draw_luascript(L, "scripts/draw_shapes.lua");
	draw_luascript(L, "scripts/draw_images.lua");
	draw_luascript(L, "scripts/draw_text.lua");
	draw_luascript(L, "scripts/draw_directional.lua");
	draw_luascript(L, "scripts/draw_animation.lua");
	draw_luascript(L, "scripts/draw_custom.lua");
	draw_loop(draw_script_from_drawable);

	lua_close(L);
	return 0;
}
