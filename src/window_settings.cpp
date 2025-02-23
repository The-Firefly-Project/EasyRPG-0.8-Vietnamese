/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <sstream>
#include <utility>
#include "game_map.h"
#include "input.h"
#include "text.h"
#include "window_settings.h"
#include "game_config.h"
#include "input_buttons.h"
#include "keys.h"
#include "output.h"
#include "baseui.h"
#include "bitmap.h"
#include "player.h"
#include "system.h"
#include "audio.h"

class MenuItem final : public ConfigParam<StringView> {
public:
    explicit MenuItem(StringView name, StringView description, StringView value) :
		ConfigParam<StringView>(name, description, "", "", value) {
	}
};

Window_Settings::Window_Settings(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;
}

void Window_Settings::DrawOption(int index) {
	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);

	auto& option = options[index];

	bool enabled = bool(option.action);
	Font::SystemColor color = enabled ? Font::ColorDefault : Font::ColorDisabled;

	contents->TextDraw(rect, color, option.text);
	contents->TextDraw(rect, color, option.value_text, Text::AlignRight);
}

Window_Settings::StackFrame& Window_Settings::GetFrame(int n) {
	auto i = stack_index - n;
	assert(i >= 0 && i < static_cast<int>(stack.size()));
	return stack[i];
}

const Window_Settings::StackFrame& Window_Settings::GetFrame(int n) const {
	auto i = stack_index - n;
	assert(i >= 0 && i < static_cast<int>(stack.size()));
	return stack[i];
}

void Window_Settings::Push(UiMode ui, int arg) {
	SavePosition();

	++stack_index;
	assert(stack_index < static_cast<int>(stack.size()));
	stack[stack_index] = { ui, arg, 0, 0};

	Refresh();
	RestorePosition();
}

void Window_Settings::Pop() {
	SavePosition();
	--stack_index;
	assert(stack_index >= 0);

	Refresh();
	RestorePosition();
}

void Window_Settings::SavePosition() {
	auto mode = GetFrame().uimode;
	if (mode != eNone) {
		auto& mem = memory[mode - 1];
		mem.index = index;
		mem.top_row = GetTopRow();
	}
}

void Window_Settings::RestorePosition() {
	auto mode = GetFrame().uimode;
	if (mode != eNone) {
		auto& mem = memory[mode - 1];
		index = mem.index;
		SetTopRow(mem.top_row);
	}
}

Window_Settings::UiMode Window_Settings::GetMode() const {
	return GetFrame().uimode;
}

void Window_Settings::Refresh() {
	options.clear();

	switch (GetFrame().uimode) {
		case eNone:
		case eMain:
			break;
		case eInput:
			RefreshInput();
			break;
		case eVideo:
			RefreshVideo();
			break;
		case eAudio:
			RefreshAudio();
			break;
		case eEngine:
			RefreshEngine();
			break;
		case eLicense:
			RefreshLicense();
			break;
		case eInputButtonCategory:
			RefreshButtonCategory();
			break;
		case eInputListButtonsGame:
		case eInputListButtonsEngine:
		case eInputListButtonsDeveloper:
			RefreshButtonList();
			break;
		default:
			break;
	}

	SetItemMax(options.size());

	if (GetFrame().uimode == eNone || options.empty()) {
		SetIndex(-1);
	}

	CreateContents();

	contents->Clear();

	for (int i = 0; i < item_max; ++i) {
		DrawOption(i);
	}
}

void Window_Settings::UpdateHelp() {
	if (index >= 0 && index < static_cast<int>(options.size())) {
		help_window->SetText(options[index].help);
	} else {
		help_window->SetText("");
	}
}

template <typename Param, typename Action>
void Window_Settings::AddOption(const Param& param,
	Action&& action)
{
	if (!param.IsOptionVisible()) {
		return;
	}
	Option opt;
	opt.text = ToString(param.GetName());
	opt.help = ToString(param.GetDescription());
	opt.value_text = param.ValueToString();
	opt.mode = eOptionNone;
	if (!param.IsLocked()) {
		opt.action = std::forward<Action>(action);
	}
	options.push_back(std::move(opt));
}

template <typename T, typename Action>
void Window_Settings::AddOption(const RangeConfigParam<T>& param,
		Action&& action
	) {
	if (!param.IsOptionVisible()) {
		return;
	}
	Option opt;
	opt.text = ToString(param.GetName());
	opt.help = ToString(param.GetDescription());
	opt.value_text = param.ValueToString();
	opt.mode = eOptionRangeInput;
	opt.current_value = static_cast<int>(param.Get());
	opt.original_value = opt.current_value;
	opt.min_value = param.GetMin();
	opt.max_value = param.GetMax();
	if (!param.IsLocked()) {
		opt.action = std::forward<Action>(action);
	}
	options.push_back(std::move(opt));
}

