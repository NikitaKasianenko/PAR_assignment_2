#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <map>
#include <iostream>
#include <stack>
using namespace std;


class TextRedactor {
public:
    TextRedactor() {
        bufferSize = 256;
        initialRowCount = 10;
        nrow = 0;
        ncol = 0;
        array = nullptr;
        initialize_array();
    }

    ~TextRedactor() {
        freeArray();
    }

    char** getArray() const {
        return array;
    }

    void initialize_array() {
        array = (char**)malloc(initialRowCount * sizeof(char*));
        if (array == nullptr) {
            printf("Memory allocation failed.");
            exit(1);
        }
        for (int i = 0; i < initialRowCount; i++) {
            array[i] = (char*)malloc(bufferSize * sizeof(char));
            if (array[i] == nullptr) {
                printf("Memory allocation failed.");
                exit(1);
            }
            array[i][0] = '\0';
        }
    }

    void newBuffer(size_t* bufferSize) {
        *bufferSize *= 2;
    }

    void freeArray() {
        if (array) {
            for (int i = 0; i < initialRowCount; i++) {
                free(array[i]);
            }
            free(array);
            array = nullptr;
        }
    }

    void reallocate_rows() {
        initialRowCount *= 2;
        char** temp = (char**)realloc(array, initialRowCount * sizeof(char*));
        if (temp == nullptr) {
            printf("Memory allocation failed.");
            exit(1);
        }
        array = temp;
        for (int i = nrow; i < initialRowCount; i++) {
            array[i] = (char*)malloc(bufferSize * sizeof(char));
            if (array[i] == nullptr) {
                printf("Memory allocation failed.");
                exit(1);
            }
            array[i][0] = '\0';
        }
    }

    char* user_input(size_t* bufferSize) {
        char* input = (char*)malloc(*bufferSize * sizeof(char));
        if (input == nullptr) {
            printf("Memory allocation failed.");
            exit(1);
        }

        int length = 0;
        int symbol;

        while ((symbol = getchar()) != '\n' && symbol != EOF) {
            if (length >= *bufferSize - 1) {
                *bufferSize *= 2;
                input = (char*)realloc(input, *bufferSize * sizeof(char));
                if (input == nullptr) {
                    printf("Memory allocation failed.");
                    exit(1);
                }
            }
            input[length++] = symbol;
        }
        input[length] = '\0';

        return input;
    }

    void sscan_user_input(int parametr) {
        char* input = nullptr;

        if (parametr == 1) {
            while (true) {
                printf("Choose line, index and number of symbols: ");
                input = user_input(&bufferSize);
                if (sscanf(input, "%d %d %d", &currow, &curcol, &amount) == 3) {
                    if (currow >= 0 && currow <= nrow && curcol >= 0 && curcol < (int)strlen(array[currow]) &&
                        amount >= 0 && amount + curcol <= (int)strlen(array[currow])) {
                        break;
                    }
                }

                free(input);
                input = nullptr;
                printf("Choose correct index and amount of symbols separated by space in format 'x y z'\n");
            }
        }

        else if (parametr == 0) {

            while (true) {
                printf("Choose line and index: ");
                input = user_input(&bufferSize);
                if (sscanf(input, "%d %d", &currow, &curcol) == 2) {
                    if (currow >= 0 && currow <= nrow && curcol >= 0 && curcol <= (int)strlen(array[currow])) {
                        break;
                    }
                }

                free(input);
                input = nullptr;
                printf("Choose correct index separated by space in format 'x y'\n");
            }

        }



    }

    void append_text() {
        printf("Enter text to append: ");
        char* input = user_input(&bufferSize);

        if (ncol + strlen(input) >= bufferSize - 1) {
            newBuffer(&bufferSize);
            array[nrow] = (char*)realloc(array[nrow], bufferSize * sizeof(char));
            if (array[nrow] == nullptr) {
                printf("Memory allocation failed.");
                exit(1);
            }
        }

        strcat(array[nrow], input);
        ncol += strlen(input);

        char* undo_info = (char*)malloc((strlen(input) + 4) * sizeof(char));
        sprintf(undo_info, "1\t%s", input);
        undoStack.push(undo_info);

        while (!redoStack.empty()) {
            free(redoStack.top());
            redoStack.pop();
        }

        free(input);
    }

    void new_line() {
        nrow++;
        ncol = 0;
        if (nrow >= initialRowCount) {
            reallocate_rows();
        }

        printf("New line is started\n");

        char* undo_info = (char*)malloc(4 * sizeof(char));
        sprintf(undo_info, "2");
        undoStack.push(undo_info);

        while (!redoStack.empty()) {
            free(redoStack.top());
            redoStack.pop();
        }
    }

