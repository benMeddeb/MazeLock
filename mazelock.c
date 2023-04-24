
/**
 * @file mazelock.c
 * @author (Ben Meddeb), (David Mcconnell)
 * @date 04/22/2023
 * @brief MazeLock simulation program
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define ENTRY 'S'
#define EXIT 'E'
#define OPEN ' '
#define CLOSED 'X'
#define VISITED '.'
#define PATH 'P'
// ANSI color codes
#define ANSI_RESET       "\033[0m"
#define ANSI_RED         "\033[31m"
#define ANSI_GREEN       "\033[32m"
#define ANSI_BRIGHT_WHITE   "\033[97m"
#define ANSI_BRIGHT_BLACK   "\033[90m"

char **matrix;
double density = 0.5;
int ROWS = 0;
int COLS = 0;
static int matrix_count = 0;
pthread_mutex_t matrix_mutex;

/**
* @brief A structure to represent a path in the matrix (Secure room)
*/
typedef struct {
    int start_x, start_y;
    int end_x, end_y;
    bool found;
} Path;

/**
 *   function prototypes for the MazeLock simulation program
 */
void allocate_matrix(int rows, int cols);
void free_matrix(int rows);
void randomize_matrix(char **matrix, double density);
void display_matrix(char **matrix);
void find_path(char **matrix);
void* matrix_generation_thread_func(void* arg);
void* path_finding_thread_func(void *arg);


/**
 * @brief The main function of the MazeLock simulation program.
 * @return 0 on successful execution, non-zero on error.
 */

int main() {
    srand(time(NULL));
    printf("Welcome to the MazeLock simulation!\n");
    printf("Enter the number of rows: ");
    scanf("%d", &ROWS);
    getchar();
    printf("Enter the number of columns: ");
    scanf("%d", &COLS);
    getchar();
    printf("Enter the density of open cells (between 0 and 1, e.g., 0.5): ");
    scanf("%lf", &density);
    getchar();

    printf("Press Enter to start the simulation.\n");
    printf("Press 'q' to quit the simulation at any time.\n");
    getchar();

    allocate_matrix(ROWS, COLS);

    pthread_t matrix_generation_thread;
    pthread_t path_finding_thread;
    pthread_mutex_init(&matrix_mutex, NULL);
    pthread_create(&matrix_generation_thread, NULL, matrix_generation_thread_func, NULL);
    pthread_create(&path_finding_thread, NULL, path_finding_thread_func, NULL);

    char input;
    while ((input = getchar()) != 'q') {
        usleep(100);
    }

    pthread_cancel(matrix_generation_thread);
    pthread_cancel(path_finding_thread);

    pthread_join(matrix_generation_thread, NULL);
    pthread_join(path_finding_thread, NULL);

    pthread_mutex_destroy(&matrix_mutex);
    free_matrix(ROWS);

    return 0;
}

/**
 * @brief Allocate memory for the matrix.
 * @param rows The number of rows in the matrix.
 * @param cols The number of columns in the matrix.
 */
void allocate_matrix(int rows, int cols) {
    matrix = (char **)malloc(rows * sizeof(char *));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (char *)malloc(cols * sizeof(char));
    }
}
/**
 * @brief Free memory allocated for the matrix.
 * @param rows The number of rows in the matrix.
 */
void free_matrix(int rows) {
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

/**
 * @brief Place entry and exit points on the edge of the matrix.
 * @param matrix The matrix to place entry and exit points in.
 */
void place_entry_exit_points(char **matrix) {
    int edge_length = 2 * (ROWS + COLS) - 4;
    int entry_position = rand() % edge_length;
    int exit_position;

    do {
        exit_position = rand() % edge_length;
    } while (exit_position == entry_position);

    int counter = 0;
    bool entry_placed = false;
    bool exit_placed = false;

    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (row == 0 || row == ROWS - 1 || col == 0 || col == COLS - 1) {
                if (counter == entry_position && !entry_placed) {
                    matrix[row][col] = ENTRY;
                    entry_placed = true;
                } else if (counter == exit_position && !exit_placed) {
                    matrix[row][col] = EXIT;
                    exit_placed = true;
                }
                counter++;
            }
        }
    }
}

