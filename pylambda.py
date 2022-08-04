# Lexer:
# 1. Check number of brackets
# 2. Evaluate all sets of brackets
# 3. Only one token?
# 3a. If its in alphabet, convert to variable
# 3b. If not, return that token
# 4. Starts with lambda?
# 4a. Get argument name
# 4b. Check dot
# 4c. Get lambda body
# 5. Anything else?
# 5a. Take first two tokens, solve them and use application
# 5b. Repeat 5a until only 1 token is left (aka a b c -> (a b)c)

# Reducer:
# Variable: no modification
# Lambda:
# 1. Reduce body
# 2. If body is application, use beta reduction
# Call:
# 1. Reduce function
# 2. Reduce argument
# 3. If function is application, use beta reduction
# 4. If argument is application, use beta reduction

import copy
from abc import ABCMeta, abstractmethod

rules = """
ZERO := λfx.x
ONE := λfx.f x
TWO := λfx.f (f x)
THREE := λfx.f (f (f x))
INC := λnfx.f (n f x)
PLUS := λmn.m INC n
MULT := λmnf.m (n f)
POW := λbe.e b
DEC := λnfx.n (λgh.h (g f)) (λu.x) (λu.u)
SUB := λmn.n DEC m
TRUE := λxy.x
FALSE := λxy.y
AND := λpq.p q p
OR := λpq.p p q
NOT := λp.p FALSE TRUE
IF := λpab.p a b
ISZERO := λn.n (λx.FALSE) TRUE
PAIR := λxyf.f x y
FIRST := λp.p TRUE
SECOND := λp.p FALSE
NIL := λx.TRUE
NULL := λp.p (λxy.FALSE)
"""

# Iota
# I:=λf.(f(λxyz.xz(yz)))(λxy.x)
# SKI notation of Iota: S (S I (K S)) (K K)
# ((I(I(I(I I))))(I(I(I(I I)))(I I)((I(I(I I)))(I(I(I(I I))))))((I(I(I I)))(I(I(I I)))))

class Expression(metaclass=ABCMeta):
    @abstractmethod
    def __str__(self):
        pass

    @abstractmethod
    def free_variables(self):
        pass

    @abstractmethod
    def bound_variables(self):
        pass

    @abstractmethod
    def reduce(self, to_be_reduced):
        pass

    @abstractmethod
    def substitute(self, before, after):
        pass

    @abstractmethod
    def convert(self, before, after):
        pass

    @abstractmethod
    def copy(self):
        pass

class Variable(Expression):
    def __init__(self, name):
        self.name = name

    def __str__(self):
        return self.name

    def free_variables(self):
        return {self.name}

    def bound_variables(self):
        return set()

    def reduce(self, to_be_reduced):
        return False

    def substitute(self, before, after):
        # This should never be called.
        return False

    def convert(self, before, after):
        if self.name == before:
            self.name = after

    def copy(self):
        return Variable(self.name)

class Lambda(Expression):
    def __init__(self, arg, body):
        self.arg = arg
        self.body = body

    def __str__(self):
        body = str(self.body)
        if body.startswith("("):
            body = body[1:-1]
        return f"(λ{self.arg}.{body})" # .replace("", "")

    def free_variables(self):
        return self.body.free_variables() - {self.arg}

    def bound_variables(self):
        return {self.arg} | self.body.bound_variables()

    def reduce(self, to_be_reduced):
        modified, self.body = Reducer.betareduce(self.body, to_be_reduced)
        return modified

    def substitute(self, before, after):
        if before == self.arg:
            return False
        free = after.free_variables()
        if self.arg in free and len(free) != 26:
            available = list(set(alpha) - free - self.body.bound_variables())
            available.append(self.arg)
            available.sort()
            idx = available.index(self.arg)
            new_var = available[idx + 1 if idx != len(available) - 1 else 0]
            print(f"α-converting {self.arg} to {new_var}: {self}")
            self.body.convert(self.arg, new_var)
            old_var = self.arg
            self.arg = new_var
            print(f"α-converted {old_var} to {new_var}: {self}")

        if self.arg not in free:
            if isinstance(self.body, Variable):
                if self.body.name == before:
                    self.body = after.copy()
                    return True
                return False
            return self.body.substitute(before, after)
        return False

    def convert(self, before, after):
        if before != self.arg and after != self.arg:
            self.body.convert(before, after)

    def copy(self):
        return Lambda(self.arg, self.body.copy())

class Call(Expression):
    def __init__(self, func, arg):
        self.func = func
        self.arg = arg

    def __str__(self):
        func = str(self.func)
        if isinstance(self.func, Variable):
            func = func + " "
        return f"({func}{self.arg})"

    def free_variables(self):
        return self.func.free_variables() | self.arg.free_variables()

    def bound_variables(self):
        return self.func.bound_variables() | self.arg.bound_variables()

    def reduce(self, to_be_reduced):
        func_modified, self.func = Reducer.betareduce(self.func, to_be_reduced)
        if func_modified:
            return True
        arg_modified, self.arg = Reducer.betareduce(self.arg, to_be_reduced)
        return arg_modified

    def substitute(self, before, after):
        modified = False
        if isinstance(self.func, Variable):
            if self.func.name == before:
                self.func = after.copy()
                modified = True
        else:
            modified = self.func.substitute(before, after)
        if isinstance(self.arg, Variable):
            if self.arg.name == before:
                self.arg = after.copy()
                modified = True
        else:
            modified = self.arg.substitute(before, after) or modified
        return modified

    def convert(self, before, after):
        self.func.convert(before, after)
        self.arg.convert(before, after)

    def copy(self):
        return Call(self.func.copy(), self.arg.copy())

    def expand(self):
        if isinstance(self.func, Lambda):
            body = self.func.body
            if isinstance(body, Variable):
                if body.name == self.func.arg:
                    return True, self.arg
                else:
                    return True, body
            body.substitute(self.func.arg, self.arg)
            return True, body
        return False, self

