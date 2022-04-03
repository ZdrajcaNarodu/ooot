#include "ultra64/types.h"
#include "macros.h"
#include "keyboard.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <unordered_map>

#if !defined(DISABLE_SDL_CONTROLLER)
#include <SDL2/SDL.h>
#endif
#include "../player/players.h"
#include "../options.h"

extern "C"
{
	void set_fullscreen(bool value);
}

#define MAX_BUTTONS (SDL_BUTTON_X2 + 1)

static u8* expandMouseButtonBits(u32 bits, u8* out)
{
	for(int i = 1; i < MAX_BUTTONS; i++)
	{
		out[i] = (bits & (1 << (i - 1))) ? 1 : 0;
	}
	return out;
}

static u8 getButtonId(const std::string& str)

{
	if(str == "BUTTON_LEFT")
		return SDL_BUTTON_LEFT;
	if(str == "BUTTON_MIDDLE")
		return SDL_BUTTON_MIDDLE;
	if(str == "BUTTON_RIGHT")
		return SDL_BUTTON_RIGHT;
	if(str == "BUTTON_X1")
		return SDL_BUTTON_X1;
	if(str == "BUTTON_X2")
		return SDL_BUTTON_X2;
	return 0;
}

static const char* getButtonName(u8 button)
{
	switch(button)
	{
		case SDL_BUTTON_LEFT:
			return "BUTTON_LEFT";
		case SDL_BUTTON_RIGHT:
			return "BUTTON_RIGHT";
		case SDL_BUTTON_MIDDLE:
			return "BUTTON_MIDDLE";
		case SDL_BUTTON_X1:
			return "BUTTON_X1";
		case SDL_BUTTON_X2:
			return "BUTTON_X2";
	}
	return "UNKNOWN";
}

bool saveJson(rapidjson::Document& doc, const std::string& jsonFilePath)
{
	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);

	if(!doc.Accept(writer))
	{
		return false;
	}

	std::ofstream out(jsonFilePath.c_str(), std::ofstream::trunc);
	out << buffer.GetString();

	return true;
}

namespace oot::hid
{
	namespace controller
	{
		const char* getInputName(Button input)
		{
			switch(input)
			{
				case Button::STICK_X_UP:
					return "STICK_X_UP";
				case Button::STICK_X_LEFT:
					return "STICK_X_LEFT";
				case Button::STICK_X_DOWN:
					return "STICK_X_DOWN";
				case Button::STICK_X_RIGHT:
					return "STICK_X_RIGHT";
				case Button::A_BUTTON:
					return "A_BUTTON";
				case Button::B_BUTTON:
					return "B_BUTTON";
				case Button::Z_TRIG:
					return "Z_TRIG";
				case Button::U_CBUTTONS:
					return "U_CBUTTONS";
				case Button::L_CBUTTONS:
					return "L_CBUTTONS";
				case Button::D_CBUTTONS:
					return "D_CBUTTONS";
				case Button::R_CBUTTONS:
					return "R_CBUTTONS";
				case Button::R_TRIG:
					return "R_TRIG";
				case Button::L_TRIG:
					return "L_TRIG";
				case Button::START_BUTTON:
					return "START_BUTTON";
				case Button::WALK_BUTTON:
					return "WALK_BUTTON";
				case Button::DEBUG_MENU:
					return "DEBUG_MENU";
				case Button::FAST_FORWARD:
					return "FAST_FORWARD";
				case Button::CENTER_CAMERA:
					return "CENTER_CAMERA";

				case Button::OCARINA:
					return "OCARINA";
				case Button::HOOKSHOT:
					return "HOOKSHOT";
				case Button::BOW_ARROW:
					return "BOW_ARROW";
				case Button::LENS_OF_TRUTH:
					return "LENS_OF_TRUTH";
				case Button::BOOTS_TOGGLE:
					return "BOOTS_TOGGLE";
				case Button::SWORD_TOGGLE:
					return "SWORD_TOGGLE";
				case Button::SHIELD_TOGGLE:
					return "SHIELD_TOGGLE";
				case Button::TUNIC_TOGGLE:
					return "TUNIC_TOGGLE";
			}
			return "";
		}

