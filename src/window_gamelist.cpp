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
#include <iomanip>
#include <sstream>
#include "window_gamelist.h"
#include "game_party.h"
#include "bitmap.h"
#include "font.h"

Window_GameList::Window_GameList(int ix, int iy, int iwidth, int iheight) :
	Window_Selectable(ix, iy, iwidth, iheight) {
	column_max = 1;
}

bool Window_GameList::Refresh(FilesystemView filesystem_base, bool show_dotdot) {
	base_fs = filesystem_base;
	if (!base_fs) {
		return false;
	}

	game_directories.clear();

	this->show_dotdot = show_dotdot;

	auto files = base_fs.ListDirectory();

	// Find valid game diectories
	for (auto& dir : *files) {
		assert(!dir.second.name.empty() && "VFS BUG: Empty filename in the folder");

#ifdef EMSCRIPTEN
		if (dir.second.name == "Save") {
			continue;
		}
#endif

		if (StringView(dir.second.name).ends_with(".save")) {
			continue;
		}
		if (dir.second.type == DirectoryTree::FileType::Regular) {
			auto sv = StringView(dir.second.name);
			if (sv.ends_with(".zip") || sv.ends_with(".easyrpg")) {
				game_directories.emplace_back(dir.second.name);
			}
		} else if (dir.second.type == DirectoryTree::FileType::Directory) {
			game_directories.emplace_back(dir.second.name);
		}
	}

	// Sort game list in place
	std::sort(game_directories.begin(), game_directories.end(),
			  [](const std::string& s, const std::string& s2) {
				  return strcmp(Utils::LowerCase(s).c_str(), Utils::LowerCase(s2).c_str()) <= 0;
			  });

	if (show_dotdot) {
		game_directories.insert(game_directories.begin(), "..");
	}

	if (HasValidEntry()) {
		item_max = game_directories.size();

		CreateContents();

		contents->Clear();

		for (int i = 0; i < item_max; ++i) {
			DrawItem(i);
		}
	}
	else {
		item_max = 1;

		SetContents(Bitmap::Create(width - 16, height - 16));

		if (show_dotdot) {
			DrawItem(0);
		}

		DrawErrorText(show_dotdot);
	}

	return true;
}

void Window_GameList::DrawItem(int index) {
	Rect rect = GetItemRect(index);
	contents->ClearRect(rect);

	std::string text;

	if (HasValidEntry()) {
		text = game_directories[index];
	}

	contents->TextDraw(rect.x, rect.y, Font::ColorDefault, game_directories[index]);
}

void Window_GameList::DrawErrorText(bool show_dotdot) {
	std::vector<std::string> error_msg = {
#ifdef EMSCRIPTEN
		"Bạn có nhập sai URL không?",
		"",
		"Nếu bạn nghĩ đây là lỗi, hãy liên hệ cho chủ sở",
		"hửu của website này.",
		"",
		"Thông tin thêm: không thể tìm thấy tệp index.json.",
		"",
		"Hãy chắc chắn qua công cụ phát triển trên trình",
		"duyệt của bạn rằng các tệp tin đã ở đúng vị trí.",
#else
		"Với EasyRPG Player bạn có thể chơi các trò chơi",
		"được tạo bằng RPG Maker 2000 và RPG Maker 2003.",
		"",
		"Các trò chơi này có một tệp tin RPG_RT.ldb và",
		"chúng có thể được giải nén hoặc ở tệp nén ZIP.",
		"",
		"Các nền tảng mới hơn như RPG Maker XP, VX, MV và",
		"MZ không được hỗ trợ."
#endif
	};

	int y = (show_dotdot ? 4 + 14 : 0);

#ifdef EMSCRIPTEN
	contents->TextDraw(0, y, Font::ColorKnockout, "Không thể tìm thấy trò chơi.");
#else
	contents->TextDraw(0, y, Font::ColorKnockout, "Không có trò chơi nào ở thư mục hiện tại.");
#endif

	y += 14 * 2;
	for (size_t i = 0; i < error_msg.size(); ++i) {
		contents->TextDraw(0, y + 14 * i, Font::ColorCritical, error_msg[i]);
	}
}

bool Window_GameList::HasValidEntry() {
	size_t minval = show_dotdot ? 1 : 0;
	return game_directories.size() > minval;
}

std::pair<FilesystemView, std::string> Window_GameList::GetGameFilesystem() const {
	return { base_fs.Create(game_directories[GetIndex()]), game_directories[GetIndex()] };
}