class StopParse(Exception):
    pass

class Reducer:
    current_token = None
    reduced = False

    @classmethod
    def betareduce(cls, token, stack):
        cls.reduced = False
        if isinstance(token, Call):
            modified, new = token.expand()
            if modified:
                print("β-reducing", cls.current_token)
                cls.reduced = True
                return True, new
        stack.append(token)
        return False, token

    @classmethod
    def print(cls, current):
        if cls.reduced:
            print("β-reduced to", cls.current_token)

alpha = list("abcdefghijklmnopqrstuvwxyz")
alphaupper = list("ABCDEFGHIJKLMNOPQRSTUVWXYZ")

def find_parentheses(tokens):
    start = tokens.index("(")
    char = start - 1
    count = 0
    while char < len(tokens) - 1:
        char += 1
        if tokens[char] == "(":
            count += 1
        elif tokens[char] == ")":
            count -= 1
        if count == 0:
            break
    return start, char

def parse_tokens(tokens, variables):
    char = -1
    while char < len(tokens) - 1:
        char += 1
        if tokens[char] in alphaupper:
            word = ""
            while len(tokens) > char:
                letter = tokens.pop(char)
                if letter in alphaupper:
                    word += letter
                else:
                    tokens.insert(char, letter)
                    break
            if word not in variables:
                print(f"Error: value {word} not found")
                raise StopParse
            tokens.insert(char, "$" + word)
        elif tokens[char] == "_":
            if "_" in variables:
                tokens[char] = variables["_"].copy()
            else:
                print(f"Error: value _ not found")
                raise StopParse

    for i in range(len(tokens) - 1, -1, -1):
        if isinstance(tokens[i], str) and tokens[i].isspace():
            tokens.pop(i)

    if tokens.count("(") != tokens.count(")"):
        print("Error: invalid bracket count")
        raise StopParse
    while "(" in tokens:
        start, end = find_parentheses(tokens)
        tokens[start:end + 1] = [parse_tokens(tokens[start + 1:end], variables)]

    if len(tokens) == 1:
        if isinstance(tokens[0], str) and tokens[0].startswith("$") and len(tokens[0]) > 1:
            return variables[tokens[0][1:]].copy()
        if tokens[0] in alpha:
            return Variable(tokens[0])
        elif isinstance(tokens[0], str):
            print(f"Error: Unexpected {tokens[0]!r}")
            raise StopParse
        return tokens[0]
    elif tokens[0] == "λ":
        # Abstraction
        if "." not in tokens:
            print("Error: Missing dot after argument list")
            raise StopParse
        end = tokens.index(".")
        arguments = tokens[1:end]
        if len(arguments) == 0:
            print("Error: Must specify at least 1 argument")
            raise StopParse
        elif not all(x in alpha for x in arguments):
            print(f"Error: invalid argument list f{''.join(arguments)}")
            raise StopParse
        if len(arguments) == 1:
            body = parse_tokens(tokens[3:], variables)
            return Lambda(arguments[0], body)
        else:
            new_tokens = []
            for arg in arguments:
                new_tokens.append("λ")
                new_tokens.append(arg)
                new_tokens.append(".")
            new_tokens.extend(tokens[end + 1:])
            return parse_tokens(new_tokens, variables)
    else:
        while len(tokens) > 1:
            # Application
            tokens[0] = parse_tokens([tokens[0]], variables)
            tokens[1] = parse_tokens([tokens[1]], variables)
            tokens[:2] = [Call(tokens[0], tokens[1])]
        return tokens[0]

def evaluate(token):
    modified = True
    Reducer.current_token = token
    while modified:
        modified = False
        stack = []
        modified, token = Reducer.betareduce(token, stack)
        Reducer.current_token = token
        Reducer.print(token)
        while stack:
            current = stack.pop(0)
            modified = current.reduce(stack)
            Reducer.print(current)
            if modified:
                break
    return token

def main():
    print("Lambda: λ")
    variables = {}
    while True:
        try:
            expression = input("In: ").lstrip().rstrip()
            if not expression or expression.isspace():
                continue
            save = ""
            if ":=" in expression:
                save, expression = expression.split(":=")
                save = save.rstrip()
                if not save or not all(x in alphaupper for x in save):
                    print(f"Error: invalid value {save!r}")
                    raise StopParse
            expression = expression.lstrip()

            token = parse_tokens(list(expression), variables)
            if save:
                variables[save] = token
                variables["_"] = token
            else:
                result = evaluate(token)
                variables["_"] = result
                formatted = str(result)
                if formatted.startswith("(") and formatted.endswith(")"):
                    formatted = formatted[1:-1]
                print("Out:", formatted)
        except StopParse:
            pass
        except EOFError:
            break
        except SystemExit:
            print()
            break
        except KeyboardInterrupt:
            print("^C")
            continue

if __name__ == "__main__":
    main()
