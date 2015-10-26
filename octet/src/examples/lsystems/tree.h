namespace octet {
    class tree : public resource {

        dynarray<char> variables;
        dynarray<char> constants;
        dynarray<char> axiom;
        hash_map<char, dynarray<char>> rules;

        // store data from text file in variables
        void read_lsystem_data(dynarray<uint8_t> file_content) {

            dynarray<uint8_t> clean_data;
            for each(uint8_t c in file_content) {
                if (c != ' ' && c != '\n' && c != '\r') {
                    clean_data.push_back(c);
                }
            }

            int cursor = 0;
            // get variables
            for (; cursor < clean_data.size(); ++cursor) {
                char current_char = clean_data[cursor];
                if (current_char == ';') {
                    break;
                } else if (current_char == ',') {
                    continue;
                } else {
                    variables.push_back(current_char);
                }
            }
            ++cursor;

            // get constants
            for (; cursor < clean_data.size(); ++cursor) {
                char current_char = clean_data[cursor];
                if (current_char == ';') {
                    break;
                } else if (current_char == ',') {
                    continue;
                } else {
                    constants.push_back(current_char);
                }
            }
            ++cursor;

            // get axiom
            for (; cursor < clean_data.size(); ++cursor) {
                char current_char = clean_data[cursor];
                if (current_char == ';') {
                    break;
                } else {
                    axiom.push_back(current_char);
                }
            }
            ++cursor;

            // get rules
            while (cursor < clean_data.size()) {
                char key = clean_data[cursor];
                cursor += 3;

                for (; cursor < clean_data.size(); ++cursor) {
                    char current_char = clean_data[cursor];
                    if (current_char == ';') {
                        break;
                    } else {
                        rules[key].push_back(current_char);
                    }
                }
                ++cursor;
            }
        }

    public:

        void read_file(string name) {
            dynarray<uint8_t> file_content;
            app_utils::get_url(file_content, name);

            read_lsystem_data(file_content);
        }

    };
}