		Button getInputValue(const std::string& input)
		{
			if(input == "STICK_X_UP")
				return Button::STICK_X_UP;
			if(input == "STICK_X_LEFT")
				return Button::STICK_X_LEFT;
			if(input == "STICK_X_DOWN")
				return Button::STICK_X_DOWN;
			if(input == "STICK_X_RIGHT")
				return Button::STICK_X_RIGHT;
			if(input == "A_BUTTON")
				return Button::A_BUTTON;
			if(input == "B_BUTTON")
				return Button::B_BUTTON;
			if(input == "Z_TRIG")
				return Button::Z_TRIG;
			if(input == "U_CBUTTONS")
				return Button::U_CBUTTONS;
			if(input == "L_CBUTTONS")
				return Button::L_CBUTTONS;
			if(input == "D_CBUTTONS")
				return Button::D_CBUTTONS;
			if(input == "R_CBUTTONS")
				return Button::R_CBUTTONS;
			if(input == "R_TRIG")
				return Button::R_TRIG;
			if(input == "L_TRIG")
				return Button::L_TRIG;
			if(input == "START_BUTTON")
				return Button::START_BUTTON;
			if(input == "WALK_BUTTON")
				return Button::WALK_BUTTON;
			if(input == "CENTER_CAMERA")
				return Button::CENTER_CAMERA;

			if(input == "DEBUG_MENU")
				return Button::DEBUG_MENU;
			if(input == "FAST_FORWARD")
				return Button::FAST_FORWARD;
			if(input == "OCARINA")
				return Button::OCARINA;
			if(input == "HOOKSHOT")
				return Button::HOOKSHOT;
			if(input == "BOW_ARROW")
				return Button::BOW_ARROW;
			if(input == "LENS_OF_TRUTH")
				return Button::LENS_OF_TRUTH;
			if(input == "BOOTS_TOGGLE")
				return Button::BOOTS_TOGGLE;
			if(input == "SWORD_TOGGLE")
				return Button::SWORD_TOGGLE;
			if(input == "SHIELD_TOGGLE")
				return Button::SHIELD_TOGGLE;
			if(input == "TUNIC_TOGGLE")
				return Button::TUNIC_TOGGLE;

			return (Button)0;
		}

		class Keyboard : public Controller
		{
			public:
			Keyboard() : Controller()
			{
				memset(m_lastKeyState, 0, sizeof(m_lastKeyState));
				resetBindingsImpl();
#ifndef __SWITCH__
				loadKeyBindings();
#endif
			}

