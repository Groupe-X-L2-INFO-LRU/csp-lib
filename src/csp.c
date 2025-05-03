/**
 * @file csp.c
 * @brief Source file for the CSP (Constraint Satisfaction Problem) project.
 *
 * This file contains the implementation of the CSP solver whose content has been partially
 * generated using copilot.
 *
 * The CSP solver is a backtracking algorithm that solves a CSP by
 * assigning values to variables from their domains and checking
 * if the assignment is consistent.
 *
 * @author Ch. Demko
 * @date 2024-2025
 * @version 1.0
 */

#include "./csp.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./csp.inc"

static int counter = 0;

static void _verify(void) {
    assert(!csp_initialised());
}

bool csp_init(void) {
    // This static flag ensures we only register the exit handler once,
    // regardless of how many times csp_init() is called
    static bool first = true;
    if (first) {
        // Register _verify function to be called at program exit
        // This ensures the library has been properly finished before exit
        assert(atexit(_verify) == 0);
        first = false;
    }

    // Increment the initialization counter and print message on first initialization
    if (!counter++) {
        assert(printf("CSP initialised\n"));
    }
    return true;
}

bool csp_finish(void) {
    // Check if the library is currently initialized
    if (counter) {
        // Decrement the initialization counter
        if (!--counter) {
            // If counter reaches zero, print message (complete shutdown)
            assert(printf("CSP finished\n"));
        }
        return true;
    } else {
        // Can't finish what hasn't been initialized
        return false;
    }
}

bool csp_initialised(void) {
    return counter > 0;
}

CSPConstraint *csp_constraint_create(size_t arity, CSPChecker *check) {
    assert(csp_initialised());
    assert(arity > 0);      // Constraints must involve at least one variable
    assert(check != NULL);  // Must provide a valid constraint checking function
    assert(printf("Creating constraint with arity %lu\n", arity));

    // Allocate memory for the constraint structure with a flexible array member
    // We allocate exactly the right amount of space for the 'variables' array
    // based on the constraint's arity (number of variables involved)
    CSPConstraint *constraint = malloc(sizeof(CSPConstraint) + arity * sizeof(size_t));

    if (constraint != NULL) {
        // Initialize the constraint properties
        constraint->arity = arity;
        constraint->check = check;

        // Initialize all variables to 0
        // (These will be set to actual variable indices later with csp_constraint_set_variable)
        memset(constraint->variables, 0, arity * sizeof(size_t));
    }

    // Return the created constraint, or NULL if allocation failed
    return constraint;
}

void csp_constraint_destroy(CSPConstraint *constraint) {
    assert(csp_initialised());
    assert(printf("Destroying constraint with arity %lu\n", constraint->arity));
    free(constraint);
}

size_t csp_constraint_get_arity(const CSPConstraint *constraint) {
    assert(csp_initialised());
    return constraint->arity;
}

CSPChecker *csp_constraint_get_check(const CSPConstraint *constraint) {
    assert(csp_initialised());
    return constraint->check;
}

void csp_constraint_set_variable(CSPConstraint *constraint, size_t index, size_t variable) {
    assert(csp_initialised());
    assert(index < constraint->arity);
    constraint->variables[index] = variable;
}

size_t csp_constraint_get_variable(const CSPConstraint *constraint, size_t index) {
    assert(csp_initialised());
    assert(index < constraint->arity);
    return constraint->variables[index];
}

bool csp_constraint_to_check(const CSPConstraint *constraint, size_t index) {
    assert(csp_initialised());

    // Check if all variables in this constraint have indices less than 'index'
    // This determines if the constraint can be checked at the current stage of solving
    //
    // In backtracking, we only check constraints when all their variables have been assigned
    // Since we assign variables in order (0, 1, 2, ...), if any variable in the constraint
    // has index >= the current assignment index, it means that variable hasn't been assigned yet
    for (size_t i = 0; i < constraint->arity; i++) {
        if (constraint->variables[i] >= index) {
            // At least one variable in this constraint hasn't been assigned yet,
            // so we can't check this constraint at this point
            return false;
        }
    }

    // All variables in this constraint have been assigned,
    // so we can check if the constraint is satisfied
    return true;
}