template <typename T, typename Action, size_t S>
void Window_Settings::AddOption(const EnumConfigParam<T, S>& param,
			Action&& action
	) {
	if (!param.IsOptionVisible()) {
		return;
	}
	Option opt;
	opt.text = ToString(param.GetName());
	opt.help = ToString(param.GetDescription());
	opt.value_text = param.ValueToString();
	opt.mode = eOptionPicker;
	opt.current_value = static_cast<int>(param.Get());
	opt.original_value = opt.current_value;
	int idx = 0;
	for (auto& s: param.GetValues()) {
		if (param.IsValid(static_cast<T>(idx))) {
			opt.options_text.push_back(ToString(s));
			opt.options_index.push_back(idx);
		}
		++idx;
	}

	idx = 0;
	for (auto& s: param.GetDescriptions()) {
		if (param.IsValid(static_cast<T>(idx))) {
			opt.options_help.push_back(ToString(s));
		}
		++idx;
	}
	if (!param.IsLocked()) {
		opt.action = std::forward<Action>(action);
	}
	options.push_back(std::move(opt));
}

void Window_Settings::RefreshVideo() {
	auto cfg = DisplayUi->GetConfig();

	AddOption(cfg.renderer,	[](){});
	AddOption(cfg.fullscreen, [](){ DisplayUi->ToggleFullscreen(); });
	AddOption(cfg.window_zoom, [](){ DisplayUi->ToggleZoom(); });
	AddOption(cfg.vsync, [](){ DisplayUi->ToggleVsync(); });
	AddOption(cfg.fps_limit, [this](){ DisplayUi->SetFrameLimit(GetCurrentOption().current_value); });
	AddOption(cfg.show_fps, [](){ DisplayUi->ToggleShowFps(); });
	AddOption(cfg.fps_render_window, [](){ DisplayUi->ToggleShowFpsOnTitle(); });
	AddOption(cfg.stretch, []() { DisplayUi->ToggleStretch(); });
	AddOption(cfg.scaling_mode, [this](){ DisplayUi->SetScalingMode(static_cast<ScalingMode>(GetCurrentOption().current_value)); });
	AddOption(cfg.touch_ui, [](){ DisplayUi->ToggleTouchUi(); });
	AddOption(cfg.game_resolution, [this]() { DisplayUi->SetGameResolution(static_cast<GameResolution>(GetCurrentOption().current_value)); });
}

void Window_Settings::RefreshAudio() {
	auto cfg = DisplayUi->GetAudio().GetConfig();

	AddOption(cfg.music_volume, [this](){ DisplayUi->GetAudio().BGM_SetGlobalVolume(GetCurrentOption().current_value); });
	AddOption(cfg.sound_volume, [this](){ DisplayUi->GetAudio().SE_SetGlobalVolume(GetCurrentOption().current_value); });
	/*AddOption("Midi Backend", LockedConfigParam<std::string>("Unknown"), "",
			[](){},
			"Which MIDI backend to use");
	AddOption("Midi Soundfont", LockedConfigParam<std::string>("Default"), "",
			[](){},
			"Which MIDI soundfont to use");*/
}

void Window_Settings::RefreshEngine() {
	auto& cfg = Player::player_config;

	// FIXME: Remove the &cfg after moving to VS2022
	AddOption(cfg.settings_autosave, [&cfg](){ cfg.settings_autosave.Toggle(); });
	AddOption(cfg.settings_in_title, [&cfg](){ cfg.settings_in_title.Toggle(); });
	AddOption(cfg.settings_in_menu, [&cfg](){ cfg.settings_in_menu.Toggle(); });
}

