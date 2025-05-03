#ifndef CSP_H_
#define CSP_H_

#include <stdbool.h>
#include <stddef.h>

/**
 * @file csp.h
 * @brief Header file for the CSP (Constraint Satisfaction Problem) library.
 *
 * This file defines the core structures and functions for working with Constraint
 * Satisfaction Problems. CSP is a mathematical problem where variables must be
 * assigned values from their domains while satisfying a set of constraints.
 * This library provides mechanisms for defining, manipulating and solving CSPs
 * using various algorithms like backtracking and forward checking with heuristics.
 *
 * @author Ch. Demko (original)
 * @date 2024-2025
 * @version 1.0
 */

/**
 * @brief The constraint of a CSP problem.
 *
 * A constraint defines a relation between variables that must be satisfied
 * for a valid solution. This is an opaque data type; use the API functions
 * to interact with constraints.
 */
typedef struct _CSPConstraint CSPConstraint;

/**
 * @brief The CSP problem.
 *
 * Represents a complete Constraint Satisfaction Problem, containing the set of
 * variables, their domains, and the constraints between them. This is an opaque
 * data type; use the API functions to interact with CSP problems.
 */
typedef struct _CSPProblem CSPProblem;

/**
 * @brief The check function of a CSP constraint.
 *
 * This function type is used to check whether a constraint is satisfied
 * for a given assignment of values to variables.
 *
 * @param constraint The constraint to check.
 * @param values The current assignment of values to variables.
 * @param data User-provided context data passed to the check function.
 * @return true if the constraint is satisfied, false otherwise.
 * @pre constraint != NULL
 * @pre values != NULL
 */
typedef bool CSPChecker(const CSPConstraint *, const size_t *, const void *);

/**
 * @brief Initializes the CSP library.
 *
 * This function must be called before using any other function in the CSP library.
 * The library uses a reference counting mechanism, so each call to csp_init() must
 * be matched with a corresponding call to csp_finish().
 *
 * @return true if the library was successfully initialized (or was already initialized),
 *         false otherwise.
 * @post The library's internal reference counter is incremented.
 * @post The library is ready for use.
 */
extern bool csp_init(void);

/**
 * @brief Finalizes the CSP library.
 *
 * This function should be called when the CSP library is no longer needed.
 * Due to the reference counting mechanism, the library is only actually
 * finalized when the number of calls to csp_finish() matches the number
 * of calls to csp_init().
 *
 * @return true if the library was successfully finalized or decremented its
 *         reference count, false if the library was not initialized.
 * @post The library's internal reference counter is decremented.
 * @post If the reference counter reaches zero, all resources are freed.
 */
extern bool csp_finish(void);

/**
 * @brief Checks if the CSP library is initialized.
 *
 * This function can be used to check the initialization state of the library
 * before attempting to use other functions.
 *
 * @return true if the library is currently initialized, false otherwise.
 * @post The library state is unchanged.
 */
extern bool csp_initialised(void);

/**
 * @brief Creates a new constraint with the specified arity and check function.
 *
 * A constraint represents a relationship between variables that must be satisfied
 * in a valid CSP solution. The arity is the number of variables involved in the
 * constraint, and the check function determines whether a particular assignment
 * of values satisfies the constraint.
 *
 * @param arity The number of variables involved in this constraint.
 * @param check The function that checks whether the constraint is satisfied.
 * @return A newly created constraint object, or NULL if memory allocation failed.
 * @pre The CSP library must be initialized via csp_init().
 * @pre arity must be greater than 0.
 * @pre check must be a valid function pointer (not NULL).
 * @post The constraint's variables are all initialized to 0.
 * @post The constraint's arity is set to the specified arity.
 * @post The constraint's check function is set to the specified check function.
 */
extern CSPConstraint *csp_constraint_create(size_t arity, CSPChecker *check);

/**
 * @brief Destroys a constraint and frees all associated resources.
 *
 * This function deallocates all memory used by the constraint object.
 * After calling this function, the constraint pointer is no longer valid
 * and should not be used.
 *
 * @param constraint The constraint to destroy.
 * @pre The CSP library must be initialized via csp_init().
 * @pre constraint must not be NULL.
 * @post All memory associated with the constraint is freed.
 */
extern void csp_constraint_destroy(CSPConstraint *constraint);
/**
 * @brief Gets the arity of a constraint.
 *
 * The arity refers to the number of variables involved in the constraint.
 * For example, a binary constraint has arity 2, as it relates two variables.
 *
 * @param constraint The constraint whose arity to retrieve.
 * @return The number of variables involved in this constraint.
 * @pre The CSP library must be initialized via csp_init().
 * @pre constraint must not be NULL.
 */
