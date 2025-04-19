#include "lambda.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <map>
#ifdef _WIN32
#include <Windows.h>
#endif

const std::string symbols = "xyzabcdefg";
const std::string lambda = "λ";
std::string pixels[16][16] = {
    {" ", "𜺨", "𜺫", "🮂", "𜴀", "▘", "𜴁", "𜴂", "𜴃", "𜴄", "▝", "𜴅", "𜴆", "𜴇", "𜴈", "▀"},
    {"𜴉", "𜴊", "𜴋", "𜴌", "🯦", "𜴍", "𜴎", "𜴏", "𜴐", "𜴑", "𜴒", "𜴓", "𜴔", "𜴕", "𜴖", "𜴗"},
    {"𜴘", "𜴙", "𜴚", "𜴛", "𜴜", "𜴝", "𜴞", "𜴟", "🯧", "𜴠", "𜴡", "𜴢", "𜴣", "𜴤", "𜴥", "𜴦"},
    {"𜴧", "𜴨", "𜴩", "𜴪", "𜴫", "𜴬", "𜴭", "𜴮", "𜴯", "𜴰", "𜴱", "𜴲", "𜴳", "𜴴", "𜴵", "🮅"},
    {"𜺣", "𜴶", "𜴷", "𜴸", "𜴹", "𜴺", "𜴻", "𜴼", "𜴽", "𜴾", "𜴿", "𜵀", "𜵁", "𜵂", "𜵃", "𜵄"},
    {"▖", "𜵅", "𜵆", "𜵇", "𜵈", "▌", "𜵉", "𜵊", "𜵋", "𜵌", "▞", "𜵍", "𜵎", "𜵏", "𜵐", "▛"},
    {"𜵑", "𜵒", "𜵓", "𜵔", "𜵕", "𜵖", "𜵗", "𜵘", "𜵙", "𜵚", "𜵛", "𜵜", "𜵝", "𜵞", "𜵟", "𜵠"},
    {"𜵡", "𜵢", "𜵣", "𜵤", "𜵥", "𜵦", "𜵧", "𜵨", "𜵩", "𜵪", "𜵫", "𜵬", "𜵭", "𜵮", "𜵯", "𜵰"},
    {"𜺠", "𜵱", "𜵲", "𜵳", "𜵴", "𜵵", "𜵶", "𜵷", "𜵸", "𜵹", "𜵺", "𜵻", "𜵼", "𜵽", "𜵾", "𜵿"},
    {"𜶀", "𜶁", "𜶂", "𜶃", "𜶄", "𜶅", "𜶆", "𜶇", "𜶈", "𜶉", "𜶊", "𜶋", "𜶌", "𜶍", "𜶎", "𜶏"},
    {"▗", "𜶐", "𜶑", "𜶒", "𜶓", "▚", "𜶔", "𜶕", "𜶖", "𜶗", "▐", "𜶘", "𜶙", "𜶚", "𜶛", "▜"},
    {"𜶜", "𜶝", "𜶞", "𜶟", "𜶠", "𜶡", "𜶢", "𜶣", "𜶤", "𜶥", "𜶦", "𜶧", "𜶨", "𜶩", "𜶪", "𜶫"},
    {"▂", "𜶬", "𜶭", "𜶮", "𜶯", "𜶰", "𜶱", "𜶲", "𜶳", "𜶴", "𜶵", "𜶶", "𜶷", "𜶸", "𜶹", "𜶺"},
    {"𜶻", "𜶼", "𜶽", "𜶾", "𜶿", "𜷀", "𜷁", "𜷂", "𜷃", "𜷄", "𜷅", "𜷆", "𜷇", "𜷈", "𜷉", "𜷊"},
    {"𜷋", "𜷌", "𜷍", "𜷎", "𜷏", "𜷐", "𜷑", "𜷒", "𜷓", "𜷔", "𜷕", "𜷖", "𜷗", "𜷘", "𜷙", "𜷚"},
    {"▄", "𜷛", "𜷜", "𜷝", "𜷞", "▙", "𜷟", "𜷠", "𜷡", "𜷢", "▟", "𜷣", "▆", "𜷤", "𜷥", "█"},
};

Expression::~Expression() {}

std::deque<char> Abstraction::symbol_stack;