    void write_in_file() {
        printf("Enter the file name for saving: ");
        char* input = user_input(&bufferSize);
        char path[256];
        sprintf(path, "C:\\Windows\\Temp\\%s.txt", input);
        file = fopen(path, "w");

        if (file == nullptr) {
            printf("Can't open file\n");
            free(input);
            return;
        }

        for (int i = 0; i <= nrow; i++) {
            fprintf(file, "%s\n", array[i]);
        }
        printf("Successful\n");
        fclose(file);
        file = nullptr;
        free(input);
    }

    void read_from_file() {
        nrow = 0;
        printf("Enter path to file: ");
        char* input = user_input(&bufferSize);

        char mystring[1000];
        file = fopen(input, "r");
        free(input);
        if (file == nullptr) {
            printf("Error opening file\n");
            return;
        }

        while (fgets(mystring, sizeof(mystring), file) != nullptr) {
            if (nrow >= initialRowCount) {
                reallocate_rows();
            }
            strncpy(array[nrow], mystring, strlen(mystring));
            array[nrow][strlen(mystring) - 1] = '\0';
            nrow++;
        }

        fclose(file);
        file = nullptr;
    }

    void print() {
        for (int i = 0; i <= nrow; i++) {
            printf("%s\n", array[i]);
        }
    }

    void insert_text() {
        char* input = nullptr;

        printf("Enter text to insert: ");
        input = user_input(&bufferSize);

        int text_length = strlen(input);
        if (text_length + strlen(array[currow]) >= bufferSize) {
            newBuffer(&bufferSize);
            array[currow] = (char*)realloc(array[currow], bufferSize * sizeof(char));
            if (array[currow] == nullptr) {
                printf("Memory allocation failed");
                free(input);
                exit(1);
            }
        }

        for (int i = strlen(array[currow]); i >= curcol; i--) {
            array[currow][i + text_length] = array[currow][i];
        }

        for (int i = 0; i < text_length; i++) {
            array[currow][curcol + i] = input[i];
        }

        char* undo_info = (char*)malloc((strlen(input) + 50) * sizeof(char));
        sprintf(undo_info, "6\t%d\t%d\t%s", currow, curcol, input);
        undoStack.push(undo_info);

        while (!redoStack.empty()) {
            free(redoStack.top());
            redoStack.pop();
        }

        free(input);
    }

    void delete_text() {
        char* input = nullptr;

        int new_length = (int)strlen(array[currow]) - amount;
        char* deleted_text = (char*)malloc((amount + 1) * sizeof(char));

        strncpy(deleted_text, &array[currow][curcol], amount);
        for (int i = curcol; i < new_length; ++i) {
            array[currow][i] = array[currow][i + amount];
        }
        array[currow][new_length] = '\0';
        deleted_text[amount] = '\0';
        ncol -= amount;

        char* undo_info = (char*)malloc((amount + 20) * sizeof(char));
        sprintf(undo_info, "8\t%d\t%d\t%s", currow, curcol, deleted_text);
        undoStack.push(undo_info);

        while (!redoStack.empty()) {
            free(redoStack.top());
            redoStack.pop();
        }

        free(deleted_text);
    }

    void search() {
        printf("Enter text to search: ");
        char* input = user_input(&bufferSize);
        char* to_search = input;
        bool found = false;
        char* name = nullptr;

        for (int i = 0; i <= nrow; i++) {
            name = array[i];
            while ((name = strstr(name, to_search)) != nullptr) {
                printf("Substring found at index: %d %d\n", i, (int)(name - array[i]));
                found = true;
                name++;
            }
        }

        if (!found) {
            printf("Substring not found\n");
        }
        free(input);
    }

    void cut() {
        copy();
        delete_text();

        while (!redoStack.empty()) {
            free(redoStack.top());
            redoStack.pop();
        }
    }

    void insert_rp() {
        char* input = nullptr;

        char* original_text = _strdup(array[currow]);

        printf("Enter text: ");
        input = user_input(&bufferSize);

        int text_length = strlen(input);

        if (text_length + strlen(array[currow]) >= bufferSize) {
            newBuffer(&bufferSize);
            array[currow] = (char*)realloc(array[currow], bufferSize * sizeof(char));
            if (array[currow] == nullptr) {
                printf("Memory allocation failed");
                free(input);
                exit(1);
            }
        }

        for (int i = 0; i < text_length; i++) {
            array[currow][curcol + i] = input[i];
        }
        array[currow][curcol + text_length] = '\0';

        char* undo_info = (char*)malloc((strlen(original_text) + 50) * sizeof(char));
        sprintf(undo_info, "16\t%d\t%d\t%s", currow, curcol, original_text);
        undoStack.push(undo_info);

        free(original_text);
        free(input);

        while (!redoStack.empty()) {
            free(redoStack.top());
            redoStack.pop();
        }
    }

