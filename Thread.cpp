//
// Created by Gilad Lumbroso on 21/03/2017.
//

#include "Thread.h"

#define STACK_SIZE 4096

char stack1[STACK_SIZE];
char stack2[STACK_SIZE];

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
            "rol    $0x11,%0\n"
    : "=g" (ret)
    : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
        "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif


void Thread::setup()
{
    address_t sp, pc;
    sp = (address_t) stack1 + STACK_SIZE - sizeof(address_t);
    pc = (address_t) f;
    sigsetjmp(env, 1);
    (env->__jmpbuf)[JB_SP] = translate_address(sp);
    (env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&(env)->__saved_mask);
}

Thread::Thread(int id, void (*f)(void))
{
    this->state = READY;
    this->id = id;
    this->quantums = 1;
    this->f = f;
    setup();
    this->isSynced = false;
    this->isBlocked = false;
}

int Thread::getId()
{
    return id;
}

Thread::~Thread()
{

}

int Thread::getState()
{
    return state;
}

bool Thread::operator==(const Thread &other) const
{
    return this->id == other.id;
}

bool Thread::operator!=(const Thread &other) const
{
    return this->id != other.id;
}

void Thread::setState(int s) {
    state = s;
}

int Thread::saveState() {
    int result = sigsetjmp(env, 1);
    return result;
}

void Thread::loadState() {
    siglongjmp(env, 1);
}

void Thread::sync(Thread *t) {
    synced.push_back(t);
}
