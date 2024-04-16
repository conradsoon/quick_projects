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

    def __repr__(self) -> str:
        # print sorted attributes
        return f"{', '.join(sorted(self.lhs))} -> {', '.join(sorted(self.rhs))}"


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

    def project(self, attributes):
        projected_attributes = set(attributes)
        if not projected_attributes.issubset(self.attributes):
            raise ValueError(
                "The subset of attributes is not a valid subset of the relation's attributes.")

        projected_dependencies = []
        for dep in self.dependencies:
            # only consider if lhs is a subset of the projected attributes
            if dep.lhs.issubset(projected_attributes):
                # append the subset of rhs
                projected_dependencies.append(
                    FunctionalDependency(dep.lhs, dep.rhs.intersection(projected_attributes)))
        return projected_attributes, projected_dependencies


def is_dependency_implied(relation, dependency):
    # Calculate the closure of the LHS of the dependency
    lhs_closure = relation.find_closure(dependency.lhs)
    # Check if the RHS of the dependency is fully contained within the LHS closure
    is_implied = dependency.rhs.issubset(lhs_closure)
    return is_implied


def is_dependency_preserving(relation, decomposed_subsets):
    # Project each subset of attributes to get relevant dependencies
    decomposed_dependencies = set()
    for subset in decomposed_subsets:
        projected_attributes, projected_dependencies = relation.project(subset)
        for dep in projected_dependencies:
            decomposed_dependencies.add(
                (frozenset(dep.lhs), frozenset(dep.rhs)))

    # Reconstruct the combined relation with the union of all dependencies
    combined_attributes = set.union(*map(set, decomposed_subsets))
    reconstructed_deps = [FunctionalDependency(
        dep[0], dep[1]) for dep in decomposed_dependencies]
    reconstructed_relation = Relation(combined_attributes, reconstructed_deps)

    # Check if the closure of the original relation is preserved in the reconstructed relation
    return check_closure_differences(relation, reconstructed_relation)


def is_bcnf(relation):
    # iterater through all possible closures
    for i in range(1, len(relation.attributes) + 1):
        for combination in combinations(relation.attributes, i):
            closure = relation.find_closure(combination)
            # if lcosure is not equivalent to itself or the whole relation, then it is not in BCNF
            if closure != set(combination) and closure != relation.attributes:
                return False
    return True


def is_3nf(relation):
    prime_attributes = relation.prime_attributes()
    for i in range(1, len(relation.attributes) + 1):
        for combination in combinations(relation.attributes, i):
            closure = relation.find_closure(combination)
            if closure != set(combination) and closure != relation.attributes:
                # calculate difference
                diff = closure.difference(set(combination))
                # check if the difference is a subset of the prime attributes
                if not diff.issubset(prime_attributes):
                    return False
    return True


def check_closure_differences(relation1, relation2):
    attributes = relation1.attributes
    differences = []
    for i in range(1, len(attributes) + 1):
        for combination in combinations(attributes, i):
            closure1 = relation1.find_closure(combination)
            closure2 = relation2.find_closure(combination)
            if closure1 != closure2:
                difference_info = {
                    "combination": combination,
                    "closure1": closure1,
                    "closure2": closure2
                }
                differences.append(difference_info)

    if differences:
        # for diff in differences:
        #     print(f"Closure difference found for {diff['combination']}:")
        #     print(f"  Closure in Relation 1: {diff['closure1']}")
        #     print(f"  Closure in Relation 2: {diff['closure2']}")
        return False
    return True


# questions 1-4
# attributes = {'A', 'B', 'C', 'D', 'E', 'F'}
# dependencies = [
#     FunctionalDependency({'F'}, {'D', 'E'}),
#     FunctionalDependency({'C', 'E'}, {'D', 'F'}),
#     FunctionalDependency({'C', 'E', 'F'}, {'D'}),
#     FunctionalDependency({'D', 'E'}, {'A', 'F'}),
#     FunctionalDependency({'A', 'B', 'D'}, {'C', 'F'})
# ]
# relation = Relation(attributes, dependencies)
# # find all combinations of attributes, and generate closures
# for i in range(1, len(attributes) + 1):
#     for combination in combinations(attributes, i):
#         closure_result = relation.find_closure(combination)
#         print(f"Closure of {combination}: {closure_result}")

# # print the minimal keys
# print("Minimal keys:")
# # print as sorted list of sorted tuples
# sorted_keys = sorted(map(tuple, map(sorted, relation.minimal_keys())))
# for key in sorted_keys:
#     print(key)