    void copy() {
        char* input = nullptr;

        char* text = (char*)malloc((amount + 1) * sizeof(char));
        strncpy(text, &array[currow][curcol], amount);
        text[amount] = '\0';

        bufferStack.push(text);
        char* undo_info = (char*)malloc((strnlen(text, bufferSize)) * sizeof(char));
        sprintf(undo_info, "13\t%s", text);
        undoStack.push(undo_info);

        while (!redoStack.empty()) {
            free(redoStack.top());
            redoStack.pop();
        }
    }

    void paste() {
        if (bufferStack.empty()) {
            printf("Nothing to paste.\n");
            return;
        }

        char* input = nullptr;

        input = bufferStack.top();
        int text_length = strlen(input);

        if (text_length + strlen(array[currow]) >= bufferSize) {
            newBuffer(&bufferSize);
            array[currow] = (char*)realloc(array[currow], bufferSize * sizeof(char));
            if (array[currow] == nullptr) {
                printf("Memory allocation failed");
                exit(1);
            }
        }

        for (int i = strlen(array[currow]); i >= curcol; i--) {
            array[currow][i + text_length] = array[currow][i];
        }

        for (int i = 0; i < text_length; i++) {
            array[currow][curcol + i] = input[i];
        }

        ncol += text_length;
        char* undo_info = (char*)malloc((strlen(input) + 50) * sizeof(char));
        sprintf(undo_info, "12\t%d\t%d\t%s", currow, curcol, input);
        undoStack.push(undo_info);

        while (!redoStack.empty()) {
            free(redoStack.top());
            redoStack.pop();
        }
    }

    void help() {
        printf("You open a text redactor with these functions:\n");
        printf("Command-'1': Append text \n");
        printf("Command-'2': Start the new line \n");
        printf("Command-'3': Write your text in file\n");
        printf("Command-'4': Read text from file\n");
        printf("Command-'5': Print the current text to console\n");
        printf("Command-'6': Insert the text by line and symbol index\n");
        printf("Command-'7': Search\n");
        printf("Command-'8': Delete\n");
        printf("Command-'9': Cut\n");
        printf("Command-'10': Exit\n");
        printf("Command-'11': Clear console\n");
        printf("Command-'12': Paste\n");
        printf("Command-'13': Copy\n");
        printf("Command-'14': Undo\n");
        printf("Command-'15': Redo\n");
        printf("Command-'16': Insert with replacement\n\n");
    }

