from collections import deque
from itertools import combinations


class Node:
    def __init__(self):
        self.input_nodes = set()
        self.output_nodes = set()
        self.activated = False

    def add_input(self, node):
        self.input_nodes.add(node)

    def add_output(self, node):
        self.output_nodes.add(node)

    def _activate(self):
        if not self.activated:
            self.activated = True
            self._execute()

    def _execute(self):
        for node in self.output_nodes:
            node.try_activate()

    def try_activate(self) -> bool:
        pass  # To be implemented in subclasses


class AttributeNode(Node):
    def __init__(self, attribute):
        super().__init__()
        self.attribute = attribute

    def try_activate(self):
        if any(node.activated for node in self.input_nodes):
            self._activate()
            return True
        return False

    def __repr__(self):
        return f"AttributeNode({self.attribute})"


class CombinationNode(Node):
    def __init__(self, attributes):
        super().__init__()
        self.attributes = frozenset(attributes)

    def try_activate(self):
        if all(node.activated for node in self.input_nodes):
            self._activate()
            return True
        return False

    def __repr__(self):
        return f"CombinationNode({', '.join(sorted(self.attributes))})"


class FunctionalDependency:
    def __init__(self, lhs, rhs):
        self.lhs = lhs
        self.rhs = rhs


class Relation:
    def __init__(self, attributes, dependencies):
        self.attributes = attributes
        self.dependencies = dependencies
        self.nodes = {}
        self.init_attributes()
        self.build_nodes()

    def init_attributes(self):
        for attr in self.attributes:
            self.get_or_create_node({attr})

    def build_nodes(self):
        for dep in self.dependencies:
            lhs_node = self.get_or_create_node(dep.lhs)
            rhs_nodes = [self.get_or_create_node({attr}) for attr in dep.rhs]
            for rhs_node in rhs_nodes:
                lhs_node.add_output(rhs_node)
                rhs_node.add_input(lhs_node)

    def get_or_create_node(self, attributes):
        key = frozenset(attributes)
        if len(attributes) == 1:
            attribute = next(iter(attributes))
            return self.nodes.setdefault(key, AttributeNode(attribute))
        return self.nodes.setdefault(key, CombinationNode(attributes))

    def _reset_activations(self):
        for node in self.nodes.values():
            node.activated = False

    def find_closure(self, attributes):
        self._reset_activations()
        for attr in attributes:
            node = self.get_or_create_node({attr})
            node._activate()
        closure = set()
        for node in self.nodes.values():
            if isinstance(node, AttributeNode) and node.activated:
                closure.add(node.attribute)
        return closure


attributes = {'A', 'B', 'C', 'D', 'E'}
dependencies = [
    FunctionalDependency({'A'}, {'B', 'C'}),
    FunctionalDependency({'C'}, {'D'}),
    FunctionalDependency({'B', 'C'}, {'E'})
]

relation = Relation(attributes, dependencies)
# find all combinations of attributes, and generate closures
for i in range(1, len(attributes) + 1):
    for combination in combinations(attributes, i):
        closure_result = relation.find_closure(combination)
        print(f"Closure of {combination}: {closure_result}")
