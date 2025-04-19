from abc import ABCMeta, abstractmethod

symbols = "xyzabcdef"

class Expression(metaclass=ABCMeta):
    @abstractmethod
    def reduce(self): pass

    @abstractmethod
    def copy(self): pass

    @abstractmethod
    def __repr__(self): pass

    @abstractmethod
    def beta(self, index, value): pass

    @abstractmethod
    def shift_free(self, index, shift): pass

class Abstraction(Expression):
    symbol_stack = []

    def __init__(self, expression):
        self.expression = expression

    def reduce(self):
        expression = self.expression.reduce()
        if expression is not None:
            return Abstraction(expression)
        return None

    def copy(self):
        return Abstraction(self.expression.copy())

    def __repr__(self):
        Abstraction.symbol_stack.insert(0, symbols[len(Abstraction.symbol_stack)])
        body = repr(self.expression)
        if isinstance(self.expression, (Abstraction, Application)):
            body = body[1:-1]
        string = "(λ" + Abstraction.symbol_stack[0] + "." + body + ")"
        Abstraction.symbol_stack.pop(0)
        return string

    def beta(self, index, value):
        return Abstraction(self.expression.beta(index + 1, value))

    def shift_free(self, index, shift):
        return Abstraction(self.expression.shift_free(index + 1, shift))

class Application(Expression):
    def __init__(self, left, right):
        self.left = left
        self.right = right

    def reduce(self):
        if isinstance(self.left, Abstraction):
            body = self.left.expression
            body = body.beta(0, self.right.shift_free(0, 1))
            return body.shift_free(0, -1)

        reduced = self.left.reduce()
        if reduced is not None:
            return Application(reduced, self.right)

        reduced = self.right.reduce()
        if reduced is not None:
            return Application(self.left, reduced)

        return None

    def copy(self):
        return Application(self.left.copy(), self.right.copy())

    def __repr__(self):
        if isinstance(self.left, Variable) and isinstance(self.right, Variable):
            return "(" + repr(self.left) + " " + repr(self.right) + ")"
        elif isinstance(self.left, Application):
            if isinstance(self.right, Variable):
                return repr(self.left)[:-1] + " " + repr(self.right) + ")"
            else:
                return repr(self.left) + repr(self.right)
        return "(" + repr(self.left) + repr(self.right) + ")"

    def beta(self, index, value):
        left = self.left.beta(index, value)
        right = self.right.beta(index, value)
        return Application(left, right)

    def shift_free(self, index, shift):
        left = self.left.shift_free(index, shift)
        right = self.right.shift_free(index, shift)
        return Application(left, right)

class Variable(Expression):
    def __init__(self, index):
        self.index = index

    def reduce(self):
        return None

    def copy(self):
        return Variable(self.index)

    def __repr__(self):
        return Abstraction.symbol_stack[self.index]

    def beta(self, index, value):
        if self.index == index:
            return value.shift_free(0, index)
        return self.copy()

    def shift_free(self, index, shift):
        if self.index >= index:
            return Variable(self.index + shift)
        return self.copy()

def A(expr):
    if isinstance(expr, int):
        return Variable(expr)
    elif isinstance(expr, (list, tuple)):
        if len(expr) == 2:
            return Application(A(expr[0]), A(expr[1]))
        elif len(expr) == 1:
            return A(expr[0])
        return Application(A(expr[:-1]), A(expr[-1]))
    elif isinstance(expr, Expression):
        return expr

def L(expression):
    return Abstraction(A(expression))

def parse_string(string):
    if string.startswith("λ"):
        Abstraction.symbol_stack.insert(0, string[1])
        abstraction = L(parse_string(string[3:]))
        Abstraction.symbol_stack.pop(0)
        return abstraction
    elif len(string) == 1:
        return Abstraction.symbol_stack.index(string)

    applications = []
    while string:
        if string[0] == "(":
            end = 0
            brackets = 0
            for i, char in enumerate(string):
                if char == "(":
                    brackets += 1
                elif char == ")":
                    brackets -= 1
                if brackets == 0:
                    end = i
                    break
            applications.append(parse_string(string[1:end]))
            string = string[end + 1:]
        else:
            applications.append(Abstraction.symbol_stack.index(string[0]))
            string = string[1:]
            if string.startswith(" "):
                string = string[1:]
    return applications

