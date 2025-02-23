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

#ifndef EP_GAME_CONFIG_GAME_H
#define EP_GAME_CONFIG_GAME_H

/**
 * This class manages game specific configuration.
 * For engine specific settings see Game_Config.
 *
 * All settings here are currently readonly (To be set in EasyRPG.ini)
 *
 * @see Game_Config
 */

#include "config_param.h"
#include "filesystem_stream.h"

class CmdlineParser;

struct Game_ConfigGame {
	// FIXME? Editing these settings through the config scene is not supported

	BoolConfigParam new_game{ "Bắt đầu trò chơi mới", "Bỏ qua màn hình bắt đầu và bắt đầu trò chơi mới trực tiếp", "Game", "NewGame", false };
	StringConfigParam engine_str{ "Phần mềm", "", "Game", "Engine", std::string() };
	BoolConfigParam fake_resolution{ "Số liệu giả", "Làm cho trò chơi chạy ở độ phân giải cao hơn (với một chút tỉ lệ thành công)", "Game", "FakeResolution", false };
	BoolConfigParam patch_dynrpg{ "DynRPG", "", "Patch", "DynRPG", false };
	BoolConfigParam patch_maniac{ "Bản vá Maniac", "", "Patch", "Maniac", false };
	BoolConfigParam patch_common_this_event{ "sự kiện chung", "Hỗ trợ \"Sự kiện này\" trong Các sự kiện chung", "Patch", "CommonThisEvent", false };
	BoolConfigParam patch_unlock_pics{ "Mở khoá hình ảnh", "Cho phép các lệnh hình ảnh trong khi thông báo được hiển thị", "Patch", "PicUnlock", false };
	BoolConfigParam patch_key_patch{ "Bản vá khóa Ineluki", "Hỗ trợ \"Bản vá khóa Ineluki\"", "Patch", "KeyPatch", false };
	BoolConfigParam patch_rpg2k3_commands{ "Lệnh sự kiện RPG2k3", "Cho phép hỗ trợ các lệnh sự kiện RPG2k3", "Patch", "RPG2k3Commands", false };

	// Command line only
	BoolConfigParam patch_support{ "Các bản vá hỗ trợ", "Khi ở trạng thái TẮT thì tất cả hỗ trợ bản vá bị tắt", "", "", true };

	// Indicators of overridden settings to disable auto detection
	bool patch_override = false;

	int engine = 0;

	/**
	 * Create a game config from the config file in the game directory, then loads command line arguments.
	 */
	static Game_ConfigGame Create(CmdlineParser& cp);

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
};

#endif