Abstraction::Abstraction(std::unique_ptr<Expression> e): expression(std::move(e)) {}

std::unique_ptr<Expression> Abstraction::reduce(bool* changed) {
    std::unique_ptr<Expression> body = this->expression->reduce(changed);
    if (body != nullptr) {
        this->expression = std::move(body);
    }
    return nullptr;
}

std::unique_ptr<Expression> Abstraction::copy() const {
    std::unique_ptr<Expression> abstraction(new Abstraction(this->expression->copy()));
    return abstraction;
}

std::string Abstraction::tostring() {
    Abstraction::symbol_stack.emplace_front(symbols[Abstraction::symbol_stack.size()]);
    std::string body = this->expression->tostring();
    if (body[0] == '(') {
        body = body.substr(1, body.length() - 2);
    }
    std::ostringstream stream;
    stream << "(" << lambda << Abstraction::symbol_stack[0] << '.' << body << ')';
    Abstraction::symbol_stack.pop_front();
    return stream.str();
}

std::unique_ptr<Expression> Abstraction::beta(int index, const Expression& value) {
    std::unique_ptr<Expression> body = this->expression->beta(index + 1, value);
    if (body != nullptr) {
        this->expression = std::move(body);
    }
    return nullptr;
}

void Abstraction::shift_free(int index, int shift) {
    this->expression->shift_free(index + 1, shift);
}

Application::Application(
    std::unique_ptr<Expression> l,
    std::unique_ptr<Expression> r): left(std::move(l)), right(std::move(r)) {}

std::unique_ptr<Expression> Application::reduce(bool* changed) {
    Abstraction *abstraction = dynamic_cast<Abstraction*>(this->left.get());
    if (abstraction != nullptr) {
        std::unique_ptr<Expression> body = std::move(abstraction->expression);
        this->right->shift_free(0, 1);
        *changed = true;

        std::unique_ptr<Expression> result = body->beta(0, *this->right);
        if (result != nullptr) {
            result->shift_free(0, -1);
            return result;
        }
        body->shift_free(0, -1);
        return body;
    }

    bool left_changed;
    std::unique_ptr<Expression> left = this->left->reduce(&left_changed);
    if (left_changed) {
        if (left != nullptr) {
            this->left = std::move(left);
        }
        *changed = true;
        return nullptr;
    }

    bool right_changed;
    std::unique_ptr<Expression> right = this->right->reduce(&right_changed);
    if (right_changed) {
        if (right != nullptr) {
            this->right = std::move(right);
        }
        *changed = true;
        return nullptr;
    }

    *changed = false;
    return nullptr;
}

std::unique_ptr<Expression> Application::copy() const {
    std::unique_ptr<Expression> application(new Application(this->left->copy(), this->right->copy()));
    return application;
}

std::string Application::tostring() {
    std::string left = this->left->tostring();
    std::string right = this->right->tostring();
    std::ostringstream stream;
    if (left.length() == 1 && right.length() == 1) {
        stream << '(' << left << ' ' << right << ')';
        return stream.str();
    } else if (left[0] == '(' && left.substr(1, lambda.size()) != lambda) {
        if (right.length() == 1) {
            stream << left.substr(0, left.length() - 1) << ' ' << right << ')';
        } else {
            stream << '(' << left << right << ')';
        }
        return stream.str();
    }
    stream << '(' << left << right << ')';
    return stream.str();
}

std::unique_ptr<Expression> Application::beta(int index, const Expression& value) {
    std::unique_ptr<Expression> left = this->left->beta(index, value);
    if (left != nullptr) {
        this->left = std::move(left);
    }
    std::unique_ptr<Expression> right = this->right->beta(index, value);
    if (right != nullptr) {
        this->right = std::move(right);
    }
    return nullptr;
}

void Application::shift_free(int index, int shift) {
    this->left->shift_free(index, shift);
    this->right->shift_free(index, shift);
}

Variable::Variable(int i): index(i) {}

std::unique_ptr<Expression> Variable::reduce(bool* changed) {
    *changed = false;
    return nullptr;
}

std::unique_ptr<Expression> Variable::copy() const {
    std::unique_ptr<Expression> variable(new Variable(this->index));
    return variable;
}

std::string Variable::tostring() {
    std::string string(1, Abstraction::symbol_stack[this->index]);
    return string;
}

