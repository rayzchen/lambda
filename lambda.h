#ifndef LAMBDA_H
#define LAMBDA_H

#include <string>
#include <deque>
#include <memory>

class Expression {
    public:
        virtual ~Expression();
        virtual std::unique_ptr<Expression> reduce(bool* changed) = 0;
        virtual std::unique_ptr<Expression> copy() const = 0;
        virtual std::string tostring() = 0;
        virtual std::unique_ptr<Expression> beta(int index, const Expression& value) = 0;
        virtual void shift_free(int index, int shift) = 0;
};

class Abstraction : public Expression {
    public:
        Abstraction(std::unique_ptr<Expression> e);
        std::unique_ptr<Expression> reduce(bool* changed);
        std::unique_ptr<Expression> copy() const;
        std::string tostring();
        std::unique_ptr<Expression> beta(int index, const Expression& value);
        void shift_free(int index, int shift);

        static std::deque<char> symbol_stack;
        friend class Application;
        friend void dump(std::unique_ptr<Expression>& expression, bool alternative);

    private:
        std::unique_ptr<Expression> expression;
};

class Application : public Expression {
    public:
        Application(std::unique_ptr<Expression> l, std::unique_ptr<Expression> r);
        std::unique_ptr<Expression> reduce(bool* changed);
        std::unique_ptr<Expression> copy() const;
        std::string tostring();
        std::unique_ptr<Expression> beta(int index, const Expression& value);
        void shift_free(int index, int shift);

        friend void dump(std::unique_ptr<Expression>& expression, bool alternative);

    private:
        std::unique_ptr<Expression> left;
        std::unique_ptr<Expression> right;
};

class Variable : public Expression {
    public:
        Variable(int i);
        std::unique_ptr<Expression> reduce(bool* changed);
        std::unique_ptr<Expression> copy() const;
        std::string tostring();
        std::unique_ptr<Expression> beta(int index, const Expression& value);
        void shift_free(int index, int shift);

        friend void dump(std::unique_ptr<Expression>& expression, bool alternative);

    private:
        int index;
};

#endif // lambda.h