			void resetBindingsImpl() // dont want to call virtual functions from constructor
			{
				m_keyBindings.clear();
				m_keyBindings[SDL_SCANCODE_W] = Button::STICK_X_UP;
				m_keyBindings[SDL_SCANCODE_A] = Button::STICK_X_LEFT;
				m_keyBindings[SDL_SCANCODE_S] = Button::STICK_X_DOWN;
				m_keyBindings[SDL_SCANCODE_D] = Button::STICK_X_RIGHT;
				m_keyBindings[SDL_SCANCODE_SPACE] = Button::A_BUTTON;
				m_keyBindings[SDL_SCANCODE_F] = Button::B_BUTTON;
				m_keyBindings[SDL_SCANCODE_O] = Button::A_BUTTON;
				m_keyBindings[SDL_SCANCODE_P] = Button::B_BUTTON;
				m_keyBindings[SDL_SCANCODE_LSHIFT] = Button::Z_TRIG;
				m_keyBindings[SDL_SCANCODE_C] = Button::Z_TRIG;
				m_keyBindings[SDL_SCANCODE_I] = Button::U_JPAD;
				m_keyBindings[SDL_SCANCODE_J] = Button::L_JPAD;
				m_keyBindings[SDL_SCANCODE_K] = Button::D_JPAD;
				m_keyBindings[SDL_SCANCODE_L] = Button::R_JPAD;
				m_keyBindings[SDL_SCANCODE_UP] = Button::U_CBUTTONS;
				m_keyBindings[SDL_SCANCODE_LEFT] = Button::L_CBUTTONS;
				m_keyBindings[SDL_SCANCODE_DOWN] = Button::D_CBUTTONS;
				m_keyBindings[SDL_SCANCODE_RIGHT] = Button::R_CBUTTONS;
				m_keyBindings[SDL_SCANCODE_X] = Button::L_TRIG;
				m_keyBindings[SDL_SCANCODE_V] = Button::R_TRIG;
				m_keyBindings[SDL_SCANCODE_RSHIFT] = Button::R_TRIG;
				m_keyBindings[SDL_SCANCODE_RETURN] = Button::START_BUTTON;

				m_keyBindings[SDL_SCANCODE_F1] = Button::BOOTS_TOGGLE;
				m_keyBindings[SDL_SCANCODE_F2] = Button::SWORD_TOGGLE;
				m_keyBindings[SDL_SCANCODE_F3] = Button::SHIELD_TOGGLE;
				m_keyBindings[SDL_SCANCODE_F4] = Button::TUNIC_TOGGLE;

				m_keyBindings[SDL_SCANCODE_F5] = Button::DEBUG_MENU;
				m_keyBindings[SDL_SCANCODE_G] = Button::FAST_FORWARD;

				m_mouseBindings[SDL_BUTTON_LEFT] = Button::B_BUTTON;
				m_mouseBindings[SDL_BUTTON_RIGHT] = Button::CENTER_CAMERA;
				m_mouseBindings[SDL_BUTTON_MIDDLE] = Button::WALK_BUTTON;
			}

			void resetBindings() override
			{
				resetBindingsImpl();
			}

			void loadKeyBindings()
			{
				try
				{
					std::ifstream ifs("keyboard1.bindings.json", std::ifstream::in);

					if(ifs.is_open())
					{
						rapidjson::IStreamWrapper isw(ifs);
						rapidjson::Document d;
						d.ParseStream(isw);

						if(d.IsObject())
						{
							for(auto itr = d.MemberBegin(); itr != d.MemberEnd(); ++itr)
							{
								if(itr->name.IsString() && itr->value.IsString())
								{
									auto key = SDL_GetScancodeFromName(itr->name.GetString());
									auto value = getInputValue(itr->value.GetString());

									if(key != SDL_SCANCODE_UNKNOWN && value)
									{
										m_keyBindings[key] = value;
									}
									else
									{
										// TODO FIX oot::log("could not bind key: \"%s\" -> \"%s\"\n", itr->value.GetString(), itr->name.GetString());
									}
								}
							}
						}
					}
				}
				catch(...)
				{
				}

				try
				{
					std::ifstream ifs("mouse1.bindings.json", std::ifstream::in);

					if(ifs.is_open())
					{
						rapidjson::IStreamWrapper isw(ifs);
						rapidjson::Document d;
						d.ParseStream(isw);

						if(d.IsObject())
						{
							for(auto itr = d.MemberBegin(); itr != d.MemberEnd(); ++itr)
							{
								if(itr->name.IsString() && itr->value.IsString())
								{
									auto key = getButtonId(itr->name.GetString());
									auto value = getInputValue(itr->value.GetString());

									if(key != 0 && value)
									{
										m_mouseBindings[key] = value;
									}
									else
									{
										// TODO FIX oot::log("could not bind key: \"%s\" -> \"%s\"\n", itr->value.GetString(), itr->name.GetString());
									}
								}
							}
						}
					}
				}
				catch(...)
				{
				}
			}