extern size_t csp_constraint_get_arity(const CSPConstraint *constraint);

/**
 * @brief Gets the check function associated with a constraint.
 *
 * The check function is used to determine whether the constraint is satisfied
 * for a given assignment of values to variables.
 *
 * @param constraint The constraint whose check function to retrieve.
 * @return The function pointer to the constraint's check function.
 * @pre The CSP library must be initialized via csp_init().
 * @pre constraint must not be NULL.
 */
extern CSPChecker *csp_constraint_get_check(const CSPConstraint *constraint);

/**
 * @brief Sets which CSP variable is related to a specific position in the constraint.
 *
 * This function specifies which CSP variable corresponds to each position in the constraint.
 * For example, in a binary constraint between variables 3 and 5, you would call this function
 * twice: once with index=0, variable=3 and once with index=1, variable=5.
 *
 * @param constraint The constraint to modify.
 * @param index The position within the constraint (from 0 to arity-1).
 * @param variable The variable ID to assign to this position in the constraint.
 * @pre The CSP library must be initialized via csp_init().
 * @pre constraint must not be NULL.
 * @pre index must be less than the constraint's arity.
 * @post The specified position in the constraint now references the given variable.
 */
extern void csp_constraint_set_variable(CSPConstraint *constraint, size_t index, size_t variable);

/**
 * @brief Gets the variable ID associated with a specific position in the constraint.
 *
 * This function retrieves which CSP variable is associated with a given position in the constraint.
 *
 * @param constraint The constraint to query.
 * @param index The position within the constraint (from 0 to arity-1).
 * @return The variable ID at the specified position in the constraint.
 * @pre The CSP library must be initialized via csp_init().
 * @pre constraint must not be NULL.
 * @pre index must be less than the constraint's arity.
 */
extern size_t csp_constraint_get_variable(const CSPConstraint *constraint, size_t index);
/**
 * @brief Determines if a constraint can be checked at the current stage of problem solving.
 *
 * During backtracking or forward checking, we only want to check constraints when
 * all variables involved in the constraint have been assigned values. This function
 * verifies whether all variables in the constraint have indices less than the provided index,
 * which means they have already been assigned values in the current solving process.
 *
 * @param constraint The constraint to verify.
 * @param index The index of the current variable in the assignment process.
 *              All variables with indices less than this are considered assigned.
 * @return true if the constraint can be checked (all its variables have been assigned),
 *         false otherwise.
 * @pre The CSP library must be initialized via csp_init().
 * @pre constraint must not be NULL.
 */
extern bool csp_constraint_to_check(const CSPConstraint *constraint, size_t index);

/**
 * @brief Creates a new CSP problem instance with the specified parameters.
 *
 * A CSP problem consists of variables, their domains (possible values), and constraints
 * between these variables. This function allocates memory for a new CSP problem with
 * the specified number of variables (domains) and constraints.
 *
 * @param num_domains The number of variables in the CSP problem.
 * @param num_constraints The number of constraints in the CSP problem.
 * @return A new CSP problem instance, or NULL if memory allocation failed.
 * @pre The CSP library must be initialized via csp_init().
 * @pre num_domains must be greater than 0.
 * @pre num_constraints can be 0 for testing purposes, although typically CSPs have at least one constraint.
 * @post Each variable's domain size is initialized to 0.
 * @post Each constraint slot is initialized to NULL.
 * @post The CSP problem has the specified number of domains and constraints.
 */
extern CSPProblem *csp_problem_create(size_t num_domains, size_t num_constraints);

/**
 * @brief Destroys a CSP problem instance and frees all associated resources.
 *
 * This function deallocates the memory used by the CSP problem, including its
 * domains and constraint arrays. It does not destroy the constraint objects themselves,
 * as these are supposed to be managed by the caller.
 *
 * @param csp The CSP problem to destroy.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @post All memory directly owned by the CSP problem is freed.
 */
extern void csp_problem_destroy(CSPProblem *csp);
/**
 * @brief Gets the number of constraints in the CSP problem.
 *
 * Constraints define relationships between variables that must be satisfied 
 * for a valid solution.
 *
 * @param csp The CSP problem to query.
 * @return The total number of constraints in the problem.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 */
extern size_t csp_problem_get_num_constraints(const CSPProblem *csp);

