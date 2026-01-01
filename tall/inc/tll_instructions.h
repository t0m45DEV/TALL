#ifndef _TLL_INSTRUCTIONS
#define _TLL_INSTRUCTIONS

typedef enum {
    // Invalid instruction, means an error of some sort
    OP_ERROR = -1,

    // Empty instruction, does nothing
    OP_PASS,

    // Stack managment instructions
    OP_PUSH,
    OP_POP,
    OP_PEEK,
    OP_CLEAR,

    // Print out info to the console instructions
    OP_SHOWF, // For floats
    OP_SHOWI, // For integers
    OP_SHOWC, // For chars

    // Arithmetic instructions
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,

    // Control flow instructions
    OP_JMP,

    // Total instructions count
    OP_CODES_COUNT
} tll_op_code_t;

typedef enum {
    NO_OPERAND = 0,
    ONE_OPERAND,
    MAX_OPERANDS
} tll_operand_cnt_t;

/**
 * Represents an operation, with an operation code and an operand
 *
 * If the instruction given by OP_CODE does not have an operand, the value inside OPERAND should be ignored
 */
typedef struct {
    const tll_op_code_t OP_CODE;
    const tll_operand_cnt_t OPERANDS_NEEDED;
    const char* OPERANDS[MAX_OPERANDS];
    const char* TEXT;
} tll_op_t;

/**
 * Initialize the necesary structures so we can have quick instruction searchs
 *
 * Returns 0 if everything went well, returns 1 otherwise
 */
int init_instructions_set(void);

/**
 * Frees the memory of the quick search structures used
 */
void destroy_instructions_set(void);

/**
 * Returns the operation code corresponding to the given token
 *
 * @note The token is asumed with the form of "INSTRUCTION\0"
 */
tll_op_t get_op_code(const char* token);

/**
 * Prints out into STDOUT the given operation in a readable format
 */
void print_op(const tll_op_t op);

/**
 * Returns a new operation with the same information as the one given
 *
 * @note The caller MUST free the memory allocated for the copy
 */
const tll_op_t* copy_op(const tll_op_t op);

#endif