void Window_Settings::RefreshLicense() {
	AddOption(MenuItem("EasyRPG Player", "Phần mềm mà bạn đang sử dụng đấy :)", "GPLv3+"), [this](){
		Push(eAbout);
	});
	AddOption(MenuItem("liblcf", "Xử lý các dự án RPG Maker 2000/2003 và EasyRPG", "MIT"), [](){});
	AddOption(MenuItem("libpng", "Đọc và ghi các tập tin hình ảnh PNG", "zlib"), [](){});
	AddOption(MenuItem("zlib", "Triển khai công việc nén được sử dụng tệp tin ZIP và PNG", "zlib"), [](){});
	AddOption(MenuItem("Pixman", "Thư viện xử lý các điểm ảnh", "MIT"), [](){});
	AddOption(MenuItem("fmtlib", "Thư viện định dạng văn bản", "BSD"), [](){});
	// No way to detect them - Used by liblcf
	AddOption(MenuItem("expat", "Trình phân tích cú pháp XML", "MIT"), [](){});
	AddOption(MenuItem("ICU", "Thư viện Unicode", "ICU"), [](){});
#if USE_SDL == 1
	AddOption(MenuItem("SDL", "Lớp trừu tượng cho đồ họa, âm thanh, đầu vào và hơn thế nữa", "LGPLv2.1+"), [](){});
#endif
#if USE_SDL == 2
	AddOption(MenuItem("SDL2", "Lớp trừu tượng cho đồ họa, âm thanh, đầu vào và hơn thế nữa", "zlib"), [](){});
#endif
#ifdef HAVE_FREETYPE
	AddOption(MenuItem("Freetype", "Thư viện phân tích phông chữ và rasterization", "Freetype"), [](){});
#endif
#ifdef HAVE_HARFBUZZ
	AddOption(MenuItem("Harfbuzz", "Công cụ định hình văn bản", "MIT"), [](){});
#endif
#ifdef SUPPORT_AUDIO
	// Always shown because the Midi synth is compiled in
	AddOption(MenuItem("FmMidi", "Trình phân tích cú pháp tệp MIDI và bộ tổng hợp FM Yamaha YM2608", "BSD"), [](){});
#ifdef HAVE_LIBMPG123
	AddOption(MenuItem("mpg123", "Giải mã lớp âm thanh MPEG 1, 2 và 3", "LGPLv2.1+"), [](){});
#endif
#ifdef HAVE_LIBSNDFILE
	AddOption(MenuItem("libsndfile", "Giải mã dữ liệu âm thanh được lấy mẫu (WAV)", "LGPLv2.1+"), [](){});
#endif
#ifdef HAVE_OGGVORBIS
	AddOption(MenuItem("ogg", "Thư viện định dạng vùng chứa Ogg", "BSD"), [](){});
	AddOption(MenuItem("vorbis", "Giải mã codec âm thanh Ogg Vorbis miễn phí", "BSD"), [](){});
#endif
#ifdef HAVE_TREMOR
	AddOption(MenuItem("tremor", "Giải mã codec âm thanh Ogg Vorbis miễn phí", "BSD"), [](){});
#endif
#ifdef HAVE_OPUS
	AddOption(MenuItem("opus", "Giải mã codec âm thanh OPUS miễn phí", "BSD"), [](){});
#endif
#ifdef HAVE_WILDMIDI
	AddOption(MenuItem("WildMidi", "Bộ tổng hợp MIDI", "LGPLv3+"), [](){});
#endif
#ifdef HAVE_FLUIDSYNTH
	AddOption(MenuItem("FluidSynth", "Bộ tổng hợp MIDI hỗ trợ SoundFont 2", "LGPLv2.1+"), [](){});
#endif
#ifdef HAVE_FLUIDLITE
	AddOption(MenuItem("FluidLite", "Bộ tổng hợp MIDI hỗ trợ SoundFont 2 (phiên bản rút gọn)", "LGPLv2.1+"), [](){});
#endif
#ifdef HAVE_XMP
	AddOption(MenuItem("xmp-lite", "Bộ tổng hợp mô-đun (MOD, S3M, XM và IT)", "MIT"), [](){});
#endif
#ifdef HAVE_LIBSPEEXDSP
	AddOption(MenuItem("speexdsp", "Bộ lấy mẫu lại âm thanh", "BSD"), [](){});
#endif
#ifdef HAVE_LIBSAMPLERATE
	AddOption(MenuItem("samplerate", "Bộ lấy mẫu lại âm thanh", "BSD"), [](){});
#endif
#ifdef WANT_DRWAV
	AddOption(MenuItem("dr_wav", "Giải mã dữ liệu âm thanh được lấy mẫu (WAV)", "MIT-0"), [](){});
#endif
#ifdef HAVE_ALSA
	AddOption(MenuItem("ALSA", "Hỗ trợ âm thanh Linux (được sử dụng để phát lại MIDI)", "LGPL2.1+"), [](){});
#endif
#endif
	AddOption(MenuItem("rang", "Hiển thị các màu sắc cho đầu ra của nhật ký", "Không"), [](){});
#ifdef _WIN32
	AddOption(MenuItem("dirent", "Giao diện trực tiếp cho Microsoft Visual Studio", "MIT"), [](){});
#endif
	AddOption(MenuItem("Baekmuk", "Họ phông chữ Hàn Quốc", "Baekmuk"), [](){});
	AddOption(MenuItem("Shinonome", "Họ phông chữ tiếng Nhật", "Public Domain"), [](){});
	AddOption(MenuItem("ttyp0", "Họ phông chữ ttyp0", "ttyp0"), [](){});
	AddOption(MenuItem("WenQuanYi", "Họ phông chữ WenQuanYi (CJK)", "GPLv2+ with FE"), [](){});
#ifdef EMSCRIPTEN
	AddOption(MenuItem("PicoJSON", "Trình phân tích cú pháp/trình tuần tự hóa JSON", "BSD"), [](){});
	AddOption(MenuItem("Teenyicons", "Biểu tượng 1px tối thiểu nhỏ", "MIT"), [](){});
#endif
}