    void undo() {
        if (undoStack.empty()) {
            printf("No actions to undo.\n");
            return;
        }

        char* last_action = undoStack.top();
        char* last_action_copy = _strdup(last_action);

        redoStack.push(last_action);
        undoStack.pop();

        int action_type;
        sscanf(last_action, "%d", &action_type);

        if (action_type == 1) {
            // Undo append
            char* text = strchr(last_action_copy, '\t') + 1;
            size_t len = strlen(text);
            ncol -= len;
            array[nrow][ncol] = '\0';
        }
        else if (action_type == 2) {
            // Undo new line
            nrow--;
            ncol = strlen(array[nrow]);
        }
        else if (action_type == 6) {
            // Undo insert
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* text = _strdup(token);

            int text_length = strlen(text);
            int new_length = (int)strlen(array[currow]) - text_length;

            for (int i = curcol; i <= new_length; ++i) {
                array[currow][i] = array[currow][i + text_length];
            }
            array[currow][new_length] = '\0';
            ncol = strnlen(array[currow], bufferSize);
        }
        else if (action_type == 12) {
            // Undo paste
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* text = _strdup(token);

            int text_length = strlen(text);
            int new_length = (int)strlen(array[currow]) - text_length;

            for (int i = curcol; i <= new_length; ++i) {
                array[currow][i] = array[currow][i + text_length];
            }
            array[currow][new_length] = '\0';
            ncol = strnlen(array[currow], bufferSize);

            free(text);
        }
        else if (action_type == 8) {
            // Undo delete
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* deleted_text = _strdup(token);

            int deleted_text_length = strlen(deleted_text);
            int new_length = (int)strlen(array[currow]) + deleted_text_length;

            if (new_length >= bufferSize) {
                newBuffer(&bufferSize);
                array[currow] = (char*)realloc(array[currow], bufferSize * sizeof(char));
                if (array[currow] == nullptr) {
                    printf("Memory allocation failed");
                    exit(1);
                }
            }

            for (int i = strlen(array[currow]); i >= curcol; i--) {
                array[currow][i + deleted_text_length] = array[currow][i];
            }

            for (int i = 0; i < deleted_text_length; i++) {
                array[currow][curcol + i] = deleted_text[i];
            }
            ncol = strnlen(array[currow], bufferSize);

            free(deleted_text);
        }
        else if (action_type == 13) {
            // Undo copy
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            if (token != nullptr) {
                char* text = _strdup(token);
                if (!bufferStack.empty()) {
                    bufferUndoStack.push(bufferStack.top());
                    bufferStack.pop();
                }
                free(text);
            }
        }
        else if (action_type == 9) {
            // Undo cut
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* cut_text = _strdup(token);

            int cut_text_length = strlen(cut_text);
            int new_length = (int)strlen(array[currow]) + cut_text_length;

            if (new_length >= bufferSize) {
                newBuffer(&bufferSize);
                array[currow] = (char*)realloc(array[currow], bufferSize * sizeof(char));
                if (array[currow] == nullptr) {
                    printf("Memory allocation failed");
                    exit(1);
                }
            }

            for (int i = strlen(array[currow]); i >= curcol; i--) {
                array[currow][i + cut_text_length] = array[currow][i];
            }

            for (int i = 0; i < cut_text_length; i++) {
                array[currow][curcol + i] = cut_text[i];
            }
            ncol = strnlen(array[currow], bufferSize);

            free(cut_text);

            if (!bufferStack.empty()) {
                bufferUndoStack.push(bufferStack.top());
                bufferStack.pop();
            }
        }
        if (action_type == 16) {
            // Undo insert_rp
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* original_text = _strdup(token);

            char* text = _strdup(array[currow]);
            strcpy(array[currow], original_text);

            char* undo_info = (char*)malloc((strlen(original_text) + 50) * sizeof(char));
            sprintf(undo_info, "16\t%d\t%d\t%s", currow, curcol, text);
            redoStack.push(undo_info);

            free(original_text);
        }

        free(last_action_copy);
    }

    void redo() {
        if (redoStack.empty()) {
            printf("No actions to redo.\n");
            return;
        }

        char* last_action = redoStack.top();
        redoStack.pop();
        undoStack.push(last_action);
        char* last_action_copy = _strdup(last_action);

        int action_type;
        sscanf(last_action, "%d", &action_type);

        if (action_type == 1) {
            // Redo append
            char* text = strchr(last_action, '\t') + 1;
            strcat(array[nrow], text);
            ncol += strlen(text);
        }
        else if (action_type == 2) {
            // Redo new line
            nrow++;
            ncol = 0;
        }
        else if (action_type == 6) {
            // Redo insert
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* text = _strdup(token);

            int text_length = strlen(text);
            if (text_length + strlen(array[currow]) >= bufferSize) {
                newBuffer(&bufferSize);
                array[currow] = (char*)realloc(array[currow], bufferSize * sizeof(char));
                if (array[currow] == nullptr) {
                    printf("Memory allocation failed");
                    free(text);
                    exit(1);
                }
            }

            for (int i = strlen(array[currow]); i >= curcol; i--) {
                array[currow][i + text_length] = array[currow][i];
            }
            for (int i = 0; i < text_length; i++) {
                array[currow][curcol + i] = text[i];
            }

            free(text);
        }
        else if (action_type == 12) {
            // Redo paste
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* text = _strdup(token);

            int text_length = strlen(text);
            if (text_length + strlen(array[currow]) >= bufferSize) {
                newBuffer(&bufferSize);
                array[currow] = (char*)realloc(array[currow], bufferSize * sizeof(char));
                if (array[currow] == nullptr) {
                    printf("Memory allocation failed");
                    free(text);
                    exit(1);
                }
            }

            for (int i = strlen(array[currow]); i >= curcol; i--) {
                array[currow][i + text_length] = array[currow][i];
            }
            for (int i = 0; i < text_length; i++) {
                array[currow][curcol + i] = text[i];
            }

            free(text);
        }
        else if (action_type == 8) {
            // Redo delete
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* text = _strdup(token);

            int text_length = strlen(text);
            int new_length = (int)strlen(array[currow]) - text_length;

            for (int i = curcol; i <= new_length; ++i) {
                array[currow][i] = array[currow][i + text_length];
            }
            array[currow][new_length] = '\0';

            free(text);
        }
        else if (action_type == 13) {
            // Redo copy
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            if (token != nullptr) {
                char* text = _strdup(token);
                if (!bufferUndoStack.empty()) {
                    bufferStack.push(bufferUndoStack.top());
                    bufferUndoStack.pop();
                }
                free(text);
            }
        }
        else if (action_type == 9) {
            // Redo cut
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* text = _strdup(token);
            char* text_copy = _strdup(text);

            bufferStack.push(text_copy);

            int text_length = strlen(text);
            int new_length = (int)strlen(array[currow]) - text_length;

            for (int i = curcol; i <= new_length; ++i) {
                array[currow][i] = array[currow][i + text_length];
            }
            array[currow][new_length] = '\0';

            free(text);
        }
        else if (action_type == 16) {
            int currow, curcol;
            char* token = strtok(last_action_copy, "\t");
            token = strtok(nullptr, "\t");
            currow = atoi(token);
            token = strtok(nullptr, "\t");
            curcol = atoi(token);
            token = strtok(nullptr, "\t");
            char* original_text = _strdup(token);

            char* text = _strdup(array[currow]);


            char* undo_info = (char*)malloc((strlen(original_text) + 50) * sizeof(char));
            sprintf(undo_info, "16\t%d\t%d\t%s", currow, curcol, text);
            undoStack.push(undo_info);

            strcpy(array[currow], original_text);

            free(original_text);
        }
        free(last_action_copy);
    }

private:
    size_t bufferSize;
    FILE* file;
    int initialRowCount;
    int nrow;
    int ncol;
    int currow;
    int curcol;
    int amount;
    char** array;
    stack<char*> undoStack;
    stack<char*> redoStack;
    stack<char*> bufferStack;
    stack<char*> bufferUndoStack;

};

