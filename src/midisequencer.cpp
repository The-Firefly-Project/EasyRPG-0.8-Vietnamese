/*
Copyright (c) 2003-2006 yuno
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of copyright holders nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ''AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "system.h"

#include "midisequencer.h"
#include "output.h"

#include <cassert>
#include <algorithm>

using namespace std::chrono_literals;

namespace midisequencer{
    static uint_least32_t read_variable_value(void* fp, int(*fgetc)(void*), uint_least32_t* track_length, const char* errtext)
    {
        int ret = 0;
        int d;
        do{
            --*track_length;
            d = fgetc(fp);
            if(d == EOF){
                Output::Warning("Midi sequencer: {}", errtext);
            }
            ret <<= 7;
            ret |= (d & 0x7F);
        }while(d & 0x80);
        return ret;
    }

    inline bool operator<(const midi_message& a, const midi_message& b)
    {
        return a.time < b.time;
    }

    sequencer::sequencer():
        position(messages.begin())
    {
        loop_position = position;
    }
    void sequencer::clear()
    {
        messages.clear();
        long_messages.clear();
        position = messages.begin();
    }
    void sequencer::rewind()
    {
        position = messages.begin();
    }

    std::vector<midi_message>::iterator sequencer::rewind_to_loop()
    {
        position = loop_position;
        if (position != messages.begin()) {
            position--;
        }
        return loop_position;
    }

    bool sequencer::is_at_end()
    {
        return position == messages.end();
    }
    bool sequencer::load(void* fp, int(*fgetc)(void*))
    {
        bool result = false;
        clear();
        int b0 = fgetc(fp);
        int b1 = fgetc(fp);
        int b2 = fgetc(fp);
        int b3 = fgetc(fp);
        if(b0 == 0x4D && b1 == 0x54 && b2 == 0x68 && b3 == 0x64){
            load_smf(fp, fgetc);
            result = true;
        }else{
            Output::Warning("Midi sequencer: định dạng không được hỗ trợ");
        }
        if(!result){
            clear();
        }
        position = messages.begin();
        return result;
    }
    static int fpfgetc(void* fp)
    {
        return getc(static_cast<std::FILE*>(fp));
    }
    static bool is_loop_start(uint_least32_t msg) {
        // If the message matches the de facto standard MIDI loop instruction:
        // Which is a Control Change on any channel with Value 111
        uint8_t event_type = (msg & 0x0000F0) >> 4;
        uint8_t value1 = (msg & 0x00FF00) >> 8;
        return event_type == 0b1011 && value1 == 111;
    }
    bool sequencer::load(std::FILE* fp)
    {
        return load(fp, fpfgetc);
    }
    int sequencer::get_num_ports()const
    {
        int ret = 0;
        for(const auto& message: messages){
            if(ret < message.port){
                ret = message.port;
            }
        }
        return ret + 1;
    }
    std::chrono::microseconds sequencer::get_total_time()const
    {
        if(messages.empty()){
            return 0us;
        }else{
            return messages.back().time;
        }
    }
    std::string sequencer::get_title()const
    {
        for(const auto& message: messages){
            if(message.track == 0 && (message.message & 0xFF) == 0xFF){
                assert((message.message >> 8) < long_messages.size());
                const std::string& s = long_messages[message.message >> 8];
                if(s.size() > 1 && s[0] == 0x03){
                    return s.substr(1);
                }
            }
        }
        return std::string();
    }
    std::string sequencer::get_copyright()const
    {
        for(const auto& message: messages){
            if(message.track == 0 && (message.message & 0xFF) == 0xFF){
                assert((message.message >> 8) < long_messages.size());
                const std::string& s = long_messages[message.message >> 8];
                if(s.size() > 1 && s[0] == 0x02){
                    return s.substr(1);
                }
            }
        }
        return std::string();
    }
    std::string sequencer::get_song()const
    {
        std::string ret;
        for(const auto& message: messages){
            if(message.track == 0 && (message.message & 0xFF) == 0xFF){
                assert((message.message >> 8) < long_messages.size());
                const std::string& s = long_messages[message.message >> 8];
                assert(s.size() >= 1);
                if(s[0] == 0x05){
                    ret += s.substr(1);
                }
            }
        }
        return ret;
    }
    void sequencer::play(std::chrono::microseconds time, output* out)
    {
        if(position != messages.begin() && (position - 1)->time >= time){
            position = messages.begin();
        }
        if(position == messages.begin() && position != messages.end() && position->time < time){
            out->reset();
        }

        while(position != messages.end() && position->time < time){
            uint_least32_t message = position->message;
            int port = position->port;
            ++position;

            switch(message & 0xFF){
            case 0xF0:
                {
                    assert((message >> 8) < long_messages.size());
                    const std::string& s = long_messages[static_cast<int>(message >> 8)];
                    out->sysex_message(port, s.data(), s.size());
                }
                break;
            case 0xFF:
                {
                    assert((message >> 8) < long_messages.size());
                    const std::string& s = long_messages[static_cast<int>(message >> 8)];
                    assert(s.size() >= 1);
                    out->meta_event(static_cast<unsigned char>(s[0]), s.data() + 1, s.size() - 1);
                }
                break;
            default:
                out->midi_message(port, message);
                break;
            }
        }
    }

    void sequencer::set_time(std::chrono::microseconds time, output* out)
    {
        if(position != messages.begin() && (position - 1)->time >= time){
            position = messages.begin();
        }
        if(position == messages.begin() && position != messages.end() && position->time < time){
            out->reset();
        }

        if(position->time > time)
            position = messages.begin();

        while(position != messages.end() && position->time < time){
            uint_least32_t message = position->message;
            int port = position->port;
            ++position;

            switch(message & 0xFF){
                case 0xF0:
                {
                    assert((message >> 8) < long_messages.size());
                    const std::string& s = long_messages[static_cast<int>(message >> 8)];
                    out->sysex_message(port, s.data(), s.size());
                }
                    break;
                case 0xFF:
                {
                    assert((message >> 8) < long_messages.size());
                    const std::string& s = long_messages[static_cast<int>(message >> 8)];
                    assert(s.size() >= 1);
                    out->meta_event(static_cast<unsigned char>(s[0]), s.data() + 1, s.size() - 1);
                }
                    break;
                default:

                    if((message & 0xF0) == 0x80 || (message & 0xF0) == 0x90)
                    {
                        out->meta_event(META_EVENT_ALL_NOTE_OFF, NULL, 0);
                        break;
                    }

                    out->midi_message(port, message);
                    break;
            }
        }
    }

    std::chrono::microseconds sequencer::get_start_skipping_silence() {
        std::vector<midi_message> m;

        for (auto& msg: messages) {
            // If we find Loop Start before the first NoteOn, just start there
            if (is_loop_start(msg.message)) {
                std::chrono::microseconds time = msg.time;
                // RPG_RT always rewinds "a little"
                // This amount is based on the tempo, and this 2100000 divisor
                // I determined experimentally.
                time = std::max(0us, std::chrono::microseconds(static_cast<int>(time.count() - (msg.tempo / 2.1f))));
                return time;
            } else if ((msg.message & 0xFF) == 0xF0) {
                // SysEx message. RPG_RT doesn't skip silence if there's a SysEx
                // message in the beginning, so neither should we...
                return 0us;
            } else if ((msg.message & 0xF0) == 0x90) {
                // NoteOn -- found the first note!
                std::chrono::microseconds time = msg.time;
                // RPG_RT always rewinds "a little"
                // This amount is based on the tempo, and this 2100000 divisor
                // I determined experimentally.
                time = std::max(0us, std::chrono::microseconds(static_cast<int>(time.count() - (msg.tempo / 2.1f))));
                return time;
            }
        }
        return 0us;
    }

    void sequencer::load_smf(void* fp, int(*fgetc)(void*))
    {
        if(fgetc(fp) != 0
        || fgetc(fp) != 0
        || fgetc(fp) != 0
        || fgetc(fp) != 6
        || fgetc(fp) != 0){
            Output::Warning("Midi sequencer: tiêu đề tập tin không hợp lệ");
        }
        int format = fgetc(fp);
        if(format != 0 && format != 1){
            Output::Warning("Midi sequencer: loại định dạng không được hỗ trợ");
        }
        int t0 = fgetc(fp);
        int t1 = fgetc(fp);
        unsigned num_tracks = (t0 << 8) | t1;
        int d0 = fgetc(fp);
        int d1 = fgetc(fp);
        division = (d0 << 8) | d1;
        for(unsigned track = 0; track < num_tracks; ++track){
            if(fgetc(fp) != 0x4D || fgetc(fp) != 0x54 || fgetc(fp) != 0x72 || fgetc(fp) != 0x6B){
                Output::Warning("Midi sequencer: tiêu đề bài nhạc không hợp lệ");
            }
            int l0 = fgetc(fp);
            int l1 = fgetc(fp);
            int l2 = fgetc(fp);
            int l3 = fgetc(fp);
            uint_least32_t track_length = (static_cast<uint_least32_t>(l0) << 24)
                                        | (static_cast<uint_least32_t>(l1) << 16)
                                        | (l2 << 8)
                                        | l3;
            int running_status = 0;
            double time_offset = 0;
            uint_least32_t time = 0;
            midi_message msg;
            msg.port = 0;
            msg.track = track;
            for(;;){
                if(track_length < 4){
                    Output::Warning("Midi sequencer: EOF không mong muốn (track_length)");
                }
                uint_least32_t delta = read_variable_value(fp, fgetc, &track_length, "unexpected EOF (deltatime)");
                time += delta;
                if(division & 0x8000){
                    int fps = ~(division >> 8) + 1;
                    int frames = division & 0xFF;
                    msg.time = std::chrono::microseconds(static_cast<int>(time / (frames * fps) + time_offset));
                }else{
                    msg.time = std::chrono::microseconds(time);
                }
                int param = fgetc(fp);
                --track_length;
                switch(param){
                case 0xF0:
                    {
                        int n = read_variable_value(fp, fgetc, &track_length, "unexpected EOF (sysex length)");
                        std::string s(n + 1, '\0');
                        s[0] = static_cast<char>(0xF0);
                        for(int i = 1; i <= n; ++i){
                            s[i] = static_cast<char>(fgetc(fp));
                        }
                        if(s[n] != '\xF7'){
                            Output::Debug("Midi sequencer: missing sysex terminator");
                            s += static_cast<char>(0xF7);
                        }
                        track_length -= n;
                        msg.message = 0xF0 | (long_messages.size() << 8);
                        messages.push_back(msg);
                        long_messages.push_back(s);
                    }
                    break;
                case 0xF7:
                    /* unsupported */
                    /*
                    {
                        int n = read_variable_value(fp, fgetc, &track_length, "unexpected EOF (sysex-F7 length)");
                        std::string s(n, '\0');
                        for(int i = 0; i < n; ++i){
                            s[i] = fgetc(fp);
                        }
                        track_length -= n;
                        msg.message = 0xF0 | (long_messages.size() << 8);
                        messages.push_back(msg);
                        long_messages.push_back(s);
                    }
                    */
                    break;
                case 0xFF:
                    {
                        int type = fgetc(fp);
                        --track_length;
                        int n = read_variable_value(fp, fgetc, &track_length, "unexpected EOF (metaevent)");
                        std::string s(n + 1, '\0');
                        s[0] = static_cast<char>(type);
                        for(int i = 1; i <= n; ++i){
                            s[i] = static_cast<char>(fgetc(fp));
                        }
                        track_length -= n;
                        msg.message = 0xFF | (long_messages.size() << 8);
                        messages.push_back(msg);
                        long_messages.push_back(s);
                        switch(type){
                        case 0x21:
                            if(n == 1){
                                msg.port = static_cast<unsigned char>(s[1]);
                            }
                            break;
                        case 0x2F:
                            goto next_track;
                        case 0x54:
                            if(n != 5){
                                Output::Warning("Midi sequencer: độ dài metaevent bù đắp SMTPE không hợp lệ");
                            }
                            if(msg.time == 0us && (division & 0x8000)){
                                int hour = static_cast<unsigned char>(s[1]);
                                int min = static_cast<unsigned char>(s[2]);
                                int sec = static_cast<unsigned char>(s[3]);
                                int frame = static_cast<unsigned char>(s[4]);
                                int subframe = static_cast<unsigned char>(s[5]);
                                double fps;
                                switch(hour >> 5){
                                default: // line added by nextvolume (2015-02-25)
                                case 0:
                                    fps = 24;
                                    break;
                                case 1:
                                    fps = 25;
                                    break;
                                case 2:
                                    fps = 29.97;
                                    break;
                                case 3:
                                    fps = 30;
                                    break;
                                }
                                time = 0;
                                time_offset = (hour & 0x1F) * 3600 + min * 60 + sec + (frame + subframe / 100.0) * fps;
                            }
                            break;
                        }
                    }
                    break;
                default:
                    if(param & 0x80){
                        running_status = param;
                        param = fgetc(fp);
                        --track_length;
                    }
                    switch(running_status & 0xF0){
                    case 0xC0:
                    case 0xD0:
                        msg.message = running_status | (param << 8);
                        break;
                    case 0x80:
                    case 0x90:
                    case 0xA0:
                    case 0xB0:
                    case 0xE0:
                        msg.message = running_status | (param << 8) | (fgetc(fp) << 16);
                        --track_length;
                        break;
                    default:
                        Output::Warning("Midi sequencer: tin nhắn midi không hợp lệ");
                    }
                    messages.push_back(msg);
                    break;
                }
            }
        next_track:
            while(track_length > 0){
                if(fgetc(fp) == EOF){
                    Output::Warning("Midi sequencer: EOF không mong muốn (tailer padding)");
                }
                --track_length;
            }
        }
        std::stable_sort(messages.begin(), messages.end());
        loop_position = messages.begin();
        if(!(division & 0x8000)){
            uint_least32_t tempo = 500000;
            std::chrono::microseconds time_offset = 0us;
            std::chrono::microseconds base = 0us;
            for(auto i = messages.begin(); i != messages.end(); ++i){
                std::chrono::microseconds org_time = i->time;
                i->time = std::chrono::microseconds(static_cast<int>(static_cast<double>((i->time.count() - base.count())) * tempo / division + time_offset.count()));
                if((i->message & 0xFF) == 0xFF){
                    assert((i->message >> 8) < long_messages.size());
                    const std::string& s = long_messages[i->message >> 8];
                    if(s.size() == 4 && s[0] == 0x51){
                        tempo = (static_cast<uint_least32_t>(static_cast<unsigned char>(s[1])) << 16)
                              | (static_cast<unsigned char>(s[2]) << 8)
                              | static_cast<unsigned char>(s[3]);
                        base = org_time;
                        time_offset = i->time;
                    }
                }
                i->tempo = tempo;
                if (is_loop_start(i->message)) {
                    // Loop backwards through the messages to find the first message with the same
                    // timestamp as the loop message
                    for (auto j = i; j != messages.begin() && j->time >= i->time; j--) {
                        loop_position = j;
                    }
                }
            }
        }
    }

    uint32_t sequencer::get_division() const {
        return division;
    }
}
