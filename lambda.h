#ifndef LAMBDA_H
#define LAMBDA_H
#include <unordered_set>
#include <iostream>
#include <deque>
#include <set>

class Expression;

class Expression {
	public:
		virtual ~Expression();
		virtual void free_variables(std::set<char> vars) = 0;
		virtual bool reduce(std::deque<Expression*> to_be_reduced) = 0;
		virtual bool substitute(char before, Expression *after) = 0;
		virtual void convert(char before, char after) = 0;
		virtual std::string tostring() const = 0;
		friend std::ostream& operator<<(std::ostream& s, const Expression& exp);
		virtual Expression* copy() = 0;
};

class Variable : public Expression {
	public:
		Variable(char name);
		Variable(const char* name);
		void free_variables(std::set<char> vars);
		bool reduce(std::deque<Expression*> to_be_reduced);
		bool substitute(char before, Expression *after);
		void convert(char before, char after);
		std::string tostring() const;
		Variable* copy();
		char name;
};

class Lambda : public Expression {
	public:
		Lambda(char arg, Expression *body);
		Lambda(const char* arg, Expression *body);
		void free_variables(std::set<char> vars);
		bool reduce(std::deque<Expression*> to_be_reduced);
		bool substitute(char before, Expression *after);
		void convert(char before, char after);
		std::string tostring() const;
		Lambda* copy();
		char arg;
		Expression *body;
};

class Call : public Expression {
	public:
		Call(Expression *func, Expression *arg);
		void free_variables(std::set<char> vars);
		bool reduce(std::deque<Expression*> to_be_reduced);
		bool substitute(char before, Expression *after);
		void convert(char before, char after);
		std::string tostring() const;
		Call* copy();
		Expression* expand(bool *modified);
		Expression *func;
		Expression *arg;
};

class Reducer {
	public:
		static Expression* betareduce(Expression *token, std::deque<Expression*> stack, bool *modified);
		static void print(Expression *current);
		static Expression *current_token;
		static bool reduced;
};

#endif // lambda.h
