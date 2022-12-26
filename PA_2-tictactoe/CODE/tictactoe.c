#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

char turn;
char **matrix;
int size;
bool gamepoint = false;
char winner = '_';
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

struct pleasework
{
    /* data */
    char pl;
};

char **CreateBoard()
{
    matrix = (char **)malloc(size * sizeof(char *));
    for (int i = 0; i < size; i++)
    {
        matrix[i] = (char *)malloc(size * sizeof(char));
    }
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            matrix[i][j] = ' ';
        }
    }
    return matrix;
}

void printBoard()
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            printf("[%c]", matrix[i][j]);
        }
        printf("\n");
    }
}

bool isFull()
{
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (matrix[i][j] == ' ')
            {
                return false;
            }
        }
    }
    return true;
}
bool isWinner(char pl)
{
    int diagonal = 0;
    int revdiag = 0;
    for (int i = 0; i < size; i++)
    {
        int rowcount = 0;
        int colcount = 0;
        for (int j = 0; j < size; j++)
        {
            if (matrix[i][j] == pl)
            {
                rowcount++;
            }
        }
        if (rowcount == size)
        {
            return true;
        }
        for (int j = 0; j < size; j++)
        {
            if (matrix[j][i] == pl)
            {
                colcount++;
            }
        }
        if (colcount == size)
        {
            return true;
        }
        if (matrix[i][i] == pl)
        {
            diagonal++;
        }
        if (diagonal == size)
        {
            return true;
        }
        if (matrix[i][size - 1 - i] == pl)
        {
            revdiag++;
        }
        if (revdiag == size)
        {
            return true;
        }
    }
    return false;
}

void *gamer_boii(void *arg)
{
    struct pleasework *player = (struct pleasework *)arg;

    while (!gamepoint && !isFull())
    {
        if (turn != player->pl)
        {
            pthread_mutex_lock(&lock);
            if (!gamepoint && !isFull())
            {
                bool isempty = false;
                int a, b;
                while (!isempty)
                {
                    a = rand() % size;
                    b = rand() % size;
                    if (matrix[a][b] == ' ')
                    {
                        isempty = true;
                    }
                }
                matrix[a][b] = player->pl;
                printf("Player %c played on: (%d,%d)\n", player->pl, a, b);
                if (isWinner(player->pl))
                {
                    gamepoint = true;
                    winner = player->pl;
                }
                turn = player->pl;
            }
            pthread_mutex_unlock(&lock);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    size = atoi(argv[1]);
    printf("Board Size: %dx%d\n", size, size);
    matrix = CreateBoard();
    turn = 'o';
    pthread_t PlayerX, PlayerO;
    struct pleasework X = {'x'};
    struct pleasework O = {'o'};

    if (pthread_create(&PlayerX, NULL, gamer_boii, &X))
    {
        printf("Thread could not be created. Terminating\n");
        exit(1);
    }
    if (pthread_create(&PlayerO, NULL, gamer_boii, &O))
    {
        printf("Thread could not be created. Terminating\n");
        exit(1);
    }
    pthread_join(PlayerX, NULL);
    pthread_join(PlayerO, NULL);

    printf("Game end\n");
    if (winner == '_' && isFull())
    {
        printf("It is a tie\n");
    }
    else
    {
        printf("Winner is %c\n", toupper(winner));
    }
    printBoard();
    for (int i = 0; i < size; i++)
    {
        free(matrix[i]);
    }
    free(matrix);
    return 0;
}