void Window_Settings::RefreshInput() {
	Game_ConfigInput& cfg = Input::GetInputSource()->GetConfig();

	AddOption(MenuItem("Tổ hợp phím/nút", "Thay đổi tổ hợp phím", ""),
		[this]() { Push(eInputButtonCategory); });
	AddOption(cfg.gamepad_swap_ab_and_xy, [&cfg](){ cfg.gamepad_swap_ab_and_xy.Toggle(); Input::ResetKeys(); });
	AddOption(cfg.gamepad_swap_analog, [&cfg](){ cfg.gamepad_swap_analog.Toggle(); Input::ResetKeys(); });
	AddOption(cfg.gamepad_swap_dpad_with_buttons, [&cfg](){ cfg.gamepad_swap_dpad_with_buttons.Toggle(); Input::ResetKeys(); });
}

void Window_Settings::RefreshButtonCategory() {
	AddOption(MenuItem("Trò chơi", "Các phím được sử dụng bởi các trò chơi", ""),
		[this]() { Push(eInputListButtonsGame, 0); });
	AddOption(MenuItem("Phần mềm", "Các phím để truy cập vào các tính năng của phần mềm", ""),
		[this]() { Push(eInputListButtonsEngine, 1); });
	AddOption(MenuItem("Nhà phát triển", "Các phím hữu dụng cho nhà phát triển", ""),
		[this]() { Push(eInputListButtonsDeveloper, 2); });
}

void Window_Settings::RefreshButtonList() {
	auto& mappings = Input::GetInputSource()->GetButtonMappings();
	auto custom_names = Input::GetInputKeyNames();

	std::vector<Input::InputButton> buttons;
	switch (GetFrame().arg) {
		case 0:
			buttons = {	Input::UP, Input::DOWN, Input::LEFT, Input::RIGHT, Input::DECISION, Input::CANCEL, Input::SHIFT,
				Input::N0, Input::N1, Input::N2, Input::N3, Input::N4, Input::N5, Input::N6, Input::N7, Input::N8, Input::N9,
				Input::PLUS, Input::MINUS, Input::MULTIPLY, Input::DIVIDE, Input::PERIOD, Input::MOUSE_LEFT,
				Input::MOUSE_MIDDLE, Input::MOUSE_RIGHT, Input::SCROLL_UP, Input::SCROLL_DOWN };
			break;
		case 1:
			buttons = {Input::SETTINGS_MENU, Input::TOGGLE_FPS, Input::TOGGLE_FULLSCREEN, Input::TOGGLE_ZOOM,
				Input::TAKE_SCREENSHOT, Input::RESET, Input::FAST_FORWARD, Input::FAST_FORWARD_PLUS,
				Input::PAGE_UP, Input::PAGE_DOWN };
			break;
		case 2:
			buttons = {	Input::DEBUG_MENU, Input::DEBUG_THROUGH, Input::DEBUG_SAVE, Input::DEBUG_ABORT_EVENT,
				Input::SHOW_LOG };
			break;
	}

	for (auto b: buttons) {
		auto button = static_cast<Input::InputButton>(b);

		std::string name = Input::kButtonNames.tag(button);

		// Improve readability of the names
		bool first_letter = true;
		for (size_t i = 0; i < name.size(); ++i) {
			auto& ch = name[i];
			if (ch >= 'A' && ch <= 'Z') {
				if (!first_letter) {
					ch += 32;
				}
				first_letter = false;
			} else if (ch == '_') {
				ch = ' ';
				first_letter = true;
			}
		}

		auto help = Input::kButtonHelp.tag(button);
		std::string value = "";

		// Append as many buttons as fit on the screen, then add ...
		int contents_w = GetContents()->width();
		int name_size = Text::GetSize(*Font::Default(), name).width;
		int value_size = 0;

		for (auto ki = mappings.LowerBound(button); ki != mappings.end() && ki->first == button; ++ki) {
			auto custom_name = std::find_if(custom_names.begin(), custom_names.end(), [&](auto& key_pair) {
				return key_pair.first == ki->second;
			});

			std::string cur_value;
			if (custom_name != custom_names.end()) {
				cur_value = custom_name->second;
			} else {
				cur_value = Input::Keys::kNames.tag(ki->second);
			}

			int cur_value_size = Text::GetSize(*Font::Default(), cur_value + ",").width;

			if (value.empty()) {
				value = cur_value;
			} else if (name_size + value_size + cur_value_size + 24 > contents_w) {
				value += ",…";
				break;
			} else {
				value += "," + cur_value;
			}

			value_size += cur_value_size;
		}

		auto param = MenuItem(name, help, value);
		AddOption(param,
				[this, button](){
				Push(eInputButtonOption, static_cast<int>(button));
			});
	}
}
