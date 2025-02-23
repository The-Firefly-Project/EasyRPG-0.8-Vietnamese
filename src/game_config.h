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

#ifndef EP_GAME_CONFIG_H
#define EP_GAME_CONFIG_H

/**
 * This class manages global engine configuration.
 * For game specific settings see Game_ConfigGame.
 *
 * @see Game_ConfigGame
 */

#include "config_param.h"
#include "filesystem.h"
#include "options.h"
#include "input_buttons.h"
#include "utils.h"

class CmdlineParser;

enum class ScalingMode {
	/** Nearest neighbour to fit screen */
	Nearest,
	/** Like NN but only scales to integers */
	Integer,
	/** Integer followed by Bilinear downscale to fit screen */
	Bilinear,
};

enum class GameResolution {
	/** 320x240 */
	Original,
	/** 416x240 */
	Widescreen,
	/** 560x240 */
	Ultrawide
};

struct Game_ConfigPlayer {
	StringConfigParam autobattle_algo{ "", "", "", "", "" };
	StringConfigParam enemyai_algo{ "", "", "", "", "" };
	BoolConfigParam settings_autosave{ "Lưu cài đặt khi thoát", "Tự động lưu cài đặt khi thoát", "Player", "SettingsAutosave", false };
	BoolConfigParam settings_in_title{ "Hiển thị cài đặt ở màn hình bắt đầu", "Hiển thị nút cài đặt ở màn hình bắt đầu", "Player", "SettingsInTitle", false };
	BoolConfigParam settings_in_menu{ "Hiển thị cài đặt ở màn hình menu", "Hiển thị nút cài đặt ở màn hình menu", "Player", "SettingsInMenu", false };

	void Hide();
};

struct Game_ConfigVideo {
	LockedConfigParam<std::string> renderer{ "Trình kết xuất", "Công nghệ kết xuất của phần mềm", "auto" };
	BoolConfigParam vsync{ "V-Sync", "Bật chế độ Đồng bộ Thẳng đứng (V-Sync) (nên bật)", "Video", "Vsync", true };
	BoolConfigParam fullscreen{ "Toàn màn hình", "Chọn giữa chế độ toàn màn hình và cửa sổ", "Video", "Fullscreen", true };
	BoolConfigParam show_fps{ "Hiển thị FPS", "Hiển thị bộ đếm Khung hình trên giây (FPS)", "Video", "ShowFps", false };
	BoolConfigParam fps_render_window{ "Hiển thị FPS ở chế độ cửa sổ", "Hiển thị bộ đếm FPS khi ở chế độ cửa sổ", "Video", "FpsRenderWindow", false };
	RangeConfigParam<int> fps_limit{ "Giới hạn FPS", "Bật giới hạn Khung hình trên giây (FPS) (nên dùng 60)", "Video", "FpsLimit", DEFAULT_FPS, 0, 99999 };
	ConfigParam<int> window_zoom{ "Phóng to cửa sổ", "Chỉnh mức độ phóng to cửa sổ", "Video", "WindowZoom", 2 };
	EnumConfigParam<ScalingMode, 3> scaling_mode{ "Phương thức chia tỉ lệ", "Màn hình sẽ được phóng to như thế nào", "Video", "ScalingMode", ScalingMode::Nearest,
		Utils::MakeSvArray("Gần nhất", "Số nguyên", "Song tuyến tính"),
		Utils::MakeSvArray("nearest", "integer", "bilinear"),
		Utils::MakeSvArray("Phóng to theo kích cỡ màn hình (có thể in vết)", "Phóng to theo cấp số nhân của độ phân giải gốc", "Giống như Gần nhất, nhưng được làm mờ để tránh bị in vết")};
	BoolConfigParam stretch{ "Giãn", "Giãn theo chiều rộng của cửa sổ/màn hình", "Video", "Stretch", false };
	BoolConfigParam touch_ui{ "Giao diện cảm ứng", "Hiển thị giao diện dành riêng cho màn hình cảm ứng", "Video", "TouchUi", true };
	EnumConfigParam<GameResolution, 3> game_resolution{ "Độ phân giải", "Độ phân giải của trò chơi. Cần khởi động lại để cập nhật thay đổi.", "Video", "GameResolution", GameResolution::Original,
		Utils::MakeSvArray("Gốc (khuyên dùng)", "Màn hình rộng (Thử nghiệm)", "Siêu rộng (Thử nghiệm)"),
		Utils::MakeSvArray("original", "widescreen", "ultrawide"),
		Utils::MakeSvArray("Độ phân giải gốc (320x240, 4:3)", "Có thể gây trục trặc (416x240, 16:9)", "Có thể gây trục trặc (560x240, 21:9)")};