CSPProblem *csp_problem_create(size_t num_domains, size_t num_constraints) {
    assert(csp_initialised());
    assert(num_domains > 0);  // A CSP must have at least one variable
    // We allow 0 constraints for testing purposes (though a real CSP would have constraints)

    assert(printf("Creating CSP problem with %lu domains and %lu constraints\n", num_domains,
                  num_constraints));

    // Step 1: Allocate memory for the main CSP problem structure
    CSPProblem *csp = malloc(sizeof(CSPProblem));
    if (csp != NULL) {
        // Step 2: Allocate memory for the domains array
        // Each entry will store the size of the domain for that variable
        // Using calloc to initialize all domains to 0
        csp->domains = calloc(num_domains, sizeof(size_t));

        if (csp->domains != NULL) {
            // Step 3: Allocate memory for the constraints array
            // This is an array of pointers to constraint objects
            // Only allocate if we have constraints, otherwise set to NULL
            csp->constraints =
                num_constraints > 0 ? malloc(num_constraints * sizeof(CSPConstraint *)) : NULL;

            // This condition handles both the case where num_constraints is 0 (no allocation
            // needed) and where the allocation succeeded
            if (num_constraints == 0 || csp->constraints != NULL) {
                // Initialize all constraint pointers to NULL
                // (They'll be set later with csp_problem_set_constraint)
                for (size_t i = 0; i < num_constraints; i++) {
                    csp->constraints[i] = NULL;
                }

                // Store the problem dimensions
                csp->num_domains = num_domains;
                csp->num_constraints = num_constraints;
            } else {
                // Failed to allocate constraints array - clean up and return NULL
                free(csp->domains);
                free(csp);
                csp = NULL;
            }
        } else {
            // Failed to allocate domains array - clean up and return NULL
            free(csp);
            csp = NULL;
        }
    } else {
        // Failed to allocate CSP problem structure - just return NULL
        csp = NULL;
    }

    // Return the created CSP problem, or NULL if any allocation failed
    return csp;
}

void csp_problem_destroy(CSPProblem *csp) {
    assert(csp_initialised());
    assert(printf("Destroying CSP problem with %lu domains and %lu constraints\n", csp->num_domains,
                  csp->num_constraints));
    free(csp->constraints);
    free(csp->domains);
    free(csp);
}

size_t csp_problem_get_num_constraints(const CSPProblem *csp) {
    assert(csp_initialised());
    return csp->num_constraints;
}

void csp_problem_set_constraint(CSPProblem *csp, size_t index, CSPConstraint *constraint) {
    assert(csp_initialised());
    assert(index < csp->num_constraints);  // Valid index in constraints array
    assert(constraint != NULL);            // Valid constraint object

    // In debug builds, verify that all variables in the constraint are valid
    // for this CSP problem (indices must be less than the number of variables)
#ifndef NDEBUG
    for (size_t i = 0; i < constraint->arity; i++) {
        assert(constraint->variables[i] < csp->num_domains);
    }
#endif

    // Set the constraint in the CSP problem
    csp->constraints[index] = constraint;
}

CSPConstraint *csp_problem_get_constraint(const CSPProblem *csp, size_t index) {
    assert(csp_initialised());
    assert(index < csp->num_constraints);
    return csp->constraints[index];
}

size_t csp_problem_get_num_domains(const CSPProblem *csp) {
    assert(csp_initialised());
    return csp->num_domains;
}

void csp_problem_set_domain(CSPProblem *csp, size_t index, size_t domain) {
    assert(csp_initialised());
    assert(index < csp->num_domains);
    csp->domains[index] = domain;
}

size_t csp_problem_get_domain(const CSPProblem *csp, size_t index) {
    assert(csp_initialised());
    assert(index < csp->num_domains);
    return csp->domains[index];
}

bool csp_problem_is_consistent(const CSPProblem *csp, const size_t *values, const void *data,
                               size_t index) {
    assert(csp_initialised());

    // Iterate through all constraints in the CSP problem
    for (size_t i = 0; i < csp->num_constraints; i++) {
        CSPConstraint *constraint = csp->constraints[i];

        // Two conditions must be met to check a constraint:
        // 1. All variables involved in the constraint must already be assigned
        //    (checked by csp_constraint_to_check)
        // 2. The constraint must be satisfied with the current assignment
        //    (checked by constraint->check)

        // If the constraint should be checked and it's not satisfied, the
        // current assignment is inconsistent
        if (csp_constraint_to_check(constraint, index) &&
            !constraint->check(constraint, values, data)) {
            return false;  // This assignment violates a constraint
        }
    }

    // If we reach here, all applicable constraints are satisfied
    return true;
}

bool csp_problem_solve(const CSPProblem *csp, size_t *values, const void *data) {
    assert(csp_initialised());
    return csp_problem_backtrack(csp, values, data, 0);
}

bool csp_problem_backtrack(const CSPProblem *csp, size_t *values, const void *data, size_t index) {
    assert(csp_initialised());

    // Base case: If we've assigned values to all variables, we've found a solution
    if (index == csp->num_domains) {
        return true;
    }

    // Try each possible value in the domain of the current variable
    for (size_t i = 0; i < csp->domains[index]; i++) {
        // Step 1: Assign this value to the current variable
        values[index] = i;

        // Step 2: Check if this assignment is consistent with all constraints
        // that involve only variables we've assigned so far
        if (csp_problem_is_consistent(csp, values, data, index + 1)) {
            // Step 3: If consistent, recursively try to assign the next variable
            if (csp_problem_backtrack(csp, values, data, index + 1)) {
                // Solution found!
                return true;
            }

            // If we reach here, this path didn't lead to a solution
            // We'll implicitly try the next value (no need to undo assignment)
        }
    }

    // If we've tried all values and none worked, backtrack to the previous variable
    return false;
}