			void saveKeyBindings()
			{
				try
				{
					rapidjson::Document d;
					d.SetObject();

					rapidjson::Document::AllocatorType& allocator = d.GetAllocator();

					for(const auto i : m_keyBindings)
					{
						rapidjson::Value value(getInputName((oot::hid::Button)i.second), allocator);
						rapidjson::Value key(SDL_GetScancodeName(i.first), allocator);
						d.AddMember(key, value, allocator);
					}

					saveJson(d, "keyboard1.bindings.json");
				}
				catch(...)
				{
				}

				try
				{
					rapidjson::Document d;
					d.SetObject();

					rapidjson::Document::AllocatorType& allocator = d.GetAllocator();

					for(const auto i : m_mouseBindings)
					{
						rapidjson::Value value(getInputName((oot::hid::Button)i.second), allocator);
						rapidjson::Value key(getButtonName(i.first), allocator);
						d.AddMember(key, value, allocator);
					}

					saveJson(d, "mouse1.bindings.json");
				}
				catch(...)
				{
				}
			}

			void clearRebindMode()
			{
			}

			void resetRebinds()
			{
			}

			bool hasMouse() const
			{
				return true;
				return m_state.has_mouse;
			}

			int keyboard_buttons_down;

			int keyboard_map_scancode(SDL_Scancode scancode)
			{
				if(m_keyBindings.count(scancode))
				{
					return m_keyBindings[scancode];
				}
				return 0;
			}

			bool on_key_down(SDL_Scancode scancode)
			{
				int mapped = keyboard_map_scancode(scancode);
				keyboard_buttons_down |= mapped;
				return mapped != 0;
			}

			bool on_key_up(SDL_Scancode scancode)
			{
				int mapped = keyboard_map_scancode(scancode);
				keyboard_buttons_down &= ~mapped;
				return mapped != 0;
			}

			void on_all_keys_up()
			{
				keyboard_buttons_down = 0;
			}

			void enableMouse()
			{
				this->state().has_mouse = true;
			}

			bool canRebind(SDL_Scancode scancode, int input)
			{
				if(m_keyBindings.count(scancode) == 0)
				{
					return true;
				}

				auto replacingInput = m_keyBindings[scancode];
				u64 count = 0;

				for(auto i : m_keyBindings)
				{
					if(i.second == replacingInput)
					{
						count++;
					}
				}

				return count != 1;
			}

			bool updateRebind(int input) override
			{
				int count = 0;
				auto state = SDL_GetKeyboardState(&count);
				u64 changed = 0;

				for(int i = 0; i < std::min(MAX_KEY_STATE, count); i++)
				{
					m_lastKeyState[i] = (state[i] ^ m_lastKeyState[i]) & state[i];
				}

				for(int i = 0; i < std::min(MAX_KEY_STATE, count); i++)
				{
					if(m_lastKeyState[i] && canRebind((SDL_Scancode)i, input))
					{
						m_keyBindings[(SDL_Scancode)i] = input;
						changed++;
						saveKeyBindings();
					}
				}

#ifdef ENABLE_MOUSE
				int mouse_delta_x = 0;
				int mouse_delta_y = 0;
				u8 mouseState[MAX_BUTTONS];

				auto buttons = SDL_GetRelativeMouseState(&mouse_delta_x, &mouse_delta_y);
				this->enableMouse();
				expandMouseButtonBits(buttons, mouseState);

				for(int i = 1; i < MAX_BUTTONS; i++)
				{
					m_lastMouseState[i] = (mouseState[i] ^ m_lastMouseState[i]) & mouseState[i];
				}

				for(int i = 1; i < MAX_BUTTONS; i++)
				{
					if(m_lastMouseState[i])
					{
						m_mouseBindings[i] = input;
						changed++;
						saveKeyBindings();
					}
				}

				memcpy(m_lastMouseState, mouseState, sizeof(mouseState));
#endif

				memcpy(m_lastKeyState, state, std::min(MAX_KEY_STATE, count));
				return changed != 0;
			}

