/** * @file csp.inc * @brief Internal structures for the CSP (Constraint
Satisfaction Problem) library. * * This file defines the internal structures
used by the CSP library. These structures * should not be accessed directly by
users of the library; instead, use the public * API functions defined in csp.h.
* * @author Ch. Demko (original) * @date 2024-2025 * @version 1.0 */ /** *
@brief The internal structure representing a constraint in a CSP problem. * * A
constraint defines a relation between variables that must be satisfied for a
valid solution. * The constraint stores: * - A check function that determines if
the constraint is satisfied * - The arity (number of variables involved in the
constraint) * - The list of variables involved in the constraint * * The
variables array is a flexible array member and is allocated dynamically * with
the constraint to accommodate exactly 'arity' variables. * * @var check A
function pointer to the constraint's check function * @var arity The number of
variables involved in this constraint * @var variables A flexible array member
storing the indices of variables involved in the constraint */ struct
_CSPConstraint { CSPChecker *check; /* Function to check if the constraint is
satisfied */ size_t arity; /* Number of variables in the constraint */ size_t
variables[]; /* Flexible array of variable indices */ }; /** * @brief The
internal structure representing a complete CSP problem. * * A CSP problem
consists of: * - Variables with their domains (possible values) * - Constraints
between these variables * * In this implementation: * - Each variable is
identified by its index (0 to num_domains-1) * - The domain of each variable
consists of values 0, 1, 2, ..., domains[i]-1 * - Each constraint is a
relationship between variables that must be satisfied * * @var num_domains The
number of variables in the problem * @var domains Array storing the domain size
for each variable * @var num_constraints The number of constraints in the
problem * @var constraints Array of pointers to constraint objects */ struct
_CSPProblem { size_t num_domains; /* Number of variables */ size_t *domains; /*
Array of domain sizes */ size_t num_constraints; /* Number of constraints */
CSPConstraint **constraints; /* Array of constraint pointers */ };
