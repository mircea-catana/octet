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

        bool is_char_in_array(char c, dynarray<char> arr) {
            for (unsigned int i = 0; i < arr.size(); ++i) {
                if (arr[i] == c) {
                    return true;
                }
            }
            return false;
        }

    public:

        void read_file(string name) {
            dynarray<uint8_t> file_content;
            app_utils::get_url(file_content, name);

            read_lsystem_data(file_content);
        }

        void iterate() {
            dynarray<char> new_axiom;
            for (unsigned int i = 0; i < axiom.size(); ++i) {
                if (is_char_in_array(axiom[i], variables)) {
                    for (unsigned int j = 0; j < rules[axiom[i]].size(); ++j) {
                        new_axiom.push_back(rules[axiom[i]][j]);
                    }
                } else {
                    new_axiom.push_back(axiom[i]);
                }
            }

            axiom.resize(new_axiom.size());
            for (unsigned int i = 0; i < new_axiom.size(); ++i) {
                //printf("%c", new_axiom[i]);
                axiom[i] = new_axiom[i];
            }
        }

        dynarray<char> get_axiom() {
            return axiom;
        }

    };
}