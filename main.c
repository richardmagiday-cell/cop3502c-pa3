/* COP 3502C PA2
This program is written by: Richard Magiday */
/******************************************************************************
Richard Magiday
cop3502c_cmb_26
01/14/26
problem: CS1 PA3

cd PA3
gcc main.c
Get-Content in1.txt | .\a.exe > out_test1.txt
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLINIC_CLOSE 480
#define MAX_NAME_LEN 25

typedef struct Cat_s // given
{
    int arrival;  // Minutes since clinic opened (Unique)
    char *name;   // Unique cat name
    int duration; // Required treatment time in minutes
} Cat;

typedef struct SLLNode_s // given
{
    Cat *cat; // Pointer to a cat
    struct SLLNode_s *next;
} SLLNode;

typedef struct Queue_s // queue struct for waiting cats
{
    SLLNode *front;
    SLLNode *back;
    int size;
} Queue;

typedef struct Stack // stack struct for exposed cats
{
    SLLNode *top;
    int size;
} Stack;

// Queue Function Prototypes
void initQueue(Queue *q);
int isEmptyQueue(Queue *q);
Cat *peekQueue(Queue *q);
void enqueueSorted(Queue *q, Cat *c);
Cat *dequeue(Queue *q);

// Stack Function Prototypes
void initStack(Stack *s);
int isEmptyStack(Stack *s);
void push(Stack *s, Cat *c);
Cat *pop(Stack *s);

// other funtions
Cat *createCat(int arrival, char *buffer, int duration);
void freeCat(Cat *c);
int canTreat(int start, int duration);

// Queue functions
void initQueue(Queue *q) // initalizing queue
{
    q->front = NULL;
    q->back = NULL;
    q->size = 0;
}

int isEmptyQueue(Queue *q) // check if queue empty
{
    return q->front == NULL;
}

Cat *peekQueue(Queue *q) // return front cat without removing
{
    if (q->front == NULL) // if queue emty retrun NULL
        return NULL;
    return q->front->cat; // retrun front most cat
}

void enqueueSorted(Queue *q, Cat *c) // insert cat in sorted order to queue
{
    SLLNode *node = malloc(sizeof(SLLNode)); // create new NOde
    node->cat = c;                           // node cat equals new cat
    node->next = NULL;                       // next null before sorting

    // Empty Queue check
    if (isEmptyQueue(q))
    {
        q->front = q->back = node; // if q is emty make front and back equal node
        q->size++;
        return;
    }

    // insert front
    if (c->arrival < q->front->cat->arrival)
    {
        node->next = q->front;
        q->front = node;
        q->size++;
        return;
    }

    // insert middle or end
    SLLNode *prev = q->front;
    SLLNode *cur = q->front->next;

    while (cur != NULL && cur->cat->arrival < c->arrival)
    {
        prev = cur;
        cur = cur->next;
    }

    prev->next = node;
    node->next = cur;

    // if inserted at end, update the back ptr
    if (cur == NULL)
        q->back = node; // Update back immediately

    q->size++;
}

// remove and return front cat
Cat *dequeue(Queue *q) // normal dequeue
{
    if (isEmptyQueue(q))
        return NULL;

    SLLNode *temp = q->front;
    Cat *c = temp->cat;

    q->front = temp->next;

    // if queue becomes empty, reset bsck ptr
    if (q->front == NULL)
        q->back = NULL;

    free(temp); // free node only
    q->size--;

    return c; // return cat ptr
}

// Stack functions
void initStack(Stack *s) // initalize Stack
{
    s->top = NULL;
    s->size = 0;
}

int isEmptyStack(Stack *s) // check is empty
{
    return s->top == NULL;
}

void push(Stack *s, Cat *c) // push new cat to top
{
    SLLNode *node = malloc(sizeof(SLLNode));
    node->cat = c;
    node->next = s->top; // new node points to old top
    s->top = node;       // update top
    s->size++;
}

Cat *pop(Stack *s) // take cat from stack
{
    if (isEmptyStack(s))
        return NULL;

    SLLNode *temp = s->top;
    Cat *c = temp->cat;

    s->top = temp->next;
    free(temp); // free node only
    s->size--;

    return c;
}

// other functions
Cat *createCat(int arrival, char *buffer, int duration) // create cats and linked list
{
    Cat *c = malloc(sizeof(Cat));
    c->arrival = arrival;
    c->duration = duration;

    // exact memory alllocation for cat name
    c->name = malloc(strlen(buffer) + 1);
    strcpy(c->name, buffer);

    return c;
}
void freeCat(Cat *c) // free cats
{
    free(c->name);
    free(c);
}
int canTreat(int start, int duration) // check if cat can be treated with enough time before store closes
{
    if (duration <= 0)
        return 0;
    if (start >= CLINIC_CLOSE)
        return 0;
    return (start + duration) <= CLINIC_CLOSE;
}

// main
int main()
{
    Queue q;
    Stack exposed;

    initQueue(&q); // initalize queue and stack
    initStack(&exposed);

    while (1) // while loop for input till end with -1 as break case
    {
        int arrival;
        if (scanf("%d", &arrival) != 1)
            break;
        if (arrival == -1)
            break;

        char buffer[MAX_NAME_LEN + 1];
        int duration;
        scanf("%s %d", buffer, &duration);

        Cat *c = createCat(arrival, buffer, duration);
        enqueueSorted(&q, c);
    }

    // initalize doctors to 0
    int unoFree = 0;
    int dosFree = 0;

    while (!isEmptyQueue(&q)) // while queue is not empty
    {
        Cat *c = dequeue(&q);

        if (c->duration == 0) // skip zero duration cats
        {
            freeCat(c);
            continue;
        }

        // Calculate earliest start time for each doctor
        int startUno = (unoFree > c->arrival) ? unoFree : c->arrival;
        int startDos = (dosFree > c->arrival) ? dosFree : c->arrival;

        int useUno;
        int start;

        // Choose doctor (Uno wins ties)
        if (startUno < startDos)
        {
            useUno = 1;
            start = startUno;
        }
        else if (startDos < startUno)
        {
            useUno = 0;
            start = startDos;
        }
        else
        {
            useUno = 1; /* Tie → Uno first */
            start = startUno;
        }

        // Check closing constraint
        if (!canTreat(start, c->duration))
        {
            printf("Cannot accommodate %s\n", c->name);
            freeCat(c);
            continue;
        }

        // Perform treatment
        if (useUno)
        {
            printf("Doctor Uno treated %s at %d\n", c->name, start);
            unoFree = start + c->duration;
            freeCat(c);
        }
        else
        {
            printf("Doctor Dos treated %s at %d\n", c->name, start);
            dosFree = start + c->duration;
            push(&exposed, c); // adding exposed cats to stack
            // printf("DEBUG: pushed %s (stack size now %d)\n", c->name, exposed.size);
        }
    }

    // Print Exposure from stack
    // printf("DEBUG: final exposed size = %d\n", exposed.size);
    if (isEmptyStack(&exposed))
    {
        printf("No Exposed Cats\n");
    }
    else
    {
        printf("Exposed Cats\n");
        while (!isEmptyStack(&exposed))
        {
            Cat *c = pop(&exposed);
            printf("%s\n", c->name);
            freeCat(c); // free after printing
        }
    }

    return 0;
}