/**
 * @brief Count the number of open cells adjacent to the given cell.
 * @param matrix The matrix containing the cells.
 * @param row The row index of the cell to check.
 * @param col The column index of the cell to check.
 * @return The number of open cells adjacent to the given cell.
 */
int count_adjacent_open_cells(char **matrix, int row, int col) {
    int count = 0;
    // Check adjacent cells (up, down, left, and right)
    if (row > 0 && matrix[row - 1][col] == OPEN) count++;
    if (row < ROWS - 1 && matrix[row + 1][col] == OPEN) count++;
    if (col > 0 && matrix[row][col - 1] == OPEN) count++;
    if (col < COLS - 1 && matrix[row][col + 1] == OPEN) count++;

    return count;
}

/**
 * @brief Checks if placing an open cell at the specified location is valid.
 * @param matrix The maze matrix.
 * @param row Row index of the cell.
 * @param col Column index of the cell.
 * @return true if the placement is valid, false otherwise.
 */
bool is_valid_open_cell_placement(char **matrix, int row, int col) {
    if (matrix[row][col] == ENTRY || matrix[row][col] == EXIT) {
        return false;
    }

    int adjacent_open_cells = count_adjacent_open_cells(matrix, row, col);
    if (adjacent_open_cells > 1) {
        return false;
    }

    // New check: Ensure that no more than two open cells are in a straight line or diagonal.
    if ((row > 0 && row < ROWS - 1 && matrix[row - 1][col] == OPEN && matrix[row + 1][col] == OPEN) ||
        (col > 0 && col < COLS - 1 && matrix[row][col - 1] == OPEN && matrix[row][col + 1] == OPEN) ||
        (row > 0 && row < ROWS - 1 && col > 0 && col < COLS - 1 && matrix[row - 1][col - 1] == OPEN && matrix[row + 1][col + 1] == OPEN) ||
        (row > 0 && row < ROWS - 1 && col > 0 && col < COLS - 1 && matrix[row - 1][col + 1] == OPEN && matrix[row + 1][col - 1] == OPEN)) {
        return false;
    }

    // Check cells two steps away
    if (row > 1 && matrix[row - 2][col] == OPEN && matrix[row - 1][col] == OPEN) {
        return false;
    }

    if (row < ROWS - 2 && matrix[row + 2][col] == OPEN && matrix[row + 1][col] == OPEN) {
        return false;
    }

    if (col > 1 && matrix[row][col - 2] == OPEN && matrix[row][col - 1] == OPEN) {
        return false;
    }

    if (col < COLS - 2 && matrix[row][col + 2] == OPEN && matrix[row][col + 1] == OPEN) {
        return false;
    }

    return true;
}



/**
 * @brief Randomizes the given matrix based on the density.
 * @param matrix The maze matrix.
 * @param density The density of open cells in the matrix.
 */
void randomize_matrix(char **matrix, double density) {
    int entry_row = -1, entry_col = -1, exit_row = -1, exit_col = -1;

    // Fill the matrix with open and closed cells based on the density
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if ((row == 0 || row == ROWS - 1 || col == 0 || col == COLS - 1) &&
                (matrix[row][col] == ENTRY || matrix[row][col] == EXIT)) {
                continue;
            }
            double random_value = (double)rand() / (double)RAND_MAX;
            if (random_value <= density && is_valid_open_cell_placement(matrix, row, col)) {
                matrix[row][col] = OPEN;
            } else {
                matrix[row][col] = CLOSED;
            }
        }
    }

    // Find entry and exit points
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (matrix[row][col] == ENTRY) {
                entry_row = row;
                entry_col = col;
            } else if (matrix[row][col] == EXIT) {
                exit_row = row;
                exit_col = col;
            }
        }
    }
    // Remove the previous entry and exit points only if they have been found
    if (entry_row != -1 && entry_col != -1) {
        matrix[entry_row][entry_col] = CLOSED;
    }
    if (exit_row != -1 && exit_col != -1) {
        matrix[exit_row][exit_col] = CLOSED;
    }
    // Place new entry and exit points
    place_entry_exit_points(matrix);
}

/**
 * @brief Generates a new matrix.
 * @param matrix The matrix.
 */
void generate_matrix(char **matrix) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            matrix[row][col] = CLOSED;
        }
    }
    randomize_matrix(matrix, density);
}