# print("Prime attributes:", relation.prime_attributes())


# attributes_p = {'A', 'B', 'C', 'D', 'E', 'F'}
# dependencies_p = [
#     FunctionalDependency({'A', 'B', 'D'}, {'C'}),
#     FunctionalDependency({'A', 'B', 'D'}, {'F'}),
#     FunctionalDependency({'C', 'E'}, {'D'}),
#     # FunctionalDependency({'C', 'E'}, {'F'}),
#     FunctionalDependency({'F'}, {'D'}),
#     FunctionalDependency({'F'}, {'E'}),
#     FunctionalDependency({'D', 'E'}, {'A'}),
#     FunctionalDependency({'D', 'E'}, {'F'})
# ]


# relation1 = Relation(attributes, dependencies)
# relation2 = Relation(attributes_p, dependencies_p)


# print(check_closure_differences(relation1, relation2))

# questions 5-onwards
attributes = {'A', 'B', 'C', 'D', 'E', 'F'}
dependencies = [
    FunctionalDependency({'F'}, {'D', 'E'}),
    FunctionalDependency({'C', 'E'}, {'D', 'F'}),
    FunctionalDependency({'C', 'E', 'F'}, {'D'}),
    FunctionalDependency({'D', 'E'}, {'A', 'F'}),
    FunctionalDependency({'A', 'B', 'D'}, {'C', 'F'})
]
relation = Relation(attributes, dependencies)

# Define decompositions for each scenario
decompositions = {
    # First decomposition
    1: [{'A', 'D', 'E', 'F'}, {'A', 'C', 'E'}, {'B', 'C', 'E'}, {'C', 'F'}],
    # Second decomposition
    2: [{'A', 'B', 'D', 'F'}, {'A', 'D', 'E', 'F'}, {'B', 'C', 'F'}, {'C', 'E', 'F'}]
}

# decompose 1
decomposed_dependencies_1 = []
for subset in decompositions[1]:
    projected_attributes, projected_dependencies = relation.project(subset)
    decomposed_dependencies_1.extend(projected_dependencies)

decomposed_relations_1 = Relation(attributes, decomposed_dependencies_1)

decomposed_dependencies_2 = []
for subset in decompositions[2]:
    projected_attributes, projected_dependencies = relation.project(subset)
    decomposed_dependencies_2.extend(projected_dependencies)

decomposed_relations_2 = Relation(attributes, decomposed_dependencies_2)

fd_bf_c = FunctionalDependency({'B', 'F'}, {'C'})
fd_ce_df = FunctionalDependency({'C', 'E'}, {'D', 'F'})
fd_f_de = FunctionalDependency({'F'}, {'D', 'E'})
fd_cef_d = FunctionalDependency({'C', 'E', 'F'}, {'D'})
fd_de_af = FunctionalDependency({'D', 'E'}, {'A', 'F'})
fd_cf_a = FunctionalDependency({'C', 'F'}, {'A'})
fd_abd_cf = FunctionalDependency({'A', 'B', 'D'}, {'C', 'F'})
fd_bde_c = FunctionalDependency({'B', 'D', 'E'}, {'C'})
print("Functional dependencies not preserved in decomposition 1:")
for fd in [fd_bf_c, fd_ce_df, fd_f_de, fd_cef_d, fd_de_af, fd_cf_a, fd_abd_cf, fd_bde_c]:
    # check if implied in 1, and not implied in 2
    implied_1 = is_dependency_implied(relation, fd)
    implied_2 = is_dependency_implied(decomposed_relations_1, fd)
    if implied_1 and not implied_2:
        print(fd)

print("Functional dependencies not preserved in decomposition 2:")
for fd in [fd_bf_c, fd_ce_df, fd_f_de, fd_cef_d, fd_de_af, fd_cf_a, fd_abd_cf, fd_bde_c]:
    # check if implied in 1, and not implied in 2
    implied_1 = is_dependency_implied(relation, fd)
    implied_2 = is_dependency_implied(decomposed_relations_2, fd)
    if implied_1 and not implied_2:
        print(fd)

# for each projection in 2 print the dependencies after projection
for p in decompositions[2]:
    attr, deps = relation.project(p)
    _relation = Relation(attr, deps)
    print("Projection:", p)
    print(deps)
    # print implies 3nf and bcnf
    print("Is 3NF:", is_3nf(_relation))
    print("Is BCNF:", is_bcnf(_relation))