/**
 * @brief Sets a constraint at the specified position in the CSP problem.
 *
 * This function assigns a constraint object to a specific position in the
 * CSP problem's constraint array. The function performs validation to ensure
 * that all variables referenced by the constraint exist in the CSP problem.
 *
 * @param csp The CSP problem to modify.
 * @param index The position in the constraint array (0 to num_constraints-1).
 * @param constraint The constraint object to assign.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @pre index must be less than the number of constraints in the CSP problem.
 * @pre constraint must not be NULL.
 * @pre All variables in the constraint must have indices less than csp->num_domains.
 * @post The constraint at the specified index is set to the provided constraint object.
 */
extern void csp_problem_set_constraint(CSPProblem *csp, size_t index, CSPConstraint *constraint);

/**
 * @brief Gets the constraint at the specified position in the CSP problem.
 *
 * @param csp The CSP problem to query.
 * @param index The position in the constraint array (0 to num_constraints-1).
 * @return The constraint object at the specified position, or NULL if no constraint is set.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @pre index must be less than the number of constraints in the CSP problem.
 */
extern CSPConstraint *csp_problem_get_constraint(const CSPProblem *csp, size_t index);
/**
 * @brief Gets the number of variables (domains) in the CSP problem.
 *
 * In this CSP implementation, each variable has a domain of possible values it can take.
 * The size of this domain is stored separately with each variable.
 *
 * @param csp The CSP problem to query.
 * @return The total number of variables in the problem.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 */
extern size_t csp_problem_get_num_domains(const CSPProblem *csp);

/**
 * @brief Sets the domain size for a variable in the CSP problem.
 *
 * This function specifies how many possible values a given variable can take.
 * For example, if a variable can be assigned values from 0 to 9, its domain size would be 10.
 * The actual values are assumed to be 0, 1, 2, ..., domain-1.
 *
 * @param csp The CSP problem to modify.
 * @param index The variable index (0 to num_domains-1).
 * @param domain The size of the domain (number of possible values) for this variable.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @pre index must be less than the number of domains in the CSP problem.
 * @post The domain size for the specified variable is set to the provided value.
 */
extern void csp_problem_set_domain(CSPProblem *csp, size_t index, size_t domain);

/**
 * @brief Gets the domain size for a variable in the CSP problem.
 *
 * This function retrieves the number of possible values that can be assigned
 * to a specific variable in the CSP problem.
 *
 * @param csp The CSP problem to query.
 * @param index The variable index (0 to num_domains-1).
 * @return The size of the domain for the specified variable.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @pre index must be less than the number of domains in the CSP problem.
 */
extern size_t csp_problem_get_domain(const CSPProblem *csp, size_t index);
/**
 * @brief Checks if the current partial assignment is consistent with all constraints.
 *
 * During the solving process, this function verifies whether the current assignment
 * of values to variables (up to the given index) satisfies all applicable constraints.
 * Only constraints whose variables have all been assigned are checked.
 *
 * @param csp The CSP problem to verify.
 * @param values The current assignment of values to variables.
 * @param data User-provided data to pass to constraint check functions.
 * @param index The index of the next variable to be assigned (all variables before
 *              this index are considered to be already assigned).
 * @return true if the current partial assignment is consistent with all applicable
 *         constraints, false otherwise.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @pre values must not be NULL.
 */
extern bool csp_problem_is_consistent(const CSPProblem *csp, const size_t *values, const void *data, size_t index);

/**
 * @brief Recursively solves the CSP problem using backtracking from a given variable.
 *
 * This function implements the core backtracking algorithm for constraint satisfaction.
 * It attempts to assign a value to the variable at the specified index and proceeds
 * recursively until a complete solution is found or all possibilities are exhausted.
 *
 * @param csp The CSP problem to solve.
 * @param values Array to store the assignments of values to variables.
 * @param data User-provided data to pass to constraint check functions.
 * @param index The index of the variable to assign next.
 * @return true if a solution is found, false if no solution exists.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @pre values must not be NULL.
 * @post If a solution is found, values contains the solution assignments.
 */
extern bool csp_problem_backtrack(const CSPProblem *csp, size_t *values, const void *data, size_t index);

/**
 * @brief Solves a CSP problem using the basic backtracking algorithm.
 *
 * This is a convenience function that initiates the backtracking process
 * from the beginning (first variable). It's the main entry point for solving
 * a CSP problem with the basic backtracking algorithm.
 *
 * @param csp The CSP problem to solve.
 * @param values Array to store the assignments of values to variables.
 * @param data User-provided data to pass to constraint check functions.
 * @return true if a solution is found, false if no solution exists.
 * @pre The CSP library must be initialized via csp_init().
 * @pre csp must not be NULL.
 * @pre values must not be NULL.
 * @post If a solution is found, values contains the solution assignments.
 */
extern bool csp_problem_solve(const CSPProblem *csp, size_t *values, const void *data);

#endif  // CSP_H_