	// These are never shown and are used to restore the window to the previous position
	ConfigParam<int> window_x{ "", "", "Video", "WindowX", -1 };
	ConfigParam<int> window_y{ "", "", "Video", "WindowY", -1 };
	ConfigParam<int> window_width{ "", "", "Video", "WindowWidth", -1 };
	ConfigParam<int> window_height{ "", "", "Video", "WindowHeight", -1 };

	void Hide();
};

struct Game_ConfigAudio {
	RangeConfigParam<int> music_volume{ "Âm lượng BGM", "Âm lượng của nhạc nền", "Audio", "MusicVolume", 100, 0, 100 };
	RangeConfigParam<int> sound_volume{ "Âm lượng SFX", "Âm lượng của hoạt ảnh", "Audio", "SoundVolume", 100, 0, 100 };

	void Hide();
};

struct Game_ConfigInput {
	BoolConfigParam gamepad_swap_analog{ "Tay cầm: Hoán đổi cần analog", "Hoán đổi cần bên trái với bên phải", "Input", "GamepadSwapAnalog", false };
	BoolConfigParam gamepad_swap_dpad_with_buttons{ "Tay cầm: Hoán đổi D-Pad với phím", "Hoán đổi D-Pad với các phím ABXY", "Input", "GamepadSwapDpad", false };
	BoolConfigParam gamepad_swap_ab_and_xy{ "Tay cầm: Hoán đổi phím AB với XY", "Hoán đổi phím A và B với phím X và Y", "Input", "GamepadSwapAbxy", false };
	Input::ButtonMappingArray buttons;

	void Hide();
};

struct Game_Config {
	/** Gameplay subsystem options */
	Game_ConfigPlayer player;

	/** Video subsystem options */
	Game_ConfigVideo video;

	/** Audio subsystem options */
	Game_ConfigAudio audio;

	/** Input subsystem options */
	Game_ConfigInput input;

	/**
	 * Create an application config. This first determines the config file path if any,
	 * loads the config file, then loads command line arguments.
	 */
	static Game_Config Create(CmdlineParser& cp);

	/** @return config file path from command line args if found */
	static std::string GetConfigPath(CmdlineParser& cp);

	/**
	 * Returns the a filesystem view to the global config directory
	 */
	static FilesystemView GetGlobalConfigFilesystem();

	/**
	 * Returns a handle to the global config file for reading.
	 * The file is created if it does not exist.
	 *
	 * @return handle to the global file
	 */
	static Filesystem_Stream::InputStream GetGlobalConfigFileInput();

	/**
	 * Returns a handle to the global config file for writing.
	 * The file is created if it does not exist.
	 *
	 * @return handle to the global file
	 */
	static Filesystem_Stream::OutputStream GetGlobalConfigFileOutput();

	/**
	 * Load configuration values from a stream;
	 *
	 * @param is stream to read from.
	 * @post values of this are updated with values found in the stream.
	 */
	void LoadFromStream(Filesystem_Stream::InputStream& is);

	/**
	 * Load configuration values from a command line arguments.
	 *
	 * @param cp the command line parser to use.
	 * @post values of this are updated with values found in command line args.
	 */
	void LoadFromArgs(CmdlineParser& cp);

	/**
	 * Writes our configuration to the given stream.
	 *
	 * @param os stream to write to
	 */
	void WriteToStream(Filesystem_Stream::OutputStream& os) const;
};

#endif
