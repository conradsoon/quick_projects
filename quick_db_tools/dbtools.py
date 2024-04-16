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
            if isinstance(lhs_node, CombinationNode):
                for attr in dep.lhs:
                    self.get_or_create_node({attr}).add_output(lhs_node)
                    lhs_node.add_input(self.get_or_create_node({attr}))

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

    def _is_key(self, attributes):
        closure = self.find_closure(attributes)
        return closure == self.attributes

    def minimal_keys(self):
        keys = []
        for i in range(1, len(self.attributes) + 1):
            for combination in combinations(self.attributes, i):
                if self._is_key(combination):
                    keys.append(combination)
        # remove all keys that are supersets of other keys
        minimal_keys = []
        for key in keys:
            if not any(set(other_key) <= set(key) for other_key in keys if key != other_key):
                minimal_keys.append(key)
        return minimal_keys

    def prime_attributes(self):
        prime_attributes = set()
        for key in self.minimal_keys():
            prime_attributes.update(key)
        return prime_attributes

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


attributes = {'A', 'B', 'C', 'D', 'E', 'F'}
dependencies = [
    FunctionalDependency({'F'}, {'D', 'E'}),
    FunctionalDependency({'C', 'E'}, {'D', 'F'}),
    FunctionalDependency({'C', 'E', 'F'}, {'D'}),
    FunctionalDependency({'D', 'E'}, {'A', 'F'}),
    FunctionalDependency({'A', 'B', 'D'}, {'C', 'F'})
]
relation = Relation(attributes, dependencies)
# find all combinations of attributes, and generate closures
for i in range(1, len(attributes) + 1):
    for combination in combinations(attributes, i):
        closure_result = relation.find_closure(combination)
        print(f"Closure of {combination}: {closure_result}")

# print the minimal keys
print("Minimal keys:")
# print as sorted list of sorted tuples
sorted_keys = sorted(map(tuple, map(sorted, relation.minimal_keys())))
for key in sorted_keys:
    print(key)

print("Prime attributes:", relation.prime_attributes())