/**
 * @brief Displays the matrix (room) on the console.
 * @param matrix The matrix.
 */
void display_matrix(char **matrix) {
    matrix_count++;
    printf("\nMatrix %d\n", matrix_count);
    for (int i = 0; i < (COLS * 2); i++) {
        putchar('-');
    }
    putchar('\n');
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            switch (matrix[row][col]) {
                case OPEN:
                    fputs(ANSI_BRIGHT_WHITE "■" ANSI_RESET, stdout);
                    break;
                case CLOSED:
                    fputs(ANSI_BRIGHT_BLACK "■" ANSI_RESET, stdout);
                    break;
                case ENTRY:
                case EXIT:
                    fputs(ANSI_GREEN "■" ANSI_RESET, stdout);
                    break;
                case PATH:
                    fputs(ANSI_RED "■" ANSI_RESET, stdout);
                    break;
                case VISITED:
                    fputs(ANSI_RESET "■", stdout);
                    break;
            }
            putchar(' ');
        }
        putchar('\n');
    }
}

/**
 * @brief Thread function to generate and display the matrix.
 * @param arg Unused argument.
 * @return Unused return value.
 */
void* matrix_generation_thread_func(void* arg) {
    (void)arg;
    // Generate the initial matrix
    pthread_mutex_lock(&matrix_mutex);
    generate_matrix(matrix);
    pthread_mutex_unlock(&matrix_mutex);
    while (true) {
        pthread_mutex_lock(&matrix_mutex);
        randomize_matrix(matrix, density);
        pthread_mutex_unlock(&matrix_mutex);
        display_matrix(matrix);
        sleep(2);
    }
}

/**
 * @brief Performs a depth-first search to find a path through the maze.
 * @param matrix The matrix.
 * @param row Row index of the starting cell.
 * @param col Column index of the starting cell.
 * @return A Path structure representing the found path, or an invalid path if not found.
 */

Path dfs(char **matrix, int row, int col) {
    Path path = {.found = false};

    if (row < 0 || row >= ROWS || col < 0 || col >= COLS) {
        return path;
    }
    if (matrix[row][col] == EXIT) {
        path.start_x = row;
        path.start_y = col;
        path.end_x = row;
        path.end_y = col;
        path.found = true;
        return path;
    } else if (matrix[row][col] == CLOSED || matrix[row][col] == VISITED) {
        return path;
    }
    matrix[row][col] = VISITED;
    Path directions[] = {dfs(matrix, row - 1, col), dfs(matrix, row + 1, col),
                         dfs(matrix, row, col - 1), dfs(matrix, row, col + 1)};

    for (int i = 0; i < 4; i++) {
        if (directions[i].found) {
            path.start_x = row;
            path.start_y = col;
            path.end_x = directions[i].end_x;
            path.end_y = directions[i].end_y;
            path.found = true;
            break;
        }
    }

    return path;
}

/**
 * @brief Finds a path through the maze and prints the result.
 * @param matrix The maze matrix.
 */
void find_path(char **matrix) {
    int entry_row = -1, entry_col = -1;
    bool entry_found = false;
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            if (row == 0 || row == ROWS - 1 || col == 0 || col == COLS - 1) {
                if (matrix[row][col] == ENTRY) {
                    entry_row = row;
                    entry_col = col;
                    entry_found = true;
                    break;
                }
            }
        }
        if (entry_found) {
            break;
        }
    }
    if (entry_row != -1 && entry_col != -1) {
        Path path = dfs(matrix, entry_row, entry_col);
        if (path.found) {
            printf("Partial path found from (%d,%d) to (%d,%d)\n", path.start_x, path.start_y, path.end_x, path.end_y);
        } else {
            printf("No path found.\n");
        }
    } else {
        printf("Entry point not found.\n");
    }
}
/**
 * @brief Thread function to find paths through the matrix.
 * @param arg Unused argument.
 * @return Unused return value.
 */
void *path_finding_thread_func(void *arg) {
    (void)arg;
    while (true) {
        pthread_mutex_lock(&matrix_mutex);
        find_path(matrix);
        pthread_mutex_unlock(&matrix_mutex);
        sleep(2);
    }
}
