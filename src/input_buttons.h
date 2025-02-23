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

#ifndef EP_INPUT_BUTTONS_H
#define EP_INPUT_BUTTONS_H

// Headers
#include <vector>
#include <array>
#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <algorithm>
#include <ostream>

#include <lcf/enum_tags.h>
#include "flat_map.h"
#include "keys.h"

struct Game_ConfigInput;

#if USE_SDL==1
#include "platform/sdl/axis.h"
#endif

/**
 * Input namespace.
 */
namespace Input {
	/** Input buttons list. */
	enum InputButton : uint8_t {
		UP,
		DOWN,
		LEFT,
		RIGHT,
		DECISION,
		CANCEL,
		SHIFT,
		N0,
		N1,
		N2,
		N3,
		N4,
		N5,
		N6,
		N7,
		N8,
		N9,
		PLUS,
		MINUS,
		MULTIPLY,
		DIVIDE,
		PERIOD,
		DEBUG_MENU,
		DEBUG_THROUGH,
		DEBUG_SAVE,
		DEBUG_ABORT_EVENT,
		SETTINGS_MENU,
		TOGGLE_FPS,
		TAKE_SCREENSHOT,
		SHOW_LOG,
		RESET,
		PAGE_UP,
		PAGE_DOWN,
		MOUSE_LEFT,
		MOUSE_RIGHT,
		MOUSE_MIDDLE,
		SCROLL_UP,
		SCROLL_DOWN,
		FAST_FORWARD,
		FAST_FORWARD_PLUS,
		TOGGLE_FULLSCREEN,
		TOGGLE_ZOOM,
		BUTTON_COUNT
	};

	constexpr auto kButtonNames = lcf::makeEnumTags<InputButton>(
		"UP",
		"DOWN",
		"LEFT",
		"RIGHT",
		"DECISION",
		"CANCEL",
		"SHIFT",
		"N0",
		"N1",
		"N2",
		"N3",
		"N4",
		"N5",
		"N6",
		"N7",
		"N8",
		"N9",
		"PLUS",
		"MINUS",
		"MULTIPLY",
		"DIVIDE",
		"PERIOD",
		"DEBUG_MENU",
		"DEBUG_THROUGH",
		"DEBUG_SAVE",
		"DEBUG_ABORT_EVENT",
		"SETTINGS_MENU",
		"TOGGLE_FPS",
		"TAKE_SCREENSHOT",
		"SHOW_LOG",
		"RESET",
		"PAGE_UP",
		"PAGE_DOWN",
		"MOUSE_LEFT",
		"MOUSE_RIGHT",
		"MOUSE_MIDDLE",
		"SCROLL_UP",
		"SCROLL_DOWN",
		"FAST_FORWARD",
		"FAST_FORWARD_PLUS",
		"TOGGLE_FULLSCREEN",
		"TOGGLE_ZOOM",
		"BUTTON_COUNT");

	constexpr auto kButtonHelp = lcf::makeEnumTags<InputButton>(
		"Phím lên trên",
		"Phím xuống dưới",
		"Phím sang trái",
		"Phím sang phải",
		"Phím Quyết định (Enter)",
		"Phím Huỷ bỏ (Esc)",
		"Phím Shift",
		"Số 0",
		"Số 1",
		"Số 2",
		"Số 3",
		"Số 4",
		"Số 5",
		"Số 6",
		"Số 7",
		"Số 8",
		"Số 9",
		"Phím cộng (+)",
		"Phím trừ (-)",
		"Phím nhân (*)",
		"Phím chia (/)",
		"Phím dấu chấm (.)",
		"(Test Play) Mở cửa sổ gỡ lỗi",
		"(Test Play) Xuyên tường",
		"(Test Play) Mở cửa sổ lưu trò chơi",
		"(Test Play) Hủy bỏ sự kiện đang hoạt động hiện tại",
		"Mở menu cài đặt này",
		"Hiển thị/ẩn FPS",
		"Chụp màn hình",
		"Hiển thị nhật ký bảng điều khiển trên màn hình",
		"Quay về màn hình bắt đầu",
		"Di chuyển lên một trang trong menu",
		"Di chuyển xuống một trang trong menu",
		"Phím lên trang",
		"Phím xuống trang",
		"Chuột trái",
		"Chuột phải",
		"Chuột giữa",
		"Phím cuộn lên",
		"Phím cuộn xuống",
		"Tua nhanh trò chơi (x3)",
		"Tua nhanh trò chơi hơn nữa (x10)",
		"Bật chế độ toàn màn hình",
		"Chuyển đổi mức thu phóng cửa sổ",
		"Tổng số nút");