class CLI {
public:
    static char* user_input(size_t* bufferSize) {
        char* input = (char*)malloc(*bufferSize * sizeof(char));
        if (input == nullptr) {
            printf("Memory allocation failed.");
            exit(1);
        }

        int length = 0;
        int symbol;

        while ((symbol = getchar()) != '\n' && symbol != EOF) {
            if (length >= *bufferSize - 1) {
                *bufferSize *= 2;
                input = (char*)realloc(input, *bufferSize * sizeof(char));
                if (input == nullptr) {
                    printf("Memory allocation failed.");
                    exit(1);
                }
            }
            input[length++] = symbol;
        }
        input[length] = '\0';

        return input;
    }

    static void execute_command(TextRedactor& redactor, const char* command) {
        if (strcmp(command, "1") == 0) {
            redactor.append_text();
        }
        else if (strcmp(command, "2") == 0) {
            redactor.new_line();
        }
        else if (strcmp(command, "3") == 0) {
            redactor.write_in_file();
        }
        else if (strcmp(command, "4") == 0) {
            redactor.read_from_file();
        }
        else if (strcmp(command, "5") == 0) {
            redactor.print();
        }
        else if (strcmp(command, "6") == 0) {
            redactor.sscan_user_input(0);
            redactor.insert_text();
        }
        else if (strcmp(command, "7") == 0) {
            redactor.search();
        }
        else if (strcmp(command, "8") == 0) {
            redactor.sscan_user_input(1);
            redactor.delete_text();
        }
        else if (strcmp(command, "9") == 0) {
            redactor.sscan_user_input(1);
            redactor.cut();
        }
        else if (strcmp(command, "10") == 0) {
            redactor.freeArray();
            exit(0);
        }
        else if (strcmp(command, "11") == 0) {
            system("cls");
            redactor.help();
        }
        else if (strcmp(command, "12") == 0) {
            redactor.sscan_user_input(0);
            redactor.paste();
        }
        else if (strcmp(command, "13") == 0) {
            redactor.sscan_user_input(1);
            redactor.copy();
        }
        else if (strcmp(command, "14") == 0) {
            redactor.undo();
        }
        else if (strcmp(command, "15") == 0) {
            redactor.redo();
        }
        else if (strcmp(command, "16") == 0) {
            redactor.sscan_user_input(0);
            redactor.insert_rp();
        }
        else {
            printf("The command is not implemented\n");
        }
    }
};

int main() {
    size_t bufferSize = 256;
    TextRedactor dynamicArray;
    dynamicArray.help();
    char* input = nullptr;

    while (true) {
        printf("Choose the command: ");
        input = CLI::user_input(&bufferSize);
        CLI::execute_command(dynamicArray, input);
        free(input);
    }
    return 0;
}
