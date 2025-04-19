#include <vector>
#include <string>
#include <map>
#include <Windows.h>
#include "lambda.h"
#include <algorithm>

#define V(x) new Variable(#x)
#define L(x, b) new Lambda(#x, b)
#define C(a, b) new Call(a, b)

Expression::~Expression() {}

std::ostream& operator<<(std::ostream& s, const Expression& exp) {
	std::string str = exp.tostring();
	if (str[0] == '(') {
		str = str.substr(1, str.length() - 2);
	}
	s << str;
	return s;
}

Variable::Variable(char name) {
	this->name = name;
}

Variable::Variable(const char* name) {
	this->name = name[0];
}

void Variable::free_variables(std::set<char> vars) {
	vars.insert(this->name);
}

bool Variable::reduce(std::deque<Expression*> to_be_reduced) {
	return false;
}

bool Variable::substitute(char before, Expression *after) {
	// This function should never be called
	return false;
}

void Variable::convert(char before, char after) {
	if (this->name == before) {
		this->name = after;
	}
}

Variable* Variable::copy() {
	return new Variable(this->name);
}

std::string Variable::tostring() const {
	return std::string(1, this->name);
}

Lambda::Lambda(char arg, Expression *body) {
	this->arg = arg;
	this->body = body;
}

Lambda::Lambda(const char* arg, Expression *body) {
	this->arg = arg[0];
	this->body = body;
}

void Lambda::free_variables(std::set<char> vars) {
	this->body->free_variables(vars);
	vars.erase(this->arg);
}

bool Lambda::reduce(std::deque<Expression*> to_be_reduced) {
	bool modified;
	this->body = Reducer::betareduce(this->body, to_be_reduced, &modified);
	return modified;
}

bool Lambda::substitute(char before, Expression *after) {
	static std::string chars = "abcdefghijklmnopqrstuvwxyz";
	static std::set<char> alpha(chars.begin(), chars.end());
	
	if (before == this->arg) {
		return false;
	}
	std::set<char> free;
	after->free_variables(free);
	
	if (free.count(this->arg) && free.size() != 26) {
		std::set<char> available;
		std::set_difference(alpha.begin(), alpha.end(), free.begin(), free.end(), std::inserter(available, available.end()));
		available.insert(this->arg);
		auto idx = available.find(this->arg);
		if (++idx == available.end()) {
			idx = available.begin();
		} else {
			idx++;
		}
		char new_var = *idx;
		this->convert(this->arg, new_var);
		this->arg = new_var;
	}
	
	if (!free.count(this->arg)) {
		Variable *body = dynamic_cast<Variable*>(this->body);
		if (body != nullptr) {
			if (body->name == before) {
				this->body = after->copy();
				return true;
			} else {
				return false;
			}
		} else {
			return this->body->substitute(before, after);
		}
	} else {
		return false;
	}
}

void Lambda::convert(char before, char after) {
	Call *body = dynamic_cast<Call*>(this->body);
	if (body == nullptr) {
		this->body->convert(before, after);
	}
}

Lambda* Lambda::copy() {
	return new Lambda(this->arg, this->body->copy());
}

std::string Lambda::tostring() const {
	std::string body = this->body->tostring();
	if (body[0] == '(') {
		body = body.substr(1, body.length() - 2);
	}
	return "(λ" + std::string(1, this->arg) + "." + body + ")";
}

Call::Call(Expression *func, Expression *arg) {
	this->func = func;
	this->arg = arg;
}

void Call::free_variables(std::set<char> vars) {
	this->func->free_variables(vars);
	this->arg->free_variables(vars);
}

bool Call::reduce(std::deque<Expression*> to_be_reduced) {
	bool func_modified;
	this->func = Reducer::betareduce(this->func, to_be_reduced, &func_modified);
	if (func_modified) {
		return true;
	}
	bool arg_modified;
	this->arg = Reducer::betareduce(this->arg, to_be_reduced, &arg_modified);
	return arg_modified;
}

bool Call::substitute(char before, Expression *after) {
	bool modified = false;
	Variable *func = dynamic_cast<Variable*>(this->func);
	if (func != nullptr) {
		if (func->name == before) {
			this->func = after->copy();
			modified = true;
		}
	} else {
		modified = this->func->substitute(before, after);
	}
	Variable *arg = dynamic_cast<Variable*>(this->arg);
	if (arg != nullptr) {
		if (arg->name == before) {
			this->arg = after->copy();
			modified = true;
		}
	} else {
		modified = this->arg->substitute(before, after) || modified;
	}
	return modified;
}

void Call::convert(char before, char after) {
	Call *func = dynamic_cast<Call*>(this->func);
	if (func == nullptr) {
		this->func->convert(before, after);
	}
	Call *arg = dynamic_cast<Call*>(this->arg);
	if (arg == nullptr) {
		this->arg->convert(before, after);
	}
}

Call* Call::copy() {
	return new Call(this->func->copy(), this->arg->copy());
}

std::string Call::tostring() const {
	std::string func = this->func->tostring();
	if (func.length() == 1) {
		func += " ";
	}
	return "(" + func + this->arg->tostring() + ")";
}

Expression* Call::expand(bool *modified) {
	Lambda *func = dynamic_cast<Lambda*>(this->func);
	if (func != nullptr) {
		Variable *body = dynamic_cast<Variable*>(func->body);
		if (body != nullptr) {
			if (body->name == func->arg) {
				*modified = true;
				return this->arg;
			} else {
				*modified = true;
				return func->body;
			}
		} else {
			func->body->substitute(func->arg, this->arg);
			*modified = true;
			return func->body;
		}
	} else {
		*modified = false;
		return this;
	}
}

Expression* Reducer::betareduce(Expression *token, std::deque<Expression*> stack, bool *modified) {
	Reducer::reduced = false;
	Call *call = dynamic_cast<Call*>(token);
	if (call != nullptr) {
		bool expand_modified;
		Expression *expanded = call->expand(&expand_modified);
		if (expand_modified) {
			std::cout << "β-reducing " << *Reducer::current_token << "\n";
			Reducer::reduced = true;
			*modified = true;
			return expanded;
		}
	}
	stack.push_back(token);
	*modified = false;
	return token;
}

void Reducer::print(Expression *current) {
	if (Reducer::reduced) {
		std::cout << "β-reduced to " << *Reducer::current_token << "\n";
	}
}

Expression* Reducer::current_token{nullptr};
bool Reducer::reduced{false};

Expression* evaluate(Expression *token) {
	bool modified = true;
	Reducer::current_token = token;
	while (modified) {
		modified = false;
		std::deque<Expression*> stack;
		Reducer::current_token = Reducer::betareduce(Reducer::current_token, stack, &modified);
		Reducer::print(Reducer::current_token);
		std::cout << stack.size() << std::endl;
		while (stack.size()) {
			Expression *current = stack.front();
			modified = current->reduce(stack);
			Reducer::print(current);
			if (modified) {
				break;
			}
		}
	}
	return Reducer::current_token;
}

int main(int argc, char **argv) {
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);

	std::string test = "";
	Expression *iota = L(f, C(C(V(f), L(x, L(y, L(z, C(C(V(x), V(z)), C(V(y), V(z))))))), L(x, L(y, V(x)))));
	Expression *i = C(iota->copy(), iota->copy());
	Expression *k = C(iota->copy(), C(iota->copy(), i->copy()));
	Expression *s = C(iota->copy(), k->copy());
	Expression *exp = C(C(s->copy(), C(C(s->copy(), i->copy()), C(k->copy(), s->copy()))), C(k->copy(), k->copy()));
	std::cout << *evaluate(i) << std::endl;
}
