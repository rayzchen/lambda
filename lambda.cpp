#include <vector>
#include <string>
#include <map>
#include <Windows.h>
#include "lambda.h"
#include <algorithm>

Expression::~Expression() {}

Variable::Variable(char name) {
	this->name = name;
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

std::ostream& operator<<(std::ostream& s, const Variable& var) {
	s << var.name;
	return s;
}

Lambda::Lambda(char arg, Expression *body) {
	this->arg = arg;
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

std::ostream& operator<<(std::ostream& s, const Lambda& lam) {
	s << "(λ" << lam.arg << "." << lam.body << ")";
	return s;
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

std::ostream& operator<<(std::ostream& s, const Call& call) {
	s << "(" << call.func << call.arg << ")";
	return s;
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
			// std::cout << "β-reducing " << Reducer::current_token << "\n";
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
		std::cout << Reducer::current_token << "\n";
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
		Expression *token = Reducer::betareduce(token, stack, &modified);
		Reducer::current_token = token;
		// Reducer::print(token);
		while (stack.size()) {
			Expression *current = stack.front();
			bool modified = current->reduce(stack);
			// Reducer::print(current);
			if (modified) {
				break;
			}
		}
	}
	return token;
}

int main(int argc, char **argv) {
	Expression *exp = new Call(new Lambda('x', new Variable('x')), new Variable('y'));
	Variable *result = dynamic_cast<Variable*>(evaluate(exp));
	std::cout << *result << std::endl;
}