std::unique_ptr<Expression> Variable::beta(int index, const Expression& value) {
    if (this->index == index) {
        std::unique_ptr<Expression> copy = value.copy();
        copy->shift_free(0, index);
        return copy;
    }
    return nullptr;
}

void Variable::shift_free(int index, int shift) {
    if (this->index >= index) {
        this->index += shift;
    }
}

std::unique_ptr<Expression> parse_string(std::string expr) {
    std::string string = expr;
    size_t index = 0;
    while (true) {
        index = string.find(lambda, index);
        if (index == std::string::npos) {
            break;
        }
        string.replace(index, lambda.size(), "#");
        index += 1;
    }

    if (string[0] == '#') {
        Abstraction::symbol_stack.emplace_front(string[1]);
        std::unique_ptr<Expression> abstraction(new Abstraction(parse_string(string.substr(3))));
        Abstraction::symbol_stack.pop_front();
        return abstraction;
    } else if (string.length() == 1) {
        int index = -1;
        for (int i = 0; i < Abstraction::symbol_stack.size(); i++) {
            if (Abstraction::symbol_stack[i] == string[0]) {
                index = i;
                break;
            }
        }

        std::unique_ptr<Expression> variable(new Variable(index));
        return variable;
    }

    std::deque<std::unique_ptr<Expression>> applications;
    while (string.length() > 0) {
        if (string[0] == '(') {
            int end = 0;
            int brackets = 0;
            for (int i = 0; i < string.length(); i++) {
                if (string[i] == '(') {
                    brackets++;
                } else if (string[i] == ')') {
                    brackets--;
                }
                if (brackets == 0) {
                    end = i;
                    break;
                }
            }
            applications.emplace_back(parse_string(string.substr(1, end - 1)));
            string = string.substr(end + 1);
        } else {
            applications.emplace_back(parse_string(string.substr(0, 1)));
            string = string.substr(1);
            if (string[0] == ' ') {
                string = string.substr(1);
            }
        }
    }

    while (applications.size() > 1) {
        std::unique_ptr<Expression> application(
            new Application(std::move(applications[0]), std::move(applications[1])));
        applications.pop_front();
        applications[0] = std::move(application);
    }
    return std::move(applications[0]);
}