			void update()
			{
				m_state.m_walk = false;
				int count = 0;
				auto state = SDL_GetKeyboardState(&count);

				if(state[SDL_SCANCODE_F10] && (m_lastKeyState[SDL_SCANCODE_F10] ^ state[SDL_SCANCODE_F10]))
				{
					oot::config().game().fullscreen() = !oot::config().game().fullscreen();
					set_fullscreen(oot::config().game().fullscreen());
				}

				if(state[SDL_SCANCODE_ESCAPE] && (m_lastKeyState[SDL_SCANCODE_ESCAPE] ^ state[SDL_SCANCODE_ESCAPE]))
				{
					if(oot::config().game().fullscreen())
					{
						oot::config().game().fullscreen() = false;
						set_fullscreen(oot::config().game().fullscreen());
					}
				}

				if(hid::isTasPlaying())
				{
					return;
				}

				for(const auto& [scancode, input] : m_keyBindings)
				{
					if(state[scancode])
					{
						processKey(input);
					}

					if(m_lastKeyState[scancode] ^ state[scancode])
					{
						if(state[scancode])
						{
							processKeyDown(input);
						}
						else
						{
							processKeyUp(input);
						}
					}
				}

#ifdef ENABLE_MOUSE
				int mouse_delta_x = 0;
				int mouse_delta_y = 0;
				u8 mouseState[MAX_BUTTONS];

				auto buttons = SDL_GetRelativeMouseState(&mouse_delta_x, &mouse_delta_y);
				this->enableMouse();
				expandMouseButtonBits(buttons, mouseState);

				for(const auto& [btn, input] : m_mouseBindings)
				{
					if(mouseState[btn])
					{
						processKey(input);
					}

					if(m_lastMouseState[btn] ^ mouseState[btn])
					{
						if(mouseState[btn])
						{
							processKeyDown(input);
						}
						else
						{
							processKeyUp(input);
						}
					}
				}

				if(isFirstPerson())
				{
					mouse_delta_y *= -1;
				}

				m_state.mouse_x += mouse_delta_x * 4;
				m_state.mouse_y += mouse_delta_y * 4;

				m_state.r_stick_x = MAX(MIN(m_state.r_stick_x + mouse_delta_x * 4, 0x7F), -0x7F);
				m_state.r_stick_y = MAX(MIN(m_state.r_stick_y + mouse_delta_y * -4, 0x7F), -0x7F);
				memcpy(m_lastMouseState, mouseState, sizeof(mouseState));
#endif

				if(m_state.m_walk)
				{
					m_state.stick_x *= 0.25f;
					m_state.stick_y *= 0.25f;
				}

				memcpy(m_lastKeyState, state, std::min(MAX_KEY_STATE, count));
			}

			protected:
			std::unordered_map<SDL_Scancode, int> m_keyBindings;
			std::unordered_map<u8, int> m_mouseBindings;
			u8 m_lastKeyState[MAX_KEY_STATE];
			u8 m_lastMouseState[MAX_BUTTONS];
		};
	} // namespace controller

	Keyboard::Keyboard() : Driver()
	{
	}

	Keyboard::~Keyboard()
	{
	}

	void Keyboard::scan(class Controllers* controllers)
	{
		if(!size())
		{
			auto controller = std::make_shared<controller::Keyboard>();
#ifdef ENABLE_MOUSE

			controller->enableMouse();
#endif
			m_controllers.push_back(controller);
			players().attach(controller, 0);
		}
	}

	void Keyboard::update()
	{
		for(auto& keyboard : m_controllers)
		{
			((controller::Keyboard*)keyboard.get())->update();
		}
	}

	void Keyboard::resetBindings()
	{
		for(auto& keyboard : m_controllers)
		{
			keyboard->resetBindings();
		}
	}
} // namespace oot::hid
