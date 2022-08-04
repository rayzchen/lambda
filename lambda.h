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
		virtual Expression* copy() = 0;
};

class Variable : public Expression {
	public:
		Variable(char name);
		void free_variables(std::set<char> vars);
		bool reduce(std::deque<Expression*> to_be_reduced);
		bool substitute(char before, Expression *after);
		void convert(char before, char after);
		Variable* copy();
		friend std::ostream& operator<<(std::ostream& s, const Variable& var);
		char name;
};

class Lambda : public Expression {
	public:
		Lambda(char arg, Expression *body);
		void free_variables(std::set<char> vars);
		bool reduce(std::deque<Expression*> to_be_reduced);
		bool substitute(char before, Expression *after);
		void convert(char before, char after);
		Lambda* copy();
		friend std::ostream& operator<<(std::ostream& s, const Lambda& lam);
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
		Call* copy();
		Expression* expand(bool *modified);
		friend std::ostream& operator<<(std::ostream& s, const Call& call);
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