def dump(expression, alternative=True, xlimit=None, ylimit=None):
    variables = []
    application_depths = []
    abstractions = {}
    abstraction_stack = []
    applications = []
    branching_points = []

    stack = [expression]
    while stack:
        expr = stack.pop()
        if isinstance(expr, str):
            if expr == "end_abstraction":
                start = abstraction_stack.pop()
                end = len(variables) - 1
                index = len(abstraction_stack)
                if index not in abstractions:
                    abstractions[index] = []
                abstractions[index].append((start, end))
            elif expr == "end_application":
                b = branching_points.pop()[0]
                if alternative:
                    a = branching_points.pop()[1]
                else:
                    a = branching_points.pop()[0]

                depth = max(
                    application_depths[a],
                    application_depths[b])
                applications.append([a, b, depth])
                application_depths[a] = depth + 1
                application_depths[b] = depth + 1
                branching_points.append((a, b))
        if isinstance(expr, Abstraction):
            abstraction_stack.append(len(variables))
            stack.append("end_abstraction")
            stack.append(expr.expression)
        elif isinstance(expr, Application):
            stack.append("end_application")
            stack.append(expr.right)
            stack.append(expr.left)
        elif isinstance(expr, Variable):
            variables.append(len(abstraction_stack) - expr.index - 1)
            application_depths.append(len(abstraction_stack))
            branching_points.append((len(variables) - 1, len(variables) - 1))

    if not alternative:
        application_depths[0] += 1

    # print("Variables:", variables)
    # print("Depths:", application_depths)
    # print("Abstractions:", abstractions)
    # print("Applications:", applications)

    width = len(variables) * 4
    height = max(application_depths) * 2
    canvas = [[0 for i in range(width)] for i in range(height)]

    for i in range(len(variables)):
        for j in range(variables[i] * 2, application_depths[i] * 2 - 1):
            canvas[j][i * 4 + 1] = 1

    for depth in abstractions:
        for abstraction in abstractions[depth]:
            start, end = abstraction
            canvas[depth * 2][start * 4:end * 4 + 3] = [1 for i in range((end - start) * 4 + 3)]

    for application in applications:
        start, end, depth = application
        y = depth * 2
        canvas[y][start * 4 + 1:end * 4 + 2] = [1 for i in range((end - start) * 4 + 1)]

    # pixels = [[" ", "▄"], ["▀", "█"]]
    # for i in range(0, height, 2):
    #     print("".join(pixels[canvas[i][x]][canvas[i+1][x]] for x in range(width)))

    pixels = [
        " 𜺨𜺫🮂𜴀▘𜴁𜴂𜴃𜴄▝𜴅𜴆𜴇𜴈▀", "𜴉𜴊𜴋𜴌🯦𜴍𜴎𜴏𜴐𜴑𜴒𜴓𜴔𜴕𜴖𜴗",
        "𜴘𜴙𜴚𜴛𜴜𜴝𜴞𜴟🯧𜴠𜴡𜴢𜴣𜴤𜴥𜴦", "𜴧𜴨𜴩𜴪𜴫𜴬𜴭𜴮𜴯𜴰𜴱𜴲𜴳𜴴𜴵🮅",
        "𜺣𜴶𜴷𜴸𜴹𜴺𜴻𜴼𜴽𜴾𜴿𜵀𜵁𜵂𜵃𜵄", "▖𜵅𜵆𜵇𜵈▌𜵉𜵊𜵋𜵌▞𜵍𜵎𜵏𜵐▛",
        "𜵑𜵒𜵓𜵔𜵕𜵖𜵗𜵘𜵙𜵚𜵛𜵜𜵝𜵞𜵟𜵠", "𜵡𜵢𜵣𜵤𜵥𜵦𜵧𜵨𜵩𜵪𜵫𜵬𜵭𜵮𜵯𜵰",
        "𜺠𜵱𜵲𜵳𜵴𜵵𜵶𜵷𜵸𜵹𜵺𜵻𜵼𜵽𜵾𜵿", "𜶀𜶁𜶂𜶃𜶄𜶅𜶆𜶇𜶈𜶉𜶊𜶋𜶌𜶍𜶎𜶏",
        "▗𜶐𜶑𜶒𜶓▚𜶔𜶕𜶖𜶗▐𜶘𜶙𜶚𜶛▜", "𜶜𜶝𜶞𜶟𜶠𜶡𜶢𜶣𜶤𜶥𜶦𜶧𜶨𜶩𜶪𜶫",
        "▂𜶬𜶭𜶮𜶯𜶰𜶱𜶲𜶳𜶴𜶵𜶶𜶷𜶸𜶹𜶺", "𜶻𜶼𜶽𜶾𜶿𜷀𜷁𜷂𜷃𜷄𜷅𜷆𜷇𜷈𜷉𜷊",
        "𜷋𜷌𜷍𜷎𜷏𜷐𜷑𜷒𜷓𜷔𜷕𜷖𜷗𜷘𜷙𜷚", "▄𜷛𜷜𜷝𜷞▙𜷟𜷠𜷡𜷢▟𜷣▆𜷤𜷥█",
    ]
    graph = ""
    for y in range(0, height, 4):
        if y > ylimit:
            break
        for x in range(0, width, 2):
            if x > xlimit:
                continue
            top = canvas[y][x] + canvas[y][x+1]*2 + canvas[y+1][x]*4 + canvas[y+1][x+1]*8
            if y == height - 2:  # height % 4 != 0
                bottom = 0
            else:
                bottom = canvas[y+2][x] + canvas[y+2][x+1]*2 + canvas[y+3][x]*4 + canvas[y+3][x+1]*8
            graph += pixels[bottom][top]
        graph += "\n"
    print(graph, end="")

s = L(L(L([[2, 0], [1, 0]])))
k = L(L(1))
i = L(0)

iota1 = A([s, [s, i, [k, s]], [k, k]])
s2 = A([iota1, [iota1, [iota1, [iota1, iota1]]]])
k2 = A([iota1, [iota1, [iota1, iota1]]])
i2 = A([iota1, iota1])

iota2 = A([s2, [s2, i2, [k2, s2]], [k2, k2]])

expr = parse_string("λx.((λy.y(y((λz.zz)(λz.λa.λb.(b(λc.λd.d))((λc.(((zz)c)((λd.dd)(λd.c(dd)))))(λc.λd.λe.λf.(fd)(e(ac)))))(λz.λa.λb.y(bz)))))(λy.λz.(z(λa.λb.a)y)))")
while expr:
    dump(expr, True, xlimit=250, ylimit=50)
    expr = expr.reduce()