	/**
	 * Return true if the given button is a system button.
	 * System buttons are refreshed on every physical frame
	 * and do not affect the game logic.
	 */
	constexpr bool IsSystemButton(InputButton b) {
		switch (b) {
			case TOGGLE_FPS:
			case TAKE_SCREENSHOT:
			case SHOW_LOG:
			case TOGGLE_ZOOM:
			case FAST_FORWARD:
			case FAST_FORWARD_PLUS:
				return true;
			default:
				return false;
		}
	}

	/**
	 * Protected buttons are buttons where unmapping them makes the Player unusable.
	 * @return true when the button is protected
	 */
	constexpr bool IsProtectedButton(InputButton b) {
		switch (b) {
			case UP:
			case DOWN:
			case LEFT:
			case RIGHT:
			case DECISION:
			case CANCEL:
			case SETTINGS_MENU: // Not really critical but needs a way to enter it
				return true;
			default:
				return false;
		}
	}

	namespace Direction {
		enum InputDirection : uint8_t {
			NONE = 0,
			DOWNLEFT = 1,
			DOWN = 2,
			DOWNRIGHT = 3,
			LEFT = 4,
			CENTER = 5,
			RIGHT = 6,
			UPLEFT = 7,
			UP = 8,
			UPRIGHT = 9,
			NUM_DIRECTIONS = 10,
		};

		static constexpr auto kNames = lcf::makeEnumTags<InputDirection>(
			"NONE",
			"DOWNLEFT",
			"DOWN",
			"DOWNRIGHT",
			"LEFT",
			"CENTER",
			"RIGHT",
			"UPLEFT",
			"UP",
			"UPRIGHT",
			"NUM_DIRECTIONS");
	};

	using ButtonMappingArray = FlatUniqueMultiMap<InputButton,Keys::InputKey>;

	/** A mapping for a button to a input key */
	using ButtonMapping = ButtonMappingArray::pair_type;

	inline std::ostream& operator<<(std::ostream& os, ButtonMapping bm) {
		os << "{ " << kButtonNames.tag(bm.first) << ", " << Keys::kNames.tag(bm.second) << " }";
		return os;
	}

	using DirectionMappingArray = FlatUniqueMultiMap<Direction::InputDirection, InputButton>;

	/** A mapping for a single direction to a button */
	using DirectionMapping = DirectionMappingArray::pair_type;

	inline std::ostream& operator<<(std::ostream& os, DirectionMapping dm) {
		os << "{ " << Direction::kNames.tag(dm.first) << ", " << kButtonNames.tag(dm.second) << " }";
		return os;
	}

	/** Provides platform-specific, human readable name for an input key */
	using KeyNamesArray = std::vector<std::pair<Keys::InputKey, std::string>>;

	/** Returns default button mappings */
	ButtonMappingArray GetDefaultButtonMappings();

	/** Returns platform-specific, human readable name for an input key */
	KeyNamesArray GetInputKeyNames();

	/** Used to declare which config options are available */
	void GetSupportedConfig(Game_ConfigInput& cfg);

#if USE_SDL==1
	SdlAxis GetSdlAxis();
#endif
}

#endif