void dump(std::unique_ptr<Expression>& expression, bool alternative = true) {
    std::vector<int> variables;
    std::vector<int> application_depths;
    std::map<int, std::vector<std::pair<int, int>>> abstractions;
    std::vector<int> abstraction_stack;
    std::vector<std::tuple<int, int, int>> applications;
    std::vector<std::pair<int, int>> branching_points;

    std::vector<std::pair<const Expression*, int>> stack;
    stack.emplace_back(expression.get(), 0);
    while (stack.size() > 0) {
        std::pair<const Expression*, int> pair = stack.back();
        stack.pop_back();
        if (pair.second == 1) {
            int start = abstraction_stack.back();
            abstraction_stack.pop_back();
            int end = variables.size() - 1;
            int index = abstraction_stack.size();
            if (abstractions.find(index) == abstractions.end()) {
                abstractions[index] = {};
            }
            abstractions[index].emplace_back(start, end);
        } else if (pair.second == 2) {
            std::pair<int, int> right = branching_points.back();
            branching_points.pop_back();
            int b = right.first;
            std::pair<int, int> left = branching_points.back();
            branching_points.pop_back();
            int a = (alternative) ? left.second : left.first;

            int depth = std::max(application_depths[a], application_depths[b]);
            applications.emplace_back(a, b, depth);
            application_depths[a] = depth + 1;
            application_depths[b] = depth + 1;
            branching_points.emplace_back(a, b);
        } else {
            const Abstraction *abstraction = dynamic_cast<const Abstraction*>(pair.first);
            if (abstraction != nullptr) {
                abstraction_stack.emplace_back(variables.size());
                stack.emplace_back(nullptr, 1);
                stack.emplace_back(abstraction->expression.get(), 0);
                continue;
            }

            const Application *application = dynamic_cast<const Application*>(pair.first);
            if (application != nullptr) {
                stack.emplace_back(nullptr, 2);
                stack.emplace_back(application->right.get(), 0);
                stack.emplace_back(application->left.get(), 0);
                continue;
            }

            const Variable *variable = dynamic_cast<const Variable*>(pair.first);
            if (variable != nullptr) {
                variables.emplace_back(abstraction_stack.size() - variable->index - 1);
                application_depths.emplace_back(abstraction_stack.size());
                branching_points.emplace_back(variables.size() - 1, variables.size() - 1);
                continue;
            }
        }
    }

    if (!alternative) {
        application_depths[0]++;
    }

    int width = variables.size() * 4;
    int height = *std::max_element(application_depths.begin(), application_depths.end()) * 2;

    int **canvas = new int*[height];
    for (int row = 0; row < height; row++) {
        canvas[row] = new int[width];
        for (int col = 0; col < width; col++) {
            canvas[row][col] = 0;
        }
    }

    for (int x = 0; x < variables.size(); x++) {
        for (int y = variables[x] * 2; y < application_depths[x] * 2 - 1; y++) {
            canvas[y][x * 4 + 1] = 1;
        }
    }

    for (auto it = abstractions.begin(); it != abstractions.end(); it++) {
        for (int i = 0; i < it->second.size(); i++) {
            std::pair<int, int> abstraction = it->second[i];
            int depth = it->first;
            int start = abstraction.first;
            int end = abstraction.second;
            for (int x = start * 4; x < end * 4 + 3; x++) {
                canvas[depth * 2][x] = 1;
            }
        }
    }

    for (int i = 0; i < applications.size(); i++) {
        int start = std::get<0>(applications[i]);
        int end = std::get<1>(applications[i]);
        int depth = std::get<2>(applications[i]);
        for (int x = start * 4 + 1; x < end * 4 + 2; x++) {
            canvas[depth * 2][x] = 1;
        }
    }

    // std::string pixels[2][2] = {{" ", "▄"}, {"▀", "█"}};
    // std::string graph = "";
    // for (int y = 0; y < height; y += 2) {
    //     for (int x = 0; x < width; x++) {
    //         graph.append(pixels[canvas[y][x]][canvas[y + 1][x]]);
    //     }
    //     graph.append("\n");
    // }
    // std::cout << graph << std::flush;

    std::string graph = "";
    for (int y = 0; y < height; y += 4) {
        for (int x = 0; x < width; x += 2) {
            int top = canvas[y][x] + canvas[y][x+1]*2 + canvas[y+1][x]*4 + canvas[y+1][x+1]*8;
            int bottom;
            if (y == height - 2) {
                bottom = 0;
            } else {
                bottom = canvas[y+2][x] + canvas[y+2][x+1]*2 + canvas[y+3][x]*4 + canvas[y+3][x+1]*8;
            }
            graph.append(pixels[bottom][top]);
        }
        graph.append("\n");
    }
    std::cout << graph << std::flush;

    for (int y = 0; y < height; y++) {
        delete[] canvas[y];
    }
    delete[] canvas;
}

#define L(expr) std::unique_ptr<Expression>(new Abstraction(expr))
#define A(l, r) std::unique_ptr<Expression>(new Application(l, r))
#define V(idx) std::unique_ptr<Expression>(new Variable(idx))
#define C(expr) expr->copy()

int main(int argc, char *argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
    auto s = L(L(L(A(A(V(2), V(0)), A(V(1), V(0))))));
    auto k = L(L(V(1)));
    auto i = L(V(0));
    auto iota1 = A(A(C(s), A(A(C(s), C(i)), A(C(k), C(s)))), A(C(k), C(k)));

    auto s2 = A(C(iota1), A(C(iota1), A(C(iota1), A(C(iota1), C(iota1)))));
    auto k2 = A(C(iota1), A(C(iota1), A(C(iota1), C(iota1))));
    auto i2 = A(C(iota1), C(iota1));
    auto iota2 = A(A(C(s2), A(A(C(s2), C(i2)), A(C(k2), C(s2)))), A(C(k2), C(k2)));

    auto expression = iota2->copy();
    std::cout << expression->tostring() << std::endl;

    bool changed = true;
    while (changed) {
        dump(expression);
        std::unique_ptr<Expression> new_expression = expression->reduce(&changed);
        if (new_expression != nullptr) {
            expression = std::move(new_expression);
        }
    }
    return 0;
}
